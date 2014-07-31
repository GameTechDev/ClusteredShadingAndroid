Intel Corporation - Clustered Shading on Android

This sample demonstrates several techniques for shading a scene with many lights

Note: “Media Elements” are the images, clip art, animations, sounds, music, shapes, video clips, 2D Images, 2D and 3D Mesh’s and mesh data, animation and animation data, and Textures included in the software. This license does not grant you any rights in the Media Elements and you may not reproduce, prepare derivative works, distribute, publicly display, or publicly perform the Media Elements.

Note: The source code sample is provided under the BSD license.  See the license within the sample source directory for additional details.


Build Requirements
==================
Install the latest Android SDK and NDK.
    http://developer.android.com/sdk/index.html
    http://developer.android.com/tools/sdk/ndk/index.html

Add the NDK and SDK to your path :
    export PATH=$ANDROID_NDK/:$ANDROID_SDK/tools/:$PATH

To Build:

1. cd to ClusteredShadingAndroid\CPUT folder

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
