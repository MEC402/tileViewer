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


// A driver class object for use when rendering with ST coordinate modifications as opposed to blitting
class STViewer {

public:
	void Init(const char* panoFileAddress, bool stereo, bool fivepanel, int viewportWidth, int viewportHeight);

	/*		Viewer-Driven Stereo Function		*/
	/*	  Necessary because of Eye geometry		*/
	void ToggleStereo(void);

	/*			Panorama Handlers				*/
	void NextPano(void);
	void PrevPano(void);
	void ReloadPano(void);
	void SelectPano(int pano);

	void reloadShaders();
	void moveCamera(float pitchChange, float yawChange, float FOVChange);

	void display();
	void update();
	void resize(int w, int h);
	void timer(int value);
	void cleanup();

	std::vector<PanoInfo> getPanoList();

#ifdef DEBUG
	void PrintAverage(void);
	void RebindVAO(void);
#endif

private:
	//----------------------------------------------//
	//				Private Functions				//
	//----------------------------------------------//

	/*			GLUT Callback Functions			*/

	/*					Builders				*/
	void initWindowAndGL(void);
	void initTextures(void);


	/*		For resetting Cube Depths and		*/
	/*		loading the next panorama in		*/
	void resetImages(void);
	void resetCubes(void);

	/*		For queueing texture load requests	*/
	void loadAllFaceDepths(void);
	void loadAllQuadDepths(void);

	/*			CURL Download Cleanup Timer		*/
	void timerCleanup(int value);

	//----------------------------------------------//
	//				Private Variables				//
	//----------------------------------------------//
	// Geometry data
	CubePoints *m_LeftEye;
	CubePoints *m_RightEye;
	GLsizei m_pointCount;

	// Pano data
	std::vector<PanoInfo> m_panolist;
	unsigned int m_currentPano;

	Shader m_shader;
	Camera m_camera;
	ImageHandler m_images;

	// Thread pool data
	Threads::ThreadPool *texturePool;	// Pool for dumping texture load requests into
	Threads::ThreadPool *workerPool;	// Helper thread that we use for menial tasks so main thread doesn't leave GL context too much
	
	// State flags so we don't spawn multiple threads to collect ThreadPool promises
	bool textureHandling;
	bool workerHandling;
	bool imagesNeedResetting;
	bool m_stereo;
	bool m_fivepanel;

	ImageQueue *m_LoadedTextures;

	bool m_usingVR;
	VRDevice m_vr;

	// Magic number for maximum depth (0 indexed)
	int m_maxDepth;

#ifdef DEBUG
	// Local face date
	int m_facecount[2][6];
	std::vector<float> m_average;
	// Timer values (declared ahead of time so we can reference it in other functions with macro NOW defines
	std::chrono::high_resolution_clock::time_point t1;
#endif // DEBUG
};

#endif // _ST_VIEWER_H