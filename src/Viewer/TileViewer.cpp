#include "TileViewer.h"
#include "KinectControl.h"
#include "ObjLoader.h"


KinectControl *kinect; // So we can call killkinect() at close and properly close our feed
RemoteClient *remote; // So we can call killserver() at close and don't crash our viewer
std::vector<PanoInfo> panolist;
bool DEBUG_FLAG;

void killkinect()
{
	if (kinect != NULL)
		kinect->StopTrackingHands();
}

void killremote()
{
	if (remote != NULL && remote->m_Serving)
		remote->Close();
}

int main(int argc, char **argv)
{
	//RemoteClient *server = new RemoteClient("127.0.0.1", 5005, "A Server", true);
	//RemoteClient *client = new RemoteClient("127.0.0.1", 5005, "A Client", false);
	//return 0;
	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	bool stereo = false;
	bool fullscreen = false;
	bool fivepanel = false;
	remote = NULL;
	kinect = NULL;

	for (int i = 0; i < argc; i++) {
		if (argv[i] == std::string("-f"))
			fullscreen = true;

		if (argv[i] == std::string("-s"))
			stereo = true;

		if (argv[i] == std::string("-5"))
			fivepanel = true;

#ifdef KINECT
		if (argv[i] == std::string("-k")) {
			kinect = new KinectControl;
			//kinect->StartTrackingHands();
			atexit(killkinect); // Cleanly shut down the kinect after closing all our GL stuff
		}
#endif

		if (argv[i] == std::string("-r")) {
			if (argc > i + 2) {
				const char *IP;
				int Port;
				const char *Name;
				IP = argv[i + 1];
				Port = std::stoi(argv[i + 2]);
				Name = (argc > i + 3) ? argv[1 + 3] : "A Computer With No Name";
				remote = new RemoteClient(IP, Port, Name, false);
			}
			else {
				fprintf(stderr, "Invalid number of arguments available for remote.\nFlag Usage: -r <IP> <port> [name]\nLaunching without remote\n");
			}
		}

		if (argv[i] == std::string("-sv")) {
			if (argc > i + 2) {
				const char *IP;
				int Port;
				const char *Name;
				IP = argv[i + 1];
				Port = std::stoi(argv[i + 2]);
				Name = (argc > i + 3) ? argv[1 + 3] : "A Computer With No Name";
				remote = new RemoteClient(IP, Port, Name, true);
				atexit(killremote);
			}
			else {
				fprintf(stderr, "Invalid number of arguments available for synchronized viewing.\nFlag Usage: -sv <IP> <port> [name]\nLaunching without remote\n");
			}
		}
	}

	DEBUG_FLAG = false;
	STViewer viewer(argv[argc - 1], stereo, fivepanel, fullscreen, 1280, 800, remote, kinect);
}