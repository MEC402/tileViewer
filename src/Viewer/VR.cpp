#include "VR.h"

// Helpers
struct OculusTextureBuffer
{
	ovrSession          Session;
	ovrTextureSwapChain ColorTextureChain;
	ovrTextureSwapChain DepthTextureChain;
	GLuint              fboId;
	OVR::Sizei          texSize;

	OculusTextureBuffer(ovrSession session, OVR::Sizei size, int sampleCount);
	~OculusTextureBuffer();
	OVR::Sizei GetSize() const;
	void SetRenderSurface();
	void UnsetRenderSurface();
	void Commit();
};
ovrGraphicsLuid GetDefaultAdapterLuid();
int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs);

OculusTextureBuffer::OculusTextureBuffer(ovrSession session, OVR::Sizei size, int sampleCount) :
	Session(session),
	ColorTextureChain(nullptr),
	DepthTextureChain(nullptr),
	fboId(0),
	texSize(0, 0)
{
	assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

	texSize = size;

	// This texture isn't necessarily going to be a rendertarget, but it usually is.
	assert(session); // No HMD? A little odd.

	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = size.w;
	desc.Height = size.h;
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = sampleCount;
	desc.StaticImage = ovrFalse;

	{
		ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &ColorTextureChain);

		int length = 0;
		ovr_GetTextureSwapChainLength(session, ColorTextureChain, &length);

		if (OVR_SUCCESS(result))
		{
			for (int i = 0; i < length; ++i)
			{
				GLuint chainTexId;
				ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain, i, &chainTexId);
				glBindTexture(GL_TEXTURE_2D, chainTexId);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}

	desc.Format = OVR_FORMAT_D32_FLOAT;

	{
		ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &DepthTextureChain);

		int length = 0;
		ovr_GetTextureSwapChainLength(session, DepthTextureChain, &length);

		if (OVR_SUCCESS(result))
		{
			for (int i = 0; i < length; ++i)
			{
				GLuint chainTexId;
				ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain, i, &chainTexId);
				glBindTexture(GL_TEXTURE_2D, chainTexId);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}

	glGenFramebuffers(1, &fboId);
}

OculusTextureBuffer::~OculusTextureBuffer()
{
	if (ColorTextureChain)
	{
		ovr_DestroyTextureSwapChain(Session, ColorTextureChain);
		ColorTextureChain = nullptr;
	}
	if (DepthTextureChain)
	{
		ovr_DestroyTextureSwapChain(Session, DepthTextureChain);
		DepthTextureChain = nullptr;
	}
	if (fboId)
	{
		glDeleteFramebuffers(1, &fboId);
		fboId = 0;
	}
}

OVR::Sizei OculusTextureBuffer::GetSize() const
{
	return texSize;
}

