package com.zj26.ffmpegplay

import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner

class PlayerManage(private val surfaceView: SurfaceView) : SurfaceHolder.Callback2,
    DefaultLifecycleObserver {

    private external fun nativeOpen(path: String, surface: Surface): String
    private var surface: Surface? = null

    companion object {
        init {
            System.loadLibrary("ffmpegplay")
        }
    }

    init {
        surfaceView.holder.addCallback(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        surface = holder.surface
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
    }

    override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
    }

    override fun onDestroy(owner: LifecycleOwner) {
        surfaceView.holder.removeCallback(this)
    }

    fun open(path: String) {
        if (surface != null) {
            nativeOpen(path, surface!!)
        }
    }

}