//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#ifndef __CPUTBASE_H__
#define __CPUTBASE_H__

// OpenGL
#ifdef CPUT_OS_ANDROID
#include <jni.h>
#include <errno.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <../middleware/ndk_helper/gestureDetector.h>
#ifdef CPUT_FOR_OGLES2
#include <GLES2/gl2.h>                // OpenGL ES 2.0
#include <GLES2/gl2ext.h>             // OpenGL ES Extensions 
#elif defined CPUT_FOR_OGLES3
#include "GLES3/gl3.h"                // OpenGL ES 3.0
#include "GLES3/gl3platform.h"        // OpenGL ES 3.0 Platform-Dependent Macros
#include <GLES2/gl2ext.h>             // OpenGL ES 2.0 Extensions 
#else
#error "No GLES version defined for Android"
#endif

// Undef those defined inside JNIHelper, define ours so we have CPUT tag
#undef LOGI
#undef LOGW
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "CPUT", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "CPUT", __VA_ARGS__))

#define CPUT_USE_ANDROID_ASSET_MANAGER // comment this out to use standard c/c++ file access in the external storage area

#endif


#if defined( CPUT_OS_WINDOWS ) && defined( CPUT_FOR_OGL )
#include <windows.h>  
#include <GL/glew.h>
#include <GL/gl.h>                    // OpenGL
#include "glext.h"                    // OpenGL Extensions 
#include "wglext.h"                   // OpenGL Windows Extensions 
#endif

#if defined( CPUT_OS_WINDOWS ) && defined( CPUT_FOR_OGLES3 )
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h> 
#endif

#ifdef CPUT_OS_LINUX
#include <GL/glew.h>
#include <GL/glx.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <assert.h>
#include <stdint.h>
#include <cstring>
#include "CPUTMath.h"
#include "CPUTEventHandler.h"
#include "CPUTCallbackHandler.h"
#include "CPUTTimer.h"

#ifdef CPUT_GPA_INSTRUMENTATION
// For D3DPERF_* calls, you also need d3d9.lib included
#include <d3d9.h>               // required for all the pix D3DPERF_BeginEvent()/etc calls
#include <ittnotify.h>
#include "CPUTITTTaskMarker.h"  // markup helper for GPA Platform Analyzer tags
#include "CPUTPerfTaskMarker.h" // markup helper for GPA Frame Analyzer tags

// GLOBAL instrumentation junk
enum CPUT_GPA_INSTRUMENTATION_STRINGS{
    GPA_HANDLE_CPUT_CREATE = 0,
    GPA_HANDLE_CONTEXT_CREATION,
    GPA_HANDLE_SYSTEM_INITIALIZATION,
    GPA_HANDLE_MAIN_MESSAGE_LOOP,
    GPA_HANDLE_EVENT_DISPATCH_AND_HANDLE,
    GPA_HANDLE_LOAD_SET,
    GPA_HANDLE_LOAD_MODEL,
    GPA_HANDLE_LOAD_MATERIAL,
    GPA_HANDLE_LOAD_TEXTURE,
    GPA_HANDLE_LOAD_CAMERAS,
    GPA_HANDLE_LOAD_LIGHTS,
    GPA_HANDLE_LOAD_VERTEX_SHADER,
    GPA_HANDLE_LOAD_GEOMETRY_SHADER,
    GPA_HANDLE_LOAD_PIXEL_SHADER,
    GPA_HANDLE_DRAW_GUI,
    GPA_HANDLE_STRING_ENUMS_SIZE,
};
#endif // CPUT_GPA_INSTRUMENTATION

#ifndef nullptr
#define nullptr NULL
#endif

// Integer types
typedef signed   char      sint8;
typedef unsigned char      uint8;
typedef signed   short     sint16;
typedef unsigned short     uint16;
typedef signed   int       sint32;
typedef unsigned int       uint32;
typedef signed   long long sint64;
typedef unsigned long long uint64;

#ifdef CPUT_FOR_DX11
// include all DX11 headers needed
#include <d3d11.h>
#include <D3DCompiler.h>    // for D3DReflect() / D3DX11Refection - IMPORTANT NOTE: include directories MUST list DX SDK include path BEFORE
// Windows include paths or you'll get compile errors with D3DShader.h
#endif

// context creation parameters
struct CPUTContextCreation
{
#ifdef CPUT_FOR_DX11
    int refreshRate;
    int swapChainBufferCount;
    DXGI_FORMAT swapChainFormat;
    DXGI_USAGE swapChainUsage;
	int swapChainSampleCount;
	CPUTContextCreation(): refreshRate(60), swapChainBufferCount(1), swapChainFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB), swapChainUsage(DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT),swapChainSampleCount(1){}
