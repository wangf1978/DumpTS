 // DumpTS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <memory.h>
#include <string>
#include <map>
#include <unordered_map>
#include <tuple>
#include <io.h>
#include <locale>
#include <algorithm>

using namespace std;

#define AUDIO_STREAM_ID	0xE0
#define FILTER_PID		0x1400//0x1011//0x1011//0x1400
#define TS_PACKET_SIZE	192

#define DUMP_RAW_OUTPUT	(1<<0)
#define DUMP_BD_M2TS	(1<<1)
#define DUMP_PES_OUTPUT	(1<<2)
#define DUMP_PTS_VIEW	(1<<3)

const char *dump_msg[] = {
	"Warning: wrong PES_length field value, read:location, read pos[%d], write pos:[%d].\r\n",
	"Error: a PES packet with a invalid start code will be ignored, read pos[%d], write pos:[%d].\r\n",
	"Error: a PES packet with too small bytes will be ignored, read pos[%d], write pos:[%d].\r\n",
};

const char* dumpparam[] = {"raw", "m2ts", "pes", "ptsview"};

const int   dumpoption[] = {1<<0, 1<<1, 1<<2, 1<<3};

__int64 GetPTSValue(unsigned char* pkt_data)
{
	__int64 ts;
	ts  = ((__int64)(pkt_data[0]&0x0e))<<29;	//requires 33 bits
	ts |= (pkt_data[1]<<22);
	ts |= (pkt_data[2]&0xfe)<<14;
	ts |= (pkt_data[3]<<7);
	ts |= (pkt_data[4]>>1);
	return ts;
}

int FlushPESBuffer(FILE* fw, unsigned char* pes_buffer, int pes_buffer_len, int dumpopt, int &raw_data_len)
{
	int iret=0;
	raw_data_len = 0;
	if(pes_buffer_len >= 9)
	{
		if( pes_buffer[0] == 0 &&
			pes_buffer[1] == 0 &&
			pes_buffer[2] == 1 )
		{
			int pes_len = pes_buffer[4]<<8 | pes_buffer[5];
			int pes_hdr_len = pes_buffer[8];

			if(dumpopt&DUMP_RAW_OUTPUT)
			{
				if(pes_buffer_len < pes_len + 6 || pes_buffer_len < pes_hdr_len + 9)
				{
					iret = -1;
				}
				else if(pes_len == 0)
				{
					raw_data_len = pes_buffer_len - pes_hdr_len - 9;
					fwrite(pes_buffer + pes_hdr_len + 9, 1, raw_data_len, fw);
				}
				else
				{
					raw_data_len = pes_len - 3 - pes_hdr_len;
					fwrite(pes_buffer + pes_hdr_len + 9, 1, raw_data_len, fw);
				}
			}
			else if(dumpopt&DUMP_PES_OUTPUT)
			{
				raw_data_len = pes_len==0?pes_buffer_len:pes_len;
				fwrite(pes_buffer, 1, raw_data_len, fw);
			}

			if(dumpopt&DUMP_PTS_VIEW)
			{
				if(pes_buffer_len < pes_len + 6 || pes_buffer_len < pes_hdr_len + 9)
				{
					iret = -1;
				}
				else
				{
					static int PktIndex = 0;
					__int64 pts = GetPTSValue(pes_buffer + 9);
					printf("PktIndex:%d, PTS value is %I64d(0X%I64X).\r\n", PktIndex++, pts, pts);
				}
			}
		}
		else
		{
			iret = -2;
		}
	}
	else if(pes_buffer_len > 0)
	{
		iret = -3;
	}

	return iret;
}

//
// DumpTS TSSourceFileName DestFileName --pid=0x100 --destpid=0x1011 --startpts=xxxxx --srcfmt=m2ts --outputfmt=es/pes/m2ts/ts --showpts
//

unordered_map<std::string, std::string> g_params;

void ParseCommandLine(int argc, char* argv[])
{
	if (argc < 2)
		return;

	g_params.insert({ "input", argv[1] });

	std::string str_arg_prefixes[] = {
		"output", "pid", "destpid", "startpts", "srcfmt", "outputfmt", "showpts"
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
				std::transform(strVal.begin(), strVal.end(), strVal.begin(), ::tolower);
				g_params.insert({ str_arg_prefixes[i],  strVal});
			}
		}
	}

	unordered_map<std::string, std::string>::const_iterator iter = g_params.cbegin();
	for (; iter != g_params.cend(); iter++)
	{
		printf("%s : %s.\r\n", iter->first.c_str(), iter->second.c_str());
	}

	// Set the default values

	return;
}

long long ConvertToLongLong(std::string& str)
{
	size_t idx = 0;
	long long ret = -1LL;
	// Check whether it is a valid PID value or not
	if (str.compare(0, 2, "0x") == 0)	// hex value
	{
		ret = std::stoll(str.substr(2), &idx, 16);
	}
	else if (str.compare(0, 1, "0") == 0)	// oct value
	{
		ret = std::stoll(str.substr(1), &idx, 8);
	}
	else
	{
		ret = std::stoll(str, &idx, 10);
	}

	return idx > 0 ? ret : -1;
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
		printf("Failed to open the input file: %s {error code: %d}.\r\n", g_params["input"].c_str(), ret);
		return false;
	}

	//
	// For output file
	//
	if (g_params.find("output") != g_params.end() && (ret = _access(g_params["output"].c_str(), 2)) != 0)
	{
		printf("The output file: %s can't be written {error code: %d}.\r\n", g_params["output"].c_str(), ret);
		return false;
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
	// For start pts
	//
	if (g_params.find("startpts") != g_params.end())
	{
		long long start_pts = ConvertToLongLong(g_params["startpts"]);
		if (!(start_pts >= 0 && start_pts <= 0x1FFFFFFFFLL))
		{
			printf("The start pts: %s is invalid, please make sure it is between 0 to 0x1FFFFFFFF(%lld) inclusive.\r\n", g_params["startpts"].c_str(), 0x1FFFFFFFFLL);
			return false;
		}
	}

	//
	// for source format
	//


	// TODO...

	return true;
}

