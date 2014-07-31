require 'ndk'

dofile( "sampleConfig.lua" )

--[[
Define command line options. These must come before the action on the command line.
--]]
newoption {
   trigger     = "gfxapi",
   value       = "API",
   description = "Choose a particular 3D API for rendering",
   allowed = {
      { "gl",   "OpenGL" },
      { "gles", "OpenGL ES" },
      { "dx",   "Direct3D" }
   }
}

newoption {
   trigger     = "os",
   value       = "TARGET",
   description = "Choose a target operating system",
   allowed = {
      { "windows", "Microsoft Windows" },
      { "linux",   "Ubuntu Linux" },
      { "android", "Android" }
   }
}

--[[
Define the solution and projects. Settings that will be applied to both the CPUT library and the sample
go under the solution. Settings that are only applicable to either the sample executable or the CPUT library
go under the respective project.
--]]
solution( sampleName )
    platforms( { "x32", "x64" } )
    language( "C++" )
    flags( { "MultiProcessorCompile", "NoMinimalRebuild", "NoRTTI" } )
    startproject( sampleName )
    
    -- Specifying a graphics api on the command line enables a subset of possible configurations
    if _OPTIONS["gfxapi"] == "gl" then
        configurations( { "Debug_GL", "Release_GL" } )
    elseif _OPTIONS["gfxapi"] == "dx" then
        configurations( { "Debug_DX", "Release_DX" } )
    elseif _OPTIONS["gfxapi"] == "gles" then
        configurations( { "Debug_GLES", "Release_GLES" } )
    else
        configurations( { "Debug_GL", "Release_GL" } )
        configurations( { "Debug_DX", "Release_DX" } )
        configurations( { "Debug_GLES", "Release_GLES" } )
    end
    
    if not _OPTIONS["os"] then
        _OPTIONS["os"] = "windows"
    end
    
    configuration( { "x32" } )
        architecture( "x32" )
    
    configuration( { "x64" } )
        architecture( "x64" )
    
    -- Settings dependent on target rendering API
    configuration( { "*_DX" } )
        defines( { "CPUT_FOR_DX11" } )
        
    configuration( { "*_GL" } )
        defines( { "CPUT_FOR_OGL", "GLEW_STATIC" } )
    
    configuration( { "*_GLES" } )
        defines( { "CPUT_FOR_OGLES3", "CPUT_FOR_OGLES" } )
        
    -- Settings dependent on target operating system
    configuration( { "os=windows" } )
        flags( { "Unicode" } )
        defines( { "CPUT_OS_WINDOWS" } )

    configuration( { "os=linux" } )
        defines( { "CPUT_OS_LINUX" } )
    
    configuration( { "os=android or *GLES" } )
        defines( { "CPUT_OS_ANDROID" } )
        removeconfigurations( { "Release*" } )
        removeplatforms( { "x32", "x64" } )
        framework( "android-18" )
        buildoptions( { "-std=c++11" } )
        platforms( { "android" } )
        abis( { "x86", "armeabi-v7a" } )
        stl( "stlport_static" )
    
    -- Settings dependent on build profile
    configuration( { "Debug*" } )
        defines( { "DEBUG" } )
        optimize( "Off" )
        flags( { "Symbols" } )

    configuration( { "Release*" } )
        optimize( "Full" )
        flags( { "LinkTimeOptimization" } )

    configuration( { "Release", "vs*" } )
        buildoptions( { "/Oi", "/Ob2" } )

    -- Settings dependent on host compiler
    configuration( { "vs*" } )
        defines( { "NOMINMAX", "_CRT_SECURE_NO_WARNINGS" } )

    if _OPTIONS["gfxapi"] == "gl" then
        CPUTSuffix = ""
        CPUTRemoveConfigs = { "*_DX", "*_GLES" }
        dofile( "CPUT/CPUT.lua" )
        dofile( "sample.lua" )
    elseif _OPTIONS["gfxapi"] == "dx" then
        CPUTSuffix = ""
        CPUTRemoveConfigs = { "*_GL", "*_GLES" }    
        dofile( "CPUT/CPUT.lua" )
        dofile( "sample.lua" )
    elseif _OPTIONS["gfxapi"] == "gles" then
        CPUTSuffix = ""
        CPUTRemoveConfigs = { "*_GL", "*_DX" }
        dofile( "CPUT/CPUT.lua" )
        dofile( "sample.lua" )
    else
        CPUTSuffix = "GL"
        CPUTRemoveConfigs = { "*_DX", "*_GLES" }
        dofile( "CPUT/CPUT.lua" )
        dofile( "sample.lua" )

        CPUTSuffix = "DX"
        CPUTRemoveConfigs = { "*_GL", "*_GLES" }    
        dofile( "CPUT/CPUT.lua" )
        dofile( "sample.lua" )
    
        CPUTSuffix = "Android"
        CPUTRemoveConfigs = { "*_GL", "*_DX" }
        dofile( "CPUT/CPUT.lua" )
        dofile( "sample.lua" )
    end
