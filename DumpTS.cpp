 // DumpTS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "crc.h"
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include "crc.h"

using namespace std;

unordered_map<std::string, std::string> g_params;

using PID_props = unordered_map<std::string, int>;
unordered_map<short, PID_props&> g_PIDs_props;

TS_FORMAT_INFO	g_ts_fmtinfo;
int				g_verbose_level = 0;
DUMP_STATUS		g_dump_status;

const char *dump_msg[] = {
	"Warning: wrong PES_length field value, read:location, read pos[%d], write pos:[%d].\r\n",
	"Error: a PES packet with a invalid start code will be ignored, read pos[%d], write pos:[%d].\r\n",
	"Error: a PES packet with too small bytes will be ignored, read pos[%d], write pos:[%d].\r\n",
};

const char* dumpparam[] = {"raw", "m2ts", "pes", "ptsview"};

const int   dumpoption[] = {1<<0, 1<<1, 1<<2, 1<<3};

extern int	RefactorTS();
extern int	DumpOneStream();
extern int	DumpPartialTS();
extern int	DumpMP4();

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

	g_params.insert({ "input", argv[1] });

	std::string str_arg_prefixes[] = {
		"output", "pid", "trackid", "boxtype", "destpid", "srcfmt", "outputfmt", "showpts", "stream_id", "stream_id_extension", "showinfo", "verbose", "removebox", "crc"
	};

	for (int iarg = 2; iarg < argc; iarg++)
	{
		std::string strArg = argv[iarg];
		if (strArg.find("--") == 0)
			strArg = strArg.substr(2);

		for (int i = 0; i < sizeof(str_arg_prefixes) / sizeof(str_arg_prefixes[0]); i++)
		{
			if (strArg.find(str_arg_prefixes[i] + "=") == 0)
			{
				string strVal = strArg.substr(str_arg_prefixes[i].length() + 1);
				if (strArg.compare("removebox") != 0)
					std::transform(strVal.begin(), strVal.end(), strVal.begin(), ::tolower);
				g_params[str_arg_prefixes[i]] = strVal;
				break;
			}
			else if (strArg == str_arg_prefixes[i])
			{
				g_params[str_arg_prefixes[i]] = "";
			}
		}
	}

	unordered_map<std::string, std::string>::const_iterator iter = g_params.cbegin();
	for (; iter != g_params.cend(); iter++)
	{
		if (iter->second.length() == 0)
		{
			if (iter->first.compare("showpts") == 0 || 
				iter->first.compare("showinfo") == 0 ||
				iter->first.compare("verbose") == 0)
				printf("%s : yes\r\n", iter->first.c_str());
			else
				printf("%s : %s\r\n", iter->first.c_str(), iter->second.c_str());
		}
		else
			printf("%s : %s\r\n", iter->first.c_str(), iter->second.c_str());
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
		printf("Please specify the input ts file path.\r\n");
		return false;
	}

	if ((ret = _access(g_params["input"].c_str(), 4)) != 0)
	{
		printf("Failed to open the input file: %s {error code: %d}.\r\n", g_params["input"].c_str(), errno);
		return false;
	}

	//
	// For output file
	//
	if (g_params.find("output") != g_params.end() && (ret = _access(g_params["output"].c_str(), 2)) != 0)
	{
		if (errno != ENOENT)
		{
			printf("The output file: %s can't be written {error code: %d}.\r\n", g_params["output"].c_str(), errno);
			return false;
		}
	}

	//
	// For PID
	//
	if (g_params.find("pid") != g_params.end())
	{
		long long pid = ConvertToLongLong(g_params["pid"]);
		if (!(pid >= 0 && pid <= 0x1FFF))
		{
			printf("The specified PID: %s is invalid, please make sure it is between 0 to 0x1FFF inclusive.\r\n", g_params["pid"].c_str());
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
			printf("The specified Destination PID: %s is invalid, please make sure it is between 0 to 0x1FFF inclusive.\r\n", g_params["destpid"].c_str());
			return false;
		}
	}

	//
	// for source format
	//


	// TODO...

	return true;
}

