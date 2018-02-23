#ifdef _WIN32

#include <windows.h>

#endif // Windows Headers
#include "stdafx.h"
#include <GL\glut.h>


// Teapot rotation speed
float SPEED = 0.3f;

// Teapot rotation angles
float turn_angle = 0.0f;
float x_angle = 0.0f;
float y_angle = 1.0f;
float z_angle = 0.0f;

void
display ()
{

    /* clear window */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( 1.0,  1.0,  5.0,
			   0.0,  0.0,  0.0,
			   0.0,  1.0,  0.0);
	
	glRotatef(turn_angle, x_angle, y_angle, z_angle);
	
    /* draw scene */
    glutWireTeapot(.5);

	turn_angle += SPEED;
	
    /* flush drawing routines to the window */
    glFlush();

}

void
reshape ( int width, int height )
{
	 if (!height)
		  height = 1;

	 float ratio = 1.0*width / height;

	 glMatrixMode(GL_PROJECTION);
	 
	 glLoadIdentity();
	 
	 glViewport(0, 0, width, height);
	 
	 gluPerspective(45, ratio, 1, 1000);
	 
	 glMatrixMode(GL_MODELVIEW);
}

void
perspective( int minx, int maxx, int miny, int maxy, int minz, int maxz)
{
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(minx, maxx, miny, maxy, minz, maxz);	 
}


void
processKeys(int key, int x, int y)
{
	 switch(key) {
	 case GLUT_KEY_UP:
		  x_angle += 0.01;
		  break;
		  
	 case GLUT_KEY_DOWN:
		  x_angle -= 0.01;
		  break;
		  
	 case GLUT_KEY_RIGHT:
		  y_angle += 0.01;
		  break;
		  
	 case GLUT_KEY_LEFT:
		  y_angle -= 0.01;
		  break;
		  
	 case GLUT_KEY_PAGE_UP:
		  z_angle += 0.01;
		  break;
		  
	 case GLUT_KEY_PAGE_DOWN:
		  z_angle -= 0.01;
		  break;
	 }
}


int
main ( int argc, char * argv[] )
{

    /* initialize GLUT, using any commandline parameters passed to the 
       program */
    glutInit(&argc,argv);

    /* setup the size, position, and display mode for new windows */
    glutInitWindowSize(800,640);
    glutInitWindowPosition(0,0);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB);

    /* create and set up a window */
    glutCreateWindow("hello, teapot!");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutIdleFunc(display);

	glutSpecialFunc(processKeys);
	
	glEnable(GL_DEPTH_TEST);
	
    /* define the projection transformation */
	perspective(-1.5, 1.5, -1.5, 1.5, -1.5, 1.5);

    /* tell GLUT to wait for events */
    glutMainLoop();
}
