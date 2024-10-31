#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>

#define PCM_DEVICE "default"
#define SAMPLE_RATE 44100
#define FREQUENCY 440.0
#define DURATION 5.0

int main() {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    int pcm;
    unsigned int rate = SAMPLE_RATE;
    int channels = 1;
    snd_pcm_uframes_t frames = 32;
    int dir;
    int size;
    short *buffer;
    int i;
    int num_samples = SAMPLE_RATE * DURATION;

    // Open PCM device for playback
    if ((pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror(pcm));
        return 1;
    }

    // Allocate parameters object and fill it with default values
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    // Set parameters
    if ((pcm = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));
        return 1;
    }

    if ((pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "ERROR: Can't set format. %s\n", snd_strerror(pcm));
        return 1;
    }

    if ((pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels)) < 0) {
        fprintf(stderr, "ERROR: Can't set channels number. %s\n", snd_strerror(pcm));
        return 1;
    }

    if ((pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0)) < 0) {
        fprintf(stderr, "ERROR: Can't set rate. %s\n", snd_strerror(pcm));
        return 1;
    }

    if ((pcm = snd_pcm_hw_params(pcm_handle, params)) < 0) {
        fprintf(stderr, "ERROR: Can't set hardware parameters. %s\n", snd_strerror(pcm));
        return 1;
    }

    // Allocate buffer to hold single period
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * channels * 2; // 2 bytes/sample, 1 channel
    buffer = (short *) malloc(size);

    // Generate sine wave
    for (i = 0; i < num_samples; i++) {
        buffer[i % frames] = 32767 * sin(2 * M_PI * FREQUENCY * i / SAMPLE_RATE);
        if ((i + 1) % frames == 0) {
            if ((pcm = snd_pcm_writei(pcm_handle, buffer, frames)) == -EPIPE) {
                snd_pcm_prepare(pcm_handle);
            } else if (pcm < 0) {
                fprintf(stderr, "ERROR: Can't write to PCM device. %s\n", snd_strerror(pcm));
            }
        }
    }

    // Clean up
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buffer);

    return 0;
}