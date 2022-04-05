#ifndef _MPEG2_DESCRIPTOR_HDMV_H_
#define _MPEG2_DESCRIPTOR_HDMV_H_

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "DumpUtil.h"
#include "AMArray.h"
#include "AMBitStream.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#define DT_HDMV_registration_descriptor				0x05
#define DT_HDMV_video_registration_descriptor		0x05
#define DT_VC1_registration_descriptor				0x05
#define DT_HDMV_LPCM_audio_registration_descriptor	0x05
#define DT_AC3_registration_descriptor				0x05
#define DT_AC3_audio_descriptor						0x81
#define DT_Caption_service_descriptor				0x86
#define DT_HDMV_copy_control_descritpor				0x88
#define DT_Partial_transport_stream_descriptor		0x63
#define DT_AVC_video_descriptor						0x28
#define DT_AVC_timing_and_HRD_descriptor			0x2A
#define DT_MVC_extension_descriptor					0x31
#define DT_BD_system_use_descriptor					0x89
#define DT_DRA_registration_descriptor				0x05

extern const char* hdmv_descriptor_tag_names[256];
extern const char* subdescriptor_tag_names[256];
extern const char* hdmv_stream_type_names[256];
extern const char* frame_rate_names[16];
extern const char* video_format_names[16];
extern const char* aspect_ratio_names[16];
extern const char* hdmv_audio_presentation_type_names[16];
extern const char* hdmv_sampling_frequency_names[16];
extern const char* ac3_sample_rate_code_names[8];
extern const unsigned short ac3_bit_rate_code_values[64];
extern const char* ac3_num_channels_names[16];
extern const char* ac3_priority_names[4];
extern const char* ac3_bsmod_names[8];
extern const char* ac3_dsurmod_names[4];

#define CHECK_DESCRIPTOR_MAP()		if ((unsigned long)descriptor_length + 2 <= ulMappedSize)goto done
#define CHECK_DESCRIPTOR_WRITE()	if (descriptor_length + 2 <= (AMBst_Tell(bs) - nSavePoint)/8)return RET_CODE_SUCCESS

namespace BST {
	
	struct CHDMVRegistrationDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		format_identifier[4];
		
		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("format_identifier", 32, format_identifier, 4, "Should be 48h 44h 4Dh 56h (ASCII code of &quot;HDMV&quot;)");
		DECLARE_FIELDPROP_END()
	}PACKED;
	
	struct CHDMVVideoRegistrationDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		format_identifier[4];
		unsigned char		stuffing_bits_0;
		unsigned char		stream_coding_type;
#ifdef _BIG_ENDIAN_
		unsigned char		video_format:4;
		unsigned char		frame_rate:4;
			
		unsigned char		aspect_ratio:4;
		unsigned char		stuffing_bits_1:4;
#else
		unsigned char		frame_rate:4;
		unsigned char		video_format:4;
			
		unsigned char		stuffing_bits_1:4;
		unsigned char		aspect_ratio:4;
