project( sampleName .. CPUTSuffix )
    location( "./" )
    kind( "WindowedApp" )
    flags( { "WinMain" } )
    includedirs( { "../CPUT/CPUT/",
                      "../CPUT/middleware/glew-1.10.0/include/",
                      "../CPUT/middleware/" } )
    targetname( sampleName .. CPUTSuffix )
    links( { "CPUT" .. CPUTSuffix } )
    staticlibs( { "android_native_app_glue" } )
    removeconfigurations( CPUTRemoveConfigs )

    configuration( { "vs*", "Release*" } )
        objdir( "$(ProjectDir)temp/$(Platform)/$(PlatformToolset)/Release" )
        libdirs( { "../lib/$(Platform)/$(PlatformToolset)/Release" } )
        
    configuration( { "vs*", "Debug*" } )
        objdir( "$(ProjectDir)temp/$(Platform)/$(PlatformToolset)/Debug" )
        libdirs( { "../lib/$(Platform)/$(PlatformToolset)/Debug" } )
        
    configuration( { "vs*" } )
        targetdir( "$(ProjectDir)bin/$(Platform)/$(PlatformToolset)" )
        debugdir( "$(OutDir)" )

    configuration( { "*_DX" } )
        files( { "SampleStartDX11.h", "SampleStartDX11.cpp"} )
        links( { "d3d11.lib", "d3dcompiler", "dxguid", "DirectXTex" } )

    configuration( { "*_GL" } )
        files( { "SampleStartGL.h", "SampleStartGL.cpp"} )

    configuration( { "os=windows", "*GL" } )
        links( { "OpenGL32" } )

    configuration( { "os=android or *GLES" } )
        files( { "../SampleCPUT_Android/jni/main.cpp" } )
        links( { "log", "android", "EGL", "GLESv3" } )
        import( { "android/native_app_glue" } )
    
    configuration( { "os=android", "ndk-makefile" } )
        location(path.getabsolute("../SampleCPUT_Android/jni/"))

    configuration( { "os=android or *GLES", "vs*" } )
        prebuildcommands { "../Utilities/buildandroid.bat" }
        location(path.getabsolute("../SampleCPUT_Android/"))
