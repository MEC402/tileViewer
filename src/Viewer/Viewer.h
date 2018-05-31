#ifndef VIEWER_H
#define VIEWER_h

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "Camera.h"
#include "Controls.h"
#include "CubePoints.h"
#include "GLhandles.h"
#include "ShaderHelper.h"
#include "ImageHandler.h"
#include "ImageQueue.h"
#include "VR.h"
#include "ThreadPool.hpp"
#include "ThreadSafeQueue.hpp"

//------------------------------------------------------//
//				Begin Function List						//
//------------------------------------------------------//

// Misc for displaying debug messages /////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void showInfo();
void LoadFace(int face, int eye);

// GLUT Callback Functions //////////////////////////////////////////////////////////////////
void poolhandler1();
void poolhandler2();
void display();
void timerFunc(int value);
void idleFunc(void);

void resetImages();

//------------------------------------------------------//
//				Begin Variable List						//
//------------------------------------------------------//

// Command line flags /////////////////////////
extern bool fullscreen;
extern bool stereo;
extern bool fivepanel;

// Pano List /////////////////////////////////
extern std::queue<char*> panolist;

// Geometry data /////////////////////////////
extern CubePoints *LeftEye;
extern CubePoints *RightEye;

// VR Flag
extern bool usingVR;

// Local record of quad depth data /////////
extern int facedepths[2][6];
extern int facecount[2][6];

#endif