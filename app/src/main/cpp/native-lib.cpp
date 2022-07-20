#include <jni.h>
#include <string>

extern "C" {
#include "libavutil/avutil.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_zj26_ffmpegplay_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    av_version_info();
    std::string hello = avutil_configuration();
    return env->NewStringUTF(hello.c_str());
}