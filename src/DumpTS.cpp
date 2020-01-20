 // DumpTS.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"
#include "crc.h"
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include "crc.h"
#include "Matroska.h"

using namespace std;

unordered_map<std::string, std::string> g_params;

using PID_props = unordered_map<std::string, int>;
unordered_map<short, PID_props&> g_PIDs_props;

TS_FORMAT_INFO	g_ts_fmtinfo;
int				g_verbose_level = 0;
DUMP_STATUS		g_dump_status;

const char *dump_msg[] = {
	"Warning: wrong PES_length field value, read:location, read pos[%d], write pos:[%d].\n",
	"Error: a PES packet with a invalid start code will be ignored, read pos[%d], write pos:[%d].\n",
	"Error: a PES packet with too small bytes will be ignored, read pos[%d], write pos:[%d].\n",
};

const char* dumpparam[] = {"raw", "m2ts", "pes", "ptsview"};

const int   dumpoption[] = {1<<0, 1<<1, 1<<2, 1<<3};

extern int	RefactorTS();
extern int	DumpOneStream();
extern int	DumpPartialTS();
extern int	DumpMP4();
extern int  DumpMKV();
extern int	DumpMMT();

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

	std::string str_arg_prefixes[] = {
		"output", 
		"pid", 
		"CID",
		"trackid", 
		"boxtype", 
		"destpid", 
		"srcfmt", 
		"outputfmt", 
		"showinfo",
		"showpts", 
		"showpack",
		"stream_id", 
		"stream_id_extension", 
		"removebox", 
		"crc",
		"dashinitmp4", 
		"VLCTypes",
		"verbose"
	};

	for (; iarg < argc; iarg++)
	{
		bool bOption = false;
		std::string strArg = argv[iarg];
		if (strArg.find("--") == 0)
		{
			bOption = true;
			strArg = strArg.substr(2);
		}

		bool bFound = false;
		for (size_t i = 0; i < _countof(str_arg_prefixes); i++)
		{
			if (strArg.find(str_arg_prefixes[i] + "=") == 0)
			{
				string strVal = strArg.substr(str_arg_prefixes[i].length() + 1);
				if (strArg.compare("removebox") != 0)
					std::transform(strVal.begin(), strVal.end(), strVal.begin(), ::tolower);
				g_params[str_arg_prefixes[i]] = strVal;
				bFound = true;
				break;
			}
			else if (strArg == str_arg_prefixes[i])
			{
				g_params[str_arg_prefixes[i]] = "";
				bFound = true;
			}
		}

		if (!bFound && bOption)
			printf("Unsupported options \"%s\"\n", strArg.c_str());
	}

	unordered_map<std::string, std::string>::const_iterator iter = g_params.cbegin();
	for (; iter != g_params.cend(); iter++)
	{
		if (iter->second.length() == 0)
		{
			if (iter->first.compare("showpack") == 0 ||
				iter->first.compare("showpts") == 0 || 
				iter->first.compare("showinfo") == 0 ||
				iter->first.compare("verbose") == 0)
				printf("%s : yes\n", iter->first.c_str());
			else
				printf("%s : %s\n", iter->first.c_str(), iter->second.c_str());
		}
		else
			printf("%s : %s\n", iter->first.c_str(), iter->second.c_str());
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


	// TODO...

	return true;
}

