#ifndef VIEWER_H
#define VIEWER_h

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <vector>


#include "CubePoints.h"


// This header file mostly exists so external classes like Controls.cpp can call functions that rely on state flags or other things in Viewer
// It is not meant to be a comprehensive header file for Viewer.cpp in full, as we only want to expose certain variables as necessary

//------------------------------------------------------//
//				Begin Function List						//
//------------------------------------------------------//

// Misc for displaying debug messages /////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void showInfo();

// GLUT Callback Functions ////////////////////////////////
void poolhandler1();
void poolhandler2();
void display();
void timerFunc(int value);
void idleFunc(void);

// Toggle Stereo state ////////////////////////////////////
void ToggleStereo();

// Pano handler functions /////////////////////////////////
void NextPano();
void PrevPano();
void ReloadPano();

// Cube mapping functions /////////////////////////////////
void resetImages();
void resetCubes();


//------------------------------------------------------//
//				Begin Variable List						//
//------------------------------------------------------//

// Command line flags /////////////////////////
extern bool fullscreen;
extern bool stereo;
extern bool fivepanel;

// Pano List /////////////////////////////////
//extern std::vector<char*> panolist;

// Geometry data /////////////////////////////
extern CubePoints *LeftEye;
extern CubePoints *RightEye;

// VR Flag
extern bool usingVR;

// Local record of quad depth data /////////
extern int facedepths[2][6];
extern int facecount[2][6];

#endif