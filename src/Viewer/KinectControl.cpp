#include "KinectControl.h"

#ifdef KINECT
KinectControl::KinectControl()
{
	std::lock_guard<std::mutex> lock(m_);
	HRESULT hr;
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
		return;

	if (m_pKinectSensor) {
		m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		hr = m_pKinectSensor->Open();
		if (FAILED(hr))
			return;

		m_pKinectSensor->OpenMultiSourceFrameReader(
			FrameSourceTypes::FrameSourceTypes_Body,
			&m_pMultiFrameReader);

		m_pixels = new unsigned char[COLOR_DIMENSIONS * 4];
		m_depthRGB = new ColorSpacePoint[DEPTH_DIMENSIONS];
		m_depthXYZ = new CameraSpacePoint[DEPTH_DIMENSIONS];
	}
	Ready = true;
}

KinectControl::~KinectControl()
{
	fprintf(stderr, "Deleting KinectControl reference\n");
	if (t_.joinable())
		t_.join();

	SafeRelease(m_pColorFrameReader);
	SafeRelease(m_pDepthFrameReader);
	SafeRelease(m_pInfraredFrameReader);
	SafeRelease(m_pBodyFrameReader);

	SafeRelease(m_pCoordinateMapper);
	SafeRelease(m_pMultiFrameReader);

	if (m_pKinectSensor)
		m_pKinectSensor->Close();

	SafeRelease(m_pKinectSensor);
}

KinectControl::HANDSTATE KinectControl::GetHandState()
{
	m_.lock();
	HANDSTATE state = NOT_TRACKING;
	if (m_tracked) {
		switch (m_righthand) {
		case HandState_Closed:
			state = CLOSED;
			break;

		case HandState_Open:
			state = OPEN;
			break;

		case HandState_Lasso:
			state = LASSO;
			break;

		case HandState_Unknown:
		case HandState_NotTracked:
		default:
			state = NOT_TRACKING;
			break;
		}

		//switch (m_lefthand) {
		//case HandState_Closed:
		//	return CLOSED;
		//case HandState_Open:
		//	return OPEN;
		//case HandState_Lasso:
		//	return LASSO;
		//
		//case HandState_Unknown:
		//case HandState_NotTracked:
		//default:
		//	break;
		//}
	}
	m_.unlock();
	return state;
}

void KinectControl::StartTrackingHands()
{
	if (!m_pMultiFrameReader)
		return;
	m_TrackHands = true;
	t_ = std::thread([&]() {
		int i = 0;
		do {
			IMultiSourceFrame *pFrame = NULL;
			HRESULT hr;
			//BOOLEAN open;
			//m_pKinectSensor->get_IsAvailable(&open);
			//if (!open) {
			//	fprintf(stderr, "Kinect was not available\n");
			//	break;
			//}
			
			m_.lock();
			while (FAILED((hr = m_pMultiFrameReader->AcquireLatestFrame(&pFrame))));
			bodyData(pFrame);
			m_.unlock();

			SafeRelease(pFrame);
			if (++i % 5 == 0)
				fprintf(stderr, "Thread loop %d\n", i);
		} while (m_TrackHands);
		fprintf(stderr, "Kinect Tracking Stopped\n");
	});
	fprintf(stderr, "Kinect Tracking Started\n");
}

void KinectControl::StopTrackingHands()
{
	m_.lock();
	m_TrackHands = false;
	m_.unlock();
}

bool KinectControl::TryLock()
{
	bool locked = m_.try_lock();
	if (locked)
		m_.unlock();
	return locked;
}

void KinectControl::GetGesture(bool guishown)
{
	if (!m_pMultiFrameReader)
		return;
	if (lastgesturetime-- != 0)
		return;
	lastgesturetime = 120;
	Ready = !guishown;

	IMultiSourceFrame *pFrame = NULL;
	HRESULT hr;
	if (FAILED((hr = m_pMultiFrameReader->AcquireLatestFrame(&pFrame)))) {
		return;
	}
	bodyData(pFrame);
	SafeRelease(pFrame);
}

void KinectControl::depthData(IMultiSourceFrame *pFrame, GLubyte *ptr)
{
	IDepthFrame *dFrame;
	IDepthFrameReference *dFrameRef = NULL;
	pFrame->get_DepthFrameReference(&dFrameRef);
	dFrameRef->AcquireFrame(&dFrame);
	SafeRelease(dFrameRef);

	if (!dFrame)
		return;

	unsigned int size;
	unsigned short *buffer;
	dFrame->AccessUnderlyingBuffer(&size, &buffer);

	m_pCoordinateMapper->MapDepthFrameToCameraSpace(size, buffer, DEPTH_DIMENSIONS, m_depthXYZ);
	m_pCoordinateMapper->MapDepthFrameToColorSpace(size, buffer, DEPTH_DIMENSIONS, m_depthRGB);

	float *dst = (float*)ptr;
	for (int i = 0; i < size; i++) {
		*dst++ = m_depthXYZ[i].X;
		*dst++ = m_depthXYZ[i].Y;
		*dst++ = m_depthXYZ[i].Z;
	}
	
	SafeRelease(dFrame);
}