#endif
		
		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("format_identifier", 32, format_identifier, 4, "Should be 0x48 44 4D 56 (ASCII code of &quot;HDMV&quot;)");
			NAV_FIELD_PROP_2NUMBER("stuffing_bits", 8, stuffing_bits_0, "");
			NAV_FIELD_PROP_2NUMBER1(stream_coding_type, 8, hdmv_stream_type_names[stream_coding_type]);
			NAV_FIELD_PROP_2NUMBER1(video_format, 4, video_format_names[video_format]);
			NAV_FIELD_PROP_2NUMBER1(frame_rate, 4, frame_rate_names[frame_rate]);
			NAV_FIELD_PROP_2NUMBER1(aspect_ratio, 4, aspect_ratio_names[aspect_ratio]);
			NAV_FIELD_PROP_2NUMBER("stuffing_bits", 4, stuffing_bits_1, "");
		DECLARE_FIELDPROP_END()
	}PACKED;
	
	struct CVC1RegistrationDescriptor: public ADVANCE_ENDIAN_MAP
	{
		struct CSubDescriptor
		{
			unsigned char		subdescriptor_tag;
			
			union{
				struct {
					unsigned char		profile_level;	
				}PACKED;
				struct {
					unsigned char		alignment_type;
				}PACKED;
				struct {
					union {
						struct {
#ifdef _BIG_ENDIAN_
							unsigned char		reserved:4;
							unsigned char		buffer_size_exponent:4;
#else
							unsigned char		buffer_size_exponent:4;
							unsigned char		reserved:4;
#endif
						};
						unsigned char	char_value;
					};
					unsigned short		hrd_buffer;
				}PACKED;
			}PACKED;
		}PACKED;
		
		unsigned char		descriptor_tag = 0;
		unsigned char		descriptor_length = 0;
		unsigned char		format_identifier[4] = { 0 };
		
		std::vector<CSubDescriptor>
							subdescriptors;
							
		unsigned char		stuffing_bytes_count = 0;
		unsigned char*		stuffing_bytes = nullptr;
		
		CVC1RegistrationDescriptor(){}
		virtual ~CVC1RegistrationDescriptor(){Unmap();}
		
		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&descriptor_tag, 6);
			
			while((pBuf[ulMappedSize] >= 0 && pBuf[ulMappedSize] <= 4) || pBuf[ulMappedSize] == 0xFF){
				CSubDescriptor sub_descriptor;
				sub_descriptor.subdescriptor_tag = pBuf[ulMappedSize];				
				if (pBuf[ulMappedSize] == 0 || pBuf[ulMappedSize] == 0xFF){
					ulMappedSize++;
				}else if(pBuf[ulMappedSize] == 1){
					ulMappedSize++;
					sub_descriptor.profile_level = pBuf[ulMappedSize++];
				}else if(pBuf[ulMappedSize] == 2){
					ulMappedSize++;
					sub_descriptor.alignment_type = pBuf[ulMappedSize++];
				}else if(pBuf[ulMappedSize] == 3){
					ulMappedSize++;
					sub_descriptor.char_value = pBuf[ulMappedSize++];
					sub_descriptor.hrd_buffer = (pBuf[ulMappedSize]<<8)|pBuf[ulMappedSize+1];
					ulMappedSize+=2;
				}else if(pBuf[ulMappedSize] == 4){
					ulMappedSize++;
				}else{
					break;
				}
				subdescriptors.push_back(sub_descriptor);	
			}
			
			if ((unsigned long)(descriptor_length + 2) > ulMappedSize){
				stuffing_bytes_count = (unsigned char)(descriptor_length + 2 - ulMappedSize);
				stuffing_bytes = pBuf + ulMappedSize;
			}

			ulMappedSize = descriptor_length + 2;
			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize != NULL){
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);
			AMBst_PutBytes(bs, format_identifier, 4);
			for(size_t i=0;i<subdescriptors.size();i++){
				unsigned char subdescriptor_tag = subdescriptors[i].subdescriptor_tag;
				AMBst_PutByte(bs, subdescriptor_tag);
				
				switch(subdescriptor_tag)
				{
				case 1: AMBst_PutByte(bs, subdescriptors[i].profile_level); break;
				case 2: AMBst_PutByte(bs, subdescriptors[i].alignment_type); break;
				case 3: 
					AMBst_PutBits(bs, 4, subdescriptors[i].reserved);
					AMBst_PutBits(bs, 4, subdescriptors[i].buffer_size_exponent);
					AMBst_PutWord(bs, subdescriptors[i].hrd_buffer);
					break;
				}
			}
			
			AMBst_PutBytes(bs, stuffing_bytes, stuffing_bytes_count);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("format_identifier", 32, format_identifier, 4, "VC-1(0x56432D31)");
			for(size_t i=0;i<subdescriptors.size();i++){
				CSubDescriptor& subdesc = subdescriptors[i];
				NAV_WRITE_TAG_BEGIN3_1("subdescriptor", i + 1, subdescriptor_tag_names[subdesc.subdescriptor_tag]);
				if (subdesc.subdescriptor_tag == 1){
					NAV_FIELD_PROP_2NUMBER1(subdesc.profile_level, 8, "")
				}else if(subdesc.subdescriptor_tag == 2){
					NAV_FIELD_PROP_2NUMBER1(subdesc.alignment_type, 8, "")
				}else if(subdesc.subdescriptor_tag == 3){
					NAV_FIELD_PROP_2NUMBER1(subdesc.reserved, 4, "")
					NAV_FIELD_PROP_2NUMBER1(subdesc.buffer_size_exponent, 4, "")
					NAV_FIELD_PROP_2NUMBER1(subdesc.hrd_buffer, 16, "")
				}
					
				NAV_WRITE_TAG_END3("subdescriptor", i+1);
			}
			NAV_FIELD_PROP_FIXSIZE_BINSTR("stuffing_bytes", (unsigned long)stuffing_bytes_count*8, stuffing_bytes, (unsigned long)stuffing_bytes_count, "")
		DECLARE_FIELDPROP_END()
	};

	struct CHDMVLPCMAudioRegistrationDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		format_identifier[4];
		unsigned char		stuffing_bits_0;
		unsigned char		stream_coding_type;

