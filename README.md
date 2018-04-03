# tileViewer
An OpenGL program for displaying tiled imagery from a remote URL on Large Format Displays


# Project Set-up

## Windows
In the TileViewer Folder that contains TileViewer.sln (SolutionFolder)
Create three folders, "dll", "include", "lib"
Inside of the "include" directory, create a folder named "GL"

Download glew version 2.1.0 binaries for Windows from glew.sourceforge.net
Extract the glew folder and copy glew32.dll to the folder (SolutionFolder)\dll
Copy glew32.lib to the folder (SolutionFolder)\lib
Copy the contents of the GL folder to (SolutionFolder)\include\GL

Download freeglut version -
do the same