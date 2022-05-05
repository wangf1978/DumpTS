/*

MIT License

Copyright (c) 2022 Ravin.Wang(wangf1978@hotmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
// DumpTS.cpp : Defines the entry point for the console application.
//
#if defined(_WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include "platcomm.h"
#include "crc.h"
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include "crc.h"
#include "Matroska.h"
#include "MMT.h"
#include "nal_parser.h"
#include "version.h"
#include "system_13818_1.h"
#include "mediaobjprint.h"

using namespace std;

using PID_props = unordered_map<std::string, int>;

map<std::string, std::string, CaseInsensitiveComparator> 
									g_params;
unordered_map<short, PID_props&>	g_PIDs_props;

TS_FORMAT_INFO						g_ts_fmtinfo;
int									g_verbose_level = 0;
DUMP_STATUS							g_dump_status;

MEDIA_SCHEME_TYPE					g_source_media_scheme_type = MEDIA_SCHEME_UNKNOWN;
MEDIA_SCHEME_TYPE					g_output_media_scheme_type = MEDIA_SCHEME_UNKNOWN;

bool								g_silent_output = false;


const char *dump_msg[] = {
	"Warning: wrong PES_length field value, read:location, read pos[%d], write pos:[%d].\n",
	"Error: a PES packet with a invalid start code will be ignored, read pos[%d], write pos:[%d].\n",
	"Error: a PES packet with too small bytes will be ignored, read pos[%d], write pos:[%d].\n",
};

const char* dumpparam[] = {"raw", "m2ts", "pes", "ptsview"};

const int   dumpoption[] = {1<<0, 1<<1, 1<<2, 1<<3};

extern int	Transcode();
extern int	ShowAV1Info();
extern int	ShowMSEHex();
extern int	ShowMSE();
extern int	ListMSE();
extern int	DumpTransportPackets();
extern int	ShowOBUs();
extern int	DiffATCDTS();
extern int	LayoutTSPacket();
extern int	ShowStreamMuxConfig(bool bOnlyShowAudioSpecificConfig);
extern int	RunH264HRD();
extern int	ShowNUs();
extern int	ShowVPS();
extern int	ShowSPS();
extern int	ShowPPS();
extern int	ShowHRD();
extern int	ShowOBUSeqHdr();
extern int	ShowMPVInfo();
extern int	ShowNALInfo();
extern int	BenchRead(int option);
extern int	ShowPCR(int option);
extern int	DiffTSATC();
extern int	RefactorTS();
extern int	DumpTSToTS();
extern int	DumpOneStream();
extern int	DumpPartialTS();
extern int	DumpMP4();
extern int	DumpMKV();
extern int	DumpMMT();
extern int	DumpOneStreamFromPS();

int GetTopRecordCount()
{
	int top = -1;
	auto iterTop = g_params.find("top");
	if (iterTop != g_params.end())
	{
		int64_t top_records = -1;
		ConvertToInt(iterTop->second, top_records);
		if (top_records < 0 || top_records > INT32_MAX)
			top = -1;
	}
	return top;
}

bool TryFitMPEGSysType(FILE* rfp, MPEG_SYSTEM_TYPE eMPEGSysType, unsigned short& packet_size, unsigned short& num_of_prefix_bytes, unsigned short& num_of_suffix_bytes, bool& bEncrypted)
{
	if (eMPEGSysType == MPEG_SYSTEM_UNKNOWN)
		return false;

	bool bRet = false;
	unsigned char packet_buf[6144];

	bEncrypted = false;
	if (eMPEGSysType == MPEG_SYSTEM_BDAV ||
		eMPEGSysType == MPEG_SYSTEM_BDMV)
	{
		if (fread(packet_buf, 1, 6144, rfp) == 6144 && packet_buf[4] == 0x47)
		{
			if ((packet_buf[0] & 0xC0) == 0)
			{
				if (packet_buf[196] == 0x47)
					bRet = true;
			}
			else // encrypted content
			{
				bEncrypted = true;
				bRet = true;
			}
		}

		if (bRet == true) {
			packet_size = 192;
			num_of_prefix_bytes = 4;
			num_of_suffix_bytes = 0;
		}
	}
	else if (eMPEGSysType == MPEG_SYSTEM_TS ||
		eMPEGSysType == MPEG_SYSTEM_TTS ||
		eMPEGSysType == MPEG_SYSTEM_TS204 ||
		eMPEGSysType == MPEG_SYSTEM_TS208)
	{
		unsigned short lead_bytes = eMPEGSysType == MPEG_SYSTEM_TTS ? 4 : 0;
		unsigned short padding_bytes = eMPEGSysType == MPEG_SYSTEM_TS204 ? 16 : (eMPEGSysType == MPEG_SYSTEM_TS208 ? 20 : 0);
		unsigned short pkt_size = lead_bytes + 188 + padding_bytes;
		if (fread(packet_buf, 1, pkt_size, rfp) == pkt_size && packet_buf[lead_bytes] == 0x47) {
			if (fread(&packet_buf[pkt_size], 1, pkt_size, rfp) == pkt_size) {
				if (packet_buf[pkt_size + lead_bytes] == 0x47)
					bRet = true;
			}
			else
				bRet = true;
		}

		if (bRet == true) {
			packet_size = pkt_size;
			num_of_prefix_bytes = lead_bytes;
			num_of_suffix_bytes = padding_bytes;
		}
	}

	return bRet;
}

//
// DumpTS TSSourceFileName DestFileName --pid=0x100 --destpid=0x1011 --srcfmt=m2ts --outputfmt=es/pes/m2ts/ts --showpts
//

void ParseCommandLine(int argc, char* argv[])
{
	if (argc < 2)
		return;

	int iarg = 2;
	if (strncmp(argv[1], "--", 2) != 0)
	{
		iarg = 1;
		g_params.insert({ "input", argv[1] });
	}

	const char* str_arg_prefixes[] = {
		"bitrate",
		"output", 
		"pid", 
		"CID",
		"trackid", 
		"boxtype", 
		"destpid", 
		"srcfmt", 
		"outputfmt",
		"MPUseqno",
		"PKTseqno",
		"PKTno",
		"PKTid",
		"showinfo",
		"showpts", 
		"showpack",
		"showIPv4pack",	// Show TLV IPv4 packet
		"showIPv6pack",	// Show TLV IPv6 packet
		"showHCIPpack",	// Show TLV Header Compressed IP packet
		"showTCSpack",	// Show TLV Transmission control signal packet
		"showSIT",
		"showPAT",
		"showPLT",
		"showPMT",
		"showMPT",
		"showCAT",
		"showEIT",
		"showDU",		// show the DU in the MMTP payload
		"showPCR",
		"showPCRDiagram",
		"showNTP",
		"showNU",
		"showVPS",
		"showSPS",
		"showPPS",
		"showHRD",
		"showOBU",
		"showMSE",
		"showMSEHex",
		"showSeqHdr",
		"showStreamMuxConfig",
		"runHRD",
		"benchRead",	// Do bench mark test to check whether the read speed is enough or not
		"listMMTPpacket",
		"listMMTPpayload",
		"listMPUtime",
		"listMSE",
		"stream_id",
		"sub_stream_id",
		"stream_id_extension", 
		"removebox", 
		"crc",
		"dashinitmp4", 
		"VLCTypes",
		"video",
		"AU",
		"MFU",
		"progseq",
		"start",
		"end",
		"verbose",
		"diffATC",
		"diffATCDTS",	// Diff ATC and DTS with the specified PIDs
		"top",			// Show the n top records
		"payload_first_last",
		"layoutpacket",
		"cost",
		"silent",		// Only show the error or verbose information
	};

	for (; iarg < argc; iarg++)
	{
		bool bOption = false;
		const char* strArg = argv[iarg];
		if (StrBeginWith(argv[iarg], "--", true, NULL, &strArg))
			bOption = true;

		bool bFound = false;
		for (size_t i = 0; i < _countof(str_arg_prefixes); i++)
		{
			bool bEqual = false;
			const char* strArgLeft = nullptr;
			if (StrBeginWith(strArg, str_arg_prefixes[i], true, &bEqual, &strArgLeft))
			{
				if (bEqual)
				{
					g_params[str_arg_prefixes[i]] = "";
					bFound = true;
				}
				else if(*strArgLeft == '=')
				{
					strArgLeft++;
					g_params[str_arg_prefixes[i]] = strArgLeft;
					bFound = true;
					break;
				}
			}
		}

		if (!bFound && bOption)
			printf("Unsupported options \"%s\"\n", strArg);
	}

	if (g_params.find("verbose") != g_params.end())
	{
		if (g_params["verbose"].length() == 0)
			g_verbose_level = 1;
		else
			g_verbose_level = (int)ConvertToLongLong(g_params["verbose"]);
	}

	if (g_verbose_level > 0)
	{
		for (int i = 0; i < argc; i++)
			printf("%s\n", argv[i]);

		auto iter = g_params.cbegin();
		for (; iter != g_params.cend(); iter++)
		{
			if (iter->second.length() == 0)
			{
				if (MBCSICMP(iter->first.c_str(), "showpack") == 0 ||
					MBCSICMP(iter->first.c_str(), "showIPv4pack") == 0 ||
					MBCSICMP(iter->first.c_str(), "showIPv6pack") == 0 ||
					MBCSICMP(iter->first.c_str(), "showHCIPpack") == 0 ||
					MBCSICMP(iter->first.c_str(), "showTCSpack") == 0 ||
					MBCSICMP(iter->first.c_str(), "showpts") == 0 ||
					MBCSICMP(iter->first.c_str(), "showinfo") == 0 ||
					MBCSICMP(iter->first.c_str(), "verbose") == 0 ||
					MBCSICMP(iter->first.c_str(), "showSIT") == 0 ||
					MBCSICMP(iter->first.c_str(), "showPMT") == 0 ||
					MBCSICMP(iter->first.c_str(), "showMPT") == 0 ||
					MBCSICMP(iter->first.c_str(), "showPAT") == 0 ||
					MBCSICMP(iter->first.c_str(), "showPLT") == 0 ||
					MBCSICMP(iter->first.c_str(), "showCAT") == 0 ||
					MBCSICMP(iter->first.c_str(), "showEIT") == 0)
					printf("%s : yes\n", iter->first.c_str());
				else
					printf("%s : %s\n", iter->first.c_str(), iter->second.c_str());
			}
			else
				printf("%s : %s\n", iter->first.c_str(), iter->second.c_str());
		}
	}

	// Set the default values

	return;
}

bool VerifyCommandLine()
{
	int ret = 0;

	//
	// For input file
	//

	// Test the source file exists or not
	if (g_params.find("input") == g_params.end())
	{
		printf("Please specify the input media file path.\n");
		return false;
	}

	if ((ret = _access(g_params["input"].c_str(), R_OK)) != 0)
	{
		printf("Failed to open the input file: %s {error code: %d}.\n", g_params["input"].c_str(), errno);
		return false;
	}

	//
	// For output file
	//
	if (g_params.find("output") != g_params.end() && (ret = _access(g_params["output"].c_str(), W_OK)) != 0)
	{
		if (errno != ENOENT)
		{
			printf("The output file: %s can't be written {error code: %d}.\n", g_params["output"].c_str(), errno);
			return false;
		}
	}

	//
	// For PID
	//
	if (g_params.find("pid") != g_params.end())
	{
		long long pid = ConvertToLongLong(g_params["pid"]);
		uint16_t max_pid = 0x1fff;

		// MMT can have PIDs > 0x1FFF
		auto iter = g_params.find("srcfmt");
		if (iter->second.compare("mmt") == 0)
			max_pid = 0xffff;

		if (!(pid >= 0 && pid <= max_pid))
		{
			printf("The specified PID: %s is invalid, please make sure it is between 0 to 0x%04x inclusive.\n", g_params["pid"].c_str(), max_pid);
			return false;
		}
	}

	//
	// For stream_id
	//
	if (g_params.find("stream_id") != g_params.end())
	{
		long long stream_id = ConvertToLongLong(g_params["stream_id"]);
		if (stream_id < 0 || stream_id > 0xff)
		{
			printf("The specified stream_id: %s is invalid, please make sure it is between 0 to %02x inclusive.\n", g_params["stream_id"].c_str(), 0xff);
			return false;
		}
	}

	//
	// For Destination PID
	//
	if (g_params.find("destpid") != g_params.end())
	{
		long long pid = ConvertToLongLong(g_params["destpid"]);
		if (!(pid >= 0 && pid <= 0x1FFF))
		{
			printf("The specified Destination PID: %s is invalid, please make sure it is between 0 to 0x1FFF inclusive.\n", g_params["destpid"].c_str());
			return false;
		}
	}

	//
	// for source format
	//

	//
	// for listMPUtime
	//
	{
		auto iterListMPUtime = g_params.find("listMPUtime");
		if (iterListMPUtime != g_params.end())
		{
			if (MBCSICMP(iterListMPUtime->second.c_str(), "full") &&
				MBCSICMP(iterListMPUtime->second.c_str(), "simple") &&
				!iterListMPUtime->second.empty())
			{
				printf("The specified MPU time list type should be 'full', 'simple' or ''\n");
				return false;
			}

			// Should specify the CID and packet_id at the same time
			if (g_params.find("CID") == g_params.end())
			{
				printf("Please specify the CID.\n");
				return false;
			}

			if (g_params.find("pid") == g_params.end())
			{
				printf("Please specify the packet id.\n");
				return false;
			}
		}
	}


	// TODO...

	return true;
}

MEDIA_SCHEME_TYPE GetMediaSchemeTypeFromFormat(const char* param_name)
{
	auto iterParam = g_params.find(param_name);
	if (iterParam == g_params.end())
		return MEDIA_SCHEME_UNKNOWN;

	MEDIA_SCHEME_TYPE media_scheme_fmt = MEDIA_SCHEME_UNKNOWN;
	if (_stricmp(iterParam->second.c_str(), "m2ts") == 0)
		media_scheme_fmt = MEDIA_SCHEME_TRANSPORT_STREAM;
	else if (_stricmp(iterParam->second.c_str(), "ts") == 0)
		media_scheme_fmt = MEDIA_SCHEME_TRANSPORT_STREAM;
	else if (_stricmp(iterParam->second.c_str(), "tts") == 0)
		media_scheme_fmt = MEDIA_SCHEME_TRANSPORT_STREAM;
	else if (_stricmp(iterParam->second.c_str(), "vob") == 0)
		media_scheme_fmt = MEDIA_SCHEME_PROGRAM_STREAM;
	else if (_stricmp(iterParam->second.c_str(), "mpg") == 0)
		media_scheme_fmt = MEDIA_SCHEME_PROGRAM_STREAM;
	else if (_stricmp(iterParam->second.c_str(), "mp4") == 0)
		media_scheme_fmt = MEDIA_SCHEME_ISOBMFF;
	else if (_stricmp(iterParam->second.c_str(), "mkv") == 0)
		media_scheme_fmt = MEDIA_SCHEME_MATROSKA;
	else if (_stricmp(iterParam->second.c_str(), "aiff") == 0)
		media_scheme_fmt = MEDIA_SCHEME_AIFF;
	else if (_stricmp(iterParam->second.c_str(), "mmt") == 0)
		media_scheme_fmt = MEDIA_SCHEME_MMT;
	else if (_stricmp(iterParam->second.c_str(), "h264") == 0)
		media_scheme_fmt = MEDIA_SCHEME_NAL;
	else if (_stricmp(iterParam->second.c_str(), "h265") == 0)
		media_scheme_fmt = MEDIA_SCHEME_NAL;
	else if (_stricmp(iterParam->second.c_str(), "h266") == 0)
		media_scheme_fmt = MEDIA_SCHEME_NAL;
	else if (_stricmp(iterParam->second.c_str(), "mpv") == 0)
		media_scheme_fmt = MEDIA_SCHEME_MPV;
	else if (_stricmp(iterParam->second.c_str(), "adts") == 0)
		media_scheme_fmt = MEDIA_SCHEME_ADTS;
	else if (_stricmp(iterParam->second.c_str(), "loas") == 0)
		media_scheme_fmt = MEDIA_SCHEME_LOAS_LATM;
	else if (_stricmp(iterParam->second.c_str(), "av1") == 0)
		media_scheme_fmt = MEDIA_SCHEME_AV1;
	else if (_stricmp(iterParam->second.c_str(), "ivf") == 0)
		media_scheme_fmt = MEDIA_SCHEME_IVF;
	return media_scheme_fmt;
}

MEDIA_SCHEME_TYPE CheckAndUpdateFileFormat(std::string& filepath, const char* param_name)
{
	MEDIA_SCHEME_TYPE media_scheme_fmt = MEDIA_SCHEME_UNKNOWN;
	size_t nPos1 = filepath.rfind('\\');
	size_t nPos2 = filepath.rfind('/');

	size_t file_name_start_pos = 0;
	if (nPos1 == std::string::npos)
	{
		if (nPos2 != std::string::npos)
			file_name_start_pos = nPos2 + 1;
	}
	else if (nPos2 == std::string::npos)
		file_name_start_pos = nPos1 + 1;
	else
		file_name_start_pos = AMP_MAX(nPos1, nPos2) + 1;

	std::string file_name = filepath.substr(file_name_start_pos);
	size_t file_ext_start_pos = file_name.rfind('.');
	if (file_ext_start_pos != std::string::npos)
	{
		std::string file_name_ext = file_name.substr(file_ext_start_pos);
		if (_stricmp(file_name_ext.c_str(), ".m2ts") == 0)
		{
			if (param_name)
				g_params[param_name] = "m2ts";
			media_scheme_fmt = MEDIA_SCHEME_TRANSPORT_STREAM;
		}
		else if (_stricmp(file_name_ext.c_str(), ".ts") == 0)
		{
			if (param_name)
				g_params[param_name] = "ts";
			media_scheme_fmt = MEDIA_SCHEME_TRANSPORT_STREAM;
		}
		else if (_stricmp(file_name_ext.c_str(), ".tts") == 0)
		{
			if (param_name)
				g_params[param_name] = "tts";
			media_scheme_fmt = MEDIA_SCHEME_TRANSPORT_STREAM;
		}
		else if (_stricmp(file_name_ext.c_str(), ".vob") == 0)
		{
			if (param_name)
				g_params[param_name] = "vob";
			media_scheme_fmt = MEDIA_SCHEME_PROGRAM_STREAM;
		}
		else if (_stricmp(file_name_ext.c_str(), ".mpg") == 0 || _stricmp(file_name_ext.c_str(), ".mpeg") == 0)
		{
			if (param_name)
				g_params[param_name] = "mpg";
			media_scheme_fmt = MEDIA_SCHEME_PROGRAM_STREAM;
		}
		else if (_stricmp(file_name_ext.c_str(), ".mp4") == 0 ||
			_stricmp(file_name_ext.c_str(), ".mov") == 0 ||
			_stricmp(file_name_ext.c_str(), ".m4a") == 0 ||
			_stricmp(file_name_ext.c_str(), ".m4v") == 0 ||
			_stricmp(file_name_ext.c_str(), ".m4s") == 0 ||
			_stricmp(file_name_ext.c_str(), ".heif") == 0 ||
			_stricmp(file_name_ext.c_str(), ".heic") == 0 ||
			_stricmp(file_name_ext.c_str(), ".avif") == 0)
		{
			if (param_name)
				g_params[param_name] = "mp4";
			media_scheme_fmt = MEDIA_SCHEME_ISOBMFF;
		}
		else if (_stricmp(file_name_ext.c_str(), ".mkv") == 0 ||
			_stricmp(file_name_ext.c_str(), ".mka") == 0 ||
			_stricmp(file_name_ext.c_str(), ".mk3d") == 0 ||
			_stricmp(file_name_ext.c_str(), ".webm") == 0)
		{
			if (param_name)
				g_params[param_name] = "mkv";
			media_scheme_fmt = MEDIA_SCHEME_MATROSKA;
		}
		else if (_stricmp(file_name_ext.c_str(), ".aiff") == 0 ||
			_stricmp(file_name_ext.c_str(), ".aif") == 0 ||
			_stricmp(file_name_ext.c_str(), ".aifc") == 0)
		{
			if (param_name)
				g_params[param_name] = "aiff";
			media_scheme_fmt = MEDIA_SCHEME_AIFF;
		}
		else if (_stricmp(file_name_ext.c_str(), ".mmts") == 0)
		{
			if (param_name)
				g_params[param_name] = "mmt";
			media_scheme_fmt = MEDIA_SCHEME_MMT;
		}
		else if (_stricmp(file_name_ext.c_str(), ".h264") == 0 || _stricmp(file_name_ext.c_str(), ".avc") == 0)
		{
			if (param_name)
				g_params[param_name] = "h264";
			media_scheme_fmt = MEDIA_SCHEME_NAL;
		}
		else if (_stricmp(file_name_ext.c_str(), ".h265") == 0 || _stricmp(file_name_ext.c_str(), ".hevc") == 0)
		{
			if (param_name)
				g_params[param_name] = "h265";
			media_scheme_fmt = MEDIA_SCHEME_NAL;
		}
		else if (_stricmp(file_name_ext.c_str(), ".h266") == 0 || _stricmp(file_name_ext.c_str(), ".vvc") == 0)
		{
			if (param_name)
				g_params[param_name] = "h266";
			media_scheme_fmt = MEDIA_SCHEME_NAL;
		}
		else if (_stricmp(file_name_ext.c_str(), ".m2v") == 0 || _stricmp(file_name_ext.c_str(), ".mpv") == 0 || _stricmp(file_name_ext.c_str(), ".mp2v") == 0)
		{
			if (param_name)
				g_params[param_name] = "mpv";
			media_scheme_fmt = MEDIA_SCHEME_MPV;
		}
		else if (_stricmp(file_name_ext.c_str(), ".adts") == 0)
		{
			if (param_name)
				g_params[param_name] = "adts";
			media_scheme_fmt = MEDIA_SCHEME_ADTS;
		}
		else if (_stricmp(file_name_ext.c_str(), ".loas") == 0)
		{
			if (param_name)
				g_params[param_name] = "loas";
			media_scheme_fmt = MEDIA_SCHEME_LOAS_LATM;
		}
		else if (_stricmp(file_name_ext.c_str(), ".av1") == 0 || _stricmp(file_name_ext.c_str(), ".obu") == 0)
		{
			if (param_name)
				g_params[param_name] = "av1";
			media_scheme_fmt = MEDIA_SCHEME_AV1;
		}
		else if (_stricmp(file_name_ext.c_str(), ".ivf") == 0)
		{
			if (param_name && _stricmp(param_name, "srcfmt") == 0)
			{
				g_params["container"] = "ivf";

				// check its codec type
				FILE* rfp = nullptr;
				uint8_t ivf_header[IVF_HDR_SIZE] = { 0 };
				errno_t errn_fp = fopen_s(&rfp, filepath.c_str(), "rb");
				if (errn_fp != 0 || rfp == NULL)
				{
					printf("Failed to open the file: %s {errno: %d}.\n", filepath.c_str(), errn_fp);
					goto done;
				}

				if (fread(ivf_header, 1, IVF_HDR_SIZE, rfp) != IVF_HDR_SIZE || ivf_header[0] != 'D' || ivf_header[1] != 'K' || ivf_header[2] != 'I' || ivf_header[3] != 'F')
				{
					fclose(rfp);
					goto done;
				}

				fclose(rfp);

				uint32_t codec_fourcc = ((uint32_t)ivf_header[8] << 24) | ((uint32_t)ivf_header[9] << 16) | ((uint32_t)ivf_header[10] << 8) | ivf_header[11];
				if (codec_fourcc == 'AV01')
				{
					if (param_name)
						g_params[param_name] = "av1";
					media_scheme_fmt = MEDIA_SCHEME_AV1;
				}
				else
				{
					// TODO...
				}
			}
			else
			{
				// for the output file
				if (param_name)
					g_params[param_name] = "ivf";
				media_scheme_fmt = MEDIA_SCHEME_IVF;
			}
		}
	}

done:
	return media_scheme_fmt;
}

int PrepareParams()
{
	int iRet = -1;
	bool bIgnorePreparseTS = false;

	memset(&g_ts_fmtinfo, 0, sizeof(g_ts_fmtinfo));
	g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_UNKNOWN;

	auto iterSilent = g_params.find("silent");
	if (iterSilent != g_params.end())
		g_silent_output = true;

	auto IterSrcFmt = g_params.find("srcfmt");

	if (IterSrcFmt == g_params.end())
	{
		// check the file extension
		if (g_params.find("input") != g_params.end())
		{
			std::string& str_input_file_name = g_params["input"];
			g_source_media_scheme_type = CheckAndUpdateFileFormat(str_input_file_name, "srcfmt");
		}

		IterSrcFmt = g_params.find("srcfmt");
	}
	else
	{
		g_source_media_scheme_type = GetMediaSchemeTypeFromFormat("srcfmt");
	}

	if (IterSrcFmt != g_params.end())
	{
		if (IterSrcFmt->second.compare("m2ts") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_BDMV;
			g_ts_fmtinfo.packet_size = 192;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 4;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (IterSrcFmt->second.compare("ts") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_TS;
			g_ts_fmtinfo.packet_size = 188;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (IterSrcFmt->second.compare("mpg") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_PS;
			g_ts_fmtinfo.packet_size = 0xFFFF;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (IterSrcFmt->second.compare("vob") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_DVD_VIDEO;
			g_ts_fmtinfo.packet_size = 2048;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (IterSrcFmt->second.compare("mp4") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_MP4;
			g_ts_fmtinfo.packet_size = 0xFFFF;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (IterSrcFmt->second.compare("mkv") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_MATROSKA;
			g_ts_fmtinfo.packet_size = 0xFFFF;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (IterSrcFmt->second.compare("huffman_codebook") == 0 ||
			IterSrcFmt->second.compare(0, strlen("spectrum_huffman_codebook_"), "spectrum_huffman_codebook_") == 0 ||
			IterSrcFmt->second.compare("aiff") == 0 ||
			IterSrcFmt->second.compare("mmt") == 0 ||
			IterSrcFmt->second.compare("mpv") == 0 ||
			IterSrcFmt->second.compare("h264") == 0 ||
			IterSrcFmt->second.compare("h265") == 0 || 
			IterSrcFmt->second.compare("h266") == 0 ||
			IterSrcFmt->second.compare("loas") == 0 ||
			IterSrcFmt->second.compare("adts") == 0 ||
			IterSrcFmt->second.compare("av1") == 0)
			bIgnorePreparseTS = true;
	}

	auto iterDstFmt = g_params.find("outputfmt");
	if (iterDstFmt == g_params.end())
	{
		// check the file extension
		if (g_params.find("output") != g_params.end())
		{
			std::string& str_output_file_name = g_params["output"];
			// "outputfmt" is used by container to es, don't modify it
			// If want to know its detailed output format, need use g_output_media_sheme_type
			g_output_media_scheme_type = CheckAndUpdateFileFormat(str_output_file_name, NULL/*"outputfmt"*/);
			iterDstFmt = g_params.find("outputfmt");
		}

		if (iterDstFmt == g_params.end() && g_params.find("copy") != g_params.end() && IterSrcFmt != g_params.end())
		{
			g_params["outputfmt"] = IterSrcFmt->second;
			g_output_media_scheme_type = g_source_media_scheme_type;
			iterDstFmt = g_params.find("outputfmt");
		}
	}
	else
	{
		// Have already specified the output format manually, also update the output media scheme
		g_output_media_scheme_type = GetMediaSchemeTypeFromFormat("outputfmt");
	}

	if (bIgnorePreparseTS == false && g_ts_fmtinfo.packet_size == 0 && g_params.find("crc") == g_params.end())
	{
		FILE* rfp = NULL;
		errno_t errn = fopen_s(&rfp, g_params["input"].c_str(), "rb");
		if (errn != 0 || rfp == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
			iRet = -1;
			goto done;
		}

		// Have to analyze a part of buffer to get the TS packet size
		MPEG_SYSTEM_TYPE sys_types[] = { MPEG_SYSTEM_TS, MPEG_SYSTEM_TS204, MPEG_SYSTEM_TS208, MPEG_SYSTEM_TTS };
		for (size_t idx = 0; idx < _countof(sys_types); idx++) {
			fseek(rfp, 0, SEEK_SET);
			if (TryFitMPEGSysType(rfp, sys_types[idx], g_ts_fmtinfo.packet_size, g_ts_fmtinfo.num_of_prefix_bytes, g_ts_fmtinfo.num_of_suffix_bytes, g_ts_fmtinfo.encrypted))
			{
				g_ts_fmtinfo.eMpegSys = sys_types[idx];
				iRet = 0;
				break;
			}
		}

		if (g_ts_fmtinfo.packet_size == 0)
			printf("This input file: %s seems not to be a valid TS file.\n", g_params["input"].c_str());
	}
	else
		iRet = 0;

