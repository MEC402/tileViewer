#ifndef _ST_VIEWER_H
#define _ST_VIEWER_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <chrono>
#include <stdlib.h>
#include <thread>
#include <vector>

#include "Annotations.h"
#include "Camera.h"
#include "CubePoints.h"
#include "ImageHandler.h"
#include "ObjLoader.h"
#include "PanoInfo.h"
#include "Shader.h"
#include "Shared.h"
#include "RemoteClient.h"
#include "ThreadPool.hpp"

#include "VR.h"
#include "GraphicalMenu.h"

#include "KinectControl.h"

// A driver class object for use when rendering with ST coordinate modifications as opposed to blitting
class STViewer {

public:
	STViewer(const char* panoURI, bool stereo, bool fivepanel, bool fullscreen, bool borderless,
		int viewWidth, int viewHeight, RemoteClient *remote, KinectControl *kinect);

	enum GUISTATE { PANO, HELP, OFF };


	// Does what it says
	void ToggleStereo(void);

	// Pano Controls
	void NextPano(void);
	void PrevPano(void);
	void ReloadPano(void);
	void SelectPano(int pano);

	// Control which texture set to display (useless in stereo mode)
	void ToggleComparison(void);
	void ToggleEye(int eye);
	void ReloadShaders(void);

	// Camera Controls
	void MoveCamera(float pitchChange, float yawChange, float FOVChange);
	void ResetCamera(void);
	void ToggleExactPixels(void);
	void Screenshot(void);

	// Misc Toggle controls
	void ToggleGUI(GUISTATE state);
	void ToggleLinear(void);
	void ToggleDebug(void);
	void ToggleObj(void);

	bool SetDisplayStates(void);

	// Main program loop
	void Update(double globalTime, float deltaTime);

	// atexit() call
	void Cleanup(void);
	
	std::vector<PanoInfo> GetPanos(void);
	PanoInfo GetCurrentPano();

	GraphicalMenu m_gui;
	float m_guiPanoSelection{ 0 };
	double m_lastUIInteractionTime{ 0.0 };
	Annotations m_annotations;

	GUISTATE m_guistate{ OFF };
	bool m_displayAnnotation{ false };
	float m_selectedPano;

#ifdef DEBUG
	void RebindVAO(void);
#endif

private:
	//----------------------------------------------//
	//				Private Functions				//
	//----------------------------------------------//

	/*-------------- OpenGL Builders --------------*/
	void initGL(void);
	void initVR(void);
	void initTextures(void);


	/*------------ Quad/Image Handlers ------------*/
	void resetImages(void);
	void resetCubes(void);


	/*----------- Display/Draw Functions -----------*/
	void displayAnnotations(void);
	void displayGUI(void);
	void displayObjs(void);

	//----------------------------------------------//
	//				Private Variables				//
	//----------------------------------------------//

	// Geometry data
	CubePoints *m_LeftEye;
	CubePoints *m_RightEye;
	GLsizei m_pointCount;

	// Pano data
	std::vector<PanoInfo> m_panolist;
	int m_currentPano{ 0 };
	
	bool m_displayObj{ false };
	Shader m_objshader;
	ObjLoader m_objloader;
	ObjLoader::ObjData m_objdata;

	Shader m_shader;
	Shader m_objectShader;
	Camera m_camera;
	ImageHandler m_images;

	// Thread pool data
	Threads::ThreadPool *downloadPool;
	Threads::ThreadPool *texturePool;	// Pool for dumping texture load requests into
	Threads::ThreadPool *workerPool;	// Helper thread that we use for menial tasks so main thread doesn't leave GL context too much
	
	bool m_stereo{ false };
	bool m_fivepanel{ false };
	bool m_fullscreen{ false };
	bool m_borderless{ false };
	bool m_linear{ true };
	bool m_comparisonMode{ false };
	bool m_exactpixelmode{ false };

	SafeQueue<ImageData*> *m_LoadedTextures;

	bool m_usingVR{ false };
	VRDevice m_vr;

	RemoteClient *m_remote;
	KinectControl *m_kinect;

	// Magic number for maximum depth (0 indexed)
	int m_maxDepth{ 3 };

#ifdef DEBUG
	// Local face date
	int m_facecount[2][6];
	std::vector<float> m_average;
	// Timer values (declared ahead of time so we can reference it in other functions with macro NOW defines
	std::chrono::high_resolution_clock::time_point t1;
#endif // DEBUG
};

#endif // _ST_VIEWER_H