package com.zj26.ffmpegplay

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import com.zj26.ffmpegplay.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var playerManage: PlayerManage

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        playerManage = PlayerManage(binding.surfaceView)
        lifecycle.addObserver(playerManage)
    }

    fun open(view: View) {
        playerManage.open("")
    }
}