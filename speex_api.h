
// #ifndef  SPEEX_H
#include <speex/speex.h>
#include <stdint.h>
struct wav_header
{
	char riff_id[4];			/*("RIFF"*/
	uint32_t size0;				/*file len - 8*/
	char wave_fmt[8];			/*"WAVEfmt "*/
	uint32_t size1;				/*0x10*/
	uint16_t fmttag;			/*0x01*/
	uint16_t channel;			/*1*/
	uint32_t samplespersec;			/*8000*/
	uint32_t bytepersec;			/*8000 * 2*/
	uint16_t blockalign;			/*1 * 16 / 8*/
	uint16_t bitpersamples;			/*16*/
	char data_id[4];			/*"data"*/
	uint32_t size2;				/*file len - 44*/
};
struct speex_str
{
      //encoder
      SpeexBits bits;
      void*  st;
      void*  speex;
      void*  pre;
      //int  mode;
      int  tmp;
      int  encoder_para;
      int  speex_headersz;
      //int  frame_size;
     int samplespersec;
      //decoder
       SpeexBits bitsDecode;
   
       void *stateDecode;
      int length;
       int samplerate;
       int sample_num;
       int frame_len;
       
      uint8_t spx_data[128];
      spx_int16_t  pcm_frame[320];
      uint8_t spx_frame[128];
};

//static int get_header_length( uint8_t *data, int header_len );
void speex_init( int flag, struct speex_str * entity );
void speex_feed( int flag, struct speex_str * entity );
void speex_deinit( int flag, struct speex_str * entity );

// #endif