#endif
#if (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
	int samples;
	CPUTContextCreation():samples(1){}
#endif
};

// window creation parameters
struct CPUTWindowCreationParams
{
    bool startFullscreen;
    int windowWidth;
    int windowHeight;
    int windowPositionX;
    int windowPositionY;
    int samples;
    CPUTContextCreation deviceParams;
    CPUTWindowCreationParams() : startFullscreen(false), windowWidth(1280), windowHeight(720), windowPositionX(0), windowPositionY(0), samples(1) {}
};

// TODO: Add here compile time preprocessor checks to validate sizes of variables!

// Heap corruption, ASSERT, and TRACE defines
//-----------------------------------------------------------------------------
#ifdef CPUT_OS_ANDROID
//#define DEBUG_PRINT(string) LOGI(string.c_str());
#define DEBUG_PRINT(s, ...) LOGI(s, ##__VA_ARGS__)
#else
#ifdef CPUT_OS_WINDOWS
void       DEBUG_PRINT(const char *pData, ...); // defined in CPUTOSServicesWin.cpp
void       DEBUG_PRINT(const wchar_t *pData, ...);  // defined in CPUTOSServicesWin.cpp
#else 
#define DEBUG_PRINT(...)
#endif
#endif
/*
#define ASSERT(Condition, Message) {                                                                        \
        if( !(Condition) ) {                                                                                    \
            cString msg; // = std::string(__FUNCTION__) + _L(":  ") + Message;                                      \
            DEBUG_PRINT(msg.c_str());                                                                             \
            DEBUGMESSAGEBOX(_L("Assert"), msg );                                                                \
        }                                                                                                       \
        assert(Condition);                                                                                      \
    }                     
    */
#ifdef _DEBUG
    #include <tchar.h>
    #define TRACE(String)  {DEBUG_PRINT(String);}
    #define DEBUGMESSAGEBOX(Title, Text) { CPUTOSServices::GetOSServices()->OpenMessageBox(Title, Text);}
    #define ASSERT(Condition, Message) { if( !(Condition) ) { cString msg = cString(_T(__FUNCTION__)) + _L(":  ") + Message; DEBUG_PRINT(msg.c_str());} assert(Condition);} 
    #define ASSERTA(Condition, Message) 
    /*{                                                                       \
        if( !(Condition) ) {                                                                                    \
                std::string msg = std::string(__FUNCTION__) + ":  " + Message;                                  \
                DEBUG_PRINTA(msg.c_str()); DEBUGMESSAGEBOX("Assert", msg );                               \
        }                                                                                                       \
        assert(Condition);                                                                                      \
    }*/
    #define HEAPCHECK     { int  heapstatus = _heapchk(); ASSERT(_HEAPOK == heapstatus, _L("Heap corruption") ); }
    // #define HEAPCHECK     {}
#else
    #define ASSERT(Condition, Message)
    #define ASSERTA(Condition, Message)
    #define TRACE(String)
    #define DEBUGMESSAGEBOX(Title, Text)
    #define HEAPCHECK
#endif // _DEBUG

#ifdef CPUT_OS_ANDROID
#define DEBUG_PRINT_ALWAYS(x) { LOGI("ERROR:[%s:%s:%d]", __FILE__, __FUNCTION__, __LINE__); LOGI x; }
#else
#define DEBUG_PRINT_ALWAYS(x) DEBUG_PRINT(x)
#endif

#define DEBUG_ERROR(x) { std::cerr << "ERROR: " << x << "\n\tin file: " << __FILE__ << "\n\tin func:" << __FUNCTION__ << "\n\tat line:" << __LINE__ << std::endl; }
//
//class CPUTFuncPrint
//{
//public:
//    CPUTFuncPrint(const char * func, const char * file, const int line) : mpFunc(func), mpFile(file), mLine(line)
//    {
//        DEBUG_PRINT("Entering function %s (%s:%d)" + mpFunc + mpFile + mLine);
//        //DEBUG_PRINT("Entering function %s (%s:%d)", mpFunc, mpFile, mLine);
//    }
//    
//    virtual ~CPUTFuncPrint()
//    {
//        DEBUG_PRINT("Exiting function %s (%s:%d)", mpFunc, mpFile, mLine);
//    }
//          
//protected:
//    const char * mpFunc;
//    const char * mpFile;
//    const int    mLine;
//};
//
//#define FUNC_ENTRY CPUTFuncPrint CPUTFP(__FUNCTION__, __FILE__, __LINE__)