done:
	return iRet;
}

int PrepareDump()
{
	int iRet = 0;
	FILE* rfp = NULL;
	errno_t errn = fopen_s(&rfp, g_params["input"].c_str(), "rb");

	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		iRet = -1;
		goto done;
	}

	g_dump_status.state = DUMP_STATE_INITIALIZE;

	if ((g_ts_fmtinfo.eMpegSys >= MPEG_SYSTEM_TS && g_ts_fmtinfo.eMpegSys <= MPEG_SYSTEM_BDAV) ||
		(g_ts_fmtinfo.eMpegSys == MPEG_SYSTEM_DVD_VIDEO || g_ts_fmtinfo.eMpegSys == MPEG_SYSTEM_DVD_VR))
	{
		_fseeki64(rfp, 0, SEEK_END);
		long long file_size = _ftelli64(rfp);

		g_dump_status.num_of_packs = file_size / g_ts_fmtinfo.packet_size;

		_fseeki64(rfp, 0, SEEK_SET);
	}

	// Do other preparing or initializing things...

done:
	if (rfp != NULL)
		fclose(rfp);

	return iRet;
}

void PrintHelp()
{
	printf("Usage: DumpTS.exe [SourceMediaFile] [OPTION]...\n");
	printf("\t--output\t\tThe output dumped file path\n");
	printf("\t--pid\t\t\tThe PID of dumped TS stream, or the packet_id of dumped MMT asset\n");
	printf("\t--trackid\t\tThe track id of dumped ISOBMFF\n");
	printf("\t--destpid\t\tThe PID of source stream will be placed with this PID\n");
	printf("\t--srcfmt\t\tThe source format, including: \n"
		"\t\t\t\t\tts: standard transport stream\n"
		"\t\t\t\t\tm2ts: transport stream with 4 bytes of extra arrive clock time and so on\n"
		"\t\t\t\t\tmp4: ISO-BMFF media file\n"
		"\t\t\t\t\tmkv: Matroska based media file, for example, .mkv, .mka, .mk3d and .webm files\n"
		"\t\t\t\t\taiff: Apple AIFF, or AIFC file\n"
		"\t\t\t\t\tmmt: MPEG Media Transport stream\n"
		"\t\t\t\t\thuffman_codebook: Huffman-codebook text file including VLC tables\n"
		"\t\t\t\t\tspectrum_huffman_codebook_1~11: Spectrum-Huffman-codebook text file including VLC tables\n"
		"\t\t\t\t(*)If it is not specified, decide it by its file extension or find the sync-word to decide it\n");
	printf("\t--CID\t\t\tthe context ID of a header compressed IP packet in MMT/TLV stream\n");
	printf("\t--outputfmt\t\tThe destination dumped format, including: ts, m2ts, pes, es, binary_search_table and sourcecode\n");
	printf("\t--showpts\t\tPrint the pts of every elementary stream packet\n");
	printf("\t--stream_id\t\tThe stream_id in PES header of dumped stream\n");
	printf("\t--sub_stream_id\t\tThe sub_stream_id in the private data of pack of dumped stream\n");
	printf("\t--stream_id_extension\tThe stream_id_extension in PES header of dumped stream\n");
	printf("\t--MPUseqno\t\tthe MPU sequence number of MMT stream\n");
	printf("\t--PKTseqno\t\tthe packet sequence number of MMT stream\n");
	printf("\t--MFU\t\t\tDumping the each MFU as a separate file, filename will be {MPUseqno}_xxxx.{assert_type}\n");
	printf("\t--removebox\t\tThe removed box type and its children boxes in MP4\n");
	printf("\t--boxtype\t\tthe box type FOURCC\n");
	printf("\t--showinfo\t\tPrint the media information of summary, layout or elementary stream in TS/ISOBMFF/Matroska file\n");
	printf("\t--showpack\t\tPrint the syntax details for every pack\n");
	printf("\t--showIPv4pack\t\tPrint the IP-v4 pack syntax details\n");
	printf("\t--showIPv6pack\t\tPrint the IP-v6 pack syntax details\n");
	printf("\t--showHCIPpack\t\tPrint Header-Compressed IP packet syntax details\n");
	printf("\t--showTCSpack\t\tPrint the specified or all stream packet, only support TLV/MMT\n");
	printf("\t--showSIT\t\tPrint the SIT information for DTV stream\n");
	printf("\t--showPMT\t\tPrint the PMT information in TS stream\n");
	printf("\t--showPAT\t\tPrint the PAT information in TS stream\n");
	printf("\t--showMPT\t\tPrint the MPT information in MMT/TLV stream\n");
	printf("\t--showPLT\t\tPrint the PLT information in MMT/TLV stream\n");
	printf("\t--showCAT\t\tPrint the CAT information in MMT/TLV stream\n");
	printf("\t--showEIT\t\tPrint the MH-EIT information in MMT/TLV stream\n");
	printf("\t--showPCR\t\tPrint the PCR clock information in TS stream\n");
	printf("\t--showPCRDiagram\tPrint the PCR and its related PTS, DTS diagram, export PCR, ATC, PTS/DTS into csv file\n");
	printf("\t--showNTP\t\tPrint the NTP information in MMT/TLV stream\n");
	printf("\t--showMSE\t\tPrint the syntax-view of media syntax element in the raw encoded stream or media file\n");
	printf("\t--showMSEHex\t\tPrint the hex-view of media syntax element(s) in the raw encoded stream or media file\n");
	printf("\t--diffATC\t\tShow the ATC diff which is greater than the specified threshold\n");
	printf("\t--diffATCDTS\t\tShow the ATC and DTS diff at the payload unit start point between 2 specified PIDs\n");
	printf("\t--showNU\t\tShow the access-unit, nal-unit, sei-message and sei_payload tree of AVC/HEVC/VVC stream\n");
	printf("\t--showOBU\t\tShow the temporal-unit, frame-unit and open-bitstream-unit hierarchy of AV1 bitstream\n");
	printf("\t--listMMTPpacket\tList the specified MMTP packets\n");
	printf("\t--listMMTPpayload\tList the specified MMTP payloads\n");
	printf("\t--listMPUtime\t\tList MPU presentation time and its pts/dts offset\n");
	printf("\t--listMSE\t\tList the media syntax element hierarchy layout in the raw encoded stream or media file\n");
	printf("\t--showVPS\t\tShow the VPS syntax of HEVC/VVC stream\n");
	printf("\t--showSPS\t\tShow the SPS syntax of AVC/HEVC/VVC stream\n");
	printf("\t--showPPS\t\tShow the PPS syntax of AVC/HEVC/VVC stream\n");
	printf("\t--showSeqHdr\t\tShow the AV1 Sequence-Header-OBU syntax or MPEG2-Video sequence header\n");
	printf("\t--showHRD\t\tShow the Hypothetical reference decoder parameters of AVC/HEVC/VVC stream\n");
	printf("\t--runHRD\t\tRun the Hypothetical reference decoder verifier of AVC/HEVC/VVC stream\n");
	printf("\t--showStreamMuxConfig\tShow the StreamMuxConfig in MPEG4 AAC LOAS/LATM stream\n");
	printf("\t--crc\t\t\tSpecify the crc type, if crc type is not specified, list all crc types\n");
	printf("\t--listcrc\t\tList all crc types and exit\n");
	printf("\t--listmp4box\t\tShow the ISOBMFF box-table defined in ISO14496-12/15 and QTFF and exit\n");
	printf("\t--listmkvebml\t\tShow EBML elements defined in Matroska specification and exit\n");
	printf("\t--listMMTPpacketid\tShow Assignment of Packet ID of MMTP transmitting message and data\n");
	printf("\t--listMMTSImsg\t\tShow Assignment of message identifier of MMT-SI\n");
	printf("\t--listMMTSItable\tShow Assignment of identifier of table of MMT-SI\n");
	printf("\t--listMMTSIdesc\t\tShow Assignment of descriptor tag of MMT-SI\n");
	printf("\t--dashinitmp4\t\tSpecify the DASH initialization mp4 file to process m4s\n");
	printf("\t--VLCTypes\t\tSpecify the number value literal formats, a: auto; h: hex; d: dec; o: oct; b: bin, for example, \"aah\"\n");
	printf("\t--video\t\t\tThe current dumped stream is a video stream\n");
	printf("\t--start\t\t\tSpecify where to start dumping the stream, for ts, in the unit of TS pack.\n");
	printf("\t--end\t\t\tSpecify where to stop dumping the stream, for ts, in the unit of TS pack.\n");
	printf("\t--top\t\t\tSpecify how many records are displayed.\n");
	printf("\t--verbose\t\tPrint the intermediate information during media processing\n");
	printf("\t--help\t\t\tPrint this message");

	printf("\nExamples:\n");
	printf("\tDumpTS 00001.m2ts --output=00001.hevc --pid=0x1011 --srcfmt=m2ts --outputfmt=es --showpts\n");
	printf("\tDumpTS test.ts --output=00001.m2ts --pid=0x100 --destpid=0x1011 --srcfmt=ts --outputfmt=m2ts\n");
	printf("\tDumpTS test.mp4 --output=test1.mp4 --removebox=unkn\n");
	printf("\tDumpTS test.mp4 --output=test.hevc --trackid=0\n");
	printf("\tDumpTS codebook.txt --srcfmt=huffman_codebook --showinfo\n");
	printf("\tDumpTS codebook.txt --srcfmt=huffman_codebook --outputfmt=binary_search_table\n");

	return;
}

