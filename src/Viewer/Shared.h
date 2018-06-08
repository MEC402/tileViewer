#ifndef _SHARED_H
#define _SHARED_H

#include "CubePoints.h"

#define DEBUG // Comment out to disable debug macro blocks for all files importing Shared.h
#ifdef _VR_H
#define USE_VR
#endif // _VR_H

extern bool fullscreen;
extern bool stereo;
extern bool fivepanel;

extern bool usingVR;

extern bool DEBUG_FLAG;

#endif // _SHARED_H