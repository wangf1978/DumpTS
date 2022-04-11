/*

MIT License

Copyright (c) 2021 Ravin.Wang(wangf1978@hotmail.com)

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
#include "platcomm.h"
#include "PayloadBuf.h"
#include "crc.h"
#include "DataUtil.h"
#include "system_13818_1.h"
#include <algorithm>

std::unordered_map<unsigned char, std::string> g_SIT_descriptors= {
	{0x05, "registration_descriptor"},
	{0x1C, "MPEG-4_audio_descriptor"},
	{0x42, "staffing_descriptor"},
	{0x47, "bouquet_name_descriptor"},
	{0x48, "service_descriptor"},
	{0x49, "country_availability_description"},
	{0x4A, "linkage_descriptor"},
	{0x4B, "NVOD_reference_descriptor"},
	{0x4C, "time_shifted_service_descriptor"},
	{0x4D, "short_event_descriptor"},
	{0x4E, "extended_event_descriptor"},
	{0x4F, "time_shifted_event_descriptor"},
	{0x50, "component_descriptor"},
	{0x51, "mosaic_descriptor"},
	{0x52, "Stream identifier descriptor"},
	{0x53, "Ca_identifier_descriptor"},
	{0x54, "content_descriptor"},
	{0x55, "parental_rating_descriptor"},
	{0x63, "partial_transport_stream_descriptor"},
	//{0xC2, "network_identifier_descriptor"},
	//{0xC3, "partialTS_time_descriptor"},
	//{0xC4, "audio_component_descriptor"},
	//{0xC5, "hyperlink_descriptor"},
	//{0xC7, "data_contents_descriptor"},
	//{0xCD, "TS_information_descriptor"},
	//{0xCE, "exteniosn_broadcaster_name_descriptor"},
	//{0xD5, "series_descriptor"},
	//{0xD6, "event_group_descriptor"},
	//{0xD8, "broadcaster_descriptor"},
	//{0xD9, "component_group_descriptor"},

	{0xC0, "Hierarchical transmission descriptor"},
	{0xC1, "Digital copy control descriptor"},
	{0xC2, "Network identification descriptor"},
	{0xC3, "Partial Transport Stream time descriptor"},
	{0xC4, "Audio component descriptor"},
	{0xC5, "Hyperlink descriptor"},
	{0xC6, "Target region descriptor"},
	{0xC7, "Data content descriptor"},
	{0xC8, "Video decode control descriptor"},
	{0xC9, "Download content descriptor"},
	{0xCA, "CA_EMM_TS descriptor"},
	{0xCB, "CA contract information descriptor"},
	{0xCC, "CA service descriptor"},
	{0xCD, "TS information descriptor"},
	{0xCE, "Extended broadcaster descriptor"},
	{0xCF, "Logo transmission descriptor"},
	{0xD0, "Basic local event descriptor"},
	{0xD1, "Reference descriptor"},
	{0xD2, "Node relation descriptor"},
	{0xD3, "Short node information descriptor"},
	{0xD4, "STC reference descriptor"},
	{0xD5, "Series descriptor"},
	{0xD6, "Event group descriptor"},
	{0xD7, "SI parameter descriptor"},
	{0xD8, "Broadcaster name descriptor"},
	{0xD9, "Component group descriptor"},
	{0xDA, "SI prime TS descriptor"},
	{0xDB, "Board information descriptor"},
	{0xDC, "LDT linkage descriptor"},
	{0xDD, "Connected transmission descriptor"},
	{0xDE, "Content availability descriptor"},
	{0xDF, "For tag value extension"},
	{0xE0, "Service group descriptor"},
	{0xF7, "Carousel compatible composite descriptor"},
	{0xF8, "Conditional playback descriptor"},
	{0xF9, "Cable TS division system descriptor"},
	{0xFA, "Terrestrial delivery system descriptor"},
	{0xFB, "Partial reception descriptor"},
	{0xFC, "Emergency information descriptor"},
	{0xFD, "Data component descriptor"},
	{0xFE, "System management descriptor"},
};

std::unordered_map<unsigned char, std::string> g_service_types = {
	{0x01, "Digital TV service"},
	{0x02, "Digital audio service"},
	{0xA1, "Special video service"},
	{0xA2, "Special audio service"},
	{0xA3, "Special data service"},
	{0xA4, "Engineering download service"},
	{0xA5, "Promotion video service"},
	{0xA6, "Promotion audio service"},
	{0xA7, "Promotion data service"},
	{0xA8, "Data service for accumulation beforehand"},
	{0xA9, "Data service exclusive for accumulation"},
	{0xAA, "Book mark list data service"},
	{0xC0, "Data service"},
};

const char* audio_component_descriptor_sampling_rate_names[8] = {
	"", "16kHz", "22.05kHz", "24kHz", "", "32kHz", "44.1kHz", "48kHz"
};

// 6-STD-B10v4_4-E1.pdf
// Table J-1 Denoted places of data component system and syntax
std::unordered_map<unsigned short, std::string> g_Data_component_names = {
	{0x0007, "ARIB-XML-base multimedia coding"},
	{0x0008, "ARIB-Subtitle& teletext coding"},
	{0x0009, "ARIB-Data download"},
	{0x000A, "G-guide (G-Guide Gold)"},
	{0x000B, "BML for 110°E CS"},
	{0x000C, "Multimedia coding for digital terrestrial broadcasting(A profile)"},
	{0x000D, "Multimedia coding for digital terrestrial broadcasting(C profile)"},
	{0x000E, "Multimedia coding for digital terrestrial broadcasting(P profile)"},
	{0x000F, "Multimedia coding for digital terrestrial broadcasting(E profile)"},
	{0x0010, "Real-time data service(Mobile profile)"},
	{0x0011, "Accumulation-type data service(Mobile profile)"},
	{0x0012, "Subtitle coding for digital terrestrial broadcasting(C profile)"},
	{0x0013, "Multimedia coding for digital terrestrial broadcasting (P2 profile)"},
	{0x0014, "Data carousel scheme for TYPE2 content transmission"},
	{0x0015, "DSM-CC section scheme for transmission of program start time information"},
	{0x0016, "ARIB-Descriptive language type meta data coding"},
};

std::string get_audio_component_type_name(uint8_t component_type)
{
	switch (component_type)
	{
		case 0x01: return "1/0 mode (single monaural channel)";
		case 0x02: return "1/0 + 1/0 mode (dual monaural channel)";
		case 0x03: return "2/0 mode (stereo)";
		case 0x04: return "2/1 mode";
		case 0x05: return "3/0 mode";
		case 0x06: return "2/2 mode";
		case 0x07: return "3/1 mode";
		case 0x08: return "3/2 mode";
		case 0x09: return "3/2 + LFE mode";
		case 0x40: return "Commentary for visually disabled persons";
		case 0x41: return "Voice for audibly disabled persons";
	}

	return "";
}

extern const char* stream_type_names[256];
extern const char* video_format_names[16];
extern const char* frame_rate_names[16];
extern const char* aspect_ratio_names[16];
extern const char* bit_depth_names[16];
extern const char* dynamic_range_type_names[16];
extern const char* color_space_names[16];
extern const char* MPEG4_audio_profile_and_level_names[256];

void PrintARIBDateTime(const char* szIndent, const char* szCaption,  uint8_t *p)
{
	uint16_t MJD = ((*p) << 8) | *(p + 1);
	p += 2;

	int K = 0;
	int Y1 = (int)((MJD - 15078.2) / 365.25);
	int M1 = int((MJD - 14956.1 - (int)(Y1*365.25)) / 30.6001);
	int D = MJD - 14956 - (int)(Y1*365.25) - (int)(M1*30.6001);

	if (M1 == 14 || M1 == 15)
		K = 1;

	int Y = Y1 + K;
	int M = M1 - 1 - K * 12;
	
	printf("%s%s Date(yyyy-mm-dd): %d-%d-%d\n", szIndent, szCaption, Y + 1900, M, D);
	printf("%s%s Time(HH:MM:SS): %d%d:%d%d:%d%d\n", szIndent, szCaption, (*p>>4)&0xF, *p&0xF, (*(p+1) >> 4) & 0xF, *(p+1) & 0xF, (*(p+2) >> 4) & 0xF, *(p+2) & 0xF);
}

void PrintDescriptor(int level, unsigned char* p)
{
	uint8_t* p0 = p;
	uint8_t descriptor_tag = *p++;
	uint8_t descriptor_length = *p++;
	char szIndent[128] = { 0 };
	if (level > 0)
		memset(szIndent, ' ', (size_t)level * 4);

	auto iter = g_SIT_descriptors.find(descriptor_tag);
	printf("%sdescriptor_tag/descriptor_length: 0X%02X/% 3d - %s\n", szIndent, 
		descriptor_tag, descriptor_length, iter == g_SIT_descriptors.cend() ? "Unsupported descriptor" : iter->second.c_str());

	memset(szIndent, ' ', ((size_t)level + 1) * 4);

	switch (descriptor_tag)
	{
	case 0x05:	// registration_descriptor
	{
		uint32_t format_id = (*p << 24) | (*(p + 1) << 16) | (*(p + 2) << 8) | *(p + 3);
		printf("%sformat identifier: 0x%08X(%c%c%c%c)\n", szIndent, (*p << 24) | (*(p + 1) << 16) | (*(p + 2) << 8) | *(p + 3),
			isprint(*p) ? *p : '.', isprint(*(p + 1)) ? *(p + 1) : '.', isprint(*(p + 2)) ? *(p + 2) : '.', isprint(*(p + 3)) ? *(p + 3) : '.');
		p += 4;
		
		if (format_id == 'HESE' && descriptor_length > 4)
		{
			p++;
			uint8_t stream_type = *p++;
			uint8_t video_format = ((*p) >> 4) & 0xF;
			uint8_t frame_rate = ((*p) & 0xF);
			p++;
			uint8_t aspect_ratio = ((*p) >> 4) & 0xF;
			uint8_t bit_depth = ((*p) & 0xF);
			p++;
			uint8_t dynamic_range = ((*p) >> 4) & 0xF;
			uint8_t color_space = ((*p) & 0xF);

			printf("%s  stream type: 0x%02X -- %s\n", szIndent, stream_type, stream_type_names[stream_type]);
			printf("%s video format: 0x%X  -- %s\n", szIndent, video_format, video_format_names[video_format]);
			printf("%s   frame rate: 0x%X  -- %s\n", szIndent, frame_rate, frame_rate_names[frame_rate]);
			printf("%s aspect ratio: 0x%X  -- %s\n", szIndent, aspect_ratio, aspect_ratio_names[aspect_ratio]);
			printf("%s    bit depth: 0x%X  -- %s\n", szIndent, bit_depth, bit_depth_names[bit_depth]);
			printf("%sdynamic range: 0x%X  -- %s\n", szIndent, dynamic_range, dynamic_range_type_names[dynamic_range]);
			printf("%s  color space: 0x%X  -- %s\n", szIndent, color_space, color_space_names[color_space]);
		}
		else
		{
			if (descriptor_length > 4)
			{
				printf("%sadditional_identification_info:\n", szIndent);
				print_mem(p, descriptor_length - 4, 4 * level);
			}
		}
		break;
	}
	case 0x1C:	// MPEG-4_audio_descriptor
	{
		uint8_t MPEG4_audio_profile_and_level = *p++;
		printf("%sMPEG-4_audio_profile_and_level: 0x%02X -- %s\n", szIndent, MPEG4_audio_profile_and_level, MPEG4_audio_profile_and_level_names[MPEG4_audio_profile_and_level]);
		break;
	}
	case 0x42:	// staffing_descriptor
	case 0x47:	// bouquet_name_descriptor
		break;
	case 0x48:	// service_descriptor
	{
		uint8_t service_type = *p++;
		auto iter = g_service_types.find(service_type);
		printf("%sservice_type: 0x%02x - %s\n", szIndent, service_type, iter == g_service_types.cend()?"Unsupported service type":iter->second.c_str());

		uint8_t service_provider_name_length = *p++;
		if (service_provider_name_length > 0)
		{
			printf("%sservice_provider_name(length: %d):\n", szIndent, service_provider_name_length);
			print_mem(p, service_provider_name_length, level * 4);
			p += service_provider_name_length;
		}

		uint8_t service_name_length = *p++;
		if (service_name_length > 0)
		{
			printf("%sservice_name(length: %d): \n", szIndent, service_name_length);
			print_mem(p, service_name_length, level * 4);
			p += service_name_length;
		}
		break;
	}
	case 0x49:	// country_availability_description
	case 0x4A:	// linkage_descriptor
	case 0x4B:	// NVOD_reference_descriptor
	case 0x4C:	// time_shifted_service_descriptor
		break;
	case 0x4D:	// short_event_descriptor
	{
		char ISO_639_language_code[3] = { 0 };
		for (int i = 0; i < 3; i++)
			ISO_639_language_code[i] = *p++;
		printf("%sISO_639_language_code: %c%c%c\n", szIndent, ISO_639_language_code[0], ISO_639_language_code[1], ISO_639_language_code[2]);
		uint8_t event_name_length = *p++;
		if (event_name_length > 0)
		{
			printf("%sevent name(length: %d):\n", szIndent, event_name_length);
			print_mem(p, event_name_length, level * 4);
		}
		p += event_name_length;
		uint8_t text_length = *p++;
		if (text_length > 0)
		{
			printf("%stext(length: %d):\n", szIndent, text_length);
			print_mem(p, text_length, level * 4);
		}

		break;
	}
	case 0x4E:	// extended_event_descriptor
	{
		uint8_t descriptor_number = ((*p) >> 4) & 0xF;
		uint8_t last_descriptor_number = (*p) & 0xF;
		p++;
		printf("%sdescriptor_number: 0x%x\n", szIndent, descriptor_number);
		printf("%slast_descriptor_number: 0x%x\n", szIndent, last_descriptor_number);
		char ISO_639_language_code[3] = { 0 };
		for (int i = 0; i < 3; i++)
			ISO_639_language_code[i] = *p++;
		printf("%sISO_639_language_code: %c%c%c\n", szIndent, ISO_639_language_code[0], ISO_639_language_code[1], ISO_639_language_code[2]);
		uint8_t length_of_items = *p++;
		uint8_t* p1 = p;
		int i = 0;
		while(p < p1 + length_of_items)
		{
			uint8_t item_description_length = *p++;
			if (item_description_length > 0)
			{
				printf("%sitem#%d_description(length: %d):\n", szIndent, i, item_description_length);
				print_mem(p, item_description_length, level * 4);
				p += item_description_length;
			}

			uint8_t item_length = *p++;
			if (item_length > 0)
			{
				printf("%sitem#%d(length: %d):\n", szIndent, i, item_length);
				print_mem(p, item_length, level * 4);
				p += item_length;
			}

			i++;
		}

		uint8_t text_length = *p++;
		if (text_length > 0)
		{
			printf("%stext(length: %d):\n", szIndent, text_length);
			print_mem(p, text_length, level * 4);
			p += text_length;
		}

		break;
	}
	case 0x4F:	// time_shifted_event_descriptor
		break;
	case 0x50:	// component_descriptor
	{
		uint8_t stream_content = (*p++) & 0xf;

		uint8_t component_type = *p++;
		uint8_t component_tag = *p++;
		uint8_t ISO_639_language_code[3] = { 0 };
		for (int i = 0; i < 3; i++)
			ISO_639_language_code[i] = *p++;
		printf("%sstream_content: 0x%x\n", szIndent, stream_content);
		printf("%scomponent_type: 0x%X\n", szIndent, component_type);
		printf("%scomponent_tag: %d\n", szIndent, component_tag);
		printf("%sISO_639_language_code: %c%c%c\n", szIndent, ISO_639_language_code[0], ISO_639_language_code[1], ISO_639_language_code[2]);
		if ((ptrdiff_t)descriptor_length + 2 > (p - p0))
		{
			print_mem(p, (int)((ptrdiff_t)descriptor_length + 2 - (p - p0)), (int)((int64_t)level * 4));
		}

		break;
	}
	case 0x51:	// mosaic_descriptor
	case 0x53:	// Ca_identifier_descriptor
		break;
	case 0x52:	// Stream identifier descriptor
	{
		uint8_t component_tag = *p++;
		printf("%scomponent_tag: 0x%02X\n", szIndent, component_tag);
		break;
	}
	case 0x54:	// content_descriptor
	{
		uint8_t content_nibble_level_1 = ((*p) >> 4) & 0xF;
		uint8_t content_nibble_level_2 = ((*p) & 0xF);
		p++;
		uint8_t user_nibble_1 = ((*p) >> 4) & 0xF;
		uint8_t user_nibble_2 = ((*p) & 0xF);
		p++;

		printf("%s content_nibble_level_1: %d\n", szIndent, content_nibble_level_1);
		printf("%s content_nibble_level_2: %d\n", szIndent, content_nibble_level_2);
		printf("%s user_nibble: %d\n", szIndent, user_nibble_1);
		printf("%s user_nibble: %d\n", szIndent, user_nibble_2);
		
		break;
	}
	case 0x55:	// parental_rating_descriptor
		break;
	case 0x63:	// partial_transport_stream_descriptor
	{
		{
			uint32_t peak_rate = (((*p) << 16) | ((*(p + 1)) << 8) | ((*(p + 2)))) & 0x3FFFFF;
			p += 3;
			uint32_t minimum_overall_smoothing_rate = (((*(p)) << 16) | ((*(p + 1)) << 8) | ((*(p + 2)))) & 0x3FFFFF;
			p += 3;
			uint32_t maximum_overall_smoothing_buffer = (((*(p)) << 8) | (*(p + 1))) & 0x3FFF;
			p += 2;

			printf("%speak_rate: %" PRIu32 "(%" PRIu64 "bps/%sbps)\n", szIndent, peak_rate, (uint64_t)peak_rate * 400, GetHumanReadNumber((uint64_t)peak_rate * 400).c_str());
			if (minimum_overall_smoothing_rate == 0x3FFFFF)
				printf("%sminimum_overall_smoothing_rate: undefined\n", szIndent);
			else
				printf("%sminimum_overall_smoothing_rate: %" PRIu32 "(%" PRIu64 "bps/%sbps)\n", szIndent, 
					minimum_overall_smoothing_rate, (uint64_t)minimum_overall_smoothing_rate * 400, GetHumanReadNumber((uint64_t)minimum_overall_smoothing_rate * 400).c_str());

			if (maximum_overall_smoothing_buffer == 0x3FFF)
				printf("%smaximum_overall_smoothing_buffer: undefined\n", szIndent);
			else
				printf("%smaximum_overall_smoothing_buffer: %d byte\n", szIndent, maximum_overall_smoothing_buffer);
			break;
		}
		break;
	}
	case 0xC1:	// digital_copy_control_descriptor
	{
		uint8_t digital_recording_control_data = ((*p)>>6)&0x3;
		printf("%sDigital copy control information: %d -- %s\n", szIndent, digital_recording_control_data, 
			digital_recording_control_data == 0?"Copy can be made without control condition":(
			digital_recording_control_data == 1?"Defined by service provider":(
			digital_recording_control_data == 2?"Copy can be made for only one generation":"Copy is forbidden")));
		uint8_t maximum_bitrate_flag = ((*p) >> 5) & 0x1;
		uint8_t component_control_flag = ((*p) >> 4) & 0x1;
		p++;
		if (maximum_bitrate_flag)
		{
			uint8_t maximum_bitrate = *p++;
			printf("%sMaximum bitrate: %.2f Mbps\n", szIndent, maximum_bitrate / 4.0f);
		}

		if (component_control_flag)
		{
			int idx = 0;
			uint8_t component_control_length = *p;
			while (component_control_flag >= 2)
			{
				uint8_t component_tag = *p++;
				printf("%scomponent tag: 0x%X\n", szIndent, component_tag);

				uint8_t digital_recording_control_data = ((*p)>>6)&0x3;
				printf("%s    Digital copy control information: %d -- %s\n", szIndent, digital_recording_control_data, 
					digital_recording_control_data == 0?"Copy can be made without control condition":(
					digital_recording_control_data == 1?"Defined by service provider":(
					digital_recording_control_data == 2?"Copy can be made for only one generation":"Copy is forbidden")));
				uint8_t maximum_bitrate_flag = ((*p) >> 5) & 0x1;
				p++;
				if (maximum_bitrate_flag)
				{
					uint8_t maximum_bitrate = *p++;
					printf("%s    Maximum bitrate: %.2f Mbps\n", szIndent, maximum_bitrate / 4.0f);
				}
			}
		}
		break;
	}
	case 0xC2:	// network_identifier_descriptor
	{
		char country_code[3];
		for (int i = 0; i < 3; i++)
			country_code[i] = *p++;
		printf("%sCountry Code: %c%c%c\n", szIndent, country_code[0], country_code[1], country_code[2]);
		unsigned short media_type = (*p << 8) | *(p + 1);
		p += 2;
		printf("%sMedia Type: 0X%X(%s)\n", szIndent, media_type, media_type == 0x4253?"BS/broadband CS":(
													 media_type == 0x4353?"Narrow-band CS/Advanced narrow-band CS":(
													 media_type == 0x5442?"Terrestrial broadcasting":"")));
		unsigned short network_id = (*p << 8) | *(p + 1);
		printf("%sNetwork ID: 0X%X\n", szIndent, network_id);
		break;
	}
	case 0xC3:	// partialTS_time_descriptor
	{
		uint8_t event_version_number = *p++;
		PrintARIBDateTime(szIndent, "event_start_time", p);
		p += 5;
		printf("%sduration: %02X:%02X:%02X\n", szIndent, *p, *(p + 1), *(p + 2));
		p += 3;
		printf("%soffset: %02X:%02X:%02X\n", szIndent, *p, *(p + 1), *(p + 2));
		p += 3;
		printf("%soffset_flag: %d\n", szIndent, (*p >> 2) & 0x1);
		printf("%sother_descriptor_status: %d\n", szIndent, (*p >> 1) & 0x1);
		printf("%sJST_time_flag: %d\n", szIndent, (*p) & 0x1);
		if ((*p) & 0x1)
		{
			p++;
			PrintARIBDateTime(szIndent, "JST_time", p);
		}
		break;
	}
	case 0xC4:	// audio_component_descriptor
	{
		uint8_t* p1 = p;
		uint8_t stream_content = (*p++) & 0xF;
		printf("%sstream_content: 0x%x\n", szIndent, stream_content);
		uint8_t component_type = *p++;
		printf("%scomponent_type: 0x%x -- %s\n", szIndent, component_type, get_audio_component_type_name(component_type).c_str());
		uint8_t component_tag = *p++;
		printf("%scomponent_tag: 0x%x\n", szIndent, component_tag);
		uint8_t stream_type = *p++;
		printf("%sstream_type: 0x%x\n", szIndent, stream_type);
		uint8_t simulcast_group_tag = *p++;
		printf("%ssimulcast_group_tag: 0x%x\n", szIndent, simulcast_group_tag);
		uint8_t ES_multi_lingual_flag = ((*p) >> 7) & 0x1;
		printf("%sES_multi_lingual_flag: 0x%x -- %s\n", szIndent, ES_multi_lingual_flag, ES_multi_lingual_flag?"Dual-Mono":"");
		uint8_t main_component_flag = ((*p) >> 6) & 0x1;
		printf("%smain_component_flag: 0x%x\n", szIndent, main_component_flag);
		uint8_t quality_indicator = ((*p) >> 4) & 0x3;
		printf("%squality_indicator: 0x%x -- %s\n", szIndent, quality_indicator, quality_indicator == 1 ? "Mode 1" : (quality_indicator == 2 ? "Mode 2" : (quality_indicator == 3 ? "Mode 3" : "")));
		uint8_t sampling_rate = ((*p) >> 1) & 0x7;
		printf("%ssampling_rate: 0x%x -- %s\n", szIndent, sampling_rate, audio_component_descriptor_sampling_rate_names[sampling_rate]);
		p++;
		uint8_t ISO_639_language_code[3] = { 0 };
		for (int i = 0; i < 3; i++)
			ISO_639_language_code[i] = *p++;
		printf("%sISO_639_language_code: %c%c%c\n", szIndent, ISO_639_language_code[0], ISO_639_language_code[1], ISO_639_language_code[2]);
		if (ES_multi_lingual_flag)
		{
			uint8_t ISO_639_language_code_2[3] = { 0 };
			for (int i = 0; i < 3; i++)
				ISO_639_language_code_2[i] = *p++;
			printf("%sISO_639_language_code_2: %c%c%c\n", szIndent, ISO_639_language_code_2[0], ISO_639_language_code_2[1], ISO_639_language_code_2[2]);
		}
		if (p < p1 + descriptor_length)
		{
			uint8_t text_length = (uint8_t)(p1 + descriptor_length - p);
			printf("%stext(length: %d):\n", szIndent, text_length);
			print_mem(p, text_length, level * 4);
			p += text_length;
		}
		break;
	}
	case 0xC5:	// hyperlink_descriptor
		break;
	case 0xC7:	// data_contents_descriptor
	{
		uint8_t data_component_id = ((*p) << 8) | *(p + 1);
		p += 2;
		auto iter = g_Data_component_names.find(data_component_id);
		printf("%sdata_component_id: 0x%X (%s)\n", szIndent, data_component_id, iter == g_Data_component_names.cend()?"":iter->second.c_str());
		uint8_t entry_component = *p++;
		printf("%sentry_component: 0x%X\n", szIndent, entry_component);
		uint8_t sector_length = *p++;
		if (sector_length > 0)
		{
			if (data_component_id == 0x0008)
			{
				uint8_t num_languages = *p++;
				for (int i = 0; i < num_languages; i++)
				{
					printf("%sARIB subtitle#%d:\n", szIndent, i + 1);
					uint8_t language_tag = (*p >> 5) & 0x7;
					uint8_t DMF = *p & 0xF;
					p++;
					printf("%s    language tag: %d\n", szIndent, language_tag);
					printf("%s    DMF: 0x%X\n", szIndent, DMF);

					char ISO_639_language_code[3] = { 0 };
					for (int j = 0; j < 3; j++)
						ISO_639_language_code[j] = *p++;

					printf("%s    ISO_639_language_code: %c%c%c\n", szIndent, ISO_639_language_code[0], ISO_639_language_code[1], ISO_639_language_code[2]);
				}
			}
			else
			{
				print_mem(p, sector_length, level * 4);
				p += sector_length;
			}
		}
		uint8_t num_of_component_ref = *p++;
		printf("%snum_of_component_ref: %d\n", szIndent, num_of_component_ref);
		for (int i = 0; i < num_of_component_ref; i++)
			printf("%s    component_ref[%d]: %d\n", szIndent, i, *p++);

		char ISO_639_language_code[3] = { 0 };
		for (int i = 0; i < 3; i++)
			ISO_639_language_code[i] = *p++;
		printf("%sISO_639_language_code: %c%c%c\n", szIndent, ISO_639_language_code[0], ISO_639_language_code[1], ISO_639_language_code[2]);
		uint8_t text_length = *p++;
		printf("%stext(len: %d):\n", szIndent, text_length);
		if (text_length > 0)
			print_mem(p, text_length, level * 4);
		break;
	}
	case 0xC8:	// Video decode control descriptor
	{
		uint8_t still_picture_flag = ((*p) >> 7) & 0x1;
		uint8_t sequence_end_code_flag = ((*p) >> 6) & 0x1;
		uint8_t video_encode_format = ((*p) >> 2) & 0x0F;
		printf("%s    still_picture_flag: %d\n", szIndent, still_picture_flag);
		printf("%ssequence_end_code_flag: %d\n", szIndent, sequence_end_code_flag);
		printf("%s   video_encode_format: %d\n", szIndent, video_encode_format);
		break;
	}
	case 0xCD:	// TS_information_descriptor
	case 0xCE:	// exteniosn_broadcaster_name_descriptor
	case 0xD5:	// series_descriptor
	case 0xD6:	// event_group_descriptor
		break;
	case 0xD8:	// broadcaster_descriptor
	{
		if (descriptor_length > 0)
			print_mem(p, descriptor_length, level * 4);
		break;
	}
	case 0xD9:	// component_group_descriptor
		break;
	case 0xDE:	// content_availability_descriptor
	{
		uint8_t ICT = ((*p) >> 5) & 0x1;
		uint8_t TAC = ((*p) >> 4) & 0x1;
		uint8_t time_TAC = ((*p) >> 1) & 0x7;
		uint8_t OP = (*p) & 0x1;

		printf("%s                  Image constraint token: %d\n", szIndent, ICT);
		printf("%s       Temporal accumulation control bit: %d\n", szIndent, TAC);
		printf("%s Allowable time of temporal accumulation: %d\n", szIndent, time_TAC);
		printf("%s                   Output protection bit: %d\n", szIndent, OP);
		break;
	}
	}

	printf("\n");
}

CPayloadBuf::CPayloadBuf(FILE* fw, uint8_t nTSPackSize)
	: buffer_len(0)
	, m_fw(fw)
	, m_ts_pack_size(nTSPackSize)
	, m_PID(0x1FFF)
	, m_PCR_PID(0x1FFF)
{
	buffer_alloc_size = 20 * 1024 * 1024;
	buffer = new (std::nothrow) unsigned char[buffer_alloc_size];
}

CPayloadBuf::CPayloadBuf(uint16_t PID)
	: buffer(NULL)
	, buffer_len(0)
	, buffer_alloc_size(0)
	, m_fw(NULL)
	, m_ts_pack_size(0)
	, m_PID(PID)
	, m_PCR_PID(0x1FFF)
{
}

CPayloadBuf::~CPayloadBuf()
{
	if (buffer != NULL)
	{
		delete[] buffer;
		buffer = NULL;
	}
}

int CPayloadBuf::PushTSBuf(uint32_t idxTSPack, uint8_t* pBuf, uint8_t offStart, uint8_t offEnd)
{
	if (idxTSPack != UINT32_MAX)
		slices.emplace_back(idxTSPack, offStart, offEnd);

	if (buffer == NULL)
	{
		buffer_alloc_size = 64 * 1024;
		buffer = new (std::nothrow) unsigned char[buffer_alloc_size];
		if (buffer == NULL)
		{
			printf("Failed to allocate the buffer with the size: %" PRIu32 " (bytes).\n", buffer_alloc_size);
			return -1;
		}
	}

	// check whether the buffer exceeds the next buffer length
	if (buffer_alloc_size < buffer_len + (offEnd - offStart))
	{
		unsigned char* new_buffer = new (std::nothrow) unsigned char[(size_t)buffer_alloc_size * 2];
		if (new_buffer == NULL)
		{
			printf("Failed to allocate the buffer with the size: %" PRIu32 " (bytes).\n", buffer_alloc_size << 1);
			return -1;
		}

		memcpy(new_buffer, buffer, buffer_len);
		delete[] buffer;

		buffer_alloc_size *= 2;
		buffer = new_buffer;
	}

	memcpy(buffer + buffer_len, pBuf + offStart, offEnd - offStart);

	buffer_len += offEnd - offStart;

	return 0;
}

int CPayloadBuf::Process(std::unordered_map<int, int>& pid_maps)
{
	unsigned long ulMappedSize = 0;
	unsigned char* pBuf = buffer;
	// Check the current payload buffer is a PSI buffer or not.
	if (buffer_len >= 4 && !IS_PES_PAYLOAD(buffer))
	{
		unsigned char pointer_field = *pBuf;
		unsigned char table_id;
		ulMappedSize++;
		ulMappedSize += pointer_field;

		if (ulMappedSize < buffer_len)
			table_id = pBuf[ulMappedSize];
		else
			return -1;

		if (ulMappedSize + 3 > buffer_len)
			return -1;

		//unsigned char* pSectionStart = &pBuf[ulMappedSize];
		unsigned long ulSectionStart = ulMappedSize;

		unsigned short section_length = (pBuf[ulMappedSize + 1] << 8 | pBuf[ulMappedSize + 2]) & 0XFFF;

		// The maximum number of bytes in a section of a Rec. ITU-T H.222.0 | ISO/IEC 13818-1 defined PSI table is
		// 1024 bytes. The maximum number of bytes in a private_section is 4096 bytes.
		// The DSMCC section data is also 4096 (table_id from 0x38 to 0x3F)
		if (section_length > ((pBuf[ulMappedSize] >= 0x40 && pBuf[ulMappedSize] <= 0xFE) ||
							  (pBuf[ulMappedSize] >= 0x38 && pBuf[ulMappedSize] <= 0x3F) ? 4093 : 1021))
			return -1;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

		if (ulMappedSize + 3 + section_length > buffer_len)
			return -1;	// RET_CODE_BUFFER_TOO_SMALL;

		unsigned char section_syntax_indicator = (pBuf[ulMappedSize + 1] >> 7) & 0x01;

		if ((table_id == TID_program_association_section || table_id == TID_TS_program_map_section) && !section_syntax_indicator)
			return -1;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

		if (ulMappedSize + 8 > buffer_len)
			return -1;

		ulMappedSize += 8;

		bool bChanged = false;

		// Check the PID in PAT and PMT
		if (table_id == TID_program_association_section)
		{
			// 4 bytes of CRC32, 4 bytes of PAT entry
			while (ulMappedSize + 4 + 4 <= 3 + section_length + ulSectionStart)
			{
				unsigned short PID = (pBuf[ulMappedSize + 2] & 0x1f) << 8 | pBuf[ulMappedSize + 3];

				if (pid_maps.find(PID) != pid_maps.end())
				{
					pBuf[ulMappedSize + 2] &= 0xE0;
					pBuf[ulMappedSize + 2] |= ((pid_maps[PID] >> 8) & 0x1F);
					pBuf[ulMappedSize + 3] = pid_maps[PID] & 0xFF;

					// Update the original TS pack according to buffer offset and value
					WriteBack(ulMappedSize + 2, &pBuf[ulMappedSize + 2], 2);

					bChanged = true;
				}

				ulMappedSize += 4;
			}
		}
		else if (table_id == TID_TS_program_map_section)
		{
			if (ulMappedSize + 4 > buffer_len)
				return -1;

			// Change PCR_PID
			unsigned short PID = (pBuf[ulMappedSize] & 0x1f) << 8 | pBuf[ulMappedSize + 1];
			if (pid_maps.find(PID) != pid_maps.end())
			{
				pBuf[ulMappedSize] &= 0xE0;
				pBuf[ulMappedSize] |= ((pid_maps[PID] >> 8) & 0x1F);
				pBuf[ulMappedSize + 1] = pid_maps[PID] & 0xFF;

				// Update the original TS pack according to buffer offset and value
				WriteBack(ulMappedSize, &pBuf[ulMappedSize], 2);
			}

			unsigned short program_info_length = (pBuf[ulMappedSize + 2] & 0xF) << 8 | pBuf[ulMappedSize + 3];
			ulMappedSize += 4;

			if (ulMappedSize + program_info_length > buffer_len)
				return -1;

			ulMappedSize += program_info_length;

			// Reserve 4 bytes of CRC32 and 5 bytes of basic ES info (stream_type, reserved, elementary_PID, reserved and ES_info_length)
			while (ulMappedSize + 5 + 4 <= 3 + section_length + ulSectionStart)
			{
				//unsigned char stream_type = pBuf[ulMappedSize];
				PID = (pBuf[ulMappedSize + 1] & 0x1f) << 8 | pBuf[ulMappedSize + 2];
				unsigned short ES_info_length = (pBuf[ulMappedSize + 3] & 0xF) << 8 | pBuf[ulMappedSize + 4];

				if (pid_maps.find(PID) != pid_maps.end())
				{
					pBuf[ulMappedSize + 1] &= 0xE0;
					pBuf[ulMappedSize + 1] |= ((pid_maps[PID] >> 8) & 0x1F);
					pBuf[ulMappedSize + 2] = pid_maps[PID] & 0xFF;

					// Update the original TS pack according to buffer offset and value
					WriteBack(ulMappedSize + 1, &pBuf[ulMappedSize + 1], 2);

					bChanged = true;
				}

				ulMappedSize += 5;
				ulMappedSize += ES_info_length;
			}
		}

		if (bChanged)
		{
			// Recalculate the CRC32 value
			F_CRC_InicializaTable();
			crc crc_val = F_CRC_CalculaCheckSum(&pBuf[ulSectionStart], (uint16_t)(ulMappedSize - ulSectionStart));
			pBuf[ulMappedSize] = (crc_val >> 24) & 0xFF;
			pBuf[ulMappedSize + 1] = (crc_val >> 16) & 0xFF;
			pBuf[ulMappedSize + 2] = (crc_val >> 8) & 0xFF;
			pBuf[ulMappedSize + 3] = (crc_val) & 0xFF;

			WriteBack(ulMappedSize, &pBuf[ulMappedSize], 4);
		}
	}

	return 0;
}

void CPayloadBuf::Reset()
{
	slices.clear();
	buffer_len = 0;
}

int CPayloadBuf::WriteBack(unsigned int off, unsigned char* pBuf, unsigned long cbSize)
{
	int iRet = -1;
	unsigned long cbRead = 0;
	// Find which TS pack the corresponding bytes are written into TS pack buffer
	for (std::vector<PayloadBufSlice>::iterator iter = slices.begin(); iter != slices.end(); iter++)
	{
		if (off >= cbRead && off < cbRead + iter->end_off - iter->start_off)
		{
			const unsigned long cbWritten = std::min(cbSize, cbRead + iter->end_off - iter->start_off - off);

			if (m_fw != NULL)
			{
				// Record the backup position of file
				long long backup_pos = _ftelli64(m_fw);

				_fseeki64(m_fw, (long long)iter->ts_packet_idx*m_ts_pack_size + iter->start_off + off - cbRead, SEEK_SET);
				if (fwrite(pBuf, 1, cbWritten, m_fw) != cbWritten)
				{
					printf("Failed to write back the bytes into destination file.\n");
					_fseeki64(m_fw, backup_pos, SEEK_SET);
					goto done;
				}

				// Restore the original file position
				_fseeki64(m_fw, backup_pos, SEEK_SET);
			}

			off += cbWritten;
			cbSize -= cbWritten;
			pBuf += cbWritten;
		}

		if (off + cbSize <= cbRead)
			break;

		cbRead += iter->end_off - iter->start_off;
	}

	iRet = 0;

done:
	return iRet;
}

CPSIBuf::CPSIBuf(PSI_PROCESS_CONTEXT* CtxPSIProcess, unsigned short PID)
	: CPayloadBuf(PID)
	, table_id(0)
	, version_number(0xFF)
	, section_number(0)
	, last_section_number(0)
	, ctx_psi_process(CtxPSIProcess)
{

}

/*!	@breif Process PAT and PMT to get the stream information.
@retval -1 buffer is too small
@retval -2 CRC verification failure
@retval -3 Unsupported or unimplemented */
int CPSIBuf::ProcessPSI(int dumpOptions)
{
	int iRet = -1;
	unsigned long ulMappedSize = 0;
	unsigned char* pBuf = buffer;

	if (buffer_len < 4)
		return -1;

	unordered_map<unsigned short, CPSIBuf*>& pPMTPayloads = *ctx_psi_process->pPSIPayloads;

	ctx_psi_process->bChanged = false;

	unsigned char pointer_field = *pBuf;
	ulMappedSize++;
	ulMappedSize += pointer_field;

	if (ulMappedSize < buffer_len)
		table_id = pBuf[ulMappedSize];
	else
		return -1;

	if (ulMappedSize + 3 > buffer_len)
		return -1;

	//unsigned char* pSectionStart = &pBuf[ulMappedSize];
	unsigned long ulSectionStart = ulMappedSize;

	unsigned short section_length = (pBuf[ulMappedSize + 1] << 8 | pBuf[ulMappedSize + 2]) & 0XFFF;

	// The maximum number of bytes in a section of a Rec. ITU-T H.222.0 | ISO/IEC 13818-1 defined PSI table is
	// 1024 bytes. The maximum number of bytes in a private_section is 4096 bytes.
	// The DSMCC section data is also 4096 (table_id from 0x38 to 0x3F)
	if (section_length > ((pBuf[ulMappedSize] >= 0x40 && pBuf[ulMappedSize] <= 0xFE) ||
						  (pBuf[ulMappedSize] >= 0x38 && pBuf[ulMappedSize] <= 0x3F) ? 4093 : 1021))
		return -3;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

	if (ulMappedSize + 3 + section_length > buffer_len)
		return -1;	// RET_CODE_BUFFER_TOO_SMALL;

	unsigned char section_syntax_indicator = (pBuf[ulMappedSize + 1] >> 7) & 0x01;

	if (table_id != TID_program_association_section && table_id != TID_TS_program_map_section && table_id != TID_SIT)
		return -3;	// Only support PAT, PMT and SIT

	if ((table_id == TID_program_association_section || table_id == TID_TS_program_map_section || table_id == TID_SIT) && !section_syntax_indicator)
		return -3;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

	if (section_syntax_indicator) {
		F_CRC_InicializaTable();
		if (F_CRC_CalculaCheckSum(pBuf + ulMappedSize, (size_t)section_length + 3) != 0) {
			printf("[13818-1] current PSI section failed do check-sum.\n");
			return -2;
		}
	}

	if (ulMappedSize + 8 > buffer_len)
		return -1;

	uint8_t current_next_indicator = pBuf[ulMappedSize + 5] & 0x1;
	if (current_next_indicator == 0)
		return 0;

	uint8_t current_version_number = (pBuf[ulMappedSize + 5] >> 1) & 0x1F;
	if (current_version_number != version_number)
	{
		ctx_psi_process->bChanged = true;
		version_number = current_version_number;
		section_number = pBuf[ulMappedSize + 6];
		last_section_number = pBuf[ulMappedSize + 7];
	}

	ulMappedSize += 8;

	if (m_PID == PID_PROGRAM_ASSOCIATION_TABLE && table_id == TID_program_association_section)
	{
		if ((dumpOptions&DUMP_PAT) && ctx_psi_process->bChanged)
			printf("PAT(ver: %d):\n", version_number);

		// check how many PMT entry exist in the current PAT
		int num_of_PMTs = (section_length + ulSectionStart + 3 - ulMappedSize - 4) >> 2;
		std::vector<std::tuple<unsigned short/* program number*/, unsigned short/*PMT PID*/>> PMT_PIDs;
		PMT_PIDs.reserve(num_of_PMTs);

		//unsigned short network_PID = 0x1FFF;
		// 4 bytes of CRC32, 4 bytes of PAT entry
		while (ulMappedSize + 4 + 4 <= 3 + section_length + ulSectionStart)
		{
			unsigned short program_number = (pBuf[ulMappedSize] << 8) | pBuf[ulMappedSize + 1];
			unsigned short PMT_PID = (pBuf[ulMappedSize + 2] & 0x1f) << 8 | pBuf[ulMappedSize + 3];

			// Network Information PID is also put into the vector, and it may be analyzed for SIT later
			PMT_PIDs.push_back({ program_number, PMT_PID });

			if ((dumpOptions&DUMP_PAT) && ctx_psi_process->bChanged)
				printf("    program number: 0x%04X(%05d), %s:0x%X\n", 
					program_number, program_number, program_number == 0?"Network_PID":"program_map_PID", PMT_PID);
			ulMappedSize += 4;
		}

		// Refresh the current PMT payloads
		// If the current PID in PMT payloads does not belong to PAT, delete it
		//for (auto iter = pPMTPayloads.cbegin(); iter != pPMTPayloads.cend(); iter++)
		auto iter = pPMTPayloads.cbegin();
		while(iter != pPMTPayloads.cend())
		{
			auto PMT_iter = PMT_PIDs.cbegin();
			for (; PMT_iter != PMT_PIDs.cend() && iter->first != std::get<1>(*PMT_iter); PMT_iter++);

			if (PMT_iter == PMT_PIDs.cend() && iter->first != PID_PROGRAM_ASSOCIATION_TABLE)
			{
				auto iter_del = iter;
				iter++;
				delete iter_del->second;
				pPMTPayloads.erase(iter_del);
			}
			else
			{
				iter++;
			}
		}

		// If the PMT ID does not exist in pPMTPayloads, create it.
		for (auto PMT_iter = PMT_PIDs.cbegin(); PMT_iter != PMT_PIDs.cend(); PMT_iter++)
		{
			if (pPMTPayloads.find(std::get<1>(*PMT_iter)) == pPMTPayloads.end())
			{
				if (std::get<0>(*PMT_iter) == 0)
					pPMTPayloads[std::get<1>(*PMT_iter)] = new CPSIBuf(ctx_psi_process, std::get<0>(*PMT_iter));
				else
					pPMTPayloads[std::get<1>(*PMT_iter)] = new CPMTBuf(ctx_psi_process, std::get<1>(*PMT_iter), std::get<0>(*PMT_iter));
			}
		}

		iRet = 0;
	}
	else if (table_id == TID_TS_program_map_section)
	{
		if ((dumpOptions&DUMP_PMT) && ctx_psi_process->bChanged)
			printf("PMT(ver: %d):\n", version_number);

		// Process the current PMT, and get the related information.
		if (ulMappedSize + 4 > buffer_len)
			return -1;

		// Change PCR_PID
		m_PCR_PID = (pBuf[ulMappedSize] & 0x1f) << 8 | pBuf[ulMappedSize + 1];

		if ((dumpOptions&DUMP_PMT) && ctx_psi_process->bChanged)
			printf("    PCR_PID: 0x%X\n", m_PCR_PID);

		unsigned short program_info_length = (pBuf[ulMappedSize + 2] & 0xF) << 8 | pBuf[ulMappedSize + 3];
		ulMappedSize += 4;

		if (ulMappedSize + program_info_length > buffer_len)
			return -1;

		unsigned char* p = &pBuf[ulMappedSize];
		ulMappedSize += program_info_length;

		while (program_info_length >= 2)
		{
			unsigned char descriptor_tag = *p;
			unsigned char descriptor_length = *(p + 1);

			if ((dumpOptions&DUMP_PMT) && ctx_psi_process->bChanged)
				PrintDescriptor(1, p);

			p += (ptrdiff_t)descriptor_length + 2;
			if (program_info_length >= 2 + descriptor_length)
				program_info_length -= 2 + descriptor_length;
			else
				program_info_length = 0;
		}

		// Reserve 4 bytes of CRC32 and 5 bytes of basic ES info (stream_type, reserved, elementary_PID, reserved and ES_info_length)
		while (ulMappedSize + 5 + 4 <= 3 + section_length + ulSectionStart)
		{
			unsigned char stream_type = pBuf[ulMappedSize];
			unsigned short ES_PID = (pBuf[ulMappedSize + 1] & 0x1f) << 8 | pBuf[ulMappedSize + 2];
			m_stream_types[ES_PID] = stream_type;
			unsigned short ES_info_length = (pBuf[ulMappedSize + 3] & 0xF) << 8 | pBuf[ulMappedSize + 4];

			if ((dumpOptions&DUMP_PMT) && ctx_psi_process->bChanged)
				printf("    ES_PID: 0x%X, stream_type: 0x%02X -- (%s):\n", ES_PID, stream_type, stream_type_names[stream_type]);

			ulMappedSize += 5;
			unsigned char* p = &pBuf[ulMappedSize];
			ulMappedSize += ES_info_length;

			if (ulMappedSize + 4 /*CRC32*/ > 3 + section_length + ulSectionStart)
			{
				// broken data
				break;
			}

			while (ES_info_length >= 2)
			{
				unsigned char descriptor_tag = *p;
				unsigned char descriptor_length = *(p + 1);

				if ((dumpOptions&DUMP_PMT) && ctx_psi_process->bChanged)
					PrintDescriptor(3, p);

				p += (ptrdiff_t)descriptor_length + 2;
				if (ES_info_length >= 2 + descriptor_length)
					ES_info_length -= 2 + descriptor_length;
				else
					ES_info_length = 0;
			}
		}

		iRet = 0;
	}
	else if (table_id == TID_SIT && ctx_psi_process->bChanged)
	{
 		unsigned short Transmissioninfo_loop_length = ((pBuf[ulMappedSize] & 0xF) << 8) | pBuf[ulMappedSize + 1];
		ulMappedSize += 2;

		unsigned char* p = &pBuf[ulMappedSize];
		ulMappedSize += Transmissioninfo_loop_length;

		if (dumpOptions&DUMP_DTV_SIT)
			printf("SIT(ver: %d):\n", version_number);

		while (Transmissioninfo_loop_length >= 2)
		{
			unsigned char descriptor_tag = *p;
			unsigned char descriptor_length = *(p + 1);

			if (dumpOptions&DUMP_DTV_SIT)
				PrintDescriptor(1, p);

			p += (ptrdiff_t)descriptor_length + 2;
			if (Transmissioninfo_loop_length >= 2 + descriptor_length)
				Transmissioninfo_loop_length -= 2 + descriptor_length;
			else
				Transmissioninfo_loop_length = 0;
		}

		while (ulMappedSize + 4 /*CRC32*/ + 4 /*loop header*/ <= 3 + section_length + ulSectionStart)
		{
			unsigned short service_id = (pBuf[ulMappedSize] << 8) | pBuf[ulMappedSize + 1];
			ulMappedSize += 2;
			unsigned char running_state = (pBuf[ulMappedSize] >> 4) & 0x7;
			unsigned short service_loop_length = ((pBuf[ulMappedSize] & 0xF) << 8) | pBuf[ulMappedSize + 1];
			ulMappedSize += 2;

			if (dumpOptions&DUMP_DTV_SIT)
				printf("    service_id: 0X%X, running_state: %d\n", service_id, running_state);

			p = &pBuf[ulMappedSize];
			ulMappedSize += service_loop_length;

			if (ulMappedSize + 4 /*CRC32*/ > 3 + section_length + ulSectionStart)
			{
				// broken data
				break;
			}

			while (service_loop_length >= 2)
			{
				unsigned char descriptor_tag = *p;
				unsigned char descriptor_length = *(p + 1);

				if (dumpOptions&DUMP_DTV_SIT)
					PrintDescriptor(3, p);

				p += (ptrdiff_t)descriptor_length + 2;
				if (service_loop_length >= 2 + descriptor_length)
					service_loop_length -= 2 + descriptor_length;
				else
					service_loop_length = 0;
			}
		}

		if (dumpOptions&DUMP_DTV_SIT)
			printf("\n\n");
	}
	else
		iRet = -3;

	return iRet;
}

CPMTBuf::CPMTBuf(PSI_PROCESS_CONTEXT* CtxPSIProcess, unsigned short PID, unsigned short nProgramNumber)
	: CPSIBuf(CtxPSIProcess, PID)
	, program_number(nProgramNumber)
{
	table_id = TID_TS_program_map_section;
}

int CPMTBuf::GetPMTInfo(unsigned short ES_PID, unsigned char& stream_type)
{
	if (m_stream_types.find(ES_PID) == m_stream_types.end())
		return -1;

	stream_type = m_stream_types[ES_PID];
	return 0;
}

unordered_map<unsigned short, unsigned char>& CPMTBuf::GetStreamTypes()
{
	return m_stream_types;
}
