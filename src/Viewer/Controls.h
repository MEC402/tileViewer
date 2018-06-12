#ifndef CONTROLS_H
#define CONTROLS_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <stdio.h>
#include <numeric>

#include "Camera.h"
#include "ImageHandler.h"
#include "STViewer.h"

class Controls {
public:
	static STViewer viewer;

	static void init(const char* panoFileAddress, bool stereo, bool fullscreen, bool fivepanel);
	static void FlipDebug();
	static void MouseMove(int posx, int posy);
	static void MouseWheel(int button, int direction, int x, int y);
	static void ProcessGLUTKeys(int key, int x, int y);
	static void ProcessKeys(unsigned char key, int x, int y);
	static void MainMenu(int choice);
	static void PanoMenu(int choice);
	static void display();
	static void idle();
	static void resize(int w, int h);
	static void cleanup();

private:
	static int DEBUG_row;
	static int DEBUG_col;
	static float DEBUG_camerastep;
	static float DEBUG_fov;
};

#endif