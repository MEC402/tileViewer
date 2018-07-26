#include "TileViewer.h"
#include "KinectControl.h"
#include "OptMenu.h"

KinectControl *kinect; // So we can call killkinect() at close and properly close our feed
RemoteServent *remote; // So we can call killserver() at close and don't crash our viewer
std::vector<PanoInfo> panolist;
bool DEBUG_FLAG;
bool UPDATE = true;

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
	char *filepath = NULL;
	remote = NULL;
	kinect = NULL;

	ParseArgs(argc, argv, stereo, fullscreen, fivepanel, borderless, width, height, kinect, remote);

	if (argc == 0)
		filepath = Menu(argc, argv, stereo, fullscreen, fivepanel, borderless, width, height, kinect, remote);
	else
		filepath = argv[argc - 1];

	if (kinect != NULL)
		atexit(killkinect);
	if (remote != NULL)
		atexit(killremote);

	DEBUG_FLAG = false;

	STViewer viewer(filepath, stereo, fivepanel, fullscreen, borderless, width, height, remote, kinect);
}