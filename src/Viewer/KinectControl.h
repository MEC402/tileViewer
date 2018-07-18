#ifdef WIN32
#include "Shared.h"

#ifndef _KINECTCONTROL_H
#define _KINECTCONTROL_H

#ifdef KINECT

#include <Windows.h>
#include <comdef.h>
#include <Ole2.h>
#include <Kinect.h>
#include <mutex>
#include <atomic>

#define KWIDTH 1920
#define KHEIGHT 1080
#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424

#define COLOR_DIMENSIONS KWIDTH * KHEIGHT
#define DEPTH_DIMENSIONS DEPTH_WIDTH * DEPTH_HEIGHT

class KinectControl {

public:
	KinectControl();
	~KinectControl();
	void StartTrackingHands(void);
	void StopTrackingHands(void);
	bool TryLock(void);
	void GetGesture(bool guishown);
	enum HANDSTATE { OPEN = 0, CLOSED, LASSO, NOT_TRACKING, UNKNOWN };

	HANDSTATE GetHandState(void);

	bool Ready{ false };

private:
	void depthData(IMultiSourceFrame *pFrame, GLubyte *ptr);
	void rgbData(IMultiSourceFrame *pFrame, GLubyte *ptr);
	void bodyData(IMultiSourceFrame *pFrame);
	void kinectData(void);

	template <class Interface>
	inline void SafeRelease(Interface *&p)
	{
		if (p != NULL) {
			p->Release();
			p = NULL;
		}
	}

	inline void error(const char *s, HRESULT hr)
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		fprintf(stderr, "%s %ls\n", s, errMsg);
	}

	std::mutex m_;
	std::thread t_;
	std::atomic<bool> m_TrackHands{ false };
	std::atomic<bool> m_ThreadDone{ false };

	int lastgesturetime{ 1 };

	IKinectSensor *m_pKinectSensor;
	IColorFrameReader *m_pColorFrameReader;
	IDepthFrameReader *m_pDepthFrameReader;
	IInfraredFrameReader *m_pInfraredFrameReader;
	IBodyFrameReader *m_pBodyFrameReader;
	IMultiSourceFrameReader *m_pMultiFrameReader;

	ICoordinateMapper *m_pCoordinateMapper;
	GLuint m_textureId;
	GLubyte *m_pColorRGBX;

	INPUT m_input;

	GLuint m_vboID;
	GLuint m_cboID;

	// Intermediate Buffers
	unsigned char *m_pixels;
	ColorSpacePoint *m_depthRGB;
	CameraSpacePoint *m_depthXYZ;

	BOOLEAN m_tracked;
	Joint m_joints[JointType_Count];

	HandState m_lefthand;
	HandState m_righthand;
};

#else // ifdef KINECT
class KinectControl {
public:
	KinectControl();
	~KinectControl() = default;
	void GetGesture(bool);
	void StopTrackingHands();
};
#endif // KINECT
#endif // _KINECTCONTROL_H
#endif // WIN32