extern void PrintCRCList();
extern CRC_TYPE GetCRCType(const char* szCRCName);
extern uint8_t GetCRCWidth(CRC_TYPE type);
extern const char* GetCRCName(CRC_TYPE type);
extern void GenerateHuffmanBinarySearchArray(const char* szHeaderFileName, INT_VALUE_LITERAL_FORMAT fmts[3], const char* szOutputFile = nullptr, int spectrum_hcb_idx = -1);
extern void PrintHuffmanTree(const char* szHeaderFileName, INT_VALUE_LITERAL_FORMAT fmts[3], const char* szOutputFile = nullptr, int spectrum_hcb_idx=-1);
extern int DumpHuffmanCodeBook();
extern int DumpSpectrumHuffmanCodeBook(int nCodebookNumber);
extern int DumpAIFF();

void CalculateCRC()
{
	if (g_params.find("crc") == g_params.end())
	{
		printf("Please specify the crc type.\n");
		return;
	}

	FILE* rfp = NULL;
	errno_t errn = fopen_s(&rfp, g_params["input"].c_str(), "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		return;
	}

	// try to find the CRC type in string
	if (g_params["crc"] == "all")
	{
		for (int i = 0; i < CRC_MAX; i++)
		{
			CRC_HANDLE crc_handle = BeginCRC((CRC_TYPE)i);
			if (crc_handle == NULL)
			{
				printf("Unsupported crc type: %s.\n", g_params["crc"].c_str());
				continue;
			}

			uint8_t buf[2048];
			while (!feof(rfp))
			{
				size_t nRead = 0;
				if ((nRead = fread_s(buf, sizeof(buf), 1U, sizeof(buf), rfp)) > 0)
				{
					ProcessCRC(crc_handle, buf, nRead);
				}
			}
			_fseeki64(rfp, 0, SEEK_SET);

			char szFmtStr[256];
			sprintf_s(szFmtStr, sizeof(szFmtStr), "%%20s result: 0X%%0%dllX\n", (GetCRCWidth((CRC_TYPE)i) + 3) / 4);

			printf((const char*)szFmtStr, GetCRCName((CRC_TYPE)i), EndCRC(crc_handle));
		}

		fclose(rfp);
	}
	else
	{
		CRC_TYPE crc_type = GetCRCType(g_params["crc"].c_str());

		if (crc_type == CRC_MAX)
		{
			printf("Unsupported crc type: %s.\n", g_params["crc"].c_str());
			return;
		}

		CRC_HANDLE crc_handle = BeginCRC(crc_type);
		if (crc_handle == NULL)
		{
			printf("Unsupported crc type: %s.\n", g_params["crc"].c_str());
			return;
		}

		uint8_t buf[2048];
		while (!feof(rfp))
		{
			size_t nRead = 0;
			if ((nRead = fread_s(buf, sizeof(buf), 1U, sizeof(buf), rfp)) > 0)
			{
				ProcessCRC(crc_handle, buf, nRead);
			}
		}
		fclose(rfp);

		char szFmtStr[256];
		sprintf_s(szFmtStr, sizeof(szFmtStr), "%%s result: 0X%%0%dllX\n", (GetCRCWidth(crc_type) + 3) / 4);

		printf((const char*)szFmtStr, g_params["crc"].c_str(), EndCRC(crc_handle));
	}
}

