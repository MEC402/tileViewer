#include "TileViewer.h"
#include "RemoteClient.h"

std::vector<PanoInfo> panolist;

bool remote;
const char *IP;
int Port;
const char *Name;

bool fullscreen;
bool stereo;
bool fivepanel;
bool usingVR;

bool changepano;

bool DEBUG_FLAG;

int main(int argc, char **argv)
{
	//RemoteClient *server = new RemoteClient();
	//std::thread t(&RemoteClient::Serve, server);
	//t.detach();

	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	for (int i = 0; i < argc; i++) {
		if (argv[i] == std::string("-f"))
			fullscreen = true;

		if (argv[i] == std::string("-s"))
			stereo = true;

		if (argv[i] == std::string("-5"))
			fivepanel = true;

		if (argv[i] == std::string("-r")) {
			if (argc > i + 2) {
				remote = true;
				IP = argv[i + 1];
				Port = std::stoi(argv[i + 2]);
				if (argc > i + 3)
					Name = argv[i + 3];
				else
					Name = "A Computer With No Name";
			}
			else {
				fprintf(stderr, "Invalid number of arguments available for remote.\nFlag Usage: -r <IP> <port> [name]\n");
			}
		}
	}

	ImageHandler::InitPanoList(argv[argc - 1]);


	DEBUG_FLAG = false;

#ifdef USE_VR
	VRDevice vrDevice;
	STViewer::Init(vrDevice);
#else
	STViewer::Init();
#endif
}