#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include <speex/speex.h>
#include <speex/speex_preprocess.h>

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

static void init_wav_header(char *wav_buf, uint32_t wav_len, int samplerate)
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

int get_header_length(uint8_t *data, int header_len)
{
	int length = 0;

	if (header_len == 1) length = *data;
	if (header_len == 2) {
		length = *data | *(data + 1) << 8;
	}
	if (header_len == 4) {
		length = *data | *(data + 1) << 8 | *(data + 2) << 16 | *(data + 3) << 24;
	}

	return length;
}

int16_t frame_buffer[16000 / (1000 / 20)];     /* maximal 16000 sample rate */
char data[320];
spx_int16_t pcm_frame[320];

void main(int argc, char *argv[])
{
	int header_len = 2;
	int samplerate = 16000;
	int length;
	int out_fd;
	int pcm_length = 0;
	int sample_num = samplerate / (1000 / 20);
	int frame_index = 0;

	int in_fd = open(argv[1], O_RDONLY, 0);
	if (in_fd < 0) {
		printf("open %s error\n", argv[1]);
		return ;
	} else {
		out_fd = open("dummy.wav", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (out_fd < 0) {
			printf("open dummy.wav error\n");
			return ;
		}
		int frame_len;
		void *stateDecode;
		SpeexBits bitsDecode;
		struct wav_header header;
		int mode;

		if (samplerate > 12500) mode = SPEEX_MODEID_WB;
		else mode = SPEEX_MODEID_NB;

		printf("speex decode mode: %s\n", mode == SPEEX_MODEID_WB ? "WB" : "NB");
		printf("speex sample rate: %d\n", samplerate);

		stateDecode = speex_decoder_init(speex_lib_get_mode(mode));
		speex_bits_init(&bitsDecode);

		/* write header firstly */
		init_wav_header(&header, 0, 0);
		write(out_fd, &header, sizeof(struct wav_header));

		while ((read(in_fd, data, 320) > 0 )) {
			frame_len = get_header_length(data, header_len);
			printf("%d frame, length => %d\n", frame_index++, frame_len);

			speex_bits_reset(&bitsDecode);
			speex_bits_read_from(&bitsDecode, data + header_len, frame_len);

			int ret = speex_decode_int(stateDecode, &bitsDecode, (spx_int16_t*)pcm_frame);

			write(out_fd, pcm_frame, sample_num * sizeof(uint16_t));
			pcm_length += sample_num * sizeof(uint16_t);
			//data += (header_len + frame_len);
			//length -= (header_len + frame_len);
			memset(data, 0, 320);
		}
		printf("read in file over\n");

		speex_bits_destroy(&bitsDecode);
		speex_decoder_destroy(stateDecode);

		/* rewrite wav header */
		lseek(out_fd, 0, SEEK_SET);
		init_wav_header(&header, pcm_length, samplerate);
		write(out_fd, &header, sizeof(struct wav_header));
		close(out_fd);
		close(in_fd);
	}
}