int CheckFixedFeature(int argc, char* argv[])
{
	if (_stricmp(argv[1], "--huffmantree") == 0)
	{
		INT_VALUE_LITERAL_FORMAT fmts[3] = { FMT_AUTO, FMT_AUTO, FMT_AUTO };
		PrintHuffmanTree(NULL, fmts);
		GenerateHuffmanBinarySearchArray(NULL, fmts);
		return 0;
	}

	if (_stricmp(argv[1], "--listcrc") == 0)
	{
		PrintCRCList();
		return 0;
	}
	else if (_stricmp(argv[1], "--listmp4box") == 0)
	{
		BST::ISOBMFF::PrintISOBMFFBox(-1LL);
		return 0;
	}
	else if (_stricmp(argv[1], "--listmkvEBML") == 0)
	{
		BST::Matroska::PrintEBMLElements(INVALID_EBML_ID);
		return 0;
	}
	else if (_stricmp(argv[1], "--listMMTPpacketid") == 0)
	{
		MMT::PrintPacketIDAssignment();
		return 0;
	}
	else if (_stricmp(argv[1], "--listMMTSImsg") == 0)
	{
		MMT::PrintMMTSIMessage();
		return 0;
	}
	else if (_stricmp(argv[1], "--listMMTSItable") == 0)
	{
		MMT::PrintMMTSITable();
		return 0;
	}
	else if (_stricmp(argv[1], "--listMMTSIdesc") == 0)
	{
		MMT::PrintMMTSIDescriptor();
		return 0;
	}
	else if (_stricmp(argv[1], "--help") == 0)
	{
		PrintHelp();
		return 0;
	}
	else if (_stricmp(argv[1], "--version") == 0)
	{
		printf("version: %s\n", APP_VERSION);
		return 0;
	}

	return -1;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	CoInitializeEx(0, COINIT_MULTITHREADED);
#endif


	int nDumpRet = 0;
	auto tm_prog_start = std::chrono::system_clock::now();

	memset(&g_dump_status, 0, sizeof(g_dump_status));

	//for (int i = 0; i < argc; i++)
	//	printf("%s\n", argv[i]);

	if (argc < 2)
	{
		PrintHelp();
		return -1;
	}

	// Check whether it is a fixed command to only show the information
	if (AMP_SUCCEEDED(nDumpRet = CheckFixedFeature(argc, argv)))
	{
		return nDumpRet;
	}

	// Parse the command line
	ParseCommandLine(argc, argv);

	// Prepare the dumping parameters
	if (PrepareParams() < 0)
	{
		printf("Failed to prepare the dump parameters.\n");
		return -1;
	}

	// Verify the command line
	if (VerifyCommandLine() == false)
	{
		//PrintHelp();
		printf("run 'DumpTS --help' or visit 'https://github.com/wangf1978/DumpTS' to get more information.\n");
		return -1;
	}

	// Prepare the dump
	if (PrepareDump() < 0)
	{
		printf("Failed to prepare the dump procedure.\n");
		return -1;
	}

	auto iter_srcfmt = g_params.find("srcfmt");
	auto iter_dstfmt = g_params.find("outputfmt");
	auto iter_bitrate = g_params.find("bitrate");

	// If the output format is ES/PES, and PID is also specified, go into the DumpOneStream mode
	if (g_params.find("crc") != g_params.end())
	{
		if (g_params["crc"] == "")
		{
			PrintCRCList();
		}
		else
		{
			CalculateCRC();
		}
	}
	else if (iter_bitrate != g_params.end() && iter_bitrate->second.length() > 0)
	{
		nDumpRet = Transcode();
	}
	else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("mp4") == 0)
	{
		nDumpRet = DumpMP4();
	}
	else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("mkv") == 0)
	{
		nDumpRet = DumpMKV();
	}
	else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("huffman_codebook") == 0)
	{
		nDumpRet = DumpHuffmanCodeBook();
	}
	else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare(0, strlen("spectrum_huffman_codebook_"), "spectrum_huffman_codebook_") == 0)
	{
		int64_t idx_spectrum_hcb = -1;
		auto str_idx = iter_srcfmt->second.substr(strlen("spectrum_huffman_codebook_"));
		const char* p_str_idx_start = str_idx.c_str();
		const char* p_str_idx_end = p_str_idx_start + str_idx.length();
		if (ConvertToInt((char*)p_str_idx_start, (char*)p_str_idx_end, idx_spectrum_hcb))
		{
			nDumpRet = DumpSpectrumHuffmanCodeBook((int)idx_spectrum_hcb);
		}
		else
		{
			nDumpRet = -1;
			printf("Failed to specify the spectrum huffman codebook: %s.\n", iter_srcfmt->second.c_str());
		}
	}
	else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("aiff") == 0)
	{
		nDumpRet = DumpAIFF();
	}
	else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("mmt") == 0)
	{
		nDumpRet = DumpMMT();
	}
	else if (iter_srcfmt != g_params.end() && (iter_srcfmt->second.compare("vob") == 0 || iter_srcfmt->second.compare("mpg") == 0))
	{
		if (g_params.find("stream_id") != g_params.end())
		{
			if (iter_dstfmt == g_params.end())
				iter_dstfmt = g_params.insert({ "outputfmt", "es" }).first;

			std::string& str_output_fmt = iter_dstfmt->second;

			if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes") == 0 || str_output_fmt.compare("wav") == 0 || str_output_fmt.compare("pcm") == 0))
			{
				nDumpRet = DumpOneStreamFromPS();
			}
		}
	}
	else if (g_params.find("pid") != g_params.end() && (g_params.find("diffATC") == g_params.end() && g_params.find("diffATCDTS") == g_params.end()))
	{
		if (iter_dstfmt == g_params.end())
			iter_dstfmt = g_params.insert({ "outputfmt", "es" }).first;

		std::string& str_output_fmt = iter_dstfmt->second;

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes") == 0 || str_output_fmt.compare("wav") == 0 || str_output_fmt.compare("pcm") == 0))
		{
			nDumpRet = DumpOneStream();
		}
		else if (str_output_fmt.compare("ts") == 0 || str_output_fmt.compare("tts") == 0 || str_output_fmt.compare("m2ts") == 0)
		{
			nDumpRet = DumpTSToTS();
		}
	}
	else
	{
		auto iterOutput = g_params.find("output");
		if (g_params.find("showPCRDiagram") != g_params.end())
		{
			int optionShowPCRDiagram = 5;
			nDumpRet = ShowPCR(optionShowPCRDiagram);
			goto done;
		}

		if (g_source_media_scheme_type == g_output_media_scheme_type && g_source_media_scheme_type == MEDIA_SCHEME_TRANSPORT_STREAM)
		{
			nDumpRet = DumpTSToTS();
			goto done;
		}
		else if (g_params.find("output") == g_params.end())
		{
			if (g_params.find("showinfo") != g_params.end())
			{
				if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("loas") == 0)
				{
					nDumpRet = ShowStreamMuxConfig(true);
					goto done;
				}
				else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("adts") == 0)
				{
					printf("Oops! Not implement yet....\n");
					goto done;
				}
				else if (iter_srcfmt != g_params.end() && (
					iter_srcfmt->second.compare("h264") == 0 ||
					iter_srcfmt->second.compare("h265") == 0))
				{
					nDumpRet = ShowNALInfo();
					goto done;
				}
				else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("mpv") == 0)
				{
					nDumpRet = ShowMPVInfo();
					goto done;
				}
				else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("av1") == 0)
				{
					nDumpRet = ShowAV1Info();
					goto done;
				}
				else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("h266") == 0)
				{
					printf("Oops! Not implement yet....\n");
					goto done;
				}
				else
				{
					g_params["pid"] = "0";
					nDumpRet = DumpOneStream();
					goto done;
				}
			}
			else if (g_params.find("showpack") != g_params.end())
			{
				nDumpRet = DumpTransportPackets();
				goto done;
			}
			else if (g_params.find("showSIT") != g_params.end() || 
					 g_params.find("showPAT") != g_params.end() || 
					 g_params.find("showPMT") != g_params.end())
			{
				// No output file is specified
				// Check whether pid and showinfo is there
				g_params["pid"] = "0";
				nDumpRet = DumpOneStream();
				goto done;
			}
			else if (g_params.find("diffATC") != g_params.end())
			{
				nDumpRet = DiffTSATC();
				goto done;
			}
			else if (g_params.find("showPCR") != g_params.end())
			{
				// showPCR provides 4 options: 1: only PCR; 2: PCR + Video; 3: PCR + Audio; 4: PCR + All elementary streams
				int optionShowPCR = 1;
				auto& strShowPCROption = g_params["showPCR"];
				long long llV = ConvertToLongLong(strShowPCROption);
				if (llV >= 1 && llV <= 4)
					optionShowPCR = (int)llV;
				else
				{
					if (MBCSICMP(strShowPCROption.c_str(), "full") == 0)
						optionShowPCR = 4;
					else if (MBCSICMP(strShowPCROption.c_str(), "audio") == 0)
						optionShowPCR = 3;
					else if (MBCSICMP(strShowPCROption.c_str(), "video") == 0)
						optionShowPCR = 2;
				}

				nDumpRet = ShowPCR(optionShowPCR);
				goto done;
			}
			else if (g_params.find("showNTP") != g_params.end())
			{
				nDumpRet = DumpMMT();
				goto done;
			}
			else if (g_params.find("benchRead") != g_params.end())
			{
				nDumpRet = BenchRead(0);
				goto done;
			}
			else if (g_params.find("showNU") != g_params.end())
			{
				nDumpRet = ShowNUs();
				goto done;
			}
			else if (g_params.find("showVPS") != g_params.end())
			{
				nDumpRet = ShowVPS();
				goto done;
			}
			else if (g_params.find("showSPS") != g_params.end())
			{
				nDumpRet = ShowSPS();
				goto done;
			}
			else if (g_params.find("showPPS") != g_params.end())
			{
				nDumpRet = ShowPPS();
				goto done;
			}
			else if (g_params.find("showSeqHdr") != g_params.end())
			{
				if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("mpv") == 0)
				{
					nDumpRet = RET_CODE_ERROR_NOTIMPL;
					printf("Oops! Not implement yet....\n");
					goto done;
				}
				else if (iter_srcfmt != g_params.end() && iter_srcfmt->second.compare("av1") == 0)
				{
					nDumpRet = ShowOBUSeqHdr();
					goto done;
				}
			}
			else if (g_params.find("showHRD") != g_params.end())
			{
				nDumpRet = ShowHRD();
				goto done;
			}
			else if (g_params.find("runHRD") != g_params.end())
			{
				nDumpRet = RunH264HRD();
				goto done;
			}
			else if (g_params.find("showOBU") != g_params.end())
			{
				nDumpRet = ShowOBUs();
				goto done;
			}
			else if (g_params.find("showMSE") != g_params.end())
			{
				nDumpRet = ShowMSE();
				goto done;
			}
			else if (g_params.find("showMSEHex") != g_params.end())
			{
				nDumpRet = ShowMSEHex();
				goto done;
			}
			else if (g_params.find("listMSE") != g_params.end())
			{
				nDumpRet = ListMSE();
				goto done;
			}
			else if (g_params.find("showStreamMuxConfig") != g_params.end())
			{
				nDumpRet = ShowStreamMuxConfig(false);
				goto done;
			}
			else if (g_params.find("layoutpacket") != g_params.end())
			{
				nDumpRet = LayoutTSPacket();
				goto done;
			}
			else if (g_params.find("diffATCDTS") != g_params.end())
			{
				nDumpRet = DiffATCDTS();
				goto done;
			}
			else
			{
				printf("Please specify a output file name with --output.\n");
				goto done;
			}
		}

		nDumpRet = DumpTSToTS();
	}