void PrintHelp()
{
	printf("Usage: DumpTS.exe TSSourceFileName [OPTION]...\r\n");
	printf("\t--output\t\tThe output dumped file path\r\n");
	printf("\t--pid\t\t\tThe PID of dumped stream\r\n");
	printf("\t--destpid\t\tThe PID of source stream will be placed with this PID\r\n");
	printf("\t--startpts\t\tThe start PTS of TS file will be adjusted to start with this pts\r\n");
	printf("\t--srcfmt\t\tThe source TS format, including: ts, m2ts, if it is not specified, find the sync-word to decide it\r\n");
	printf("\t--outputfmt\t\tThe destination dumped format, including: ts, m2ts, pes and es\r\n");
	printf("\t--showpts\t\tPrint the pts of every elementary stream packet\n");

	printf("Examples:\r\n");
	printf("DumpTS c:\\00001.m2ts --output=c:\\00001.hevc --pid=0x1011 --srcfmt=m2ts --outputfmt=es --showpts\r\n");
	printf("DumpTS c:\\test.ts --output=c:\\00001.m2ts --pid=0x100 --destpid=0x1011 --srcfmt=ts --outputfmt=m2ts\r\n");

	return;
}

// Dump a stream with specified PID to a pes/es stream
int DumpOneStream()
{
	return -1;
}

// Dump a partial TS
int DumpPartialTS()
{
	return -1;
}

// Refactor TS, for example, change PID, PTS, or changing from ts to m2ts and so on
int RefactorTS()
{
	return -1;
}

int main(int argc, char* argv[])
{
	int nDumpRet = 0;

	if(argc < 2)
	{
		PrintHelp();
		return 0;
	}

	ParseCommandLine(argc, argv);

	if (VerifyCommandLine() == false)
	{
		PrintHelp();
		return 0;
	}

	//
	// Check what dump case should be gone through
	//

	// If the output format is ES/PES, and PID is also specified, go into the DumpOneStream mode
	if (g_params.find("pid") != g_params.end())
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];
		std::string& str_pid = g_params["pid"];

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes")))
		{
			nDumpRet = DumpOneStream();
		}
		else if (str_output_fmt.compare("ts") == 0 || str_output_fmt.compare("m2ts") == 0)
		{
			nDumpRet = DumpPartialTS();
		}
	}



	int sPID = FILTER_PID;

	if(argc >= 4)
		sscanf(strlwr(argv[3]), "%x", &sPID);

	int dumpopt = 0;
	for(int iarg=0;iarg<argc-4;iarg++)
	{
		for(int iparam=0;iparam<sizeof(dumpparam)/sizeof(dumpparam[0]);iparam++)
		{
			if(strnicmp(argv[iarg+4], dumpparam[iparam], strlen(dumpparam[iparam])) == 0)
			{
				dumpopt |= dumpoption[iparam];
			}
		}
	}

	unsigned char buf[TS_PACKET_SIZE];
	FILE *fp = fopen(argv[1],"rb");
	if(fp == NULL)
		return 0;

	FILE *fw = NULL;
	if(dumpopt&DUMP_RAW_OUTPUT || dumpopt&DUMP_PES_OUTPUT)
	{
		if((fw = fopen(argv[2],"wb+")) == NULL)
		{
			fclose(fp);
			return 0;
		}
	}

	size_t pes_hdr_location;
	int raw_data_len, dump_ret;
	unsigned long pes_buffer_len = 0;
	unsigned char* pes_buffer = new unsigned char[20*1024*1024];
	int buf_head_offset = dumpopt&DUMP_BD_M2TS?4:0;
	int ts_pack_size	= dumpopt&DUMP_BD_M2TS?TS_PACKET_SIZE:TS_PACKET_SIZE-4;

	while(true)
	{
		int nRead = fread(buf,1,ts_pack_size,fp);
		if( nRead < ts_pack_size )
			break;

		unsigned short PID = (buf[buf_head_offset+1]&0x1f)<<8 | buf[buf_head_offset+2];
		if(PID != sPID)
			continue;

		int index = buf_head_offset+4;
		unsigned char payload_unit_start = buf[buf_head_offset+1] & 0x40;
		unsigned char adaptation_field_control = (buf[buf_head_offset+3]>>4)&0x03;
		unsigned char discontinuity_counter = buf[buf_head_offset+3]&0x0f;

		if(payload_unit_start)
		{
			if((dump_ret = FlushPESBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
				printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
			pes_buffer_len = 0;
			pes_hdr_location = ftell(fp) - nRead;
		}

		if(adaptation_field_control&0x02)
			index += buf[buf_head_offset+4] + 1;

		if(payload_unit_start || !payload_unit_start && pes_buffer_len > 0)
		{
			memcpy(pes_buffer + pes_buffer_len, buf+index, TS_PACKET_SIZE-index);
			pes_buffer_len += TS_PACKET_SIZE-index;
		}
	}

	if((dump_ret = FlushPESBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
		printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));

	delete [] pes_buffer;
	pes_buffer = NULL;

	if (fp)
		fclose(fp);
	if (fw)
		fclose(fw);

	return 0;
}

