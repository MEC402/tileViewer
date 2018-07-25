#ifdef WIN32
#include "RemoteServent.h"
#include <chrono>

RemoteServent::RemoteServent(const char *IP, int port, const char *name, std::string position) :
	m_IP(IP),
	m_port(port),
	m_name(name)
{
	if (position != "") {
		for (int i = 0; i < position.length(); i += 2) {
			int n = position[i + 1] - '0';
			switch (position[i]) {
			case 'u':
				m_vertOffset = n * 2.0;
				break;
			case 'd':
				m_vertOffset = -(n * 2.0);
				break;
			case 'r':
				m_horzOffset = n * 2.0;
				break;
			case 'l':
				m_horzOffset = -(n * 2.0);
				break;
			}
		}
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
			//if (d.HasMember("command")
			//	&& d["command"].GetString() == std::string(m_cmd[SET_DISTRIBUTED]))
			//{
			//	fprintf(stderr, "Setting rc.position value\n");
			//	rc.position = (POSITION)d["body"]["position"].GetInt();
			//}
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

void RemoteServent::ChangePano(int panoindex)
{
	m_panoIndex = panoindex;
	rapidjson::Document r;
	r.Parse(MSG_TEMPLATE);
	r["command"] = rapidjson::StringRef(m_cmd[SET_IMAGE].c_str());
	r["body"].AddMember("pano_index", m_panoIndex, r.GetAllocator());
	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	r.Accept(writer);

	broadcastMessage(buffer.GetString());
}

void RemoteServent::GetMedia()
{
	fprintf(stderr, "RemoteServent::GetMedia not implemented\n");
}

std::string RemoteServent::GetPanoURI()
{
	std::lock_guard<std::mutex> lock(m_);
	m_changepano = false;
	return m_panoURI;
}

int RemoteServent::GetPanoIndex()
{
	std::lock_guard<std::mutex> lock(m_);
	m_changepano = false;
	return m_panoIndex;
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
		//r["body"].AddMember("position", m_position, r.GetAllocator());
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
	
	m_Update = !m_DistributedView; // Don't notify STViewer to pull camera updates if we're in DistrView
	if (m_DistributedView) {
		camera->SetCamera(m_pitch, m_yaw);
		camera->SetViewportOffset(m_vertOffset, m_horzOffset);
	}
}

void RemoteServent::execute(int toExecute, rapidjson::Value &body)
{
	switch ((MSG)toExecute) {
	case GET_MEDIA_RESPONSE:
		GetMedia();
		break;
	case SET_IMAGE:
		fprintf(stderr, "Switching panorama\n");
		if (body.HasMember("uri")) {
			setImage(body["uri"].GetString());
		}
		else if (body.HasMember("pano_index")) {
			m_panoIndex = body["pano_index"].GetInt();
			m_changepano = true;
		}
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