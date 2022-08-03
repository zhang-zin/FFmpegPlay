//
// Created by 张锦 on 2022/8/3.
//

#ifndef FFMPEGPLAY_JAVACALLHELPER_H
#define FFMPEGPLAY_JAVACALLHELPER_H

#include <jni.h>
#include "macro.h"

class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *_javaVm, JNIEnv *_jniEnv, jobject &jobject_);

    ~JavaCallHelper();

    void onError(int thread, int code);

    void onPrepare(int thread);

    void onProgress(int thread, int progress);

private:
    JavaVM *javaVm;
    JNIEnv *env;
    jobject jobj;
    jmethodID jmid_prepare;
    jmethodID jmid_error;
    jmethodID jmid_progress;
};


#endif //FFMPEGPLAY_JAVACALLHELPER_H