#ifdef _BIG_ENDIAN_
		unsigned char		audio_presentation_type:4;
		unsigned char		sampling_frequency:4;

		unsigned char		bits_per_sample:2;
		unsigned char		stuffing_bits_1:6;	
#else
		unsigned char		sampling_frequency:4;
		unsigned char		audio_presentation_type:4;

		unsigned char		stuffing_bits_1:6;
		unsigned char		bits_per_sample:2;
#endif

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("format_identifier", 32, format_identifier, 4, "Should be 0x48 44 4D 56 (ASCII code of &quot;HDMV&quot;)");
			NAV_FIELD_PROP_2NUMBER("stuffing_bits", 8, stuffing_bits_0, "");
			NAV_FIELD_PROP_2NUMBER1(stream_coding_type, 8, hdmv_stream_type_names[stream_coding_type]);
			NAV_FIELD_PROP_2NUMBER1(audio_presentation_type, 4, audio_presentation_type==1?"single mono channel":(
																audio_presentation_type==3?"stereo(2-channel)":(
																audio_presentation_type==6?"multi-channel":"Unknown")));
			NAV_FIELD_PROP_2NUMBER1(sampling_frequency, 4, sampling_frequency==1?"48KHZ":(
														   sampling_frequency==4?"96KHZ":(
														   sampling_frequency==5?"192KHZ":"Unknown")));
			NAV_FIELD_PROP_2NUMBER1(bits_per_sample, 2, bits_per_sample==0?"reserved":(
														bits_per_sample==1?"16 bits/sample":(
														bits_per_sample==2?"20 bits/sample":(
														bits_per_sample==3?"24 bits/sample":"Unknown"))));
			NAV_FIELD_PROP_2NUMBER("stuffing_bits", 6, stuffing_bits_1, "");
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CAC3RegistrationDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		format_identifier[4];
		
		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("format_identifier", 32, format_identifier, 4, "Should be 41h 43h 2Dh 33h (&quot;AC-3&quot;).");
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CAC3AudioDescriptor: public ADVANCE_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

#ifdef _BIG_ENDIAN_
		unsigned char		sample_rate_code:3;
		unsigned char		bsid:5;

		unsigned char		bit_rate_code:6;
		unsigned char		surround_mode:2;

		unsigned char		bsmod:3;
		unsigned char		num_channels:4;
		unsigned char		full_svc:1;
#else
		unsigned char		bsid:5;
		unsigned char		sample_rate_code:3;

		unsigned char		surround_mode:2;
		unsigned char		bit_rate_code:6;

		unsigned char		full_svc:1;
		unsigned char		num_channels:4;
		unsigned char		bsmod:3;
#endif

		unsigned char		langcod;
		unsigned char		langcod2;

		union{
			struct {
#ifdef _BIG_ENDIAN_
				unsigned char	mainid:3;
				unsigned char	priority:2;
				unsigned char	reserved_0:3;
#else
				unsigned char	reserved_0:3;
				unsigned char	priority:2;
				unsigned char	mainid:3;
#endif
			}PACKED;
			unsigned char	asvcflags;
		}PACKED;

		union{
			struct {
#ifdef _BIG_ENDIAN_
				unsigned char		textlen:7;
				unsigned char		textcode:1;
#else
				unsigned char		textcode:1;
				unsigned char		textlen:7;
#endif
			}PACKED;
			unsigned char	text_charvalue;
		}PACKED;

		unsigned char		text[128];
		
		// The below field is optional
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned char		language_flag:1;
				unsigned char		language_flag_2:1;
				unsigned char		reserved_1:6;
