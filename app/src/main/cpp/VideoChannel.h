//
// Created by 张锦 on 2022/8/3.
//

#ifndef FFMPEGPLAY_VIDEOCHANNEL_H
#define FFMPEGPLAY_VIDEOCHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include "BaseChannel.h"
#include "AudioChannel.h"

typedef void (*RenderFrame)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {

public:
    VideoChannel(int id, JavaCallHelper *pHelper, AVCodecContext *pContext, AVRational time_base);

    virtual void play();

    virtual void stop();

    void decodePacket();

    void synchronizeFrame();

    void setRenderCallback(RenderFrame renderFrame);

    void setFps(int fps_);

private:
    pthread_t pid_video_play = 0;
    pthread_t pid_synchronize = 0;
    RenderFrame renderFrame = nullptr;
    int fps = 0;

public:
    AudioChannel *audioChannel;
};


#endif //FFMPEGPLAY_VIDEOCHANNEL_H