// Error codes
//-----------------------------------------------------------------------------
typedef enum CPUTResult
{
    // success
    CPUT_SUCCESS = 0x00000000,

    // warnings
//    CPUT_WARNING_OUT_OF_RANGE,
    CPUT_WARNING_NOT_FOUND,
//    CPUT_WARNING_ALREADY_EXISTS,
//    CPUT_WARNING_FILE_IN_SEARCH_PATH_BUT_NOT_WHERE_SPECIFIED,
//    CPUT_WARNING_PHONG_SHADER_MISSING_TEXTURE,
    CPUT_WARNING_CANCELED,
//    CPUT_WARNING_NO_SUITABLE_FORMAT_FOUND,
//
    CPUT_WARNING_SHADER_INPUT_SLOT_NOT_MATCHED,
    CPUT_WARNING_NO_ASSETS_LOADED,
//
// just an error
//
    CPUT_ERROR = 0xF0000000,
//
//    // file errors
    CPUT_ERROR_FILE_NOT_FOUND = 0xF0000001,
    CPUT_ERROR_FILE_READ_ERROR = CPUT_ERROR_FILE_NOT_FOUND+1,
    CPUT_ERROR_FILE_CLOSE_ERROR = CPUT_ERROR_FILE_NOT_FOUND+2,
    CPUT_ERROR_FILE_IO_ERROR = CPUT_ERROR_FILE_NOT_FOUND+3,
    CPUT_ERROR_FILE_NO_SUCH_DEVICE_OR_ADDRESS = CPUT_ERROR_FILE_NOT_FOUND+4,
    CPUT_ERROR_FILE_BAD_FILE_NUMBER = CPUT_ERROR_FILE_NOT_FOUND+5,
    CPUT_ERROR_FILE_NOT_ENOUGH_MEMORY = CPUT_ERROR_FILE_NOT_FOUND+6,
    CPUT_ERROR_FILE_PERMISSION_DENIED = CPUT_ERROR_FILE_NOT_FOUND+7,
    CPUT_ERROR_FILE_DEVICE_OR_RESOURCE_BUSY = CPUT_ERROR_FILE_NOT_FOUND+8,
    CPUT_ERROR_FILE_EXISTS = CPUT_ERROR_FILE_NOT_FOUND+9,
    CPUT_ERROR_FILE_IS_A_DIRECTORY = CPUT_ERROR_FILE_NOT_FOUND+10,
    CPUT_ERROR_FILE_TOO_MANY_OPEN_FILES = CPUT_ERROR_FILE_NOT_FOUND+11,
    CPUT_ERROR_FILE_TOO_LARGE = CPUT_ERROR_FILE_NOT_FOUND+12,
    CPUT_ERROR_FILE_DEVICE_FULL = CPUT_ERROR_FILE_NOT_FOUND+13,
    CPUT_ERROR_FILE_FILENAME_TOO_LONG = CPUT_ERROR_FILE_NOT_FOUND+14,
    CPUT_ERROR_FILE_PATH_ERROR = CPUT_ERROR_FILE_NOT_FOUND+15,
    CPUT_ERROR_FILE_ERROR = CPUT_ERROR_FILE_NOT_FOUND+16,
//
//    CPUT_ERROR_DIRECTORY_NOT_FOUND = CPUT_ERROR_FILE_NOT_FOUND+21,
//
//    // subsystem errors
    CPUT_ERROR_INVALID_PARAMETER = 0xF0000100,
    CPUT_ERROR_NOT_FOUND = CPUT_ERROR_INVALID_PARAMETER+1,
    CPUT_ERROR_NOT_IMPLEMENTED = CPUT_ERROR_INVALID_PARAMETER + 2,
//    CPUT_ERROR_COMPONENT_NOT_INITIALIZED = CPUT_ERROR_INVALID_PARAMETER+2,
//    CPUT_ERROR_SUBSYSTEM_OUT_OF_MEMORY = CPUT_ERROR_INVALID_PARAMETER+3,
//    CPUT_ERROR_OUT_OF_BOUNDS = CPUT_ERROR_INVALID_PARAMETER+4,
//    CPUT_ERROR_HEAP_CORRUPTION = CPUT_ERROR_INVALID_PARAMETER+5,
//
//    // image format errors
    CPUT_ERROR_UNSUPPORTED_IMAGE_FORMAT = 0xF0000200,
//    CPUT_ERROR_ERROR_LOADING_IMAGE = CPUT_ERROR_UNSUPPORTED_IMAGE_FORMAT+1,
    CPUT_ERROR_UNSUPPORTED_SRGB_IMAGE_FORMAT,
//
//    // shader loading errors
//    CPUT_SHADER_LOAD_ERROR = 0xF0000300,
//    CPUT_SHADER_COMPILE_ERROR = CPUT_SHADER_LOAD_ERROR+1,
//    CPUT_SHADER_LINK_ERROR = CPUT_SHADER_LOAD_ERROR+2,
//    CPUT_SHADER_REGISTRATION_ERROR = CPUT_SHADER_LOAD_ERROR+3,
//    CPUT_SHADER_CONSTANT_BUFFER_ERROR = CPUT_SHADER_LOAD_ERROR+4,
//    CPUT_SHADER_REFLECTION_ERROR = CPUT_SHADER_LOAD_ERROR+5,
//
//    // texture loading errors
    CPUT_TEXTURE_LOAD_ERROR = 0xF0000400,
    CPUT_ERROR_TEXTURE_FILE_NOT_FOUND = CPUT_TEXTURE_LOAD_ERROR+1,
//
//    // GUI errors
    CPUT_GUI_GEOMETRY_CREATION_ERROR = 0xF0000500,
//    CPUT_GUI_SAMPLER_CREATION_ERROR = CPUT_GUI_GEOMETRY_CREATION_ERROR+1,
//    CPUT_GUI_TEXTURE_CREATION_ERROR = CPUT_GUI_GEOMETRY_CREATION_ERROR+2,
//    CPUT_GUI_CANNOT_CREATE_CONTROL = CPUT_GUI_GEOMETRY_CREATION_ERROR+3,
    CPUT_GUI_INVALID_CONTROL_ID = CPUT_GUI_GEOMETRY_CREATION_ERROR+4,
//
//    // Texture loading errors
//    CPUT_FONT_TEXTURE_TYPE_ERROR = 0xF0000600,
//    CPUT_FONT_TEXTURE_LOAD_ERROR = CPUT_FONT_TEXTURE_TYPE_ERROR+1,
//
//    // Model loading errors
//    CPUT_ERROR_MODEL_LOAD_ERROR = 0xF0000650,
//    CPUT_ERROR_MODEL_FILE_NOT_FOUND = CPUT_ERROR_MODEL_LOAD_ERROR+1,
//
//    // Shader errors
    CPUT_ERROR_VERTEX_LAYOUT_PROBLEM = 0xF0000700,
//    CPUT_ERROR_VERTEX_BUFFER_CREATION_PROBLEM = CPUT_ERROR_VERTEX_LAYOUT_PROBLEM+1,
//    CPUT_ERROR_INDEX_BUFFER_CREATION_PROBLEM = CPUT_ERROR_VERTEX_LAYOUT_PROBLEM+2,
//    CPUT_ERROR_UNSUPPORTED_VERTEX_ELEMENT_TYPE = CPUT_ERROR_VERTEX_LAYOUT_PROBLEM+3,
//    CPUT_ERROR_INDEX_BUFFER_LAYOUT_PROBLEM = CPUT_ERROR_VERTEX_LAYOUT_PROBLEM+4,
    CPUT_ERROR_SHADER_INPUT_SLOT_NOT_MATCHED = CPUT_ERROR_VERTEX_LAYOUT_PROBLEM+5,
//
//
//    // Context creation errors
//    CPUT_ERROR_CONTEXT_CREATION_FAILURE = 0xF0000C00,
//    CPUT_ERROR_SWAP_CHAIN_CREATION_FAILURE = CPUT_ERROR_CONTEXT_CREATION_FAILURE+1,
//    CPUT_ERROR_RENDER_TARGET_VIEW_CREATION_FAILURE = CPUT_ERROR_CONTEXT_CREATION_FAILURE+2,
//
//    // Depth buffer errors
//    CPUT_ERROR_DEPTH_BUFFER_CREATION_ERROR = 0xF0000800,
//    CPUT_ERROR_DEPTH_STENCIL_BUFFER_CREATION_ERROR = CPUT_ERROR_DEPTH_BUFFER_CREATION_ERROR+1,
//    CPUT_ERROR_RASTER_STATE_CREATION_ERROR = CPUT_ERROR_DEPTH_BUFFER_CREATION_ERROR+2,
//
//    // GUI shaders
    CPUT_ERROR_INITIALIZATION_GUI_VERTEX_SHADER_NOT_FOUND = 0xF0000130,
    CPUT_ERROR_INITIALIZATION_GUI_PIXEL_SHADER_NOT_FOUND = CPUT_ERROR_INITIALIZATION_GUI_VERTEX_SHADER_NOT_FOUND+1,
    CPUT_ERROR_INITIALIZATION_GUI_CONTROL_TEXTURES_NOT_FOUND = CPUT_ERROR_INITIALIZATION_GUI_VERTEX_SHADER_NOT_FOUND+2,
//
//    // gfx system errors
//    CPUT_ERROR_GFX_SUBSYSTEM_BUSY = 0xF0000B00,
//    CPUT_ERROR_GFX_SUBSYSTEM_TO_MANY_OBJECTS = CPUT_ERROR_GFX_SUBSYSTEM_BUSY+1,
//
//    // window layer errors
    CPUT_ERROR_WINDOW_CANNOT_REGISTER_APP = 0xF0000D00,
    CPUT_ERROR_WINDOW_ALREADY_EXISTS = CPUT_ERROR_WINDOW_CANNOT_REGISTER_APP+1,
//    CPUT_ERROR_CANNOT_GET_WINDOW_CLASS = CPUT_ERROR_WINDOW_CANNOT_REGISTER_APP+3,
    CPUT_ERROR_CANNOT_GET_WINDOW_INSTANCE = CPUT_ERROR_WINDOW_CANNOT_REGISTER_APP+4,
//    CPUT_ERROR_WINDOW_OS_PROPERTY_GET_ERROR = CPUT_ERROR_WINDOW_CANNOT_REGISTER_APP+5,
//
//    // AssetLibrary/AssetSet errors
    CPUT_ERROR_ASSET_LIBRARY_INVALID_LIBRARY = 0xF0000E00,
//    CPUT_ERROR_ASSET_SET_INVALID_TYPE = CPUT_ERROR_ASSET_LIBRARY_INVALID_LIBRARY+1,
//    CPUT_ERROR_ASSET_LIBRARY_OBJECT_NOT_FOUND,
//    CPUT_ERROR_ASSET_ALREADY_EXISTS = CPUT_ERROR_ASSET_LIBRARY_INVALID_LIBRARY+3,
//
//    // Paramter block errors.
    CPUT_ERROR_PARAMETER_BLOCK_NOT_FOUND = 0xF0000F00,
//
//    // misc errors
//    CPUT_ERROR_FULLSCREEN_SWITCH_ERROR = 0xF0000F00,
} CPUTResult;

