#ifndef CONTROLS_H
#define CONTROLS_H

#include "STViewer.h"

class Controls {
public:
	static STViewer *viewer;

	static void SetViewer(STViewer *v);
	static void MouseMove(int posx, int posy);
	static void MouseWheel(int button, int direction, int x, int y);
	static void ProcessGLUTKeys(int key, int x, int y);
	static void ProcessKeys(unsigned char key, int x, int y);

private:
	static int DEBUG_row;
	static int DEBUG_col;
	static float DEBUG_camerastep;
	static float DEBUG_fov;
};

#endif