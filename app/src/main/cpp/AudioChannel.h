//
// Created by 张锦 on 2022/8/3.
//

#ifndef FFMPEGPLAY_AUDIOCHANNEL_H
#define FFMPEGPLAY_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/time.h>
}

class AudioChannel : public BaseChannel {

public:
    AudioChannel(int id, JavaCallHelper *pHelper, AVCodecContext *pContext, AVRational time_base);

    ~AudioChannel();

    virtual void play();

    virtual void stop();

private:
    SwrContext *swrContext = nullptr;
    pthread_t pid_audio_play = 0;
    pthread_t pid_audio_decode = 0;
    int out_channels;
    int out_sample_size;
    int out_sample_rate;

public:
    uint8_t *buffer;

    void initOpenSL();

    void decode();

    int getPcm();
};


#endif //FFMPEGPLAY_AUDIOCHANNEL_H
