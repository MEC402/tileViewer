#ifdef WIN32
#include "RemoteServent.h"
#include <chrono>

RemoteServent::RemoteServent(const char *IP, int port, const char *name) :
	m_IP(IP),
	m_port(port),
	m_name(name)
{
	m_thread = std::thread([=]() {
		if (!connect()) {
			fprintf(stderr, "Encountered error when connecting to %s:%d\n", m_IP, m_port);
			return;
		}
		recvMessage();
	});
}

RemoteServent::RemoteServent(const char *IP, int port, const char *name, char position) :
	m_IP(IP),
	m_port(port),
	m_name(name)
{
	switch (position) {
	case 'u':
		m_position = UP;
		break;
	case 'd':
		m_position = DOWN;
		break;
	case 'l':
		m_position = LEFT;
		break;
	case 'r':
		m_position = RIGHT;
		break;
	}

	m_thread = std::thread([=]() {
		if (!connect()) {
			fprintf(stderr, "Encountered error when connecting to %s:%d\n", m_IP, m_port);
			return;
		}
		recvMessage();
	});
}

RemoteServent::RemoteServent(const char *IP, int port, const char *name, bool serve, bool distributedView) :
	m_IP(IP),
	m_port(port),
	m_name(name),
	m_Serving(serve),
	m_DistributedView(distributedView)
{
	m_thread = std::thread([=]() {
		Serve();
	});
}

RemoteServent::RemoteServent()
{
	m_IP = "127.0.0.1";
	m_port = 5555;
	m_name = "Billy Bob Thorton";
}

RemoteServent::~RemoteServent()
{
	if (m_Serving)
		Close();

	if (m_thread.joinable())
		m_thread.join();

	if (m_socket)
		m_socket->Close();

	delete m_socket;
}

void RemoteServent::SetCameraPtr(Camera *c)
{
	camera = c;
}

void RemoteServent::SetPosition(POSITION position)
{
	m_position = position;
}

void RemoteServent::Close()
{
	std::lock_guard<std::mutex> lock(m_);
	rapidjson::Document outMsg;
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	outMsg.Parse(MSG_TEMPLATE);
	outMsg["command"] = rapidjson::StringRef(m_cmd[CLOSE].c_str());
	outMsg.Accept(writer);
	broadcastMessage(buffer.GetString());
	for (RemoteClient rc : m_remoteClients)
		rc.socket->Close();
}

void RemoteServent::acceptClient(SocketServer &in)
{
	Socket *s = in.Accept();
	RemoteClient rc;
	rc.socket = s;
	rc.socket->ReceiveEOF();

	m_.lock();
	if (m_DistributedView) {
		rapidjson::Document outMsg;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		outMsg.Parse(MSG_TEMPLATE);
		outMsg["command"] = rapidjson::StringRef(m_cmd[SET_DISTRIBUTED].c_str());
		outMsg["body"].AddMember("nil", 0, outMsg.GetAllocator());
		buffer.Clear();
		outMsg.Accept(writer);
		rc.socket->SendEOF(buffer.GetString());

		std::string inMsg = rc.socket->ReceiveEOF();
		rapidjson::Document d;
		d.Parse(inMsg.c_str());
		fprintf(stderr, "Received %s\n", inMsg.c_str());
		if (d.HasMember("command") 
			&& d["command"].GetString() == std::string(m_cmd[SET_DISTRIBUTED]))
		{
			fprintf(stderr, "Setting rc.position value\n");
			rc.position = (POSITION)d["body"]["position"].GetInt();
		}
	}

	m_remoteClients.push_back(rc);
	fprintf(stderr, "Leaving acceptClient\n");
	m_.unlock();
}

// TODO: This can be made to handle multiple clients with a threaded lambda to append new sockets
// Not a major priority for now
void RemoteServent::Serve()
{
	m_Update = false;
	bool m_AcceptClients = true;
	SocketServer in(m_port, 3);

	for (int i = 0; i < 3; i++) {
		acceptClient(in);
	}

	//std::thread([&]() {
	//	while (m_AcceptClients && in.OpenSockets() < 3)
	//		acceptClient(in);
	//});

	while (m_AcceptClients) {
		while (!m_Update) {
			std::unique_lock<std::mutex> lock(m_);
			m_updateClients.wait(lock); // Begin await
		}
		m_.lock(); // Begin lock

		rapidjson::Document outMsg;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		outMsg.Parse(MSG_TEMPLATE);
		outMsg["command"] = rapidjson::StringRef(m_cmd[UPDATE_CAMERA].c_str());
		outMsg["body"].AddMember("yaw", m_yaw, outMsg.GetAllocator());
		outMsg["body"].AddMember("pitch", m_pitch, outMsg.GetAllocator());
		//outMsg["body"].AddMember("uri", m_panoURI, outMsg.GetAllocator());

		buffer.Clear();
		writer.Reset(buffer);
		outMsg.Accept(writer);
		broadcastMessage(buffer.GetString());
		m_Update = false;

		m_.unlock(); // End lock
	}
}

// TODO: Forward current panoramas
// Not included (extremely trivial to do) for now because URIs are pointing to network drives,
// and not all machines have access or equivalent drive naming schemes
// We need the webserver to be running for this to truly work well
void RemoteServent::UpdateClients(float yaw, float pitch) //,std::string pano)
{
	std::lock_guard<std::mutex> lock(m_);
	m_yaw = yaw;
	m_pitch = pitch;
	//m_panoURI = pano;
	m_Update = true;
	m_updateClients.notify_one();
}

void RemoteServent::UpdateClients(float yaw, float pitch, float xFOV, float yFOV)
{
	m_yaw = yaw;
	m_pitch = pitch;
	updateCamera(xFOV, yFOV);
}

