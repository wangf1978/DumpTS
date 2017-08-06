 // DumpTS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>

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

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("Usage: DumpTS.exe TSFileName DumpFileName [pid] [raw] [m2ts].\r\n");
		return 0;
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

#if 0
	if(rawmode == true)
		return 0;

	// rip PES header.
	unsigned char raw_buffer[2048];
	fp = fopen(argv[2],"rb");
	fw = fopen("c:\\dump.raw","wb+");

	int find_pos = 0;
	int required_bytes = 0;
	int left_over = 0;
	while(true)
	{
		int nRead = fread(raw_buffer+left_over,1,2048-left_over,fp);

		if(nRead <= 0)
			break;
	
		if(required_bytes > 0)
		{
			int nwrite = fwrite(raw_buffer+find_pos, 1, required_bytes>2048?2048:required_bytes,fw);
			required_bytes -= nwrite;
		}

		find_pos += required_bytes;

		// Find PES_header
		while(true)
		{
			if(find_pos + 9 < 2048)
			{
				if( raw_buffer[find_pos] == 0 &&
					raw_buffer[find_pos + 1] == 0 &&
					raw_buffer[find_pos + 2] == 1 &&
					raw_buffer[find_pos + 3] == AUDIO_STREAM_ID)
				{
					int pes_len = raw_buffer[find_pos + 4]<<8 | raw_buffer[find_pos+5];
					int pes_hdr_len = raw_buffer[find_pos + 8];// + 4;
					int raw_data_len = pes_len - pes_hdr_len  - 3;
					
					find_pos += 9;
					required_bytes = raw_data_len;

					if(find_pos + pes_hdr_len < 2048)
					{
						find_pos += pes_hdr_len;
						if(2048 - find_pos > raw_data_len)
						{
							fwrite(raw_buffer+find_pos,1,raw_data_len,fw);
							required_bytes = 0;
							left_over = 2048 -find_pos - raw_data_len;
							memmove(raw_buffer, raw_buffer + 2048 - left_over, left_over);
						}
						else
						{
							fwrite(raw_buffer+find_pos, 1, 2048 - find_pos, fw);
							required_bytes -= 2048 - find_pos;
							left_over = 0;
						}
						find_pos = 0;
						break;
					}
					else
					{
						find_pos = find_pos + pes_hdr_len - 2048;
						left_over = 0;
						break;
					}
				}
				else
				{
					find_pos++;
				}
			}
			else
			{
				required_bytes = 0;
				memmove(raw_buffer, raw_buffer + 2048 - 8, 8);
				left_over = 8;
				find_pos = 0;
				break;
			}
		}
	}

	fclose(fw);
	fclose(fp);
#endif

	return 0;
}