static int gRefCount = 0;
//handy defines
//-----------------------------------------------------------------------------
#define SAFE_RELEASE(p)     {if((p)){HEAPCHECK; gRefCount = (p)->Release(); (p)=NULL; HEAPCHECK;} }
#define SAFE_DELETE(p)      {if((p)){HEAPCHECK; delete (p);     (p)=NULL;HEAPCHECK; }}
#define SAFE_DELETE_ARRAY(p){if((p)){HEAPCHECK; delete[](p);    (p)=NULL;HEAPCHECK; }}
#define UNREFERENCED_PARAMETER(P) (P)

// CPUT data types
//-----------------------------------------------------------------------------
#define CPUTSUCCESS(returnCode) ((returnCode) < 0xF0000000)
#define CPUTFAILED(returnCode) ((returnCode) >= 0xF0000000)


//typedef UINT CPUTResult;
typedef unsigned int UINT;
//typedef unsigned long DWORD;

// color
struct CPUTColor4
{
    float r;
    float g;
    float b;
    float a;

    bool operator == (const CPUTColor4& rhs) const
    {
        return((rhs.r == r) && 
               (rhs.g == g) &&
               (rhs.b == b) &&
               (rhs.a == a));
    }
    bool operator != (const CPUTColor4& rhs) const
    {
        return((rhs.r != r) || 
               (rhs.g != g) ||
               (rhs.b != b) ||
               (rhs.a != a));
    }
};

