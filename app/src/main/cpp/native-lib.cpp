#include <jni.h>
#include <string>

extern "C" {
#include "libavutil/avutil.h"
#include <libavcodec/avcodec.h>
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_zj26_ffmpegplay_PlayerManage_nativeOpen(JNIEnv *env, jobject thiz, jstring path,
                                                 jobject surface) {
    const jchar *url = env->GetStringChars(path, 0);
    avcodec_register_all();
}