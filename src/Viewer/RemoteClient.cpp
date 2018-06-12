#ifdef WIN32
#include "RemoteClient.h"
#include <chrono>

RemoteClient::RemoteClient(const char *IP, int port, const char *name) : 
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
	//recvMessage();
}

RemoteClient::RemoteClient()
{
	m_IP = "127.0.0.1";
	m_port = 5555;
	m_name = "Billy Bob Thorton";
}

RemoteClient::~RemoteClient()
{
	if (m_thread.joinable())
		m_thread.join();

	m_socket->Close();
	delete m_socket;
}

void RemoteClient::Close()
{
	std::lock_guard<std::mutex> lock(m_);
	m_socket->Close();
}

void RemoteClient::Serve()
{
	SocketServer in(m_port, 1);

	Socket* s = in.Accept();

	
	std::string r = s->ReceiveEOF();
	if (r.empty()) {
		delete s;
		return;
	}
	
	rapidjson::Document d;
	d.Parse(r.c_str());

	if (d.HasMember("body")) {
		rapidjson::Value& g = d["body"];
		if (g.HasMember("Name")) {
			auto name = g["Name"].GetString();
			fprintf(stderr, "%s\n", name);
		}
	}
	
	d["command"] = rapidjson::StringRef(m_cmd[SET_IMAGE].c_str());
	d["body"].RemoveAllMembers();
	d["body"].AddMember("uri", "File:U:\\scratch\\CAtiles\\bridgeonly.json", d.GetAllocator());

	rapidjson::StringBuffer b;
	b.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> w(b);
	d.Accept(w);

	s->SendEOF(b.GetString());
	
	d["command"] = rapidjson::StringRef(m_cmd[CLOSE].c_str());
	b.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> w2(b);
	d.Accept(w2);
	s->SendEOF(b.GetString());

	fprintf(stderr, "Closing socket\n");
	s->Close();
	delete s;
}

bool RemoteClient::connect()
{
	try {
		m_socket = new SocketClient(m_IP, m_port);
		fprintf(stderr, "Socket successfully opened to server\n");
		sendMessage(CONNECT);
	}
	catch (const char* s) {
		fprintf(stderr, "Socket Exception: %s\n", s);
		return false;
	}
	catch (std::string s) {
		fprintf(stderr, "Socket Exception: %s\n", s.c_str());
		return false;
	}
	catch (...) {
		fprintf(stderr, "Unhandled Exception\n");
		return false;
	}
	return true;
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
	changepano = true;
}

void RemoteClient::execute(int toExecute, rapidjson::Value &body)
{
	switch (toExecute) {
	case 2:
		GetMedia();
		break;
	case 3:
		fprintf(stderr, "Switching panorama source\n");
		setImage(body["uri"].GetString());
		break;
	case 4:
		fprintf(stderr, "Server closed connection\n");
		break;
	case 0:
	case 1:
	default:
		fprintf(stderr, "Unknown method\n");
		return;
	}
}

void RemoteClient::recvMessage()
{
	while (true) {
		std::string inMsg = m_socket->ReceiveEOF();

		rapidjson::Document d;
		d.Parse(inMsg.c_str());

		bool match = false;
		for (int i = 0; i < m_cmd->length(); i++) {
			if (d["command"].GetString() == m_cmd[i]) {
				execute(i, d["body"]);
				match = true;
				break;
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