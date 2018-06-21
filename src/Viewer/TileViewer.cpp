#include "TileViewer.h"
#include "KinectControl.h"

KinectControl *kinect; // So we can call killkinect() at close and properly close our feed
std::vector<PanoInfo> panolist;
bool DEBUG_FLAG;

void killkinect()
{
	if (kinect != NULL)
		kinect->StopTrackingHands();
}

int main(int argc, char **argv)
{
	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	bool stereo = false;
	bool fullscreen = false;
	bool fivepanel = false;
	RemoteClient *remote = NULL;
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
				if (argc > i + 3)
					Name = argv[i + 3];
				else
					Name = "A Computer With No Name";
				remote = new RemoteClient(IP, Port, Name);
			}
			else {
				fprintf(stderr, "Invalid number of arguments available for remote.\nFlag Usage: -r <IP> <port> [name]\n");
			}
		}
	}

	DEBUG_FLAG = false;
	STViewer viewer(argv[argc - 1], stereo, fivepanel, fullscreen, 1280, 800, remote, kinect);
}