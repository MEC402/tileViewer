#ifdef WIN32
#include "RemoteClient.h"
#include <chrono>

RemoteClient::RemoteClient(const char *IP, int port, const char *name, bool serve) :
	m_IP(IP),
	m_port(port),
	m_name(name),
	m_Serving(serve)
{
	m_thread = std::thread([=]() {
		if (m_Serving)
			Serve();
		else {
			if (!connect()) {
				fprintf(stderr, "Encountered error when connecting to %s:%d\n", m_IP, m_port);
				return;
			}
			recvMessage();
		}
	});
}

RemoteClient::RemoteClient()
{
	m_IP = "127.0.0.1";
	m_port = 5555;
	m_name = "Billy Bob Thorton";
}

RemoteClient::~RemoteClient()
{
	if (m_Serving)
		Close();

	if (m_thread.joinable())
		m_thread.join();

	if (m_socket)
		m_socket->Close();

	delete m_socket;
}

void RemoteClient::Close()
{
	std::lock_guard<std::mutex> lock(m_);
	rapidjson::Document outMsg;
	outMsg.Parse(MSG_TEMPLATE);
	outMsg["command"] = rapidjson::StringRef(m_cmd[CLOSE].c_str());
	m_outSocket->Close();
}

// TODO: This can be made to handle multiple clients with a threaded lambda to append new sockets
// Not a major priority for now
void RemoteClient::Serve()
{
	m_Update = false;
	SocketServer in(m_port, 1);
	m_outSocket = in.Accept();

	while (m_outSocket) {
		while (!m_Update) {
			std::unique_lock<std::mutex> lock(m_);
			m_updateClients.wait(lock); // Begin await
		}
		m_.lock(); // Begin lock

		rapidjson::Document outMsg;
		outMsg.Parse(MSG_TEMPLATE);
		outMsg["command"] = rapidjson::StringRef(m_cmd[UPDATE_CAMERA].c_str());
		outMsg["body"].AddMember("yaw", m_yaw, outMsg.GetAllocator());
		outMsg["body"].AddMember("pitch", m_pitch, outMsg.GetAllocator());
		//outMsg["body"].AddMember("uri", m_panoURI, outMsg.GetAllocator());

		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		outMsg.Accept(writer);
		m_outSocket->SendEOF(buffer.GetString());
		m_Update = false;

		m_.unlock(); // End lock
	}
}

// TODO: Forward current panoramas
// Not included (extremely trivial to do) for now because URIs are pointing to network drives,
// and not all machines have access or equivalent drive naming schemes
// We need the webserver to be running for this to truly work well
void RemoteClient::UpdateClients(float yaw, float pitch) //,std::string pano)
{
	std::lock_guard<std::mutex> lock(m_);
	m_yaw = yaw;
	m_pitch = pitch;
	//m_panoURI = pano;
	m_Update = true;
	m_updateClients.notify_one();
}

bool RemoteClient::connect()
{
	try {
		m_socket = new SocketClient(m_IP, m_port);
		m_connected = true;
		fprintf(stderr, "Socket successfully opened to server\n");
		sendMessage(CONNECT);
		return true;
	}
	catch (const char* s) {
		fprintf(stderr, "Socket Exception: %s\n", s);
	}
	catch (std::string s) {
		fprintf(stderr, "Socket Exception: %s\n", s.c_str());
	}
	catch (...) {
		fprintf(stderr, "Unhandled Exception\n");
	}
	m_connected = false;
	return false;
}

bool RemoteClient::ChangePano()
{
	return m_changepano;
}

void RemoteClient::GetMedia()
{
	fprintf(stderr, "RemoteClient::GetMedia not implemented\n");
}

std::string RemoteClient::GetPano()
{
	std::lock_guard<std::mutex> lock(m_);
	return m_panoURI;
}

void RemoteClient::setImage(const char *path)
{
	std::lock_guard<std::mutex> lock(m_);
	m_panoURI = std::string(path);
	m_changepano = true;
}

void RemoteClient::GetCameraUpdate(float &yaw, float &pitch)
{
	std::lock_guard<std::mutex> lock(m_);
	yaw = m_yaw;
	pitch = m_pitch;
	m_Update = false;
}

void RemoteClient::updateCamera(rapidjson::Value &body)
{
	std::lock_guard<std::mutex> lock(m_);
	if (body.HasMember("yaw")) {
		m_yaw = body["yaw"].GetFloat();
		//fprintf(stderr, "Yaw Received: %f\n", m_yaw);
	}
	if (body.HasMember("pitch")) {
		m_pitch = body["pitch"].GetFloat();
		//fprintf(stderr, "Pitch Received: %f\n", m_pitch);
	}
	m_Update = true;
}

void RemoteClient::execute(int toExecute, rapidjson::Value &body)
{
	switch ((MSG)toExecute) {
	case GET_MEDIA_RESPONSE:
		GetMedia();
		break;
	case SET_IMAGE:
		fprintf(stderr, "Switching panorama source\n");
		setImage(body["uri"].GetString());
		break;
	case CLOSE:
		fprintf(stderr, "Server closed connection\n");
		m_connected = false;
		break;
	case UPDATE_CAMERA:
		updateCamera(body);
		break;
	case CONNECT:
	case GET_MEDIA:
	default:
		fprintf(stderr, "Unknown method\n");
		return;
	}
}

void RemoteClient::recvMessage()
{
	while (m_connected) {
		std::string inMsg = m_socket->ReceiveEOF();

		rapidjson::Document d;
		d.Parse(inMsg.c_str());

		bool match = false;
		if (inMsg == "") {
			m_connected = false;
			fprintf(stderr, "NULL message, Disconnecting\n");
			continue;
		}
		if (d.HasParseError()) {
			m_connected = false;
			fprintf(stderr, "Parse error, disconnecting\n");
			continue;
		}


		if (d.HasMember("command")) {
			for (unsigned int i = 0; i < m_cmd->length(); i++) {
				if (d["command"].GetString() == m_cmd[i]) {
					execute(i, d["body"]);
					match = true;
					break;
				}
			}
		}
		if (!match) {
			sendMessage(UNKNOWN);
		}
	}
}

void RemoteClient::sendMessage(MSG TYPE)
{
	rapidjson::Document r;
	r.Parse(MSG_TEMPLATE);

	switch (TYPE) {
	case CONNECT:
		r["command"] = rapidjson::StringRef(m_cmd[TYPE].c_str());
		r["body"].AddMember("name", rapidjson::StringRef(m_name), r.GetAllocator());
		break;
	case UPDATE_CAMERA:
		break;
	case GET_MEDIA:
	case GET_MEDIA_RESPONSE:
	case SET_IMAGE:
		r["command"] = rapidjson::StringRef(m_cmd[TYPE].c_str());
		break;
	case UNKNOWN:
		r["command"] = "ERR";
		r["body"].AddMember(MSG_UNKNOWN_BODY, r.GetAllocator());
		break;
	default:
		fprintf(stderr, "No valid ENUM provided\n");
		return;
	}

	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	r.Accept(writer);

	m_socket->SendEOF(buffer.GetString());
}


#endif