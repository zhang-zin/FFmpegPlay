package com.zj26.ffmpegplay

import android.Manifest
import android.os.Bundle
import android.os.Environment
import android.view.View
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.content.PermissionChecker
import com.zj26.ffmpegplay.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var playerManage: PlayerManage
    private val path: String = File(Environment.getExternalStorageDirectory(), "test.mp4").absolutePath

    //ActivityResultContracts.RequestMultiplePermissions() 请求多项权限
    private val requestPermissionLauncher =
        registerForActivityResult(RequestPermission()) { isGranted: Boolean ->
            if (isGranted) {
                playerManage.open(path)
            }
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        playerManage = PlayerManage(binding.surfaceView)
        lifecycle.addObserver(playerManage)
    }

    fun open(view: View) {
        val checkSelfPermission =
            ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
        //shouldShowRequestPermissionRationale(Manifest.permission.READ_EXTERNAL_STORAGE)
        if (checkSelfPermission == PermissionChecker.PERMISSION_GRANTED) {
            playerManage.open(path)
        } else {
            requestPermissionLauncher.launch(Manifest.permission.READ_EXTERNAL_STORAGE)
        }
    }
}