// where the loader should start looking from to locate files
enum CPUT_PATH_SEARCH_MODE
{
    CPUT_PATH_SEARCH_RESOURCE_DIRECTORY,
    CPUT_PATH_SEARCH_NONE,
};

// string size limitations
const UINT CPUT_MAX_PATH = 2048;
const UINT CPUT_MAX_STRING_LENGTH = 1024;
const UINT CPUT_MAX_SHADER_ERROR_STRING_LENGTH = 8192;
const UINT CPUT_MAX_DIGIT_STRING_LENGTH = 5;



// Data format types used in interpreting mesh data
enum CPUT_DATA_FORMAT_TYPE
{
    CPUT_UNKNOWN=0,

    CPUT_DOUBLE=1,
    CPUT_F32=2,

    CPUT_U64=3,
    CPUT_I64=4,

    CPUT_U32=5,
    CPUT_I32=6,

    CPUT_U16=7,
    CPUT_I16=8,

    CPUT_U8=9,
    CPUT_I8=10,

    CPUT_CHAR=11,
    CPUT_BOOL=12,
};

// Corresponding sizes (in bytes) that match CPUT_DATA_FORMAT_TYPE
const int CPUT_DATA_FORMAT_SIZE[] =
{
        0, //CPUT_UNKNOWN=0,
        
        8, //CPUT_DOUBLE,
        4, //CPUT_F32,
        
        8, //CPUT_U64,
        8, //CPUT_I64,
        
        4, //CPUT_U32,
        4, //CPUT_I32,
        
        2, //CPUT_U16,
        2, //CPUT_I16,
        
        1, //CPUT_U8,
        1, //CPUT_I8,

        1, //CPUT_CHAR
        1, //CPUT_BOOL
};

