#ifndef VR_H
#define VR_H

#include <assert.h>
#include <GL/glew.h>
#include "LibOVRKernel/Kernel/OVR_Types.h"
#include "LibOVR/OVR_CAPI_GL.h"
#include "LibOVR/Extras/OVR_Math.h"

#if defined(_WIN32)
    #include <dxgi.h> // for GetDefaultAdapterLuid
    #pragma comment(lib, "dxgi.lib")
#endif


struct OculusTextureBuffer;

struct VRDevice
{
	OculusTextureBuffer * eyeRenderTexture[2];
	ovrPosef EyeRenderPose[2];
	ovrSession session;
	ovrHmdDesc hmdDesc;
	GLuint mirrorFBO;
	ovrMirrorTexture mirrorTexture;
	long long frameIndex;
	double sensorSampleTime;
	ovrTimewarpProjectionDesc posTimewarpProjectionDesc;
	int mirrorWindowWidth;
	int mirrorWindowHeight;
};

// Call once to initialize
// The 'mirror' is the image on a PC showing what someone wearing the headset can see.
bool createVRDevice(VRDevice* out_vr, int mirrorWindowWidth, int mirrorWindowHeight);

// Call if the window has been resized.
void resizeMirrorTexture(VRDevice* vr, int mirrorWindowWidth, int mirrorWindowHeight);

// Call once at the beginning of each frame.
void updateVRDevice(VRDevice* vr);

// Gets the view and projection matrices for a headset eye to be passed to OpenGL.
// These may be transposed differently than another math library!
// EyeIndex can be 0 and 1.
OVR::Matrix4f buildVRViewMatrix(VRDevice* vr, int eyeIndex, float cameraX, float cameraY, float cameraZ);
OVR::Matrix4f buildVRProjectionMatrix(VRDevice* vr, int eyeIndex);

// Call before rendering the scene.
void bindEyeRenderSurface(VRDevice* vr, int eyeIndex);

// Call after rendering the scene.
void commitEyeRenderSurface(VRDevice* vr, int eyeIndex);

// Call after both eyes have been rendered.
void finishVRFrame(VRDevice* vr);

// Allows you to see what the person wearing the headset sees.
// mirrorDisplayFramebuffer: The headset view will be copied to this framebuffer (just enter 0 for the main window).
void blitHeadsetView(VRDevice* vr, GLuint mirrorDisplayFramebuffer);

// TODO: probably leaking memory. Need to destroy the textures and stuff.
void destroyVRDevice(VRDevice* vr);

#endif