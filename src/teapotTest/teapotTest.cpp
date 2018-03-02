#ifdef _WIN32
	#include <windows.h>
	#include <GL\glut.h>
	#include <SOIL.h>
#endif // Windows Headers
#ifdef __linux__
	#include <gl/glut.h>
#endif


// Teapot rotation speed
float SPEED = 0.003f;

// Teapot rotation angles
float turn_angle = 0.0f;
float x_angle = 0.0f;
float y_angle = 1.0f;
float z_angle = 0.0f;

GLuint texture[1];

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
    //glutWireTeapot(.5);
	/*
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glutSolidCube(1.0);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	*/
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(0.5f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(0.5f, 0.5f); glVertex3f(1.0f, 1.0f, 1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.5f); glVertex3f(-1.0f, 1.0f, 1.0f);  // Top Left Of The Texture and Quad
															  // Back Face
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
															   // Top Face
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);  // Top Right Of The Texture and Quad
															  // Bottom Face
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom Right Of The Texture and Quad
															   // Right face
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom Left Of The Texture and Quad
															  // Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glEnd();

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
LoadGLTextures()
{
	//http://nehe.gamedev.net/tutorial/lesson_06_texturing_update/47002/
	texture[0] = SOIL_load_OGL_texture(
		"checkerboard-256x256.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y);
	if (texture[0] == 0)
		return false;

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return true;
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

	if (!LoadGLTextures())
		exit(1);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    /* tell GLUT to wait for events */
    glutMainLoop();
}
