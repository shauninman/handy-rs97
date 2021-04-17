#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include "handy_sdl_main.h"
#include "handy_sound.h"

#define SOUND_SAMPLES_SIZE	512
#define BUFFSIZE_MULT		8
#define MAX_SKIP_COUNT		4

static int32_t BUFFSIZE;
static uint8_t *buffer;
static uint32_t buf_read_pos = 0;
static uint32_t buf_write_pos = 0;
static int32_t buffered_bytes = 0;

static int32_t sdl_read_buffer(uint8_t* data, int32_t len)
{
	if (buffered_bytes >= len) 
	{
		if(buf_read_pos + len <= BUFFSIZE ) 
		{
			memcpy(data, buffer + buf_read_pos, len);
		} 
		else 
		{
			int32_t tail = BUFFSIZE - buf_read_pos;
			memcpy(data, buffer + buf_read_pos, tail);
			memcpy(data + tail, buffer, len - tail);
		}
		buf_read_pos = (buf_read_pos + len) % BUFFSIZE;
		buffered_bytes -= len;
	}

	return len;
}

static void sdl_write_buffer(uint8_t* data, int32_t len)
{
	SDL_LockAudio();
	for(uint32_t i = 0; i < len; i += 4) 
	{
		while (buffered_bytes == BUFFSIZE) {
			SDL_UnlockAudio();
			usleep(1000);
			SDL_LockAudio();
		}
		*(int32_t*)(buffer + buf_write_pos) = *(int32_t*)(data + i);
		//memcpy(buffer + buf_write_pos, data + i, 4);
		buf_write_pos = (buf_write_pos + 4) % BUFFSIZE;
		buffered_bytes += 4;
	}
	SDL_UnlockAudio();
}

void sdl_callback(void *unused, uint8_t *stream, int32_t len)
{
	sdl_read_buffer((uint8_t *)stream, len);
}

int handy_audio_init(void) {
    /* If we don't want sound, return 0 */
    if(gAudioEnabled == FALSE) return 0;
	
	SDL_AudioSpec aspec, obtained;

	BUFFSIZE = (SOUND_SAMPLES_SIZE * 2 * 2) * BUFFSIZE_MULT;
	buffer = (uint8_t *) malloc(BUFFSIZE);

	/* Add some silence to the buffer */
	buffered_bytes = 0;
	buf_read_pos = 0;
	buf_write_pos = 0;

	aspec.format   = AUDIO_S16SYS;
	aspec.freq     = HANDY_AUDIO_SAMPLE_FREQ;
	aspec.channels = 2;
	aspec.samples  = SOUND_SAMPLES_SIZE;
	aspec.callback = (sdl_callback);
	aspec.userdata = NULL;

	/* initialize the SDL Audio system */
	if (SDL_InitSubSystem (SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)) 
	{
		printf("SDL: Initializing of SDL Audio failed: %s.\n", SDL_GetError());
		return 0;
	}

	/* Open the audio device and start playing sound! */
	if(SDL_OpenAudio(&aspec, &obtained) < 0) 
	{
		printf("SDL: Unable to open audio: %s\n", SDL_GetError());
		return 0;
	}
	
	SDL_PauseAudio(0);
	
	return 1;
}
void handy_audio_loop() {
	mpLynx->Update();
	if (gAudioEnabled && gAudioBufferPointer >= HANDY_AUDIO_BUFFER_SIZE/2)
	{
		gAudioBufferPointer = 0;	
		sdl_write_buffer((uint8_t*)gAudioBuffer, HANDY_AUDIO_BUFFER_SIZE/2);
	}
}
void handy_audio_close() {
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	buffer = NULL;
}