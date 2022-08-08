//
// Created by 张锦 on 2022/8/3.
//

#include "AudioChannel.h"

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    auto *audioChannel = static_cast<AudioChannel *>(context);
    int dataLen = audioChannel->getPcm();
    if (dataLen > 0) {
        (*bq)->Enqueue(bq, audioChannel->buffer, dataLen);
    }
}

void *audioDecode(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();
    return nullptr;
}

void *audioPlay(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->initOpenSL();
    return nullptr;
}

AudioChannel::AudioChannel(int id, JavaCallHelper *pHelper, AVCodecContext *pContext,
                           AVRational time_base) :
        BaseChannel(id, pHelper, pContext, time_base) {
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    //CD音频标准
    //44100 双声道 2字节    out_sample_size  16位  2个字节   out_channels  2
    buffer = (uint8_t *) malloc(out_sample_rate * out_sample_size * out_channels);
}

AudioChannel::~AudioChannel() {
    free(buffer);
    buffer = nullptr;
}

void AudioChannel::play() {
    swrContext = swr_alloc_set_opts(nullptr,
                                    AV_CH_LAYOUT_STEREO,
                                    AV_SAMPLE_FMT_S16,
                                    out_sample_rate,
                                    avCodecContext->channel_layout,
                                    avCodecContext->sample_fmt,
                                    avCodecContext->sample_rate,
                                    0, nullptr);
    swr_init(swrContext);
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    // 创建初始化OpenSl ES线程
    pthread_create(&pid_audio_play, nullptr, audioPlay, this);
    // 创建初始化音频解码线程
    pthread_create(&pid_audio_decode, nullptr, audioDecode, this);
}

void AudioChannel::stop() {
    isPlaying = false;
}

void AudioChannel::initOpenSL() {
    // 音频引擎
    SLEngineItf engineInterface = nullptr;
    // 音频对象
    SLObjectItf engineObject = nullptr;
    // 混音器
    SLObjectItf outputMixObject = nullptr;
    // 播放器
    SLObjectItf bqPlayerObject = nullptr;
    // 回调接口
    SLPlayItf bqPlayerInterface = nullptr;
    // 缓冲队列
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = nullptr;

    // 1.初始化播放引擎
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 音频接口 相当于surfaceHolder
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 2.
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, nullptr,
                                                 nullptr);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, // 播放pcm格式的数据
                            2,     // 2个声道
                            SL_SAMPLINGRATE_44_1, //采样率 44100hz
                            SL_PCMSAMPLEFORMAT_FIXED_16, // 位数
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, //立体声（前左前右）
                            SL_BYTEORDER_LITTLEENDIAN // 小端模式
    };
    SLDataSource slDataSource = {&android_queue, &pcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&outputMix, nullptr};
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject,
                                          &slDataSource, // 播放器器参数 播放器缓冲队列 播放格式
                                          &audioSink,    // 播放器缓冲区
                                          1,             // 播放接口回调个数
                                          ids,           // 设置播放队列ID
                                          req            // 是否采用内置的播放队列
    );
    // 初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

    // 设置接口
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    // 设置播放状态
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

void AudioChannel::decode() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = pkt_queue.deQueue(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAVPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            break;
        }
        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(10 * 1000);
            //continue;
        }
        frame_queue.enQueue(frame);
    }
}

int AudioChannel::getPcm() {
    AVFrame *frame = nullptr;
    int data_size = 0;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (!ret) continue;
        if (!isPlaying) break;
        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swrContext, frame->sample_rate) + frame->nb_samples,
                out_sample_rate,
                frame->sample_rate,
                AV_ROUND_UP);
        // 转换
        int nb = swr_convert(swrContext, &buffer, dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);
        //转换后多少数据  buffer size  44110*2*2
        data_size = nb * out_channels * out_sample_size;
        clock = frame->pts * av_q2d(time_base);
        break;
    }
    if (javaCallHelper) {
        javaCallHelper->onProgress(THREAD_CHILD, clock);
    }
    releaseAVFrame(frame);
    return data_size;
}