//-----------------------------------------------------------------------------
enum eCPUTMapType
{
    CPUT_MAP_UNDEFINED = 0,
    CPUT_MAP_READ = 1,
    CPUT_MAP_WRITE = 2,
    CPUT_MAP_READ_WRITE = 3,
    CPUT_MAP_WRITE_DISCARD = 4,
    CPUT_MAP_NO_OVERWRITE = 5
};

// routines to support unicode + multibyte
// TODO: Move to string file
//-----------------------------------------------------------------------------
#if defined (UNICODE) || defined(_UNICODE)

    // define string and literal types
    #define cString std::wstring
    #define cStringStream std::wstringstream
    #define cFile std::wfstream
	#define cInputFile std::wifstream
    #define _L(x)      L##x
    #define to_cString std::to_wstring
    // convert integer to wide/unicode ascii
    //-----------------------------------------------------------------------------
    inline std::wstring itoc(const int integer)
    {
        wchar_t wcstring[CPUT_MAX_STRING_LENGTH];
        swprintf(&wcstring[0], CPUT_MAX_STRING_LENGTH, _L("%d"), integer);
        std::wstring ws(wcstring);

        return ws;
    }

    // convert pointer to wide/unicode ascii
    //-----------------------------------------------------------------------------
    inline std::wstring ptoc(const void *pPointer)
    {
        wchar_t wcstring[65];
        swprintf_s(wcstring, _L("%p"), pPointer);

        return wcstring;
    }

    // convert char* to wide/unicode string
    //-----------------------------------------------------------------------------
    inline std::wstring s2ws(const char* stringArg)
    {
        // compute the size of the buffer I need to allocate
        size_t numConvertedChars;
        //FIXME: double check this is correct
        //numConvertedChars = mbstowcs(NULL, stringArg, 0) * 2;
        numConvertedChars = mbstowcs(NULL, stringArg, 0);
        numConvertedChars++;  // +1 for null termination
        if(numConvertedChars>CPUT_MAX_STRING_LENGTH)
        {
            numConvertedChars = CPUT_MAX_STRING_LENGTH;
        }

        // allocate the converted string and copy
        wchar_t *pWString = new wchar_t[numConvertedChars];
        mbstowcs(pWString, stringArg, numConvertedChars);
        std::wstring ws(pWString);
        delete [] pWString;
        return ws;

    }

    // convert wide/unicode string to char
    //-----------------------------------------------------------------------------
    inline char* ws2s(std::wstring string)
    {
		//char* pBuffer = new char[CPUT_MAX_STRING_LENGTH];
        //wcstombs(pBuffer, string.c_str(), CPUT_MAX_STRING_LENGTH);
		
        size_t numConverted, finalCount;

        // what size of buffer (in bytes) do we need to allocate for conversion?
        wcstombs_s(&numConverted, NULL, 0, string.c_str(), CPUT_MAX_STRING_LENGTH);
        //numConverted+=2; // for null termination
        //numConverted--;
		char *pBuffer = new char[numConverted];

        // do the actual conversion
        wcstombs_s(&finalCount, pBuffer, numConverted, string.c_str(), numConverted);//CPUT_MAX_STRING_LENGTH);


        return pBuffer;
    }

    // conversion from wide string to cstring. Returns a copy of sourceString.
    // since UNICODE is defined, cString is a std::wstring, so no conversion is necessary
    //-----------------------------------------------------------------------------
    inline cString ws2cs(const std::wstring &sourceString)
    {
        return sourceString;
    }

    // conversion from std::string to cString
    // since UNICODE is defined, cString is a std::wstring
    //-----------------------------------------------------------------------------
    inline cString s2cs(const std::string &sourceString)
    {
        std::wstring destString;
        destString.assign(sourceString.begin(), sourceString.end());

        return destString;
    }

    // conversion from cString to char *
    // since UNICODE is defined, cString is a std::wstring
    //-----------------------------------------------------------------------------
    inline char * cs2s(const cString &sourceString)
    {	
        size_t numConverted, finalCount;

        // what size of buffer (in bytes) do we need to allocate for conversion?
        wcstombs_s(&numConverted, NULL, 0, sourceString.c_str(), CPUT_MAX_STRING_LENGTH);
		char *pBuffer = new char[numConverted];

        // do the actual conversion
        wcstombs_s(&finalCount, pBuffer, numConverted, sourceString.c_str(), numConverted);//CPUT_MAX_STRING_LENGTH);

        return pBuffer;
    }

    // conversion from cString to std::wstring
    // since UNICODE is defined, cString is a std::wstring and no conversion is required
    //-----------------------------------------------------------------------------
    inline std::wstring cs2ws(const cString &sourceString)
    {	
        return sourceString;
    }

