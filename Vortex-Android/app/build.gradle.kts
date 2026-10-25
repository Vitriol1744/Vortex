plugins {
    id("com.android.application")
}

android {
    namespace = "com.v1tr10l7.vortex"
    compileSdk = 34
    ndkVersion = "30.0.14904198"

    buildFeatures {
        prefab = true
        buildConfig = true
    }

    defaultConfig {
        applicationId = "com.v1tr10l7.vortex"
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "0.1"

        ndk {
            abiFilters += setOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                cppFlags("-fexceptions -frtti")

                // Pass all CMake arguments inside defaultConfig
                arguments(
                    "-DCMAKE_CXX_STANDARD=23",
                    "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
                    "-DANDROID_CPP_FEATURES=rtti exceptions",
                    "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
                )
            }
        }
    }

    buildTypes {
        getByName("debug") {
            manifestPlaceholders["nativeLibName"] = "Vortex-Engined"
            buildConfigField("String", "NATIVE_LIB_NAME", "\"Vortex-Engined\"")
        }
        getByName("release") {
            manifestPlaceholders["nativeLibName"] = "Vortex-Engine"
            buildConfigField("String", "NATIVE_LIB_NAME", "\"Vortex-Engine\"")
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    // Outer block handles ONLY project layout path and CMake executable version
    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.25.0+"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}