int PrepareParams()
{
	int iRet = -1;
	bool bIgnorePreparseTS = false;

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
					_stricmp(file_name_ext.c_str(), ".m4v") == 0 ||
					_stricmp(file_name_ext.c_str(), ".m4s") == 0 ||
					_stricmp(file_name_ext.c_str(), ".heif") == 0 ||
					_stricmp(file_name_ext.c_str(), ".heic") == 0 ||
					_stricmp(file_name_ext.c_str(), ".avif") == 0)
					g_params["srcfmt"] = "mp4";
				else if (_stricmp(file_name_ext.c_str(), ".mkv") == 0 ||
					_stricmp(file_name_ext.c_str(), ".mka") == 0 ||
					_stricmp(file_name_ext.c_str(), ".mk3d") == 0 ||
					_stricmp(file_name_ext.c_str(), ".webm") == 0)
					g_params["srcfmt"] = "mkv";
				else if (_stricmp(file_name_ext.c_str(), ".aiff") == 0 ||
					_stricmp(file_name_ext.c_str(), ".aif") == 0 ||
					_stricmp(file_name_ext.c_str(), ".aifc") == 0)
					g_params["srcfmt"] = "aiff";
				else if (_stricmp(file_name_ext.c_str(), ".mmts") == 0)
					g_params["srcfmt"] = "mmt";
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
		else if (iter->second.compare("mkv") == 0)
		{
			g_ts_fmtinfo.eMpegSys = MPEG_SYSTEM_MATROSKA;
			g_ts_fmtinfo.packet_size = 0xFFFF;
			g_ts_fmtinfo.encrypted = false;
			g_ts_fmtinfo.num_of_prefix_bytes = 0;
			g_ts_fmtinfo.num_of_suffix_bytes = 0;
		}
		else if (iter->second.compare("huffman_codebook") == 0 ||
			iter->second.compare(0, strlen("spectrum_huffman_codebook_"), "spectrum_huffman_codebook_") == 0 ||
			iter->second.compare("aiff") == 0 ||
			iter->second.compare("mmt") == 0)
			bIgnorePreparseTS = true;
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
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		iRet = -1;
		goto done;
	}

	g_dump_status.state = DUMP_STATE_INITIALIZE;

	if (g_ts_fmtinfo.eMpegSys >= MPEG_SYSTEM_TS && g_ts_fmtinfo.eMpegSys <= MPEG_SYSTEM_BDAV)
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
	printf("\t--removebox\t\tThe removed box type and its children boxes in MP4\n");
	printf("\t--showpts\t\tPrint the pts of every elementary stream packet\n");
	printf("\t--stream_id\t\tThe stream_id in PES header of dumped stream\n");
	printf("\t--stream_id_extension\tThe stream_id_extension in PES header of dumped stream\n");
	printf("\t--boxtype\t\tthe box type FOURCC\n");
	printf("\t--showinfo\t\tPrint the media information of summary, layout or elementary stream in TS/ISOBMFF/Matroska file\n");
	printf("\t--crc\t\t\tSpecify the crc type, if crc type is not specified, list all crc types\n");
	printf("\t--listcrc\t\tList all crc types and exit\n");
	printf("\t--listmp4box\t\tShow the ISOBMFF box-table defined in ISO14496-12/15 and QTFF and exit\n");
	printf("\t--listmkvebml\t\tShow EBML elements defined in Matroska specification and exit\n");
	printf("\t--dashinitmp4\t\tSpecify the DASH initialization mp4 file to process m4s\n");
	printf("\t--VLCTypes\t\tSpecify the number value literal formats, a: auto; h: hex; d: dec; o: oct; b: bin, for example, \"aah\"\n");
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

int main(int argc, char* argv[])
{
	int nDumpRet = 0;

	memset(&g_dump_status, 0, sizeof(g_dump_status));

	if (argc < 2)
	{
		PrintHelp();
		return -1;
	}

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
		ISOBMFF::PrintISOBMFFBox(-1LL);
		return 0;
	}
	else if (_stricmp(argv[1], "--listmkvEBML") == 0)
	{
		Matroska::PrintEBMLElements(INVALID_EBML_ID);
		return 0;
	}
	else if (_stricmp(argv[1], "--help") == 0)
	{
		PrintHelp();
		return 0;
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
		PrintHelp();
		return -1;
	}

	// Prepare the dump
	if (PrepareDump() < 0)
	{
		printf("Failed to prepare the dump procedure.\n");
		return -1;
	}

	auto iter_srcfmt = g_params.find("srcfmt");

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
	else if (g_params.find("pid") != g_params.end())
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];

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
				printf("Please specify a output file name with --output.\n");
				goto done;
			}
		}

		nDumpRet = RefactorTS();
	}

done:
	return nDumpRet;
}