void OculusTextureBuffer::SetRenderSurface()
{
	GLuint curColorTexId;
	GLuint curDepthTexId;
	{
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(Session, ColorTextureChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain, curIndex, &curColorTexId);
	}
	{
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(Session, DepthTextureChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain, curIndex, &curDepthTexId);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curColorTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, curDepthTexId, 0);

	glViewport(0, 0, texSize.w, texSize.h);
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void OculusTextureBuffer::UnsetRenderSurface()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

void OculusTextureBuffer::Commit()
{
	ovr_CommitTextureSwapChain(Session, ColorTextureChain);
	ovr_CommitTextureSwapChain(Session, DepthTextureChain);
}

ovrGraphicsLuid GetDefaultAdapterLuid()
{
	ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
	IDXGIFactory* factory = nullptr;

	if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
	{
		IDXGIAdapter* adapter = nullptr;

		if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
		{
			DXGI_ADAPTER_DESC desc;

			adapter->GetDesc(&desc);
			memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
			adapter->Release();
		}

		factory->Release();
	}
#endif

	return luid;
}


int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
{
	return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
}

bool createVRDevice(VRDevice* out_vr, int mirrorWindowWidth, int mirrorWindowHeight)
{
	*out_vr = { 0 };
	out_vr->frameIndex = 0;

	// Initializes LibOVR, and the Rift
	ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	if (!OVR_SUCCESS(result)) {
		return false;
	}

	ovrGraphicsLuid luid;
	result = ovr_Create(&out_vr->session, &luid);
	if (!OVR_SUCCESS(result)) {
		return false;
	}
	if (Compare(luid, GetDefaultAdapterLuid())) // If luid that the Rift is on is not the default adapter LUID...
	{
		printf("OpenGL supports only the default graphics adapter.");
		return false;
	}

	out_vr->hmdDesc = ovr_GetHmdDesc(out_vr->session);

	GLuint fboId;
	glGenFramebuffers(1, &fboId);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	// Make eye render buffers
	for (int eye = 0; eye < 2; ++eye)
	{
		ovrSizei idealTextureSize = ovr_GetFovTextureSize(out_vr->session, ovrEyeType(eye), out_vr->hmdDesc.DefaultEyeFov[eye], 1);
		out_vr->eyeRenderTexture[eye] = new OculusTextureBuffer(out_vr->session, idealTextureSize, 1);

		if (!out_vr->eyeRenderTexture[eye]->ColorTextureChain || !out_vr->eyeRenderTexture[eye]->DepthTextureChain)
		{
			printf("Failed to create texture.");
			return false;
		}
	}

	glGenFramebuffers(1, &out_vr->mirrorFBO);
	resizeMirrorTexture(out_vr, mirrorWindowWidth, mirrorWindowHeight);

	// Turn off vsync to let the compositor do its magic
	//wglSwapIntervalEXT(0); //TODO: Might be important!

	ovr_SetTrackingOriginType(out_vr->session, ovrTrackingOrigin_FloorLevel);

	return true;
}

void resizeMirrorTexture(VRDevice* vr, int mirrorWindowWidth, int mirrorWindowHeight)
{
	if (vr->mirrorTexture) {
		ovr_DestroyMirrorTexture(vr->session, vr->mirrorTexture);
	}

	vr->mirrorWindowWidth = mirrorWindowWidth;
	vr->mirrorWindowHeight = mirrorWindowHeight;

	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = vr->mirrorWindowWidth;
	desc.Height = vr->mirrorWindowHeight;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	// Create mirror texture and an FBO used to copy mirror texture to back buffer
	vr->mirrorTexture = nullptr;
	ovrResult result = ovr_CreateMirrorTextureWithOptionsGL(vr->session, &desc, &vr->mirrorTexture);
	if (!OVR_SUCCESS(result))
	{
		printf("Failed to create mirror texture.");
	}

	// Configure the mirror read buffer
	GLuint texId;
	ovr_GetMirrorTextureBufferGL(vr->session, vr->mirrorTexture, &texId);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, vr->mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		const char* errString = (char*)gluErrorString(err);
		printf(errString);
	}
}

void updateVRDevice(VRDevice* vr)
{
	ovrSessionStatus sessionStatus;
	ovr_GetSessionStatus(vr->session, &sessionStatus);
	if (sessionStatus.ShouldRecenter)
		ovr_RecenterTrackingOrigin(vr->session);

	// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
	vr->eyeRenderDesc[0] = ovr_GetRenderDesc(vr->session, ovrEye_Left, vr->hmdDesc.DefaultEyeFov[0]);
	vr->eyeRenderDesc[1] = ovr_GetRenderDesc(vr->session, ovrEye_Right, vr->hmdDesc.DefaultEyeFov[1]);

	// Get eye poses, feeding in correct IPD offset
	ovrPosef HmdToEyePose[2] = { vr->eyeRenderDesc[0].HmdToEyePose,
		vr->eyeRenderDesc[1].HmdToEyePose };

	ovrPosef useHmdToEyePose[2] = { vr->eyeRenderDesc[0].HmdToEyePose,
		vr->eyeRenderDesc[1].HmdToEyePose };

	double ftiming = ovr_GetPredictedDisplayTime(vr->session, 0);
	vr->trackingState = ovr_GetTrackingState(vr->session, ftiming, ovrTrue);
	ovr_CalcEyePoses(vr->trackingState.HeadPose.ThePose, useHmdToEyePose, vr->eyeRenderPose);
}

