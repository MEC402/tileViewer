#include "TileViewer.h"

std::vector<PanoInfo> panolist;
bool fullscreen;
bool stereo;
bool fivepanel;
bool usingVR;
bool DEBUG_FLAG;

int main(int argc, char **argv)
{
	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	for (int i = 0; i < argc; i++) {
		if (argv[i] == std::string("-f"))
			fullscreen = true;

		if (argv[i] == std::string("-s"))
			stereo = true;

		if (argv[i] == std::string("-5"))
			fivepanel = true;
	}

	ImageHandler::InitPanoListFromOnlineFile(argv[argc - 1]);

	VRDevice vrDevice;

	DEBUG_FLAG = false;

	STViewer::Init(vrDevice);
}