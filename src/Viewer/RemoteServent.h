/*
	General purpose remote socket interface for talking to HTTP web servers for remote operation, or for communicating between two TileViewer instances for synchronized remote viewing
	And no, this isn't mispelled.  "SERVer/cliENT" -> Servent
*/

#ifdef WIN32
#ifndef _RemoteServent_H
#define _RemoteServent_H
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Socket.h"
#include "Camera.h"
#include "Shared.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define MSG_TEMPLATE "{\
\"id\": 9001,\
	\"command\" : \"no_command\",\
	\"body\" : {\
		\
	}\
}"

#define MSG_UNKNOWN_BODY "Message", "Unrecognized command received"
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

class RemoteServent {

private:
	// this used to have more stuff in it and I don't wanna refactor it out
	struct RemoteClient {
		Socket *socket;
	};

public:
	RemoteServent(const char *IP, int port, const char *name, std::string position = "");
	RemoteServent(const char *IP, int port, const char *name, bool serve, bool distributedView);
	~RemoteServent(void);

	void SetCameraPtr(Camera *c);
	void Close(void);
	void Serve(void);
	bool ChangePano(void);
	void ChangePano(int panoindex);
	void GetMedia(void);
	std::string GetPanoURI();
	int GetPanoIndex();

	void GetCameraUpdate(float &yaw, float &pitch); // For receiving new camera data
	void UpdateClients(float yaw, float pitch, float yFOV = 0.0f); // For serving new camera data
	bool m_Serving;
	bool m_DistributedView;
	bool m_Update;

private:
	enum MSG { CONNECT = 0, GET_MEDIA, GET_MEDIA_RESPONSE, SET_IMAGE, CLOSE, UPDATE_CAMERA, SET_DISTRIBUTED, UNKNOWN };
	std::string m_cmd[7] = { "client_connect", "get_media", "get_media_response", "set_image", "close_connection", "update_camera", "set_distributed" };

	const char *m_IP;
	int m_port;
	const char *m_name;
	float m_yaw;
	float m_pitch;
	float m_fov;

	double m_vertOffset{ 0.0 };
	double m_horzOffset{ 0.0 };
	Camera *camera;

	std::vector<RemoteClient> m_remoteClients;
	SocketClient *m_socket;
	std::thread m_thread;
	std::mutex m_;
	std::condition_variable m_updateClients;
	std::string m_panoURI;
	int m_panoIndex{ 0 };
	bool m_changepano{ false };
	bool m_connected;

	void acceptClient(SocketServer &in);

	bool connect(void);
	void recvMessage(void);
	void sendMessage(MSG MSG_TYPE);
	void broadcastMessage(std::string);

	void execute(int toExecute, rapidjson::Value &body);
	void setImage(const char* path);
	void updateCamera(rapidjson::Value &body);
	void updateClientCameras(float yFOV, float pitch, float yaw);
};

#endif // _RemoteServent_H
#endif // WIN32