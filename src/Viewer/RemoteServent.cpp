#ifdef WIN32
#include "RemoteServent.h"
#include <chrono>

RemoteServent::RemoteServent(const char *IP, int port, const char *name, std::string position) :
	m_IP(IP),
	m_port(port),
	m_name(name)
{
	// C++ doesn't allow string switches, and we won't necessarily have a single char
	// Waterfall wheee!
	if (position != "") {
		if (position == "u")
			m_position = UP;
		if (position == "d")
			m_position = DOWN;
		if (position == "r")
			m_position = RIGHT;
		if (position == "l")
			m_position = LEFT;
		if (position == "ul")
			m_position = U_LEFT;
		if (position == "ur")
			m_position = U_RIGHT;
		if (position == "dl")
			m_position = D_LEFT;
		if (position == "dr")
			m_position = D_RIGHT;
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
		PRINT_DEBUG_MESSAGE("Starting serving thread");
		Serve();
	});
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

/* ------------------------------------------------------------------- */
/* --------------------- Begin Server Functions ---------------------- */
/* ------------------------------------------------------------------- */

/* ------------------- Public Functions ------------------- */

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

// TODO: This can be made to handle multiple clients with a threaded lambda to append new sockets
// Not a major priority for now
void RemoteServent::Serve()
{
	m_Update = false;
	bool m_AcceptClients = true;
	PRINT_DEBUG_MESSAGE("Attempting to create SocketServer object");
	SocketServer in(m_port, 8);
	PRINT_DEBUG_MESSAGE("Starting to accept clients");

	while (m_AcceptClients) {
		Socket *s = in.Accept();
		if (s == NULL)
			return PRINT_DEBUG_MESSAGE("Error getting socket")

		RemoteClient rc;
		rc.socket = s;
		rc.socket->ReceiveEOF();

		m_.lock();
		// When we get a new client, send them some info so they can set themselves as distributed mode
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
		m_.unlock();
	}
}

// TODO: Forward current panoramas
// Not included for now because we're using lots of networked drive paths, which may not be consistent
void RemoteServent::UpdateClients(float yaw, float pitch, float yFOV)
{
	m_yaw = yaw;
	m_pitch = pitch;
	updateClientCameras(yFOV, pitch, yaw);
}

/* ------------------- Private Functions ------------------- */

void RemoteServent::updateClientCameras(float yFOV, float pitch, float yaw)
{
	rapidjson::Document r;
	r.Parse(MSG_TEMPLATE);
	r["command"] = rapidjson::StringRef(m_cmd[UPDATE_CAMERA].c_str());
	r["body"].AddMember("yaw", yaw, r.GetAllocator());
	r["body"].AddMember("pitch", pitch, r.GetAllocator());
	if (yFOV > 0)
		r["body"].AddMember("yFOV", yFOV, r.GetAllocator());
	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	r.Accept(writer);

	broadcastMessage(buffer.GetString());
}

void RemoteServent::broadcastMessage(std::string msg)
{
	for (RemoteClient rc : m_remoteClients) {
		rc.socket->SendEOF(msg);
	}
}

/* ------------------------------------------------------------------- */
/* --------------------- Begin Client Functions ---------------------- */
/* ------------------------------------------------------------------- */

/* ------------------- Public Functions ------------------- */
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


void RemoteServent::GetCameraUpdate(float &yaw, float &pitch)
{
	std::lock_guard<std::mutex> lock(m_);
	yaw = m_yaw;
	pitch = m_pitch;
	m_Update = false;
}

/* ------------------- Private Functions ------------------- */

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

void RemoteServent::setImage(const char *path)
{
	std::lock_guard<std::mutex> lock(m_);
	m_panoURI = std::string(path);
	m_changepano = true;
}

void RemoteServent::updateCamera(rapidjson::Value &body)
{
	std::lock_guard<std::mutex> lock(m_);
	if (body.HasMember("yaw"))
		m_yaw = body["yaw"].GetFloat();

	if (body.HasMember("pitch"))
		m_pitch = body["pitch"].GetFloat();
	
	if (body.HasMember("yFOV")) {
		m_fov = body["yFOV"].GetFloat();
		camera->SetFOV(m_fov);
	}

	if (body.HasMember("fovChange")) {
		float fovDelta = body["fovChange"].GetFloat();
		m_fov += body["fovChange"].GetFloat();
		camera->ChangeFOV(fovDelta);
	}

	float vertOffset = 0, horzOffset = 0;
	switch (m_position) {
	case UP:
		vertOffset = 2.0f;
		break;
	case DOWN:
		vertOffset = -2.0f;
		break;
	case U_LEFT:
		vertOffset = 2.0f;
		horzOffset = -2.0f;
		break;
	case D_LEFT:
		vertOffset = 2.0f;
		horzOffset = -2.0f;
		break;
	case LEFT:
		horzOffset = -2.0f;
		break;
	case U_RIGHT:
		vertOffset = 2.0f;
		horzOffset = 2.0f;
		break;
	case D_RIGHT:
		vertOffset = -2.0f;
		horzOffset = 2.0f;
		break;
	case RIGHT:
		horzOffset = 2.0f;
		break;
	default:
		vertOffset = 0.0f;
		horzOffset = 0.0f;
	}
	
	m_Update = !m_DistributedView; // Don't notify STViewer to pull camera updates if we're in DistrView
	if (m_DistributedView) {
		camera->SetCamera(m_pitch, m_yaw);
		camera->SetViewportOffset(vertOffset, horzOffset);
	}
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

#endif