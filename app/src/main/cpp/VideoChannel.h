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

typedef void (*RenderFrame)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {

public:
    VideoChannel(int id, JavaCallHelper *pHelper, AVCodecContext *pContext);

    virtual void play();

    virtual void stop();

    void decodePacket();

    void synchronizeFrame();

    void setRenderCallback(RenderFrame renderFrame);

private:
    pthread_t pid_video_play = 0;
    pthread_t pid_synchronize = 0;
    RenderFrame renderFrame = nullptr;
};


#endif //FFMPEGPLAY_VIDEOCHANNEL_H
