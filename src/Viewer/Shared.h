#ifndef _SHARED_H
#define _SHARED_H

#define DEBUG // Comment out to disable debug macro blocks for all files importing Shared.h
#define USE_VR

extern bool changepano;
extern bool remote;
extern int Port;
extern const char *Name;
extern const char *IP;

extern bool fullscreen;
extern bool stereo;
extern bool fivepanel;

extern bool usingVR;

extern bool DEBUG_FLAG;

#endif // _SHARED_H