#else

    // define string and literal types
    #define cString std::string
    #define cStringStream std::stringstream
    #define cFile std::fstream
    #define cInputFile std::ifstream
    #define _L(x)      x

#ifdef CPUT_OS_ANDROID
    inline std::string to_cString(int integer) 
    {
        cStringStream sstream;
        sstream << integer;
        return sstream.str();
    }
#else
#define to_cString std::to_string
#endif
    // conversion routine
    //-----------------------------------------------------------------------------
    inline std::string s2ws(const char* stringArg) { return std::string(stringArg); }

    // convert integer to char string
    //-----------------------------------------------------------------------------
    inline std::string itoc(const int integer)
    {
        char string[CPUT_MAX_STRING_LENGTH];
		snprintf(string, CPUT_MAX_STRING_LENGTH, "%d", integer);
        std::string s(string);

        return s;
    }

    // convert pointer to wide/unicode ascii
    //-----------------------------------------------------------------------------
    inline std::string ptoc(const void *pPointer)
    {
        std::ostringstream stream;

        stream << pPointer;

        std::string address;
        address = stream.str();

        return address;
    }

    // conversion from ws2s
    // Doesn't do anything in multibyte version since string is already a char*
    //-----------------------------------------------------------------------------
    inline char* ws2s(const char* string)
    {
        char* pBuffer = new char[CPUT_MAX_STRING_LENGTH];
        strncpy(pBuffer, string, CPUT_MAX_STRING_LENGTH);
        
        return pBuffer;
    }

    // conversion from wide string to cstring
    // since UNICODE is not defined, cString is a std::string
    //-----------------------------------------------------------------------------
    inline cString ws2cs(const std::wstring &sourceString)
    {
        std::string destString;
        destString.assign(sourceString.begin(), sourceString.end());

        return destString;
    }

    // conversion from string to cstring. Returns a copy of sourceString.
    // since UNICODE is not defined, cString is a std::string, so no conversion is necessary
    //-----------------------------------------------------------------------------
    inline cString s2cs(const std::string &sourceString)
    {
        return sourceString;
    }
    
    inline char* ws2s(std::string string)
    {
        char* pBuffer = new char[string.length()+1];
        memcpy(pBuffer, string.c_str(), string.length());
        pBuffer[string.length()] = 0;
        DEBUG_PRINT("%s", pBuffer);
        return pBuffer;
    }

    inline char* ws2s(std::wstring string)
    {
		char* pBuffer = new char[CPUT_MAX_STRING_LENGTH];
        wcstombs(pBuffer, string.c_str(), CPUT_MAX_STRING_LENGTH);
		/*
        size_t numConverted, finalCount;

        // what size of buffer (in bytes) do we need to allocate for conversion?
        wcstombs_s(&numConverted, NULL, 0, string.c_str(), CPUT_MAX_STRING_LENGTH);
        numConverted+=2; // for null termination
        char *pBuffer = new char[numConverted];

        // do the actual conversion
        wcstombs_s(&finalCount, pBuffer, numConverted, string.c_str(), CPUT_MAX_STRING_LENGTH);
*/
        return pBuffer;
    }

    // conversion from cString to char *
    // since UNICODE is not defined, cString is a std::string
    //-----------------------------------------------------------------------------
    inline char * cs2s(const cString &sourceString)
    {	
        size_t length = sourceString.length();
        char *pBuffer = new char[length + 1];
        sourceString.copy(pBuffer, length, 0);
        pBuffer[length] = '\0';

        return pBuffer;
    }

    // conversion from cString to std::wstring
    // since UNICODE is not defined, cString is a std::string
    //-----------------------------------------------------------------------------
    inline std::wstring cs2ws(const cString &sourceString)
    {	
        std::wstring destString;
        destString.assign(sourceString.begin(), sourceString.end());

        return destString;
    }
