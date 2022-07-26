//
// Created by 张锦 on 2022/8/3.
//

#ifndef FFMPEGPLAY_BASECHANNEL_H
#define FFMPEGPLAY_BASECHANNEL_H

#include "JavaCallHelper.h"
#include "safe_queue.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

class BaseChannel {
public:
    BaseChannel(int id, JavaCallHelper *pHelper, AVCodecContext *pContext, AVRational time_base) :
            channelId(id), javaCallHelper(pHelper), avCodecContext(pContext), time_base(time_base) {
        pkt_queue = SafeQueue<AVPacket *>();
        frame_queue = SafeQueue<AVFrame *>();
        pkt_queue.setReleaseHandle(releaseAVPacket);
        frame_queue.setReleaseHandle(releaseAVFrame);
    };

    ~BaseChannel() {
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        pkt_queue.clear();
        frame_queue.clear();
    }

    void clear(){
        pkt_queue.clear();
        frame_queue.clear();
    }

    void startWork(){
        pkt_queue.setWork(1);
        frame_queue.setWork(1);
    }

    void stopWork(){
        pkt_queue.setWork(0);
        frame_queue.setWork(0);
    }

    virtual void play() = 0;

    virtual void stop() = 0;

    static void releaseAVPacket(AVPacket *&packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = 0;
        }
    }

    static void releaseAVFrame(AVFrame *&frame) {
        if (frame) {
            av_frame_free(&frame);
            frame = 0;
        }
    }

    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext;
    JavaCallHelper *javaCallHelper;
    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;

    AVRational time_base; // 时间基
    double clock = 0;

};

#endif //FFMPEGPLAY_BASECHANNEL_H
