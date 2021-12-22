#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // for close
#include <speex/speex.h>
#include"speex_api.h"

//spx_int16_t pcm_frame[320];



static void init_wav_header( char *wav_buf, uint32_t wav_len, int samplerate )
{
	struct wav_header *header = (struct wav_header *)wav_buf;

	strcpy(header->riff_id, "RIFF");
	header->size0 = wav_len - 8;
	strcpy(header->wave_fmt, "WAVEfmt ");
	header->size1 = 16;
	header->fmttag = 1;
	header->channel = 1;
	header->samplespersec = samplerate;
	header->bytepersec = samplerate * 2;
	header->blockalign = 2;
	header->bitpersamples = 16;
	strcpy(header->data_id, "data");
	header->size2 = wav_len - 44;
}

static int get_header_length( uint8_t *data, int header_len ){
	int length = 0;

	if (header_len == 1) length = *data;
	if (header_len == 2) {
		length = *data | *(data + 1) << 8;
	}
	if (header_len == 4) {
		length = *data | *(data + 1) << 8 | *(data + 2)
		<< 16 | *(data + 3) << 24;
	}

	return length;
}

int main(int argc, char *argv[])
{
	//  create a structure

	// uint8_t spx_data[128] = {0};
    struct speex_str entity = {.spx_data = {0}};
    struct wav_header header;
	int header_len = 2;
	//int frame_len;
	//int samplerate = 16000;
	entity.samplerate = 16000;
	int length;
	int in_fd;
	int out_fd;
	int pcm_length = 0;
	int sample_num = entity.samplerate / (1000 / 20);

    
	if ( (argc > 4) || (argc < 2) ) {
		printf("usage: speexdec in_speex_file [out_wav_file]");
		return -1;
	}

   in_fd = open( argv[1], O_RDONLY,  0666 );
	if ( in_fd < 0 ) {
		printf( "open %s error\n", argv[1] );
		return -1;
	}

	if ( argc == 3 ) out_fd = open( argv[2],
					O_WRONLY | O_CREAT | O_TRUNC, 0666 );
	else out_fd = open( "dummy.wav", O_WRONLY | O_CREAT | O_TRUNC, 0666 );

	if ( out_fd < 0 ) {
		printf( "open wav file error\n" );
		return -1;
	}
   
   // initialization
   {
		speex_init( 0, &entity );
		init_wav_header( (char *)&header, 0, 0 );
		write( out_fd, &header, sizeof(struct wav_header) );
   }

    while (1) {
            /* read header */
            length = read( in_fd, entity.spx_data, header_len );
            if ( length != header_len ) break;
            entity.frame_len = get_header_length( entity.spx_data, header_len );
            length = read( in_fd, entity.spx_data, entity.frame_len );
            if( length != entity.frame_len ) break;
            
			// feed
			speex_feed( 0, &entity );

			write(out_fd, entity.pcm_frame, sample_num * sizeof(uint16_t));
            pcm_length += sample_num * sizeof(uint16_t);
		}

   close( in_fd );
   speex_deinit( 0, &entity );

   /* rewrite wav header*/
   lseek( out_fd, 0, SEEK_SET );
   init_wav_header( (char *)&header, pcm_length, entity.samplerate );
   write( out_fd, &header, sizeof( struct wav_header ) );
   close( out_fd );
}
