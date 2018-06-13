#ifdef WIN32
#ifndef _REMOTECLIENT_H
#define _REMOTECLIENT_H
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Socket.h"
#include "Shared.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <iostream>
#include <mutex>
#include <thread>


#define MSG_TEMPLATE "{\
\"id\": "",\
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

class RemoteClient {
public:
	RemoteClient(const char *IP, int port, const char *name);
	RemoteClient(void);
	~RemoteClient(void);
	void Close(void);
	void Serve(void);
	void GetMedia(void);
	std::string GetPano();

private:
	enum MSG { CONNECT = 0, GET_MEDIA, GET_MEDIA_RESPONSE, SET_IMAGE, CLOSE, UNKNOWN };

	const char *m_IP;
	int m_port;
	const char *m_name;
	std::string m_cmd[5] = { "client_connect", "get_media", "get_media_response", "set_image", "close_connection" };

	SocketClient *m_socket;
	std::thread m_thread;

	std::string m_panoURI;
	std::mutex m_;

	bool connect(void);
	void recvMessage(void);
	void sendMessage(MSG MSG_TYPE);

	void execute(int toExecute, rapidjson::Value &body);
	void setImage(const char* path);
};

#endif // _REMOTECLIENT_H
#endif // WIN32