void KinectControl::rgbData(IMultiSourceFrame *pFrame, GLubyte *ptr)
{
	IColorFrame *cFrame;
	IColorFrameReference *cFrameRef = NULL;
	pFrame->get_ColorFrameReference(&cFrameRef);
	cFrameRef->AcquireFrame(&cFrame);
	SafeRelease(cFrameRef);

	if (!cFrame)
		return;

	cFrame->CopyConvertedFrameDataToArray(COLOR_DIMENSIONS * 4, m_pixels, ColorImageFormat_Rgba);

	ColorSpacePoint *p = m_depthRGB;
	float *dst = (float*)ptr;
	for (int i = 0; i < DEPTH_HEIGHT; i++) {
		for (int j = 0; j < DEPTH_WIDTH; j++) {
			if (p->X < 0 || p->Y < 0 || p->X > KWIDTH || p->Y > KHEIGHT) {
				*dst++ = 0;
				*dst++ = 0;
				*dst++ = 0;
			}
			else {
				int idx = (int)p->X + KWIDTH * (int)p->Y;
				*dst++ = m_pixels[4 * idx + 0] / 255.0;
				*dst++ = m_pixels[4 * idx + 1] / 255.0;
				*dst++ = m_pixels[4 * idx + 2] / 255.0;
			}
			*dst++; //Skip alpha channel
		}
	}
	SafeRelease(cFrame);
}

void KinectControl::bodyData(IMultiSourceFrame *pFrame)
{
	static bool wasClosed = false;
	IBodyFrame *bFrame;
	IBodyFrameReference *bFrameRef = NULL;
	pFrame->get_BodyFrameReference(&bFrameRef);
	bFrameRef->AcquireFrame(&bFrame);
	SafeRelease(bFrameRef);

	if (!bFrame)
		return;

	IBody *body[BODY_COUNT] = { 0 };
	bFrame->GetAndRefreshBodyData(BODY_COUNT, body);
	for (int i = 0; i < BODY_COUNT; i++) {
		body[i]->get_IsTracked(&m_tracked);
		if (m_tracked) {
			body[i]->GetJoints(JointType_Count, m_joints);
			body[i]->get_HandLeftState(&m_lefthand);
			body[i]->get_HandRightState(&m_righthand);

			m_input.type = INPUT_KEYBOARD;
			m_input.ki.wScan = 0;
			m_input.ki.time = 0;
			m_input.ki.dwExtraInfo = 0;


			if (m_righthand == HandState_Lasso) {
				if (!Ready)
					break;
				// Press the "F4" key
				m_input.ki.wVk = 0x73;
				m_input.ki.dwFlags = 0;
				SendInput(1, &m_input, sizeof(INPUT));

				// Release the "F4" key
				m_input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &m_input, sizeof(INPUT));
			}

			if (m_righthand == HandState_Closed) {
				if (Ready)
					break;

				fprintf(stderr, "Sending 'n'\n");
				// Press the "n" key
				m_input.ki.wVk = 0x4E;
				m_input.ki.dwFlags = 0;
				SendInput(1, &m_input, sizeof(INPUT));

				// Release the "n" key
				m_input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &m_input, sizeof(INPUT));
			}

			if (m_lefthand == HandState_Closed) {
				fprintf(stderr, "Lefthand Closed\n");
				if (Ready)
					break;

				fprintf(stderr, "Sending 'p'\n");
				// 'p' key
				m_input.ki.wVk = 0x50;
				m_input.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &m_input, sizeof(INPUT));

				// Release the 'p' key
				m_input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &m_input, sizeof(INPUT));
			}

			if (m_lefthand == HandState_Lasso) {
				fprintf(stderr, "Lefthand Lasso\n");
				if (Ready)
					break;

				// Press the ' ' key
				m_input.ki.wVk = 0x20;
				m_input.ki.dwFlags = 0;
				SendInput(1, &m_input, sizeof(INPUT));

				// Release the ' ' key
				m_input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &m_input, sizeof(INPUT));
			}

			break;
		}
	}
	//delete[]body;
	SafeRelease(bFrame);
}

void KinectControl::kinectData()
{
	if (!m_pMultiFrameReader)
		return;

	IMultiSourceFrame *pFrame = NULL;
	HRESULT hr;

	while (FAILED((hr = m_pMultiFrameReader->AcquireLatestFrame(&pFrame))));

	if (SUCCEEDED(hr)) {
		GLubyte *ptr;
		glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
		ptr = (GLubyte*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (ptr)
			depthData(pFrame, ptr);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, m_cboID);
		ptr = (GLubyte*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (ptr)
			rgbData(pFrame, ptr);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		bodyData(pFrame);
	}
	else {
		error("Error in kinectData() : ", hr);
	}
	SafeRelease(pFrame);
}
#else
KinectControl::KinectControl(){}
void KinectControl::GetGesture(bool unused){}
void KinectControl::StopTrackingHands() {}
#endif