#endif

    // Android has clashes in ctype.h with _L, so easiest to just redefine the tolower function
    inline int tolow(int c)
    {
#ifdef CPUT_OS_ANDROID
        const int low = 'A' - 'a';
        if ((c <= 'Z') && (c >= 'A'))
            return c - low;
        return c;
#else
        return tolower(c);
#endif
    }

typedef struct CPUT_SHADER_MACRO
{
    char *Name;
    char *Definition;
} CPUT_SHADER_MACRO;

char* ConvertShaderMacroToChar(CPUT_SHADER_MACRO *pShaderMacros);

#include "CPUTRenderTarget.h"
#ifdef CPUT_FOR_DX11
#include "CPUTRenderTarget.h"
#elif defined(CPUT_FOR_OGL)
//#include "CPUTRenderTargetOGL.h"
#else    
//#error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
//this should cause an error
#endif

class CPUTCamera;
class CPUTRenderStateBlock;

// CPUT class
//-----------------------------------------------------------------------------
class CPUT:public CPUTEventHandler, public CPUTCallbackHandler
{
protected:
    CPUTCamera  *mpCamera;
    CPUTCamera  *mpShadowCamera;
    CPUTTimer   *mpTimer;
    float4       mLightColor; // TODO: Get from light(s)
    float4       mAmbientColor;
    CPUTBuffer  *mpBackBuffer;
    CPUTBuffer  *mpDepthBuffer;
    CPUTTexture *mpBackBufferTexture;
    CPUTTexture *mpDepthBufferTexture;

public:
    CPUT() :
        mpCamera(NULL),
        mpShadowCamera(NULL),
        mAmbientColor(0.2f, 0.2f, 0.2f, 1.0f),
        mLightColor(0.8f, 0.8f, 0.8f, 1.0f),
        mpBackBuffer(NULL),
        mpDepthBuffer(NULL),
        mpBackBufferTexture(NULL),
        mpDepthBufferTexture(NULL)

	{}
    virtual ~CPUT() {}

    CPUTCamera  *GetCamera() { return mpCamera; }
    CPUTCamera  *GetShadowCamera() { return mpShadowCamera; } // TODO: Support more than one.
    virtual void InnerExecutionLoop() {;}
    virtual void ResizeWindowSoft(UINT width, UINT height) {UNREFERENCED_PARAMETER(width);UNREFERENCED_PARAMETER(height);}
    virtual void ResizeWindow(UINT width, UINT height) {
#ifdef CPUT_FOR_DX11
        CPUTRenderTargetColor::SetActiveWidthHeight( width, height );
        CPUTRenderTargetDepth::SetActiveWidthHeight( width, height );
#endif
    }
    virtual void DeviceShutdown(){}

    virtual CPUTEventHandledCode CPUTHandleKeyboardEvent(CPUTKey key, CPUTKeyState state) {UNREFERENCED_PARAMETER(key);return CPUT_EVENT_UNHANDLED;}
    virtual CPUTEventHandledCode CPUTHandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message) {UNREFERENCED_PARAMETER(x);UNREFERENCED_PARAMETER(y);UNREFERENCED_PARAMETER(wheel);UNREFERENCED_PARAMETER(state);return CPUT_EVENT_UNHANDLED;}

    float4 &GetAmbientColor() { return mAmbientColor; }
    void    SetAmbientColor( float4 &ambientColor ) {  mAmbientColor = ambientColor; }
    float4 &GetLightColor() { return mLightColor; }
    void    SetLightColor( float4 &lightColor ) {  mLightColor = lightColor; }
};

// Include this here to make sure ASSERT resolves correctly
//#include "CPUTOSServicesWin.h"

void CPUTSetDebugName( void *pResource, std::wstring name );

struct CPUTModelConstantBuffer
{
    float4x4 World;
    float4x4 NormalMatrix;
    float4x4 WorldViewProjection;
    float4x4 InverseWorld;
    float4x4 LightWorldViewProjection;
    float4  BoundingBoxCenterWorldSpace;
    float4  BoundingBoxHalfWorldSpace;
    float4  BoundingBoxCenterObjectSpace;
    float4  BoundingBoxHalfObjectSpace;
};
struct CPUTAnimationConstantBuffer
{
    float4x4    SkinMatrix[255];
    float4x4    SkinNormalMatrix[255];     
};

struct CPUTFrameConstantBuffer
{
    float4x4  View;
    float4x4  InverseView;
    float4x4  Projection;
    float4x4  ViewProjection;
    float4    AmbientColor;
    float4    LightColor;
    float4    LightDirection;
    float4    EyePosition;
    float4    TotalSeconds;
};

#endif // #ifndef __CPUTBASE_H__
