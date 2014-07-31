Clustered Shading on Anrdoid


Build Requirements
==================
Install the latest Android SDK and NDK.
    http://developer.android.com/sdk/index.html
    http://developer.android.com/tools/sdk/ndk/index.html

Add the NDK and SDK to your path :
    export PATH=$ANDROID_NDK/:$ANDROID_SDK/tools/:$PATH

To Build:

1. cd to ClusteredShadingAndroid\CPUT forlder

2. Build the CPUT:
    ndk-build

3. cd to ClusteredShadingAndroid folder

4. First time only, you may need to initialize your project :
    android update project -p .

5. Build the NDK component :
    ndk-build
    
6. Build the APK
    ant debug
    
7. Install the APK
    adb install -r ./bin/NativeActivity-debug.apk

8. Run it.
