#ifndef _ST_VIEWER_H
#define _ST_VIEWER_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <chrono>
#include <stdlib.h>
#include <thread>
#include <vector>

#include "CubePoints.h"
#include "ImageQueue.h"
#include "PanoInfo.h"
#include "Shared.h"
#include "ThreadPool.hpp"
#include "Shader.h"
#include "Camera.h"
#include "ImageHandler.h"

#include "VR.h"


// I really didn't want to make this static, but GL callbacks require non-member functions
// Which left us two choices, have the main object class be non-static and have some secondary, static h file
// with all the callback routines
// Or just make the whole thing static

// A driver class object for use when rendering with ST coordinate modifications as opposed to blitting
class STViewer {

public:
	static void Init(const char* panoFileAddress, bool stereo, bool fullscreen, bool fivepanel);

	/*		Viewer-Driven Stereo Function		*/
	/*	  Necessary because of Eye geometry		*/
	static void ToggleStereo(void);

	/*			Panorama Handlers				*/
	static void NextPano(void);
	static void PrevPano(void);
	static void ReloadPano(void);
	static void SelectPano(int pano);

	static void reloadShaders();
	static void moveCamera(float pitchChange, float yawChange, float FOVChange);

#ifdef DEBUG
	static void PrintAverage(void);
	static void RebindVAO(void);
#endif

private:
	//----------------------------------------------//
	//				Private Functions				//
	//----------------------------------------------//

	/*			GLUT Callback Functions			*/
	static void display(void);
	static void idleFunc(void);
	static void resizeFunc(int w, int h);
	static void timerFunc(int value);

	/*					Builders				*/
	static void initWindowAndGL(void);
	static void initTextures(void);
	static void initCallbacks(void);
	static void initMenus(void);


	/*		For resetting Cube Depths and		*/
	/*		loading the next panorama in		*/
	static void resetImages(void);
	static void resetCubes(void);

	/*		For queueing texture load requests	*/
	static void loadAllFaceDepths(void);
	static void loadAllQuadDepths(void);

	/*				Cleanup function			*/
	static void cleanup(void);

	/*			CURL Download Cleanup Timer		*/
	static void timerCleanup(int value);

	//----------------------------------------------//
	//				Private Variables				//
	//----------------------------------------------//
	// Geometry data
	static CubePoints *m_LeftEye;
	static CubePoints *m_RightEye;
	static GLsizei m_pointCount;

	// Pano data
	static std::vector<PanoInfo> m_panolist;
	static unsigned int m_currentPano;

	static Shader m_shader;
	static Camera m_camera;
	static ImageHandler m_images;

	// Thread pool data
	static Threads::ThreadPool *texturePool;	// Pool for dumping texture load requests into
	static Threads::ThreadPool *workerPool;	// Helper thread that we use for menial tasks so main thread doesn't leave GL context too much
	
	// State flags so we don't spawn multiple threads to collect ThreadPool promises
	static bool textureHandling;
	static bool workerHandling;
	static bool imagesNeedResetting;
	static bool m_stereo;
	static bool m_fullscreen;
	static bool m_fivepanel;

	// ImageQueue is no longer static, so keep a reference to one we can instantiate
	static ImageQueue *m_LoadedTextures;

	static bool m_usingVR;
	static VRDevice m_vr;

	// Magic number for maximum depth (0 indexed)
	static int m_maxDepth;

#ifdef DEBUG
	// Local face date
	static int m_facecount[2][6];
	static std::vector<float> m_average;
	// Timer values (declared ahead of time so we can reference it in other functions with macro NOW defines
	static std::chrono::high_resolution_clock::time_point t1;
#endif // DEBUG
};

#endif // _ST_VIEWER_H