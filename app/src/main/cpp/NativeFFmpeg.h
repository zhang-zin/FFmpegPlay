//
// Created by 张锦 on 2022/8/3.
//

#ifndef FFMPEGPLAY_NATIVEFFMPEG_H
#define FFMPEGPLAY_NATIVEFFMPEG_H

#include "JavaCallHelper.h"
#include "macro.h"
#include "VideoChannel.h"
#include "AudioChannel.h"
#include <cstring>
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include "libavutil/error.h"
}

class NativeFFmpeg {
public:
    NativeFFmpeg(JavaCallHelper *javaCallHelper_, const char *dataSource);

    ~NativeFFmpeg();

    void prepare();

    void prepareFFmpeg();

    void start();

    void startFFmpeg();
    void setRenderCallback(RenderFrame renderFrame_);

private:
    char *url;
    bool isPlaying;
    JavaCallHelper *javaCallHelper;
    pthread_t pid_prepare;
    pthread_t pid_play;
    AVFormatContext *avFormatContext;
    AudioChannel *audioChannel;
    VideoChannel *videoChannel;
    RenderFrame renderFrame = nullptr;
};


#endif //FFMPEGPLAY_NATIVEFFMPEG_H
