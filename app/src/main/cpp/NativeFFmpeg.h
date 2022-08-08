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

    int getDuration();

    void seek(jint i);

    void stop();

    void stopFFmpeg();

private:
    char *url;
    bool isPlaying = false;
    JavaCallHelper *javaCallHelper;
    AVFormatContext *avFormatContext;
    RenderFrame renderFrame = nullptr;
    int duration;
    AudioChannel *audioChannel;
    VideoChannel *videoChannel;

public:
    pthread_t pid_prepare;
    pthread_t pid_play;
    pthread_t pid_stop;

};


#endif //FFMPEGPLAY_NATIVEFFMPEG_H
