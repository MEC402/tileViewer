# TileViewer
An OpenGL program for displaying tiled imagery from a remote URL on Large Format Displays


# Project Set-up

## Windows
Most of the following dependencies will come together as part of the library package, but for completeness sake they have been listed individually in full.

Linux support is possible, but the appropriate libraries need to be found and a handful of WIN32 API calls will need replacements.

### DLL Dependencies
- Windows SDK 10.0.16299.0 (Visual Studio Installer method recommended)
- [freeglut.dll](http://freeglut.sourceforge.net/index.php#download) >= 3.0.0.0
- [glew32.gll](http://glew.sourceforge.net/) >= 2.1.0.0
- [glm](https://github.com/g-truc/glm) >= 0.9.4 (Nuget method recommended)
- [libcurl.dll](https://curl.haxx.se/download.html) >= 7.60.0
- [libssl-1_1.dll and libcrypto-1_1.dll](http://wiki.overbyte.eu/wiki/index.php/ICS_Download) >= 1.1.0h
- [wkhtmltox.dll](https://wkhtmltopdf.org/) >= 0.12.5.0

### .lib Dependencies
- freeglut.lib
- glew32.lib
- glew32s.lib
- libcurl.dll.a
- LibOVR.lib (Oculus VR Only)
- wkhtmltox.lib

### Library Include Dependencies
- [RapidJSON](https://github.com/Tencent/rapidjson)
- [CURL Headers](https://curl.haxx.se/download.html)
- [wkhtmltox Headers](https://wkhtmltopdf.org/)
- Freeglut, glew, and etc OpenGL related headers

### Oculus VR Support
To build with Oculus VR Support, you will also need 
- [Oculus VR SDK, LibOVR Headers, and LibOVRKernel Headers](https://developer.oculus.com/downloads/package/oculus-sdk-for-windows/)
To build without Oculus VR Support, simply open Shared.h and comment out the "#define VR" line.

### Kinect Motion Support
To build with Oculus VR Support, you will need
- Kinect V2 (Not tested with V1)
- Kinect V2.0 SDK
To build without Kinect Motion Support, simply open Shared.h and comment out the "#define KINECT" line.


### Visual Studio Setup

Place all DLLs, .lib files, and header libraries into convenient directories.
Open the .sln file, enter Viewer Properties, and:
1. General
- Make sure Windows SDK Version is set correctly
- Make sure Character Set is set to "Use Unicode Character Set"
2. VC++ Directories
- Whatever path you put all the necessary headers into (ex: $(ProjectDir)\include )
- Whatever path you put all the necessary .lib files into (ex: $ProjectDir\lib )
3. C/C++ -> Precompiled Headers
- Make sure "Precompiled Header" is set to "Not Using Precompiled Headers"
4. Linker -> Input | The Following should be added to "Additional Dependencies"
- opengl32.lib
- glew32.lib 
- freeglut.lib 
- libcurl.dll.a 
- wkhtmltox.lib
- LibOVR.lib (If compiling with Oculus Support)
- Kinect20.lib (If compiling with Kinect Support)
- Kinect20.Fusion.lib (If compiling with Kinect Support)

This project was compiled using 32bit libraries, so make sure Visual Studio is set to compile for Win32/x86.

# Using the TileViewer
From the repo, there should be three sets of shaders:
- gui.frag
- gui.vert
- obj.frag
- obj.vert
- Shader.frag
- Shader.vert
- Shader.geom
Place these inside the same directory as the compiled executable.

Run the executable from a command prompt, possible flags are:

| Name | Flag | Parameters |
| ---- | ---- | ---------- |
| Start Fullscreen | -f | None |
| Start Stereoscopic | -s | None |
| Run in 5-Panel Powerwall Mode | -5 | None |
| Run in Borderless Window | -b | \<Width\> \<Height\> |
| Use Kinect | -k | None |
| Enable Remote* | -r | \<Server IP\> \<Server Port\> [Machine Name [u\|d\|l\|r]] |
| Synchronized/Distributed Viewing** | -sv | \<Host IP\> \<Port\> [Machine Name [-d]] |

* Enabling the Remote will connect the TileViewer to either: 
- An HTTP Web Server to remotely control the Viewer, change Panorama source URIs, etc
- Another instance of the TileViewer running in Distributed Viewing mode
The optional [u|d|l|r] flags are meant to indicate this Viewers relative position to the Host Viewer, in terms of direction and screen count.

EX: u1r2 would mean "Up one screen length, right two screen lengths".

The intent is to enable distributed viewing across large panel display setups that may not be run by a single device.

** Synchronized Viewing mode sets this instance of the TileViewer to relay display information to another instance in a 1:1 relationship.

** Distributed Viewing mode sets this instance of the TileViewer to relay offset display information to other instances to create multi-panel display effects.

Both of these modes are "Hosts".  They should be connected to by other instances launched with the Remote flag enabled.