#else
				unsigned char		reserved_1:6;
				unsigned char		language_flag_2:1;
				unsigned char		language_flag:1;
#endif
			}PACKED;
			unsigned char		language_charvalue;
		}PACKED;

		unsigned char		language[3];
		unsigned char		language_2[3];

		unsigned char		stuffing_bytes_count;
		unsigned char*		stuffing_bytes;



		CAC3AudioDescriptor():stuffing_bytes_count(0), stuffing_bytes(NULL){}
		virtual ~CAC3AudioDescriptor(){Unmap();}
		
		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&descriptor_tag, 5);

			CHECK_DESCRIPTOR_MAP();
			langcod = pBuf[ulMappedSize++];

			if (num_channels == 0){
				CHECK_DESCRIPTOR_MAP();
				langcod2 = pBuf[ulMappedSize++];
			}

			CHECK_DESCRIPTOR_MAP();
			asvcflags = pBuf[ulMappedSize++];

			CHECK_DESCRIPTOR_MAP();
			text_charvalue = pBuf[ulMappedSize++];

			if (textlen > 0){
				CHECK_DESCRIPTOR_MAP();
				memcpy(text, pBuf + ulMappedSize, textlen);
				ulMappedSize += textlen;
			}

			// The below part may be optional, if the length is not enough
			if (ulMappedSize < cbSize){
				CHECK_DESCRIPTOR_MAP();
				language_charvalue = pBuf[ulMappedSize++];
				if (language_flag){
					CHECK_DESCRIPTOR_MAP();
					memcpy(language, pBuf + ulMappedSize, 3);
					ulMappedSize += 3;
				}

				if (language_flag_2){
					CHECK_DESCRIPTOR_MAP();
					memcpy(language_2, pBuf + ulMappedSize, 3);
					ulMappedSize += 3;
				}

				if ((unsigned long)(descriptor_length + 2) > ulMappedSize){
					CHECK_DESCRIPTOR_MAP();
					stuffing_bytes_count = (unsigned char)(descriptor_length + 2 - ulMappedSize);
					stuffing_bytes = pBuf + ulMappedSize;
				}
			}

