package com.zj26.ffmpegplay

import android.Manifest
import android.os.Bundle
import android.os.Environment
import android.view.View
import android.view.WindowManager
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.content.PermissionChecker
import com.zj26.ffmpegplay.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity(), PlayerManager.OnErrorListener,
    PlayerManager.OnPrepareListener, PlayerManager.OnProgressListener {

    private lateinit var binding: ActivityMainBinding
    private lateinit var playerManager: PlayerManager
    private val path: String =
        File(Environment.getExternalStorageDirectory(), "input.mp4").apply {
            setExecutable(true)
        }.absolutePath

    private val requestPermissionLauncher =
        registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) {
            val permission = it.filter { map ->
                map.value == false
            }
            if (permission.isEmpty()) {
                playerManager.setDataSource(path)
                playerManager.prepare()
            }
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        window.setFlags(
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
        )
        setContentView(binding.root)
        playerManager = PlayerManager(binding.surfaceView)
        playerManager.apply {
            onPrepareListener = this@MainActivity
            onErrorListener = this@MainActivity
            onProgressListener = this@MainActivity
        }
        lifecycle.addObserver(playerManager)
    }

    fun open(view: View) {
        val readPermission =
            ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
        val writePermission =
            ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
        //shouldShowRequestPermissionRationale(Manifest.permission.READ_EXTERNAL_STORAGE)
        if (readPermission == PermissionChecker.PERMISSION_GRANTED && writePermission == PermissionChecker.PERMISSION_GRANTED) {
            playerManager.setDataSource(path)
            playerManager.prepare()
        } else {
            requestPermissionLauncher.launch(
                arrayOf(
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
                )
            )
        }
    }

    override fun onPrepare() {
        playerManager.start()
    }

    override fun onError(code: Int) {
    }

    override fun onProgress(progress: Int) {
    }
}