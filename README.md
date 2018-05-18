# tileViewer
An OpenGL program for displaying tiled imagery from a remote URL on Large Format Displays


# Project Set-up

## Windows
Using the Visual Studio Installer install the Window SDK version 10.0.16299.0

### Installing libraries with Nuget:
Open visual studio and install:
freeglut
glew
glm

### To install libraries manually:

In the TileViewer Folder that contains TileViewer.sln (SolutionFolder)
Create three folders, "dll", "include", "lib"
Inside of the "include" directory, create a folder named "GL"

Download glew version 2.1.0 binaries for Windows from glew.sourceforge.net
Extract the download glew zip file
Open the extracted folder and copy bin/Release/Win32/glew32.dll to the folder (SolutionFolder)/dll
Copy lib/Release/Win32/glew32.lib to the folder (SolutionFolder)/lib
Copy the contents of include/GL to (SolutionFolder)/include/GL

Download the latest freeglut version
Extract the downloaded zip file
Open the extracted folder and copy freeglut.dll to the folder (SolutionFolder)/dll
Copy freeglut.lib to the folder (SolutionFolder)/lib
Copy the contents of include/GL to (SolutionFolder)/include/GL

Make sure your build settings are set to Debug - Win32 (x86).  
We're using 32bit GL libraries and the compiler gets mad otherwise.