OVR::Matrix4f buildVRViewMatrix(VRDevice* vr, int eyeIndex, float cameraX, float cameraY, float cameraZ)
{
	OVR::Vector3f cameraPosition = OVR::Vector3f(cameraX, cameraY, cameraZ);
	OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationY(0);
	OVR::Matrix4f finalRollPitchYaw = rollPitchYaw * OVR::Matrix4f(vr->eyeRenderPose[eyeIndex].Orientation);
	OVR::Vector3f finalUp = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
	OVR::Vector3f finalForward = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
	OVR::Vector3f shiftedEyePos = cameraPosition + rollPitchYaw.Transform(vr->eyeRenderPose[eyeIndex].Position);

	OVR::Matrix4f view = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
	return view;
}

OVR::Matrix4f buildVRProjectionMatrix(VRDevice* vr, int eyeIndex)
{
	OVR::Matrix4f proj = ovrMatrix4f_Projection(vr->hmdDesc.DefaultEyeFov[eyeIndex], 0.2f, 1000.0f, ovrProjection_None);
	vr->posTimewarpProjectionDesc = ovrTimewarpProjectionDesc_FromProjection(proj, ovrProjection_None);
	return proj;
}

OVR::Vector3f getVRHeadsetPosition(VRDevice* vr)
{
	OVR::Vector3f leftEye = vr->eyeRenderPose[0].Position;
	OVR::Vector3f rightEye = vr->eyeRenderPose[1].Position;
	OVR::Vector3f betweenEyes = leftEye + ((rightEye - leftEye) / 2);
	return betweenEyes;
}

VRControllerStates getVRControllerState(VRDevice* vr)
{
	VRControllerStates controllers;

	ovrPosef* pose = &vr->trackingState.HandPoses[ovrHand_Right].ThePose;
	controllers.right.position = OVR::Vector3f(pose->Position.x, pose->Position.y, pose->Position.z);
	controllers.right.rotation = OVR::Quatf(pose->Orientation.x, pose->Orientation.y, pose->Orientation.z, pose->Orientation.w);

	pose = &vr->trackingState.HandPoses[ovrHand_Left].ThePose;
	controllers.left.position = OVR::Vector3f(pose->Position.x, pose->Position.y, pose->Position.z);
	controllers.left.rotation = OVR::Quatf(pose->Orientation.x, pose->Orientation.y, pose->Orientation.z, pose->Orientation.w);

	return controllers;
}

void bindEyeRenderSurface(VRDevice* vr, int eyeIndex)
{
	// Switch to eye render target
	vr->eyeRenderTexture[eyeIndex]->SetRenderSurface();
}

void commitEyeRenderSurface(VRDevice* vr, int eyeIndex)
{
	vr->eyeRenderTexture[eyeIndex]->UnsetRenderSurface();
	// Commit changes to the textures so they get picked up frame
	vr->eyeRenderTexture[eyeIndex]->Commit();
}

void finishVRFrame(VRDevice* vr)
{
	ovrLayerEyeFovDepth ld = {};
	ld.Header.Type = ovrLayerType_EyeFovDepth;
	ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
	ld.ProjectionDesc = vr->posTimewarpProjectionDesc;

	for (int eye = 0; eye < 2; ++eye)
	{
		ld.ColorTexture[eye] = vr->eyeRenderTexture[eye]->ColorTextureChain;
		ld.DepthTexture[eye] = vr->eyeRenderTexture[eye]->DepthTextureChain;
		ld.Viewport[eye] = OVR::Recti(vr->eyeRenderTexture[eye]->GetSize());
		ld.Fov[eye] = vr->hmdDesc.DefaultEyeFov[eye];
		ld.RenderPose[eye] = vr->eyeRenderPose[eye];
		ld.SensorSampleTime = vr->sensorSampleTime;
	}
	ovrLayerHeader* layers = &ld.Header;
	ovrResult result = ovr_SubmitFrame(vr->session, vr->frameIndex, nullptr, &layers, 1);
}

void blitHeadsetView(VRDevice* vr, GLuint mirrorDisplayFramebuffer)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, vr->mirrorFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mirrorDisplayFramebuffer);
	GLint w = vr->mirrorWindowWidth;
	GLint h = vr->mirrorWindowHeight;
	glBlitFramebuffer(0, h, w, 0,
		0, 0, w, h,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}


void destroyVRDevice(VRDevice* vr)
{
	ovr_Shutdown();
}