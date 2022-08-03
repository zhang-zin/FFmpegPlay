//
// Created by 张锦 on 2022/8/3.
//

#include "JavaCallHelper.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVm, JNIEnv *_jniEnv, jobject &jobject_) : javaVm(
        _javaVm), env(_jniEnv) {
    jobj = env->NewGlobalRef(jobject_);
    jclass jclass = env->GetObjectClass(jobj);
    jmid_error = env->GetMethodID(jclass, "onError", "(I)V");
    jmid_prepare = env->GetMethodID(jclass, "onPrepare", "()V");
    jmid_progress = env->GetMethodID(jclass, "onProgress", "(I)V");
}

JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(jobj);
}

void JavaCallHelper::onError(int thread, int code) {
    if (thread == THREAD_MAIN) {
        env->CallVoidMethod(jobj, jmid_error, code);
    } else {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, nullptr) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_error, code);
        javaVm->DetachCurrentThread();
    }
}

void JavaCallHelper::onPrepare(int thread) {
    if (thread == THREAD_MAIN) {
        env->CallVoidMethod(jobj, jmid_prepare);
    } else {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, nullptr) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepare);
        javaVm->DetachCurrentThread();
    }

}

void JavaCallHelper::onProgress(int thread, int progress) {
    if (thread == THREAD_MAIN) {
        env->CallVoidMethod(jobj, jmid_progress, progress);
    } else {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, nullptr) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_progress, progress);
        javaVm->DetachCurrentThread();
    }

}
