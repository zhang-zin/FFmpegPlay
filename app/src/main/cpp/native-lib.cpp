#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>

extern "C" {
#include "libavutil/avutil.h"
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#define LOGD(...) __android_log_print(ANDROID_LOG_INFO,"zj26",__VA_ARGS__)

AVFormatContext *avFormatContext;

extern "C"
JNIEXPORT jint JNICALL
Java_com_zj26_ffmpegplay_PlayerManage_nativeOpen(JNIEnv *env, jobject thiz, jstring path,
                                                 jobject surface) {
    const char *url = env->GetStringUTFChars(path, 0);
    LOGD("播发地址: %s", url);
    //注册所有的组件
    //avcodec_register_all();
    avformat_network_init();
    //1. 获取全局上下文
    avFormatContext = avformat_alloc_context();
    AVDictionary *dictionary = NULL;
    //设置超时时间3s
    av_dict_set(&dictionary, "timeout", "3000000", 0);
    //打开视频
    if (avformat_open_input(&avFormatContext, url, NULL, &dictionary) != 0) {
        LOGD("打开视频失败\n");
        return -1;
    }

    int video_stream_index = 0;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        // 找到视频流索引
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    //流相关的编解码器参数
    AVCodecParameters *codecpar = avFormatContext->streams[video_stream_index]->codecpar;
    //找到解码器
    AVCodec *avCodec = avcodec_find_decoder(codecpar->codec_id);
    //创建解码器上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    avcodec_parameters_to_context(avCodecContext, codecpar);
    if (avcodec_open2(avCodecContext, avCodec, nullptr) != 0) {
        LOGD("打开解码器失败");
        return -1;
    }
    //视频帧渲染到屏幕
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(nativeWindow, avCodecContext->width, avCodecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);
    //释放
    ANativeWindow_release(nativeWindow);
    avcodec_close(avCodecContext);
    avformat_free_context(avFormatContext);
    env->ReleaseStringUTFChars(path, url);

    // 获取转换上下文 yuv -> rgba
    SwsContext *sws_context = sws_getContext(avCodecContext->width, avCodecContext->height, //输入宽高
                                             avCodecContext->pix_fmt,                       //输入格式
                                             avCodecContext->width, avCodecContext->height, //输出宽高
                                             AV_PIX_FMT_RGBA,                      //输出格式
                                             SWS_BILINEAR,                                  //指定用于缩放的算法和选项
                                             0, 0, 0);
    //读取包
    AVPacket *packet = av_packet_alloc();
    //循环取帧
    while (av_read_frame(avFormatContext, packet) >= 0) {
        avcodec_send_packet(avCodecContext, packet);
        AVFrame *avFrame = av_frame_alloc();
        int receiveResult = avcodec_receive_frame(avCodecContext, avFrame);
        if (receiveResult == AVERROR(EAGAIN)) {
            continue;
        } else if (receiveResult < 0) {
            LOGD("流读取结束");
            break;
        }
        uint8_t *dst_data[0];
        int dst_linesize[0];
        av_image_alloc(dst_data, dst_linesize, avCodecContext->width, avCodecContext->height,
                       AV_PIX_FMT_RGBA, 1);

    }
}