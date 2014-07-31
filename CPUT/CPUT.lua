project( "CPUT" .. CPUTSuffix )
    location( "./" )
    kind( "StaticLib" )
    files( { "CPUT/*.cpp", "CPUT/*.h" } )
    files( { "middleware/stb/stb*" } )
    removeconfigurations( CPUTRemoveConfigs )

    configuration( { "vs*", "Release*" } )
        targetdir( "$(SolutionDir)/../lib/$(Platform)/$(PlatformToolset)/Release" )
        objdir( "$(ProjectDir)temp/$(Platform)/$(PlatformToolset)/Release" )
        
    configuration( { "vs*", "Debug*" } )
        targetdir( "$(SolutionDir)/../lib/$(Platform)/$(PlatformToolset)/Debug" )
        objdir( "$(ProjectDir)temp/$(Platform)/$(PlatformToolset)/Debug" )

    configuration( { "*DX" } )
        files( { "../DirectXTex/DDSTextureLoader/DDSTextureLoader.cpp",
                "../DirectXTex/DDSTextureLoader/DDSTextureLoader.h" } )
        excludes( { "CPUT/CPUTShader.cpp" } )
        excludes( { "CPUT/*OGL*" } )
        includedirs( { "../DirectXTex/DDSTextureLoader/",
                    "../DirectXTex/DirectXTex/" } )

    configuration( { "*GL" } )
        defines( { "KTX_OPENGL=1", "SUPPORT_SOFTWARE_ETC_UNPACK=0" } )
        excludes( { "CPUT/CPUT_OGL_GLES.cpp",
                    "CPUT/CPUTRenderTarget.cpp",
                    "CPUT/CPUTRenderTarget.h",
                    "CPUT/CPUTPostProcess.cpp",
                    "CPUT/CPUTPostProcess.h",
                    "CPUT/CPUTShader.cpp" } )
        excludes( { "CPUT/*DX11*" } )
        files( { "middleware/glew-1.10.0/src/glew.c",
                "middleware/glew-1.10.0/include/GL/glew.h" } )
        files( { "middleware/libktx/*.c", "middleware/libktx/*.h" } )
        includedirs( { "middleware/glew-1.10.0/include/",
                    "middleware/" } )
                    
    configuration( { "*GLES" } )
        defines( { "KTX_OPENGL_ES3=1", "SUPPORT_SOFTWARE_ETC_UNPACK=0" } )
        files( { "middleware/libktx/*.c", "middleware/libktx/*.h" } )
        files( { "middleware/ndk_helper/*.cpp", "middleware/ndk_helper/*.h" } )
        excludes( { "CPUT/CPUT_OGL_GL.cpp",
                    "CPUT/CPUTRenderTarget.cpp",
                    "CPUT/CPUTRenderTarget.h",
                    "CPUT/CPUTPostProcess.cpp",
                    "CPUT/CPUTPostProcess.h",
                    "CPUT/CPUTShader.cpp" } )
        includedirs( { "middleware/" } )

        excludes( { "CPUT/*DX11*" } )

    configuration( { "os=windows" } )
        excludes( { "CPUT/CPUTWindowAndroid.cpp",
                    "CPUT/CPUTWindowAndroid.h",
                    "CPUT/CPUTOSServicesLinux.cpp",
                    "CPUT/CPUTTimerLinux.cpp",
                    "CPUT/CPUTTimerLinux.h",
                    "CPUT/CPUTWindowX.cpp",
                    "CPUT/CPUTWindowX.h" } )

    configuration( { "os=linux" } )
        excludes( { "CPUT/CPUTWindowAndroid.cpp",
                    "CPUT/CPUTWindowAndroid.h" } )

    configuration( { "os=android or *GLES" } )
        staticlibs( { "android_native_app_glue" } )
        location(path.getabsolute(""))
        excludes( { "CPUT/CPUTWindowX.cpp",
                    "CPUT/CPUTWindowX.h",
                    "CPUT/CPUTWindowWin.cpp",
                    "CPUT/CPUTWindowWin.h",
                    "CPUT/CPUTTimerWin.cpp",
                    "CPUT/CPUTTimerWin.h",
                    "CPUT/CPUTOSServicesWin.cpp",
                    "CPUT/CPUTSkeleton.cpp",
                    "CPUT/CPUTSkeleton.h" } )


