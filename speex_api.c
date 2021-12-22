#include <stdio.h>
#include <speex/speex.h>
#include "speex_api.h"
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<string.h>

/*flag 1: encode 0:decode*/
void speex_init( int flag, struct speex_str* entity ){

	int mode;
	//encoder
	if ( flag == 1 ){
                printf( "%d\n", entity->samplespersec );

                if (entity->samplespersec > 12500) mode = SPEEX_MODEID_WB;
                else mode = SPEEX_MODEID_NB;

                entity->st = speex_encoder_init(speex_lib_get_mode(mode));
                {
                    spx_int32_t tmp;

                    tmp = 1;
                    speex_encoder_ctl (entity->st, SPEEX_SET_COMPLEXITY,  &tmp );
                    speex_encoder_ctl( entity->st, SPEEX_SET_DTX,  &tmp );
                    speex_encoder_ctl( entity->st, SPEEX_SET_VBR,  &tmp );

                    tmp = 8;
                    speex_encoder_ctl( entity->st, SPEEX_SET_QUALITY, &tmp );
                    tmp = entity->samplespersec;
                    speex_encoder_ctl( entity->st, SPEEX_SET_SAMPLING_RATE, &tmp );
                }
                /* Speex encoding initializations */
                speex_bits_init( &(entity->bits) );

	}
	// decoder
	else if ( flag == 0 )
	{
                if ( entity->samplerate > 12500) mode = SPEEX_MODEID_WB;
                else mode = SPEEX_MODEID_NB;
                entity->stateDecode = speex_decoder_init( speex_lib_get_mode(mode) );
                speex_bits_init( &(entity->bitsDecode) );
	}

};

void speex_feed( int flag, struct speex_str * entity, spx_int16_t**pcm_frame, uint8_t** spx_frame ){
        //encoder
        if( flag == 1)
        {
            speex_bits_reset( &(entity->bits) );
            speex_encode_int( entity->st, entity->pcm_frame, &(entity->bits) );
            entity->length = speex_bits_write( &(entity->bits),
                    &(entity->spx_frame[ entity->speex_headersz]),
                    sizeof(entity->spx_frame) - entity->speex_headersz );

            (*pcm_frame) = entity->pcm_frame;
            (*spx_frame ) = entity->spx_frame;
        }
    //decoder
        else if ( flag == 0 )
        {
                speex_bits_reset( &(entity->bitsDecode) );
                speex_bits_read_from( &(entity->bitsDecode), entity->spx_data, entity->frame_len );
                
                int ret = speex_decode_int( entity->stateDecode, &(entity->bitsDecode), 
                (spx_int16_t*) entity->pcm_frame );

                (*pcm_frame) = entity->pcm_frame;
                (*spx_frame ) = entity->spx_frame;
        }
};

void speex_deinit( int flag, struct speex_str * entity ){
//encoder
   if ( flag == 1 )
   {
       speex_encoder_destroy( entity->st );
	   speex_bits_destroy( &(entity->bits) );
   }
   //decoder
   else if ( flag == 0 )
   {
	    speex_bits_destroy( &(entity->bitsDecode) );
	    speex_decoder_destroy( entity->stateDecode );
   }
};
