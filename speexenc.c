#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <speex/speex.h>
#include "speex_api.h"
#include <fcntl.h> // for open
#include <unistd.h> // for close

// spx_int16_t pcm_frame[320];
// uint8_t spx_frame[128];

static int _get_framesize( int samplerate ){
	return (samplerate * 2)/ (1000/20); /* 20ms */
}

static int _write_header( uint8_t *frame, int pdu_len, int headersz ){
	switch (headersz) {
	case 1:
		*frame = pdu_len;
		break;

	case 2:
		*(frame + 0) = pdu_len & 0xff;
		*(frame + 1) = (pdu_len >> 8) & 0xff;
		break;

	case 4:
		*(frame + 0) = pdu_len & 0xff;
		*(frame + 1) = (pdu_len >> 8) & 0xff;
		*(frame + 2) = (pdu_len >> 16) & 0xff;
		*(frame + 3) = (pdu_len >> 24) & 0xff;
		break;
	}

	return 0;
}

int main(int argc, char** argv)
{

	//int speex_headersz;
	// create the speex_str entity

    struct speex_str entity;
	int in_fd, out_fd;
	int frame_size;
    struct wav_header header;
	entity.st = 0;
	//speex_headersz = 2;
	entity.speex_headersz = 2;

	if ((argc != 2) && (argc != 3)) {
		printf("usage: speexenc in_wav_file [out_speex_file]\n");
		return -1;
	}
	in_fd = open(argv[1], O_RDONLY, 0);
	if (in_fd < 0) {
		printf("open wav failed!\n");
		return -1;
	}
    
	if (argc == 3)
		 out_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	else
		 out_fd = open("dummy.spx", O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if (out_fd < 0) {
		printf("open speex file failed!\n");
		close(in_fd);
		return -1;
	}
  

	/* read riff header */
	read( in_fd, &header, sizeof(struct wav_header) );
	frame_size = _get_framesize( header.samplespersec );
    
	// initialization
	entity.samplespersec = header.samplespersec;
	//printf( "%d\n", header.samplespersec );

	speex_init( 1, &entity );

	// feed
	while (1) {
        /* hand one frame */
         entity.length = read( in_fd, entity.pcm_frame, frame_size );
        if ( entity.length != frame_size ) break;

	    speex_feed( 1, &entity );

		_write_header(entity.spx_frame, entity.length, entity.speex_headersz);
	    write(out_fd, entity.spx_frame, entity.length + entity.speex_headersz);
	}

	//deinit
    speex_deinit( 1, &entity );
	close( in_fd );
	close( out_fd );

   return 0;
}
