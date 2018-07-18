#include "TileViewer.h"
#include "KinectControl.h"
#include "OptMenu.h"

KinectControl *kinect; // So we can call killkinect() at close and properly close our feed
RemoteServent *remote; // So we can call killserver() at close and don't crash our viewer
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
	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	bool stereo = false;
	bool fullscreen = false;
	bool fivepanel = false;
	bool borderless = false;
	int width = 1280; // Defaults
	int height = 800;
	remote = NULL;
	kinect = NULL;

	for (int i = 0; i < argc; i++) {
		if (argv[i] == std::string("-f"))
			fullscreen = true;

		if (argv[i] == std::string("-s"))
			stereo = true;

		if (argv[i] == std::string("-5"))
			fivepanel = true;

		if (argv[i] == std::string("-b")) {
			borderless = true;
			width = std::stoi(argv[++i]);
			height = std::stoi(argv[++i]);
		}

#ifdef KINECT
		if (argv[i] == std::string("-k")) {
			kinect = new KinectControl;
			//kinect->StartTrackingHands();
			atexit(killkinect); // Cleanly shut down the kinect after closing all our GL stuff
		}
#endif
		// TODO: The way we're checking arg count is terrible and NOT safe
		if (argv[i] == std::string("-r")) {
			if (argc > i + 2) {
				const char *IP;
				int Port;
				const char *Name;
				IP = argv[++i];
				Port = std::stoi(argv[++i]);
				Name = (argc > i + 1) ? argv[++i] : "A Computer With No Name";
				if (argc > i + 1)
					remote = new RemoteServent(IP, Port, Name, std::string(argv[++i]));
				else
					remote = new RemoteServent(IP, Port, Name);
			}
			else {
				fprintf(stderr, "Invalid number of arguments available for remote.\nFlag Usage: -r <IP> <port> [name [u|d|l|r|ul|ur|dl|dr]] (Up, Down, Left, Right, Upper Left, etc)\nLaunching without remote\n");
			}
		}

		if (argv[i] == std::string("-sv")) {
			if (argc > i + 2) {
				const char *IP;
				int Port;
				const char *Name;
				IP = argv[++i];
				Port = std::stoi(argv[++i]);
				Name = (argc > i + 1) ? argv[++i] : "A Computer With No Name";
				if (argc > i + 1 && argv[++i] == std::string("-d"))
					remote = new RemoteServent(IP, Port, Name, true, true);
				else
					remote = new RemoteServent(IP, Port, Name, true, false);
				atexit(killremote);
			}
			else {
				fprintf(stderr, "Invalid number of arguments available for synchronized viewing.\nFlag Usage: -sv <IP> <port> [name [-d(istributed viewing)]]\nLaunching without remote\n");
			}
		}
	}
	DEBUG_FLAG = false;

	STViewer viewer(argv[argc-1], stereo, fivepanel, fullscreen, borderless, width, height, remote, kinect);
}