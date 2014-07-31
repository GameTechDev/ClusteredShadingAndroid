//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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
#ifndef CPUTOSSERVICES_H
#define CPUTOSSERVICES_H



#include "CPUT.h"

// OS includes
#include <errno.h>  // file open error codes
#include <string>   // wstring
#include <fstream>
#include <iostream>

namespace CPUTFileSystem
{
    //Working directory manipulation
    CPUTResult GetWorkingDirectory(std::string *pPath);
    CPUTResult GetWorkingDirectory(std::wstring *pPath);
    CPUTResult SetWorkingDirectory(const std::string &path);
    CPUTResult SetWorkingDirectory(const std::wstring &path);
    CPUTResult GetExecutableDirectory(std::string *pExecutableDir);
    CPUTResult GetExecutableDirectory(std::wstring *pExecutableDir);

    // Path helpers
    CPUTResult ResolveAbsolutePathAndFilename(const std::string &fileName, std::string *pResolvedPathAndFilename);
    CPUTResult ResolveAbsolutePathAndFilename(const std::wstring &fileName, std::wstring *pResolvedPathAndFilename);
    CPUTResult SplitPathAndFilename(const std::string &sourceFilename, std::string *pDrive, std::string *pDir, std::string *pfileName, std::string *pExtension);
    CPUTResult SplitPathAndFilename(const std::wstring &sourceFilename, std::wstring *pDrive, std::wstring *pDir, std::wstring *pfileName, std::wstring *pExtension);

    // file handling
    CPUTResult DoesFileExist(const std::string &pathAndFilename);
    CPUTResult DoesFileExist(const std::wstring &pathAndFilename);
    CPUTResult DoesDirectoryExist(const std::string &path);
    CPUTResult DoesDirectoryExist(const std::wstring &path);
    CPUTResult OpenFile(const std::wstring &fileName, FILE **pFilePointer);
    CPUTResult OpenFile(const std::string &fileName, FILE **pFilePointer);
    CPUTResult ReadFileContents(const std::wstring &fileName, UINT *psizeInBytes, void **ppData, bool bAddTerminator = false, bool bLoadAsBinary = false);
	CPUTResult ReadFileContents(const std::string &fileName, UINT *psizeInBytes, void **ppData, bool bAddTerminator = false, bool bLoadAsBinary = false);

    CPUTResult TranslateFileError(int err);

    // Interface class for fstreams
    class iCPUTifstream
    {
    public:
        iCPUTifstream(const cString &fileName, std::ios_base::openmode mode = std::ios_base::in) {};
        virtual ~iCPUTifstream() {};
    
        virtual bool fail() = 0;
        virtual bool good() = 0;
        virtual bool eof() = 0;
        virtual void read(char* s, int64_t n) = 0;
        virtual void close() = 0;
    };
    
#ifdef  CPUT_USE_ANDROID_ASSET_MANAGER
    class CPUTandroidifstream : public iCPUTifstream
    {
    public:
        CPUTandroidifstream(const cString &fileName, std::ios_base::openmode mode);
        ~CPUTandroidifstream();
    
        bool fail() { return (!mpAsset); }
        bool good() { return (mpAsset != NULL); };
        bool eof()  { return mbEOF; }
        void read (char* s, int64_t n);
        void close();
    
    protected:
        AAsset    * mpAsset;
        AAssetDir * mpAssetDir;
        bool        mbEOF;    
    };
#endif // CPUT_USE_ANDROID_ASSET_MANAGER

    class CPUTifstream : public iCPUTifstream
    {
    public:
        CPUTifstream(const cString &fileName, std::ios_base::openmode mode) : iCPUTifstream(fileName, mode)
        {
            mStream.open(fileName.c_str(), mode);
        }
        
        virtual ~CPUTifstream()
        {
            mStream.close();
        }
    
        bool fail() { return mStream.fail(); }
        bool good() { return mStream.good(); }
        bool eof()  { return mStream.eof(); }
        void read (char* s, int64_t n) { mStream.read(s, n); }
        void close() { mStream.close(); }
    
    protected:
        std::ifstream mStream;
    };

#ifdef CPUT_USE_ANDROID_ASSET_MANAGER
#define CPUTOSifstream CPUTandroidifstream
#else
#define CPUTOSifstream CPUTifstream
#endif
};

namespace CPUTOSServices
{
    CPUTResult OpenMessageBox(std::wstring title, std::wstring text);
    CPUTResult OpenMessageBox(std::string title, std::string text);
};

#endif // CPUTOSSERVICES_H