int PrepareParams()
{
	int iRet = -1;

	memset(&g_ts_fmtinfo, 0, sizeof(g_ts_fmtinfo));
	g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_UNKNOWN;

	auto iter = g_params.find("srcfmt");

	if (iter == g_params.end())
	{
		// check the file extension
		if (g_params.find("input") != g_params.end())
		{
			std::string& str_input_file_name = g_params["input"];
			size_t nPos1 = str_input_file_name.rfind('\\');
			size_t nPos2 = str_input_file_name.rfind('/');
			
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

			std::string file_name = str_input_file_name.substr(file_name_start_pos);
			size_t file_ext_start_pos = file_name.rfind('.');
			if (file_ext_start_pos != std::string::npos)
			{
				std::string file_name_ext = file_name.substr(file_ext_start_pos);
				if (_stricmp(file_name_ext.c_str(), ".m2ts") == 0)
					g_params["srcfmt"] = "m2ts";
				else if (_stricmp(file_name_ext.c_str(), ".ts") == 0)
					g_params["srcfmt"] = "ts";
				else if (_stricmp(file_name_ext.c_str(), ".tts") == 0)
					g_params["srcfmt"] = "tts";
				else if (_stricmp(file_name_ext.c_str(), ".mp4") == 0 || 
					_stricmp(file_name_ext.c_str(), ".mov") == 0 ||
					_stricmp(file_name_ext.c_str(), ".m4a") == 0 ||
					_stricmp(file_name_ext.c_str(), ".m4v") == 0)
					g_params["srcfmt"] = "mp4";
			}
		}

		iter = g_params.find("srcfmt");
	}

	if (iter != g_params.end())
	{
		if (iter->second.compare("m2ts") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_BDMV;
			g_ts_fmtinfo.packet_size = 192;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 4;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (iter->second.compare("ts") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_TS;
			g_ts_fmtinfo.packet_size = 188;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (iter->second.compare("mp4") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_MP4;
			g_ts_fmtinfo.packet_size = 0xFFFF;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
	}

	if (g_ts_fmtinfo.packet_size == 0 && g_params.find("crc") == g_params.end())
	{
		FILE* rfp = NULL;
		errno_t errn = fopen_s(&rfp, g_params["input"].c_str(), "rb");
		if (errn != 0 || rfp == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["input"].c_str(), errn);
			iRet = -1;
			goto done;
		}

		// Have to analyze a part of buffer to get the TS packet size
		MPEG_SYSTEM_TYPE sys_types[] = { MPEG_SYSTEM_TS, MPEG_SYSTEM_TS204, MPEG_SYSTEM_TS208, MPEG_SYSTEM_TTS };
		for (int idx = 0; idx < _countof(sys_types); idx++) {
			fseek(rfp, 0, SEEK_SET);
			if (TryFitMPEGSysType(rfp, sys_types[idx], g_ts_fmtinfo.packet_size, g_ts_fmtinfo.num_of_prefix_bytes, g_ts_fmtinfo.num_of_suffix_bytes, g_ts_fmtinfo.encrypted))
			{
				g_ts_fmtinfo.eMpegSys = sys_types[idx];
				iRet = 0;
				break;
			}
		}

		if (g_ts_fmtinfo.packet_size == 0)
			printf("This input file: %s seems not to be a valid TS file.\r\n", g_params["input"].c_str());
	}
	else
		iRet = 0;

	if (g_params.find("verbose") != g_params.end())
	{
		if (g_params["verbose"].length() == 0)
			g_verbose_level = 1;
		else
			g_verbose_level = (int)ConvertToLongLong(g_params["verbose"]);
	}

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
		printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["input"].c_str(), errn);
		iRet = -1;
		goto done;
	}

	g_dump_status.state = DUMP_STATE_INITIALIZE;

	if (g_ts_fmtinfo.eMpegSys >= MPEG_SYSTEM_TS && g_ts_fmtinfo.eMpegSys <= MPEG_SYSTEM_BDAV)
	{
		_fseeki64(rfp, -1, SEEK_END);
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
	printf("Usage: DumpTS.exe TSSourceFileName [OPTION]...\r\n");
	printf("\t--output\t\tThe output dumped file path\r\n");
	printf("\t--pid\t\t\tThe PID of dumped stream\r\n");
	printf("\t--trackid\t\tThe track id of dumped ISOBMFF\r\n");
	printf("\t--destpid\t\tThe PID of source stream will be placed with this PID\r\n");
	printf("\t--srcfmt\t\tThe source TS format, including: ts, m2ts, if it is not specified, find the sync-word to decide it\r\n");
	printf("\t--outputfmt\t\tThe destination dumped format, including: ts, m2ts, pes and es\r\n");
	printf("\t--removebox\t\tThe removed box type and its children boxes in MP4\r\n");
	printf("\t--showpts\t\tPrint the pts of every elementary stream packet\r\n");
	printf("\t--stream_id\t\tThe stream_id in PES header of dumped stream\r\n");
	printf("\t--stream_id_extension\tThe stream_id_extension in PES header of dumped stream\r\n");
	printf("\t--showinfo\t\tPrint the media information of elementary stream in TS/M2TS file\r\n");
	printf("\t--crc\t\t\tSpecify the crc type, if crc type is not specified, list all crc types\r\n");
	printf("\t--verbose\t\tPrint the intermediate information during media processing\r\n");

	printf("Examples:\r\n");
	printf("DumpTS c:\\00001.m2ts --output=c:\\00001.hevc --pid=0x1011 --srcfmt=m2ts --outputfmt=es --showpts\r\n");
	printf("DumpTS c:\\test.ts --output=c:\\00001.m2ts --pid=0x100 --destpid=0x1011 --srcfmt=ts --outputfmt=m2ts\r\n");
	printf("DumpTS c:\\test.mp4 --output=c:\\test1.mp4 --removebox unkn\r\n");
	printf("DumpTS c:\\test.mp4 --output=c:\\test.hevc --trackid=0\r\n");

	return;
}

extern void PrintCRCList();
extern CRC_TYPE GetCRCType(const char* szCRCName);
extern uint8_t GetCRCWidth(CRC_TYPE type);
extern const char* GetCRCName(CRC_TYPE type);

void CalculateCRC()
{
	if (g_params.find("crc") == g_params.end())
	{
		printf("Please specify the crc type.\r\n");
		return;
	}

	FILE* rfp = NULL;
	errno_t errn = fopen_s(&rfp, g_params["input"].c_str(), "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["input"].c_str(), errn);
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
				printf("Unsupported crc type: %s.\r\n", g_params["crc"].c_str());
				continue;
			}

			uint8_t buf[2048];
			while (!feof(rfp))
			{
				size_t nRead = 0;
				if ((nRead = fread_s(buf, sizeof(buf), 1, sizeof(buf), rfp)) > 0)
				{
					ProcessCRC(crc_handle, buf, nRead);
				}
			}
			_fseeki64(rfp, 0, SEEK_SET);

			char szFmtStr[256];
			sprintf_s(szFmtStr, sizeof(szFmtStr), "%%20s result: 0X%%0%dllX\r\n", (GetCRCWidth((CRC_TYPE)i) + 3) / 4);

			printf((const char*)szFmtStr, GetCRCName((CRC_TYPE)i), EndCRC(crc_handle));
		}

		fclose(rfp);
	}
	else
	{
		CRC_TYPE crc_type = GetCRCType(g_params["crc"].c_str());

		if (crc_type == CRC_MAX)
		{
			printf("Unsupported crc type: %s.\r\n", g_params["crc"].c_str());
			return;
		}

		CRC_HANDLE crc_handle = BeginCRC(crc_type);
		if (crc_handle == NULL)
		{
			printf("Unsupported crc type: %s.\r\n", g_params["crc"].c_str());
			return;
		}

		uint8_t buf[2048];
		while (!feof(rfp))
		{
			size_t nRead = 0;
			if ((nRead = fread_s(buf, sizeof(buf), 1, sizeof(buf), rfp)) > 0)
			{
				ProcessCRC(crc_handle, buf, nRead);
			}
		}
		fclose(rfp);

		char szFmtStr[256];
		sprintf_s(szFmtStr, sizeof(szFmtStr), "%%s result: 0X%%0%dllX\r\n", (GetCRCWidth(crc_type) + 3) / 4);

		printf((const char*)szFmtStr, g_params["crc"].c_str(), EndCRC(crc_handle));
	}
}

