/*
 * Created by v1tr10l7 on 16.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
package com.v1tr10l7.vortex

import android.app.NativeActivity
import android.content.res.AssetManager
import android.graphics.Point
import android.media.AudioManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.View
import java.io.File
import java.io.FileInputStream
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.nio.charset.StandardCharsets
import java.util.Locale

class VortexActivity : NativeActivity() {

    companion object {
        private const val TAG = "VortexActivity"

        @JvmStatic
        private var instance: VortexActivity? = null

        @JvmStatic
        fun get(): VortexActivity? = instance

        init {
            try {
                val libraryName = BuildConfig.NATIVE_LIB_NAME
                Log.i(TAG, "Loading library '$libraryName'")
                System.loadLibrary(libraryName)
                Log.i(TAG, "Successfully loaded '$libraryName'")
            } catch (error: UnsatisfiedLinkError) {
                Log.e("Vortex-Engine", error.message.orEmpty())
            }
        }

        @JvmStatic
        fun copyAssetDir(am: AssetManager, path: String, outpath: String) {
            try {
                val res = am.list(path) ?: return
                for (re in res) {
                    val fromFile = "$path/$re"
                    val toFile = "$outpath/$re"
                    val fromStream: InputStream = try {
                        am.open(fromFile)
                    } catch (e: FileNotFoundException) {
                        File(toFile).mkdirs()
                        copyAssetDir(am, fromFile, toFile)
                        continue
                    }
                    copy(fromStream, FileOutputStream(toFile))
                }
            } catch (e: Exception) {
                Log.i("Vortex", "Error", e)
            }
        }

        @JvmStatic
        @Throws(IOException::class)
        fun copy(input: InputStream, out: OutputStream) {
            val buff = ByteArray(1024)
            var len: Int
            while (input.read(buff).also { len = it } != -1) {
                out.write(buff, 0, len)
            }
            input.close()
            out.close()
        }

        @JvmStatic
        @Throws(IOException::class)
        fun readFileText(stream: InputStream): String {
            val buffer = ByteArray(stream.available())
            stream.read(buffer)
            stream.close()
            return String(buffer, StandardCharsets.US_ASCII)
        }
    }

    @Throws(IOException::class)
    private fun extractDotnetFiles() {
        val filesDir = filesDir.absolutePath
        val am = assets
        val amRootFiles = am.list("") ?: return

        // Skip if extracted hash is the same as in the package
        val hashFile = "hash.txt"
        val currentHashFile = File("$filesDir/$hashFile")
        if (currentHashFile.exists()) {
            for (amRootFile in amRootFiles) {
                if (amRootFile == hashFile) {
                    val currentHash = readFileText(FileInputStream(currentHashFile))
                    val hash = readFileText(am.open(hashFile))
                    if (currentHash == hash) {
                        return
                    }
                    break
                }
            }
        }

        // Extract files
        Log.i("Vortex", "Extracting Dotnet files")
        File("$filesDir/Dotnet").mkdir()
        copyAssetDir(am, "Dotnet", "$filesDir/Dotnet")
        for (fromFile in amRootFiles) {
            if (!fromFile.endsWith(".dll") && fromFile != hashFile) continue
            val toFile = "$filesDir/$fromFile"
            val fromStream = am.open(fromFile)
            copy(fromStream, FileOutputStream(toFile))
        }
    }

    private fun goFullscreen() {
        @Suppress("DEPRECATION")
        window.decorView.systemUiVisibility = (
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
                View.SYSTEM_UI_FLAG_FULLSCREEN
            )
    }

    override fun onCreate(instance: Bundle?) {
        Companion.instance = this

        // Extract Dotnet files and other bundled C# libraries from APK
        try {
            extractDotnetFiles()
        } catch (e: Exception) {
            Log.i("Vortex", "Error", e)
        }

        // Set native engine platform info
        val appPackageName = packageName
        val deviceManufacturer = Build.MANUFACTURER
        val deviceModel = Build.MODEL
        val deviceBuildNumber = Build.DISPLAY
        val systemVersion = Build.VERSION.RELEASE
        val systemLanguage = Locale.getDefault().toString()

        @Suppress("DEPRECATION")
        val display = windowManager.defaultDisplay
        val displayRealSize = Point()
        @Suppress("DEPRECATION")
        display.getRealSize(displayRealSize)

        val screenWidth = displayRealSize.x
        val screenHeight = displayRealSize.y
        val cacheDir = cacheDir.toString()
        val executablePath = "${applicationInfo.nativeLibraryDir}/libVortex-Engined.so"

        // nativeSetPlatformInfo(appPackageName, deviceManufacturer, deviceModel, deviceBuildNumber, systemVersion, systemLanguage, screenWidth, screenHeight, cacheDir, executablePath)

        super.onCreate(instance)

        // Initialize
        volumeControlStream = AudioManager.STREAM_MUSIC
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            goFullscreen()
        }
    }

    override fun onResume() {
        super.onResume()
        goFullscreen()
    }

    external fun nativeSetPlatformInfo(
        appPackageName: String,
        deviceManufacturer: String,
        deviceModel: String,
        deviceBuildNumber: String,
        systemVersion: String,
        systemLanguage: String,
        screenWidth: Int,
        screenHeight: Int,
        cacheDir: String,
        executablePath: String
    )
}