void RemoteServent::UpdateClients(float yaw, float pitch, float xFOV, float yFOV, float yFOVDelta)
{
	m_yaw = yaw;
	m_pitch = pitch;
	updateCamera(xFOV, yFOV, yFOVDelta);
}

bool RemoteServent::connect()
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

bool RemoteServent::ChangePano()
{
	return m_changepano;
}

void RemoteServent::GetMedia()
{
	fprintf(stderr, "RemoteServent::GetMedia not implemented\n");
}

std::string RemoteServent::GetPano()
{
	std::lock_guard<std::mutex> lock(m_);
	return m_panoURI;
}

void RemoteServent::setImage(const char *path)
{
	std::lock_guard<std::mutex> lock(m_);
	m_panoURI = std::string(path);
	m_changepano = true;
}

void RemoteServent::GetCameraUpdate(float &yaw, float &pitch)
{
	std::lock_guard<std::mutex> lock(m_);
	yaw = m_yaw;
	pitch = m_pitch;
	m_Update = false;
}

void RemoteServent::updateCamera(rapidjson::Value &body)
{
	std::lock_guard<std::mutex> lock(m_);
	if (body.HasMember("yaw"))
		m_yaw = body["yaw"].GetFloat();

	if (body.HasMember("pitch"))
		m_pitch = body["pitch"].GetFloat();
	
	if (body.HasMember("fov")) {
		m_fov = body["fov"].GetFloat();
		camera->SetFOV(m_fov);
	}
	if (body.HasMember("fovChange")) {
		float fovDelta = body["fovChange"].GetFloat();
		m_fov += body["fovChange"].GetFloat();
		camera->ChangeFOV(fovDelta);
	}

	m_Update = !m_DistributedView; // Don't notify STViewer to pull camera updates if we're in DistrView
	if (m_DistributedView)
		camera->SetCamera(m_pitch, m_yaw);
}

void RemoteServent::execute(int toExecute, rapidjson::Value &body)
{
	switch ((MSG)toExecute) {
	case GET_MEDIA_RESPONSE:
		GetMedia();
		break;
	case SET_IMAGE:
		fprintf(stderr, "Switching panorama source\n");
		setImage(body["uri"].GetString());
		break;
	case UPDATE_CAMERA:
		updateCamera(body);
		break;
	case SET_DISTRIBUTED:
		fprintf(stderr, "Received set_distributed message\n");
		m_DistributedView = true;
		sendMessage(SET_DISTRIBUTED);
		break;
	case CLOSE:
		fprintf(stderr, "Server closed connection\n");
		m_connected = false;
		break;
	case CONNECT:
	case GET_MEDIA:
	default:
		fprintf(stderr, "Unknown method\n");
		return;
	}
}

void RemoteServent::recvMessage()
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

void RemoteServent::sendMessage(MSG TYPE)
{
	rapidjson::Document r;
	r.Parse(MSG_TEMPLATE);

	switch (TYPE) {
	case CONNECT:
		r["command"] = rapidjson::StringRef(m_cmd[TYPE].c_str());
		r["body"].AddMember("name", rapidjson::StringRef(m_name), r.GetAllocator());
		break;
	case SET_DISTRIBUTED:
		r["command"] = rapidjson::StringRef(m_cmd[TYPE].c_str());
		r["body"].AddMember("position", m_position, r.GetAllocator());
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
	fprintf(stderr, "Sending %s\n", buffer.GetString());
	m_socket->SendEOF(buffer.GetString());
}

void RemoteServent::updateCamera(float xFOV, float yFOV)
{
	for (RemoteClient rc : m_remoteClients) {
		float pitch = m_pitch, yaw = m_yaw;
		switch (rc.position) {
		case UP:
			pitch += yFOV;
			break;
		case DOWN:
			pitch -= yFOV;
			break;
		case RIGHT:
			yaw -= xFOV;
			break;
		case LEFT:
			yaw += xFOV;
			break;
		}

		rapidjson::Document r;
		r.Parse(MSG_TEMPLATE);
		r["command"] = rapidjson::StringRef(m_cmd[UPDATE_CAMERA].c_str());
		r["body"].AddMember("yaw", yaw, r.GetAllocator());
		r["body"].AddMember("pitch", pitch, r.GetAllocator());
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		r.Accept(writer);

		rc.socket->SendEOF(buffer.GetString());
	}
}

void RemoteServent::updateCamera(float xFOV, float yFOV, float yFOVChange)
{
	for (RemoteClient rc : m_remoteClients) {
		float pitch = m_pitch, yaw = m_yaw;
		switch (rc.position) {
		case UP:
			pitch += yFOV;
			break;
		case DOWN:
			pitch -= yFOV;
			break;
		case RIGHT:
			yaw -= xFOV;
			break;
		case LEFT:
			yaw += xFOV;
			break;
		}

		rapidjson::Document r;
		r.Parse(MSG_TEMPLATE);
		r["command"] = rapidjson::StringRef(m_cmd[UPDATE_CAMERA].c_str());
		r["body"].AddMember("yaw", yaw, r.GetAllocator());
		r["body"].AddMember("pitch", pitch, r.GetAllocator());
		r["body"].AddMember("fov", yFOV, r.GetAllocator());
		//r["body"].AddMember("fovChange", yFOVChange, r.GetAllocator());
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		r.Accept(writer);

		rc.socket->SendEOF(buffer.GetString());
	}
}

void RemoteServent::broadcastMessage(std::string msg)
{
	for (RemoteClient rc : m_remoteClients) {
		rc.socket->SendEOF(msg);
	}
}

#endif