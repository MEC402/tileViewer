#ifndef CONTROLS_H
#define CONTROLS_H

#include "Camera.h"

#include <vector>
class Controls {
public:
	static void FlipDebug();
	static void MouseMove(int posx, int posy);
	static void MouseWheel(int button, int direction, int x, int y);
	static void ProcessGLUTKeys(int key, int x, int y);
	static void ProcessKeys(unsigned char key, int x, int y);

private:
	static bool DEBUG;
	static int DEBUG_row;
	static int DEBUG_col;
	static float DEBUG_camerastep;
	static float DEBUG_fov;
	static float DEBUG_camera_degree_shift[5];
};

#endif