int main(int argc, char* argv[])
{
	int nDumpRet = 0;

	memset(&g_dump_status, 0, sizeof(g_dump_status));

	if(argc < 2)
	{
		PrintHelp();
		return -1;
	}

	// Parse the command line
	ParseCommandLine(argc, argv);

	// Verify the command line
	if (VerifyCommandLine() == false)
	{
		PrintHelp();
		return -1;
	}

	// Prepare the dumping parameters
	if (PrepareParams() < 0)
	{
		printf("Failed to prepare the dump parameters.\r\n");
		return -1;
	}

	// Prepare the dump
	if (PrepareDump() < 0)
	{
		printf("Failed to prepare the dump procedure.\r\n");
		return -1;
	}

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
	else if (g_params.find("srcfmt") != g_params.end() && g_params["srcfmt"].compare("mp4") == 0)
	{
		nDumpRet = DumpMP4();
	}
	else if (g_params.find("pid") != g_params.end())
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];
		std::string& str_pid = g_params["pid"];

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes") == 0 || str_output_fmt.compare("wav") == 0 || str_output_fmt.compare("pcm") == 0))
		{
			nDumpRet = DumpOneStream();
		}
		else if (str_output_fmt.compare("ts") == 0 || str_output_fmt.compare("m2ts") == 0)
		{
			if (g_params.find("destpid") == g_params.end())
				nDumpRet = DumpPartialTS();
			else
				nDumpRet = RefactorTS();
		}
	}
	else
	{
		if (g_params.find("output") == g_params.end())
		{
			// No output file is specified
			// Check whether pid and showinfo is there
			if (g_params.find("showinfo") != g_params.end())
			{
				g_params["pid"] = "0";
				DumpOneStream();
				goto done;
			}
			else
			{
				printf("Please specify a output file name with --output.\r\n");
				goto done;
			}
		}

		nDumpRet = RefactorTS();
	}

done:
	return 0;
}