done:
			ulMappedSize = descriptor_length + 2;
			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize != NULL){
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			int nSavePoint = AMBst_Tell(bs);
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);

			AMBst_PutBits(bs, 3, sample_rate_code);
			AMBst_PutBits(bs, 5, bsid);
			
			AMBst_PutBits(bs, 6, bit_rate_code);
			AMBst_PutBits(bs, 2, surround_mode);

			AMBst_PutBits(bs, 3, bsmod);
			AMBst_PutBits(bs, 4, num_channels);
			AMBst_PutBits(bs, 1, full_svc);

			CHECK_DESCRIPTOR_WRITE();
			AMBst_PutBits(bs, 8, langcod);

			if (num_channels == 0){
				CHECK_DESCRIPTOR_WRITE();
				AMBst_PutBits(bs, 8, langcod2);
			}

			CHECK_DESCRIPTOR_WRITE();
			AMBst_PutBits(bs, 8, asvcflags);

			CHECK_DESCRIPTOR_WRITE();
			AMBst_PutBits(bs, 8, text_charvalue);
			AMBst_PutBytes(bs, text, textlen);

			if (AMBst_Tell(bs) <  nSavePoint + descriptor_length)
			{
				AMBst_PutBits(bs, 8, language_charvalue);

				if (language_flag)
					AMBst_PutBytes(bs, language, 3);

				if (language_flag_2)
					AMBst_PutBytes(bs, language_2, 3);

				AMBst_PutBytes(bs, stuffing_bytes, stuffing_bytes_count);
			}

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(sample_rate_code, 3, ac3_sample_rate_code_names[sample_rate_code]);
			NAV_FIELD_PROP_2NUMBER1(bsid, 5, "Bit stream identification");
			MBCSPRINTF_S(szTemp2, _countof(szTemp2), "%d kbps", ac3_bit_rate_code_values[bit_rate_code]);
			NAV_FIELD_PROP_2NUMBER1(bit_rate_code, 6, szTemp2);
			NAV_FIELD_PROP_2NUMBER1(surround_mode, 2, ac3_dsurmod_names[surround_mode]);
			NAV_FIELD_PROP_2NUMBER1(bsmod, 3, ac3_bsmod_names[bsmod]);
			NAV_FIELD_PROP_2NUMBER1(num_channels, 4, ac3_num_channels_names[num_channels]);
			NAV_FIELD_PROP_2NUMBER1(full_svc, 1, full_svc?"this audio service is sufficiently complete to be presented to the listener without being combined with another audio service"
														 :"the service is not sufficiently complete to be presented without being combined with another audio service");

			if (descriptor_length < cbRequired + 1)return cbRequired;
			NAV_FIELD_PROP_2NUMBER1(langcod, 8, "");
			if (num_channels == 0){
				if (descriptor_length < cbRequired + 1)return cbRequired;
				NAV_FIELD_PROP_2NUMBER1(langcod2, 8, "")
			}

			if (descriptor_length < cbRequired + 1)return cbRequired;
			if (bsmod < 2){
				NAV_FIELD_PROP_2NUMBER1(mainid, 3, "")
				NAV_FIELD_PROP_2NUMBER1(priority, 2, ac3_priority_names[priority])
				NAV_FIELD_PROP_2NUMBER("reserved", 3, reserved_0, "")
			}else{
				NAV_FIELD_PROP_2NUMBER1(asvcflags, 8, "")
			}

			if (descriptor_length < cbRequired + 1)return cbRequired;
			NAV_FIELD_PROP_2NUMBER1(textlen, 7, "")
			NAV_FIELD_PROP_2NUMBER1(textcode, 1, textcode?"the text is encoded as 1-byte characters using the ISO Latin-1 alphabet (ISO 8859-1)":"the text is encoded with 2-byte Unicode characters")
			
			if (descriptor_length < cbRequired + 1)return cbRequired;
			NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("text", (long long)textlen*8, text, (unsigned long)textlen, "contain a brief textual description of the audio service");

			if (descriptor_length < cbRequired + 1)return cbRequired;
			NAV_FIELD_PROP_2NUMBER1(language_flag, 1, "")
			NAV_FIELD_PROP_2NUMBER1(language_flag_2, 1, "")
			NAV_FIELD_PROP_2NUMBER("reserved", 6, reserved_1, "")

			if (language_flag){
				if (descriptor_length < cbRequired + 1)return cbRequired;
				NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("language", 3*8, language, 3, "")
			}

			if (language_flag_2){
				if (descriptor_length < cbRequired + 1)return cbRequired;
				NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("language_2", 3*8, language_2, 3, "")
			}

			if (descriptor_length < cbRequired + 1)return cbRequired;
			NAV_FIELD_PROP_FIXSIZE_BINSTR("stuffing_bytes", (unsigned long)stuffing_bytes_count*8, stuffing_bytes, (unsigned long)stuffing_bytes_count, "")

		DECLARE_FIELDPROP_END()	

	}PACKED;

	struct CCaptionServiceDescriptor: public DIRECT_ENDIAN_MAP
	{
		struct CaptionService: public DIRECT_ENDIAN_MAP
		{
			unsigned char		language[3];
			union {
				struct {
					unsigned char		digital_cc:1;
					unsigned char		reserved_0:1;
					unsigned char		reserved_1:5;
					unsigned char		line21_field:1;
				}PACKED;
				struct {
					unsigned char		digital_cc_2:1;
					unsigned char		reserved_0_2:1;
					unsigned char		caption_service_number:6;
				}PACKED;
			}PACKED;
			unsigned char		easy_reader:1;
			unsigned char		wide_aspect_ratio:1;
			unsigned char		reserved_2:6;
			unsigned char		reserved_3;

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_FIXSIZE_BINCHARSTR1(language, 3*8, "The LANGUAGE of the service shall be encoded as a 3-character language code per ISO 639.2/B")
				NAV_FIELD_PROP_2NUMBER1(digital_cc, 1, digital_cc?"CEA-708":"CEA-608")
				NAV_FIELD_PROP_2NUMBER("reserved", 1, reserved_0, "")
				if (digital_cc == 0){
					NAV_FIELD_PROP_2NUMBER("reserved", 1, reserved_1, "")
					NAV_FIELD_PROP_2NUMBER1(line21_field, 1, "")
				}else{
					NAV_FIELD_PROP_2NUMBER1(caption_service_number, 6, "")
				}
				NAV_FIELD_PROP_2NUMBER1(easy_reader, 1, easy_reader?"the closed caption service is the EASY READER type":"no meaning")
				NAV_FIELD_PROP_2NUMBER1(wide_aspect_ratio, 1, wide_aspect_ratio?"16:9":"4:3")
				NAV_FIELD_PROP_2NUMBER("reserved", 6, reserved_2, "")
				NAV_FIELD_PROP_2NUMBER("reserved", 8, reserved_3, "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		
#ifdef _BIG_ENDIAN_
		unsigned char		reserved:3;
		unsigned char		number_of_services:5;
#else
		unsigned char		number_of_services:5;
		unsigned char		reserved:3;
#endif

		CaptionService		services[];

		int GetVarBodySize(){return descriptor_length - 1;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(reserved, 3, "")
			NAV_FIELD_PROP_2NUMBER1(number_of_services, 5, "")
			for(i=0;i<(int)number_of_services;i++){
				NAV_WRITE_TAG_BEGIN3("CaptionService", i);
					NAV_FIELD_PROP_OBJECT(services[i])
				NAV_WRITE_TAG_END3("CaptionService", i);
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CHDMVCopyControlDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

		unsigned short		CA_System_ID;
		unsigned char		private_data_byte[];

		int GetVarBodySize(){return descriptor_length - 2;}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(CA_System_ID)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(CA_System_ID, 16, "")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(private_data_byte, (unsigned long)(descriptor_length - 2)*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CPartialTransportStreamDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long long	DVB_reserved_future_use_0:2;
				unsigned long long	peak_rate:22;
				unsigned long long	DVB_reserved_future_use_1:2;
				unsigned long long	minimum_overall_smoothing_rate:22;
				unsigned long long	DVB_reserved_future_use_2:2;
				unsigned long long	maximum_overall_smoothing_buffer:14;
#else
				unsigned long long	maximum_overall_smoothing_buffer:14;
				unsigned long long	DVB_reserved_future_use_2:2;
				unsigned long long	minimum_overall_smoothing_rate:22;
				unsigned long long	DVB_reserved_future_use_1:2;
				unsigned long long	peak_rate:22;
				unsigned long long	DVB_reserved_future_use_0:2;
#endif
			}PACKED;
			unsigned long long ullValue;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			UINT64_FIELD_ENDIAN(ullValue)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(DVB_reserved_future_use_0, 2, "")
			MBCSPRINTF_S(szTemp2, _countof(szTemp2), "%f bps", (peak_rate*400.f)/1000.f);
			NAV_FIELD_PROP_2NUMBER1(peak_rate, 22, szTemp2)
			NAV_FIELD_PROP_2NUMBER1(DVB_reserved_future_use_1, 2, "")
			if (0x3FFFFF==minimum_overall_smoothing_rate)
				MBCSPRINTF_S(szTemp2, _countof(szTemp2), "Undefined");
			else
				MBCSPRINTF_S(szTemp2, _countof(szTemp2), "%f bps", (minimum_overall_smoothing_rate*400.f)/1000.f);
			NAV_FIELD_PROP_2NUMBER1(minimum_overall_smoothing_rate, 22, "")
			NAV_FIELD_PROP_2NUMBER1(DVB_reserved_future_use_2, 2, "")
			NAV_FIELD_PROP_2NUMBER1(maximum_overall_smoothing_buffer, 14, 0x3FFFFF==maximum_overall_smoothing_buffer?"Undefined":"bytes")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CHDMVGenericDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		private_data_byte[];

		int GetVarBodySize(){return descriptor_length;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, hdmv_descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(private_data_byte, (unsigned long)(descriptor_length)*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	typedef CHDMVGenericDescriptor CBDSystemUseDescriptor;
	typedef CHDMVGenericDescriptor CDRARegistrationDescriptor;
	
}	// namespace BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif


