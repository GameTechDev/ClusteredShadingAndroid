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
#include "CPUTOSServices.h"

// Retrieves the current working directory
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::GetWorkingDirectory(std::string *pPath)
{
    char pPathAsTchar[CPUT_MAX_PATH];
    DWORD result = GetCurrentDirectoryA(CPUT_MAX_PATH, pPathAsTchar);
    ASSERTA( result, "GetCurrentDirectory returned 0." );
    UNREFERENCED_PARAMETER(result);
    *pPath = pPathAsTchar;
    return CPUT_SUCCESS;
}

CPUTResult CPUTFileSystem::GetWorkingDirectory(std::wstring *pPath)
{
    wchar_t pPathAsTchar[CPUT_MAX_PATH];
    DWORD result = GetCurrentDirectoryW(CPUT_MAX_PATH, pPathAsTchar);
    ASSERT( result, L"GetCurrentDirectory returned 0." );
    UNREFERENCED_PARAMETER(result);
    *pPath = pPathAsTchar;
    return CPUT_SUCCESS;
}

// Sets the current working directory
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::SetWorkingDirectory(const std::string &path)
{
    BOOL result = SetCurrentDirectoryA(path.c_str());
    ASSERTA( 0 != result, "Error setting current directory." );
    UNREFERENCED_PARAMETER(result);
    return CPUT_SUCCESS;
}

CPUTResult CPUTFileSystem::SetWorkingDirectory(const std::wstring &path)
{
    BOOL result = SetCurrentDirectoryW(path.c_str());
    ASSERT( 0 != result, L"Error setting current directory." );
    UNREFERENCED_PARAMETER(result);
    return CPUT_SUCCESS;
}

// Gets the location of the executable's directory
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::GetExecutableDirectory(std::wstring *pExecutableDir)
{
    WCHAR   pFilename[CPUT_MAX_PATH];
    DWORD result = GetModuleFileNameW(NULL, pFilename, CPUT_MAX_PATH);
    ASSERT( 0 != result, _L("Unable to get executable's working directory."));
    UNREFERENCED_PARAMETER(result);

    // strip off the executable name+ext
    cString ResolvedPathAndFilename;
    CPUTFileSystem::ResolveAbsolutePathAndFilename(pFilename, &ResolvedPathAndFilename);
    cString Drive, Dir, Filename, Ext;
    SplitPathAndFilename(ResolvedPathAndFilename, &Drive, &Dir, &Filename, &Ext);

    // store and return
    *pExecutableDir = Drive + Dir;

    return CPUT_SUCCESS;
}

// Gets the location of the executable's directory
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::GetExecutableDirectory(std::string *pExecutableDir)
{
    char   pFilename[CPUT_MAX_PATH];
    DWORD result = GetModuleFileNameA(NULL, pFilename, CPUT_MAX_PATH);
    ASSERTA( 0 != result, "Unable to get executable's working directory.");

    // strip off the executable name+ext
    std::string ResolvedPathAndFilename;
    ResolveAbsolutePathAndFilename(pFilename, &ResolvedPathAndFilename);
    std::string Drive, Dir, Filename, Ext;
    SplitPathAndFilename(ResolvedPathAndFilename, &Drive, &Dir, &Filename, &Ext);

    // store and return
    *pExecutableDir = Drive + Dir;

    return CPUT_SUCCESS;
}

// Split up the supplied path+fileName into its constituent parts
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::SplitPathAndFilename(const std::wstring &sourceFilename, std::wstring *pDrive, std::wstring *pDir, std::wstring *pFileName, std::wstring *pExtension)
{
    WCHAR pSplitDrive[CPUT_MAX_PATH];
    WCHAR pSplitDirs[CPUT_MAX_PATH];
    WCHAR pSplitFile[CPUT_MAX_PATH];
    WCHAR pSplitExt[CPUT_MAX_PATH];

    errno_t result = _wsplitpath_s(sourceFilename.c_str(), pSplitDrive, CPUT_MAX_PATH, pSplitDirs, CPUT_MAX_PATH, pSplitFile, CPUT_MAX_PATH, pSplitExt, CPUT_MAX_PATH);
    ASSERT( 0 == result, _L("Error splitting path") );
    UNREFERENCED_PARAMETER(result);
    // return the items the user wants
    *pDrive     = pSplitDrive;
    *pDir       = pSplitDirs;
    *pFileName  = pSplitFile;
    *pExtension = pSplitExt;

    return CPUT_SUCCESS;
}

// Split up the supplied path+fileName into its constituent parts
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::SplitPathAndFilename(const std::string &sourceFilename, std::string *pDrive, std::string *pDir, std::string *pFileName, std::string *pExtension)
{
    char pSplitDrive[CPUT_MAX_PATH];
    char pSplitDirs[CPUT_MAX_PATH];
    char pSplitFile[CPUT_MAX_PATH];
    char pSplitExt[CPUT_MAX_PATH];

    errno_t result = _splitpath_s(sourceFilename.c_str(), pSplitDrive, CPUT_MAX_PATH, pSplitDirs, CPUT_MAX_PATH, pSplitFile, CPUT_MAX_PATH, pSplitExt, CPUT_MAX_PATH);
    ASSERTA( 0 == result, "Error splitting path" );

    // return the items the user wants
    *pDrive     = pSplitDrive;
    *pDir       = pSplitDirs;
    *pFileName  = pSplitFile;
    *pExtension = pSplitExt;

    return CPUT_SUCCESS;
}

// Takes a relative/full path+fileName and returns the absolute path with drive
// letter, absolute path, fileName and extension of this file.
// Truncates total path/file length to CPUT_MAX_PATH. Both source and destination may be the same string.
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::ResolveAbsolutePathAndFilename(const std::wstring &fileName, std::wstring *pResolvedPathAndFilename)
{
    WCHAR pFullPathAndFilename[CPUT_MAX_PATH];
    DWORD result = GetFullPathNameW(fileName.c_str(), CPUT_MAX_PATH, pFullPathAndFilename, NULL);
    ASSERT( 0 != result, _L("Error getting full path name") );
    *pResolvedPathAndFilename = pFullPathAndFilename;

    return CPUT_SUCCESS;
}

CPUTResult CPUTFileSystem::ResolveAbsolutePathAndFilename(const std::string &fileName, std::string *pResolvedPathAndFilename)
{
    char pFullPathAndFilename[CPUT_MAX_PATH];
    DWORD result = GetFullPathNameA(fileName.c_str(), CPUT_MAX_PATH, pFullPathAndFilename, NULL);
    ASSERTA( 0 != result, "Error getting full path name" );
    *pResolvedPathAndFilename = pFullPathAndFilename;
    UNREFERENCED_PARAMETER(result);

    return CPUT_SUCCESS;
}

// Verifies that file exists at specified path
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::DoesFileExist(const std::string &pathAndFilename)
{
    // check for file existence
    // attempt to open it where they said it was
    FILE *pFile = NULL;
    errno_t err = fopen_s(&pFile, pathAndFilename.c_str(), "r");
    if(0 == err)
    {
        // yep - file exists
        fclose(pFile);
        return CPUT_SUCCESS;
    }

    // not found, translate the file error and return it
    return TranslateFileError(err);
}

CPUTResult CPUTFileSystem::DoesFileExist(const std::wstring &pathAndFilename)
{
    // check for file existence
    // attempt to open it where they said it was
    FILE *pFile = NULL;
    errno_t err = _wfopen_s(&pFile, pathAndFilename.c_str(), L"r");
    if(0 == err)
    {
        // yep - file exists
        fclose(pFile);
        return CPUT_SUCCESS;
    }

    // not found, translate the file error and return it
    return TranslateFileError(err);
}

// Verifies that directory exists.
// Returns success if the directory exists and is readable (failure may mean
// it's busy or permissions denied on win32)
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::DoesDirectoryExist(const std::wstring &path)
{
    DWORD fileAttribs = GetFileAttributesW(path.c_str());
    ASSERT( INVALID_FILE_ATTRIBUTES != fileAttribs, L"Failed getting file attributes" );

    return CPUT_SUCCESS;
}

CPUTResult CPUTFileSystem::DoesDirectoryExist(const std::string &path)
{
    DWORD fileAttribs = GetFileAttributesA(path.c_str());
    ASSERTA( INVALID_FILE_ATTRIBUTES != fileAttribs, "Failed getting file attributes" );

    return CPUT_SUCCESS;
}

// Open a file and return file pointer to it
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::OpenFile(const std::wstring &fileName, FILE **ppFilePointer)
{
    errno_t err = _wfopen_s(ppFilePointer, fileName.c_str(), _L("r"));

    return TranslateFileError(err);
}

// Open a file and return file pointer to it
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::OpenFile(const std::string &fileName, FILE **ppFilePointer)
{
    errno_t err = fopen_s(ppFilePointer, fileName.c_str(), "r");

    return TranslateFileError(err);
}

// Read the entire contents of a file and return a pointer/size to it
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::ReadFileContents(const std::wstring &fileName, UINT *pSizeInBytes, void **ppData, bool bAddTerminator, bool bLoadAsBinary)
{
    FILE *pFile = NULL;

    errno_t err;
    if (bLoadAsBinary) {
        err = _wfopen_s(&pFile, fileName.c_str(), _L("rb"));
    } else {
        err = _wfopen_s(&pFile, fileName.c_str(), _L("r"));
    }

    if(0 == err)
    {
        // get file size
        fseek(pFile, 0, SEEK_END);
        *pSizeInBytes = ftell(pFile);
        fseek (pFile, 0, SEEK_SET);

        // allocate buffer
        *ppData = (void*) new char[*pSizeInBytes];
        ASSERT( *ppData, _L("Out of memory") );

        // read it all in
        UINT numBytesRead = (UINT) fread(*ppData, sizeof(char), *pSizeInBytes, pFile);
        if (bAddTerminator)
        {
            ((char *)(*ppData))[numBytesRead++] = '\0';
            (*pSizeInBytes)++;
        }
        //fixme - this isn't doing what it appears to be doing. counts off for Windows...
        //ASSERT( numBytesRead == *pSizeInBytes, _L("File read byte count mismatch.") );
        UNREFERENCED_PARAMETER(numBytesRead);

        // close and return
        fclose(pFile);
        return CPUT_SUCCESS;
    }

    // some kind of file error, translate the error code and return it
    return TranslateFileError(err);
}

// Read the entire contents of a file and return a pointer/size to it
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::ReadFileContents(const std::string &fileName, UINT *pSizeInBytes, void **ppData, bool bAddTerminator, bool bLoadAsBinary)
{
    FILE *pFile = NULL;

    errno_t err;
    if (bLoadAsBinary) {
        err = fopen_s(&pFile, fileName.c_str(), "rb");
    } else {
        err = fopen_s(&pFile, fileName.c_str(), "r");
    }

    if(0 == err)
    {
        // get file size
        fseek(pFile, 0, SEEK_END);
        *pSizeInBytes = ftell(pFile);
        fseek (pFile, 0, SEEK_SET);

        // allocate buffer
        *ppData = (void*) new char[*pSizeInBytes];
        ASSERT( *ppData, _L("Out of memory") );

        // read it all in
        UINT numBytesRead = (UINT) fread(*ppData, sizeof(char), *pSizeInBytes, pFile);
        if (bAddTerminator)
        {
            ((char *)(*ppData))[numBytesRead++] = '\0';
            (*pSizeInBytes)++;
        }
        //fixme this is off by something for windows at least.
        //ASSERT( numBytesRead == *pSizeInBytes, _L("File read byte count mismatch.") );
        UNREFERENCED_PARAMETER(numBytesRead);

        // close and return
        fclose(pFile);
        return CPUT_SUCCESS;
    }

    // some kind of file error, translate the error code and return it
    return TranslateFileError(err);
}


// Translate a file operation error code
//-----------------------------------------------------------------------------
CPUTResult CPUTFileSystem::TranslateFileError(errno_t err)
{
    if(0==err)
    {
        return CPUT_SUCCESS;
    }

    // see: http://msdn.microsoft.com/en-us/library/t3ayayh1.aspx
    // for list of all error codes
    CPUTResult result = CPUT_ERROR_FILE_ERROR;

    switch(err)
    {
    case ENOENT: result = CPUT_ERROR_FILE_NOT_FOUND;                 break; // file/dir not found
    case EIO:    result = CPUT_ERROR_FILE_IO_ERROR;                  break;
    case ENXIO:  result = CPUT_ERROR_FILE_NO_SUCH_DEVICE_OR_ADDRESS; break;
    case EBADF:  result = CPUT_ERROR_FILE_BAD_FILE_NUMBER;           break;
    case ENOMEM: result = CPUT_ERROR_FILE_NOT_ENOUGH_MEMORY;         break;
    case EACCES: result = CPUT_ERROR_FILE_PERMISSION_DENIED;         break;
    case EBUSY:  result = CPUT_ERROR_FILE_DEVICE_OR_RESOURCE_BUSY;   break;
    case EEXIST: result = CPUT_ERROR_FILE_EXISTS;                    break;
    case EISDIR: result = CPUT_ERROR_FILE_IS_A_DIRECTORY;            break;
    case ENFILE: result = CPUT_ERROR_FILE_TOO_MANY_OPEN_FILES;       break;
    case EFBIG:  result = CPUT_ERROR_FILE_TOO_LARGE;                 break;
    case ENOSPC: result = CPUT_ERROR_FILE_DEVICE_FULL;               break;
    case ENAMETOOLONG: result = CPUT_ERROR_FILE_FILENAME_TOO_LONG;   break;
    default:
        // unknown file error type - assert so you can add it to the list
        ASSERT(0,_L("Unkown error code"));
    }
    return result;
}

// Open a system dialog box
//-----------------------------------------------------------------------------
CPUTResult CPUTOSServices::OpenMessageBox(std::wstring title, std::wstring text)
{
    ::MessageBoxW(NULL, text.c_str(), title.c_str(), MB_OK);

    return CPUT_SUCCESS;
}

// Open a system dialog box
//-----------------------------------------------------------------------------
CPUTResult CPUTOSServices::OpenMessageBox(std::string title, std::string text)
{
    ::MessageBoxA(NULL, text.c_str(), title.c_str(), MB_OK);

    return CPUT_SUCCESS;
}

void DEBUG_PRINT(const char *pData, ...)
{
    char *sOut;
    va_list args;
    int len;

    va_start(args, pData);
    len = _vscprintf( pData, args ) + 1;

    sOut = (char *) malloc(len * sizeof(char));
    vsprintf_s(sOut, len, pData, args);
    va_end(args);

    OutputDebugStringA(sOut);
    OutputDebugStringA("\n");

    free(sOut);
}

void DEBUG_PRINT(const wchar_t *pData, ...)
{
    wchar_t *sOut;
    va_list args;
    int len;

    va_start(args, pData);
    len = _vscwprintf( pData, args ) + 1;

    sOut = (wchar_t *) malloc(len * sizeof(wchar_t));
    vswprintf_s(sOut, len, pData, args);
    va_end(args);

    OutputDebugStringW(sOut);
    OutputDebugStringW(L"\n");

    free(sOut);
}