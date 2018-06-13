#include "TileViewer.h"
#include "Controls.h"

std::vector<PanoInfo> panolist;
bool DEBUG_FLAG;


int main(int argc, char **argv)
{
	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	bool stereo = false;
	bool fullscreen = false;
	bool fivepanel = false;

	for (int i = 0; i < argc; i++) {
		if (argv[i] == std::string("-f"))
			fullscreen = true;

		if (argv[i] == std::string("-s"))
			stereo = true;

		if (argv[i] == std::string("-5"))
			fivepanel = true;
	}

	DEBUG_FLAG = false;

	STViewer viewer(argv[argc - 1], stereo, fivepanel, fullscreen, 1280, 800);
}