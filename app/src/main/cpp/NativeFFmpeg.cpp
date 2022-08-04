//
// Created by 张锦 on 2022/8/3.
//

#include "NativeFFmpeg.h"

void *startFFmpeg_(void *args) {
    auto *nativeFFmpeg = static_cast<NativeFFmpeg *>(args);
    nativeFFmpeg->startFFmpeg();
    return nullptr;
}

void *prepareFFmpeg_(void *args) {
    auto *nativeFFmpeg = static_cast<NativeFFmpeg *>(args);
    nativeFFmpeg->prepareFFmpeg();
    return nullptr;
}

NativeFFmpeg::NativeFFmpeg(JavaCallHelper *javaCallHelper_, const char *dataSource)
        : javaCallHelper(javaCallHelper_) {
    url = new char[strlen(dataSource) + 1];
    strcpy(url, dataSource);
}

NativeFFmpeg::~NativeFFmpeg() {

}

void NativeFFmpeg::prepare() {
    pthread_create(&pid_prepare, nullptr, prepareFFmpeg_, this);
}

void NativeFFmpeg::prepareFFmpeg() {
    if (!javaCallHelper) {
        return;
    }
    avformat_network_init();
    avFormatContext = avformat_alloc_context();
    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "timeout", "3000000", 0); // 设置超时时间
    int ret = avformat_open_input(&avFormatContext, url, nullptr, &opts);
    if (ret != 0) {
        char *c = new char[100];
        av_strerror(ret, c, 100);
        LOGE("error: %s", c);
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }
    // 查找流
    ret = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret < 0) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
    }
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVCodecParameters *codecParameters = avFormatContext->streams[i]->codecpar;
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        // 创建解码器上下文
        AVCodecContext *avCodecContext = avcodec_alloc_context3(codec);
        if (!avCodecContext) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        // 复制参数
        ret = avcodec_parameters_to_context(avCodecContext, codecParameters);
        if (ret < 0) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        // 打开解码器
        ret = avcodec_open2(avCodecContext, codec, nullptr);
        if (ret != 0) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频流
            audioChannel = new AudioChannel(i, javaCallHelper, avCodecContext);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频流
            videoChannel = new VideoChannel(i, javaCallHelper, avCodecContext);
            videoChannel->setRenderCallback(renderFrame);
        }
    }
    //音视频都没有
    if (!audioChannel && !videoChannel) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }
    javaCallHelper->onPrepare(THREAD_CHILD);
}

void NativeFFmpeg::start() {
    isPlaying = true;
    if (videoChannel) {
        videoChannel->play();
    }
    if (audioChannel) {
        audioChannel->play();
    }
    pthread_create(&pid_play, nullptr, startFFmpeg_, this);
}

void NativeFFmpeg::startFFmpeg() {
    int ret;
    while (isPlaying) {
        if (audioChannel && audioChannel->pkt_queue.size() > 100) {
            // 生产者的生产速度大于消费者的速度 休眠10ms
            av_usleep(1000 * 10);
            continue;
        }
        if (videoChannel && videoChannel->pkt_queue.size() > 100) {
            LOGE("videoChannel->pkt_queue.size(): %d", videoChannel->pkt_queue.size());
            av_usleep(1000 * 10);
            continue;
        }
        //读取包
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(avFormatContext, packet);
        LOGE("将数据包加入队列 ret: %d", ret);
        if (ret == 0) {
            //将数据包加入队列
            if (audioChannel && packet->stream_index == audioChannel->channelId) {
                audioChannel->pkt_queue.enQueue(packet);
            } else if (videoChannel && packet->stream_index == videoChannel->channelId) {
                videoChannel->pkt_queue.enQueue(packet);
            }
        } else if (ret == AVERROR_EOF) {
            //读取完毕 但是不一定播放完毕
            if (videoChannel->pkt_queue.empty() && videoChannel->frame_queue.empty() &&
                audioChannel->pkt_queue.empty() && audioChannel->frame_queue.empty()) {
                LOGE("播放完毕。。。");
                break;
            }
            //因为seek 的存在，就算读取完毕，依然要循环 去执行av_read_frame(否则seek了没用...)

        } else {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_READ_FRAME_FILE);
                break;
            }
        }
    }
    isPlaying = false;
    audioChannel->stop();
    videoChannel->stop();
}

void NativeFFmpeg::setRenderCallback(RenderFrame renderFrame_) {
    this->renderFrame = renderFrame_;
}
