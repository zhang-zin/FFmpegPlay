package com.zj26.ffmpegplay

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner

class PlayerManager(private val surfaceView: SurfaceView) : SurfaceHolder.Callback2,
    DefaultLifecycleObserver {

    companion object {
        init {
            System.loadLibrary("ffmpegplay")
        }
    }

    private external fun native_prepare(dataSource: String)
    private external fun native_set_surface(surface: Surface)
    private external fun native_start()
    private external fun native_getDuration(): Int
    private external fun native_seek(progress: Int)
    private external fun native_stop()

    private var dataSource: String? = null
    var onPrepareListener: OnPrepareListener? = null
    var onErrorListener: OnErrorListener? = null
    var onProgressListener: OnProgressListener? = null

    init {
        surfaceView.holder.addCallback(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        native_set_surface(holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
    }

    override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
    }

    override fun onDestroy(owner: LifecycleOwner) {
        surfaceView.holder.removeCallback(this)
    }

    fun setDataSource(path: String) {
        dataSource = path
    }

    fun prepare() {
        if (!dataSource.isNullOrEmpty()) {
            native_prepare(dataSource!!)
        }
    }

    fun start() {
        native_start()
    }

    fun getDuration(): Int {
        return native_getDuration()
    }

    fun seek(progress: Int) {
        native_seek(progress)
    }

    fun stop() {
        native_stop()
    }

    fun onError(code: Int) {
        onErrorListener?.onError(code)
        Log.e("zj26", "onError code: $code")
    }

    fun onPrepare() {
        onPrepareListener?.onPrepare()
    }

    fun onProgress(progress: Int) {
        onProgressListener?.onProgress(progress)
    }

    interface OnPrepareListener {
        fun onPrepare()
    }

    interface OnErrorListener {
        fun onError(code: Int)
    }

    interface OnProgressListener {
        fun onProgress(progress: Int)
    }

}