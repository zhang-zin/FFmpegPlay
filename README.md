# FFmpeg播放视频
## 1.准备FFmpeg
1. 创建全局上下文 avformat_network_init(); avFormatContext = avformat_alloc_context();
2. 打开播放地址 avformat_open_input(), 打开流avformat_find_stream_info()
3. for循环查找流
4. 创建解码器上下流
5. 复制参数
6. 打开解码器

第一步准备完毕，回调java方法开始播放视频

从流中读取原始包
```
while (isPlaying){
    AVPacket *packet = av_packet_alloc();
    ret = av_read_frame(avFormatContext, packet);
    if (ret == 0) {
        // 读取成功，将数据包加入队列

    }
}
```
## 2.播放视频
```
void VideoChannel::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    pthread_create(&pid_video_play, nullptr, decode, this); // 初始化视频解码线程
    pthread_create(&pid_synchronize, nullptr, synchronize, this); //初始化视频转换、同步播放线程
}
```
数据包解码
```
void VideoChannel::decodePacket() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = pkt_queue.deQueue(packet);
        ret = avcodec_send_packet(avCodecContext, packet);  // 发送原始包
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            // 失败
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame); // 解码
        if (ret == 0) {
            // 解码成功加入解码包队列
            frame_queue.enQueue(frame);
        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }
    }
}
```
转化给ANativeWindow所需要的数据
```
void VideoChannel::synchronizeFrame() {
    // 初始化转换器上下文
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,  // 输入视频宽高
                                            avCodecContext->pix_fmt,                        // 输入视频格式
                                            avCodecContext->width, avCodecContext->height,  // 输出视频宽高
                                            AV_PIX_FMT_RGBA,                                // 输出格式
                                            SWS_BILINEAR,
                                            nullptr, nullptr, nullptr);
    uint8_t *dst_data[4];
    int dst_lineSize[4];
    // 分配输出数据大小
    av_image_alloc(dst_data, dst_lineSize, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, dst_data,
                  dst_lineSize);
        // 回调出去给ANativeWindow
        renderFrame(dst_data[0], dst_lineSize[0], avCodecContext->width, avCodecContext->height);
    }
}
```
渲染视频

windown在surfaceView创建成功，调用native方法创建ANativeWindow
```
if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
window = ANativeWindow_fromSurface(env, surface);
```
```
void renderFrame(uint8_t *data, int lineSize, int w, int h) {
    // 渲染
    // 设置窗口属性
    ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    if (ANativeWindow_lock(window, &windowBuffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;
        return;
    }
    // 缓冲区  window_data[0] =255;
    auto *window_data = static_cast<uint8_t *>(windowBuffer.bits);
    int window_line_size = windowBuffer.stride * 4;
    uint8_t *src_data = data;
    for (int i = 0; i < windowBuffer.height; ++i) {
        memcpy(window_data + i * window_line_size, src_data + i * lineSize, window_line_size);
    }
    ANativeWindow_unlockAndPost(window);
}
```
## 3.播放音频
1. 创建初始化OpenSlES线程
```
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
```
2. 音频解码线程
## 4.音视频同步
音视频同步以音频为准，音频正常播放，视频跟随音频速度
```
void VideoChannel::synchronizeFrame() {
    while (isPlaying) {
        ```
        clock = frame->pts * av_q2d(time_base);   // 当前视频播放时间
        double audioClock = audioChannel->clock;  // 音频播放时间
        double diff = clock - audioClock;          
        double frame_delays = 1.0 / fps; // 播放延迟
        // 算入解码时间
        double extra_delay = frame->repeat_pict / (2 * fps);
        double delay = extra_delay + frame_delays;
        if (diff > 0) {
            // 视频超前
            if (diff > 1) {
                // 超太多了
                av_usleep(delay * 2 * 1000000);
            } else {
                av_usleep((delay + diff) * 1000000);
            }
        } else {
            // 音频超前
            if (abs(diff) > 0.05) {
                // 丢帧
                releaseAVFrame(frame);
                frame_queue.sync();
            }
        }
    }
}
```