done:
	if (g_params.find("cost") != g_params.end())
	{
		auto tm_prog_end = std::chrono::system_clock::now();
		auto running_time = std::chrono::duration_cast<std::chrono::milliseconds>(tm_prog_end - tm_prog_start).count();
		printf("Running for %lld.%03llds", running_time / 1000LL, running_time % 1000LL);
	}

	return nDumpRet;
}

#define DEFAULT_TRANSPORT_PACKETS_PER_DISPLAY		1

int DumpTransportPackets()
{
	int nDumpRet = 0;
	FILE* fp = nullptr;
	long long file_size = -1LL;
	uint8_t ts_pack_size = 192;
	uint8_t hdr_offset = 4;
	uint8_t buf[192] = { 0 };
	uint8_t obuf[192] = { 0 };
	MPEG_SYSTEM_TYPE mpeg_sys = MPEG_SYSTEM_TTS;

	int nRet = RET_CODE_SUCCESS;
	if (g_params.find("input") == g_params.end())
		return -1;

	std::string& szInputFile = g_params["input"];

	auto iterStart = g_params.find("start");
	auto iterEnd = g_params.find("end");
	auto iterPKTNo = g_params.find("PKTno");
	auto iterPKTId = g_params.find("PKTid");
	auto iterPID = g_params.find("pid");
	auto iterSrcFmt = g_params.find("srcfmt");

	if (iterSrcFmt == g_params.end() || (
		_stricmp(iterSrcFmt->second.c_str(), "ts") != 0 &&
		_stricmp(iterSrcFmt->second.c_str(), "tts") != 0 &&
		_stricmp(iterSrcFmt->second.c_str(), "m2ts") != 0))
	{
		printf("Please specify a transport stream file.\n ");
		return -1;
	}

	if (_stricmp(iterSrcFmt->second.c_str(), "ts") == 0)
	{
		ts_pack_size = 188;
		hdr_offset = 0;
		mpeg_sys = MPEG_SYSTEM_TS;
	}

	int64_t display_pages = DEFAULT_TRANSPORT_PACKETS_PER_DISPLAY;
	auto iter_showpack = g_params.find("showpack");
	if ((iter_showpack) != g_params.end())
	{
		int64_t iVal64 = -1;
		const char* szPages = iter_showpack->second.c_str();
		if (ConvertToInt((char*)szPages, (char*)szPages + iter_showpack->second.length(), iVal64))
		{
			if (iVal64 <= 0)
				display_pages = std::numeric_limits<decltype(display_pages)>::max();
			else
				display_pages = iVal64;
		}	
	}

	int64_t nStart = 0, nEnd = INT64_MAX, filter_PKTID = -1, iVal = -1LL, filter_PID = -1LL;
	if (iterStart != g_params.end())
	{
		iVal = ConvertToLongLong(iterStart->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nStart = (uint32_t)iVal;
	}

	if (iterEnd != g_params.end())
	{
		iVal = ConvertToLongLong(iterEnd->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nEnd = (uint32_t)iVal;
	}

	if (iterPKTNo != g_params.end())
	{
		if (ConvertToInt(iterPKTNo->second, iVal) && iVal > 0 && iVal <= UINT32_MAX)
			filter_PKTID = iVal - 1;
	}

	if (iterPKTId != g_params.end())
	{
		if (ConvertToInt(iterPKTId->second, iVal) && iVal >= 0 && iVal <= UINT32_MAX)
			filter_PKTID = iVal;
	}

	if (iterPID != g_params.end())
	{
		if (ConvertToInt(iterPID->second, iVal) && iVal >= 0 && iVal <= 0x1FFF)
			filter_PID = iVal;
	}

	FOPEN(fp, szInputFile.c_str(), "rb");
	if (fp == nullptr)
	{
		printf("Failed to open the file '%s'\n", szInputFile.c_str());
		return -1;
	}

	// Get file size
	_fseeki64(fp, 0, SEEK_END);
	file_size = _ftelli64(fp);
	_fseeki64(fp, 0, SEEK_SET);

	if (file_size <= 0)
	{
		printf("The file '%s' seems NOT to be valid.\n", szInputFile.c_str());
		fclose(fp);
		return -1;
	}

	int64_t number_of_ts_packs = (int64_t)(file_size / ts_pack_size);

	if (filter_PKTID >= number_of_ts_packs)
	{
		printf("The specified packet id or number is out of range.\n");
		filter_PKTID = -1;
	}
	else if (filter_PKTID >= 0)
	{
		nStart = filter_PKTID;
		nEnd = filter_PKTID + 1;
	}

	if (filter_PKTID == -1)
	{
		if (nStart >= number_of_ts_packs)
		{
			printf("The specified start transport packet id(start from 0) should be NOT greater than %" PRId64 "\n", number_of_ts_packs);
			fclose(fp);
			return -1;
		}
	}

	if (_fseeki64(fp, nStart*ts_pack_size, SEEK_SET) != 0)
	{
		printf("Failed to seek to the start position.\n");
		fclose(fp);
		return -1;
	}

	int64_t curr_ts_pkt_idx = nStart;
	while (!feof(fp))
	{
		if (curr_ts_pkt_idx >= nEnd)
		{
			printf("Reach the end of range, cease showing the pack.\n");
			break;
		}

		if (fread(buf, 1, ts_pack_size, fp) != ts_pack_size)
		{
			printf("Hitting error to read the data from the file, cease showing the pack.\n");
			break;
		}

		memcpy(obuf, buf, ts_pack_size);
		BST::CVarTSPacket var_ts_pack(mpeg_sys, curr_ts_pkt_idx);
		var_ts_pack.AttachBuf(buf, ts_pack_size);
		if (AMP_SUCCEEDED(var_ts_pack.Map()))
		{
			PrintMediaObject(&var_ts_pack, true);

			print_mem(obuf, ts_pack_size, 4);
		}

		curr_ts_pkt_idx++;

		if (curr_ts_pkt_idx < nEnd && ((curr_ts_pkt_idx- nStart)%display_pages) == 0 && display_pages != std::numeric_limits<decltype(display_pages)>::max())
		{
			printf("Press any key to continue('q': quit)...\n");
			char chk = _getch();
			if (chk == 0x3 || chk == 0x1A || chk == 'q' || chk == 'Q')	// Ctrl + C/Z, quit the loop
				break;
		}
	}

	if (fp != nullptr)
		fclose(fp);

	return nDumpRet;
}

/*
	There are below scenarios from TS to TS
	1. Direct copy, including ts->ts, tts(m2ts) to tts(m2ts) and tts(m2ts) to ts
	2. Change PID src PID -> dst PID
	3. Add ATC clock, ts -> tts(m2ts)
	4. Reconstruct a new TS with the specified PIDs by remuxing
	5. Reconstruct a new TS by copy the TS packs of the specified PIDs(with --copy)

	Totally, 
	* For the simple copy, go though DumpPartialTS
	* Need complex parsing, and change the original content, go through RefactorTS

	This function will detect this case, and let it to go through the different process routines
*/
int DumpTSToTS()
{
	int nDumpRet = -1;
	auto iter_srcfmt = g_params.find("srcfmt");
	auto iter_dstfmt = g_params.find("outputfmt");
	auto iter_inputfile = g_params.find("input");
	auto iter_outputfile = g_params.find("output");
	auto iter_PID = g_params.find("pid");
	auto iter_copy = g_params.find("copy");
	// ts -> ts, tts(m2ts) -> tts(m2ts) and tts(m2ts) -> ts: bSimpleSrcDstTransition = true
	// ts -> m2ts(tts): bSimpleSrcDstTransition = false
	bool bSimpleSrcDstTransition = false;
	std::set<uint16_t> pid_filters;

	if (g_params.find("outputfmt") == g_params.end())
	{
		// Assume the output format is the same with input format
		g_params["outputfmt"] = g_params["srcfmt"];
		iter_dstfmt = g_params.find("outputfmt");
	}

	if (iter_srcfmt == g_params.end())
	{
		printf("[TS] Unknown source transport stream format.\n");
		goto done;
	}

	if (iter_dstfmt == g_params.end())
	{
		printf("[TS] Unknown output transport stream format.\n");
		goto done;
	}

	if (iter_outputfile == g_params.end())
	{
		// give an output file name
		char szExt[_MAX_EXT];
		memset(szExt, 0, sizeof(szExt));
		std::string& src_filepath = iter_inputfile->second;
		_splitpath_s(src_filepath.c_str(), NULL, 0, NULL, 0, NULL, 0, szExt, _MAX_EXT);

		std::string strNewFilePath;
		size_t ccExt = strlen(szExt);
		if (ccExt > 0)
			strNewFilePath = src_filepath.substr(0, src_filepath.length() - strlen(szExt));
		else
			strNewFilePath = src_filepath;

		if (g_params.find("progseq") != g_params.end())
			strNewFilePath += "_ps_" + g_params["progseq"];

		if (g_params.find("start") != g_params.end())
			strNewFilePath += "_spnstart_" + g_params["start"];

		if (g_params.find("end") != g_params.end())
			strNewFilePath += "_spnend_" + g_params["end"];

		if (strNewFilePath + szExt == src_filepath)
			strNewFilePath += "_copy";

		strNewFilePath += szExt;

		g_params["output"] = strNewFilePath;
	}

	// ts -> m2ts(tts): bSimpleSrcDstTransition = false
	if (_stricmp(iter_srcfmt->second.c_str(), "ts") == 0 && (_stricmp(iter_dstfmt->second.c_str(), "tts") == 0 || _stricmp(iter_dstfmt->second.c_str(), "m2ts") == 0))
		bSimpleSrcDstTransition = false;
	else
		bSimpleSrcDstTransition = true;

	if (g_params.find("destpid") != g_params.end())
	{
		// 2. Change PID src PID -> dst PID
		nDumpRet = RefactorTS();
	}
	else if (!bSimpleSrcDstTransition)
	{
		// 3. Add ATC clock, ts -> tts(m2ts)
		nDumpRet = RefactorTS();
	}
	else
	{
		if (g_params.find("progseq") != g_params.end())
		{
			g_params["pid"] = "-1";
			nDumpRet = DumpOneStream();
		}
		else if (iter_PID == g_params.end())
		{
			// No source/destination PID is specified
			nDumpRet = DumpPartialTS();
		}
		else if (iter_copy != g_params.end())
		{
			// Simple copy a part of transport stream packs with the specified PID(s)
			nDumpRet = DumpPartialTS();
		}
		else
		{
			nDumpRet = RefactorTS();
		}
	}

done:
	return nDumpRet;
}
