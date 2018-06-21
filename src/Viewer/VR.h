#ifndef _VR_H
#define _VR_H

#include <assert.h>
#include <GL/glew.h>
#include "Shared.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#ifdef OCULUS
#include "LibOVRKernel/Kernel/OVR_Types.h"
#include "LibOVR/OVR_CAPI_GL.h"
#include "LibOVR/Extras/OVR_Math.h"
#endif // OCULUS

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

struct VRControllerStates
{
	struct Button {
		bool pressed; // true for one update when the button is first pressed
		bool down; // true as long as the button is held down
	};

	struct Controller {
		glm::vec3 position;
		glm::quat rotation;
		Button button1;
		Button button2;
		float thumbstickX;
		float thumbstickY;
		float indexFingerTrigger;
		float middleFingerTrigger;
		Button thumbstickTouch;
	};

	Controller left;
	Controller right;
};

struct OculusTextureBuffer;

struct VRDevice
{
#ifdef OCULUS
	OculusTextureBuffer * eyeRenderTexture[2];
	ovrPosef eyeRenderPose[2];
	ovrEyeRenderDesc eyeRenderDesc[2];
	ovrSession session;
	ovrTrackingState trackingState;
	ovrHmdDesc hmdDesc;
	GLuint mirrorFBO;
	ovrMirrorTexture mirrorTexture;
	long long frameIndex;
	double sensorSampleTime;
	ovrTimewarpProjectionDesc posTimewarpProjectionDesc;
	int mirrorWindowWidth;
	int mirrorWindowHeight;
	VRControllerStates controllers;
#endif // OCULUS
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
glm::mat4x4 buildVRViewMatrix(VRDevice* vr, int eyeIndex, float cameraX, float cameraY, float cameraZ);
glm::mat4x4 buildVRProjectionMatrix(VRDevice* vr, int eyeIndex);
glm::mat4x4 buildVROrthoMatrix(VRDevice* vr, int eyeIndex, float orthoDistance);
glm::vec3 getVRHeadsetPosition(VRDevice* vr);
glm::quat getVRHeadsetRotation(VRDevice* vr);
glm::vec3 getVREyeDistance(VRDevice* vr);

// Gets the state of Oculus Touch controllers. You can access the returned data directly.
VRControllerStates getVRControllerState(VRDevice* vr);

// Call before rendering the scene.
void bindEyeRenderSurface(VRDevice* vr, int eyeIndex);

// Call after rendering the scene.
void commitEyeRenderSurface(VRDevice* vr, int eyeIndex);

// Call after both eyes have been rendered.
void finishVRFrame(VRDevice* vr);

// Allows you to see what the person wearing the headset sees.
// mirrorDisplayFramebuffer: The headset view will be copied to this framebuffer (just enter 0 for the main window).
void blitHeadsetView(VRDevice* vr, GLuint mirrorDisplayFramebuffer);

// TODO: leaking memory. Need to destroy the textures and stuff.
void destroyVRDevice(VRDevice* vr);

#endif // _VR_H