//
// Created by 张锦 on 2022/8/3.
//

#include "VideoChannel.h"

void *synchronize(void *args) {
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->synchronizeFrame();
    return nullptr;
}

void *decode(void *args) {
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decodePacket();
    return nullptr;
}

VideoChannel::VideoChannel(int id, JavaCallHelper *pHelper, AVCodecContext *pContext) :
        BaseChannel(id, pHelper, pContext) {

}

void VideoChannel::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    pthread_create(&pid_video_play, nullptr, decode, this);
    pthread_create(&pid_synchronize, nullptr, synchronize, this);
}

void VideoChannel::stop() {

}

void VideoChannel::decodePacket() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = pkt_queue.deQueue(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            return;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAVPacket(packet);
        LOGE("avcodec_send_packet ret: %d", ret);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            // 失败
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == 0) {
            frame_queue.enQueue(frame);
        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }
        if (frame_queue.size() > 100 && isPlaying) {
            av_usleep(1000 * 10);
            continue;
        }
    }
    releaseAVPacket(packet);
}

void VideoChannel::synchronizeFrame() {
    // 初始化转换器上下文
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt, avCodecContext->width,
                                            avCodecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR,
                                            nullptr, nullptr, nullptr);
    uint8_t *dst_data[4];
    int dst_lineSize[4];
    av_image_alloc(dst_data, dst_lineSize, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    AVFrame *frame = nullptr;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, dst_data,
                  dst_lineSize);
        renderFrame(dst_data[0], dst_lineSize[0], avCodecContext->width, avCodecContext->height);
        av_usleep(16 * 1000000);
        releaseAVFrame(frame);
    }
    av_free(&dst_data[0]);
    isPlaying = false;
    releaseAVFrame(frame);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderFrame renderFrame_) {
    this->renderFrame = renderFrame_;
}
