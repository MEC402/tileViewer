//Jeff Chastine
#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include "RgbImage.h"
GLfloat xRotated, yRotated, zRotated;
GLuint	texture[1];			// Storage For One Texture ( NEW )
char* filename = "./texture.bmp";							
double camraPositive = .03;	
double camraNegative = -.03;
boolean pause = false;
boolean reset = false;
float speed = 0;
boolean Switch = false;
							
void loadTextureFromFile(char *filename)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	RgbImage theTexMap(filename);

	// Pixel alignment: each row is word aligned (aligned to a 4 byte boundary)
	//    Therefore, no need to call glPixelStore( GL_UNPACK_ALIGNMENT, ... );


	glGenTextures(1, &texture[0]);               // Create The Texture
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Typical Texture Generation Using Data From The Bitmap

	glTexImage2D(GL_TEXTURE_2D, 0, 3, theTexMap.GetNumCols(), theTexMap.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());




}
//float angle = 0.1;


using namespace std;

enum {
	BRASS, RED_PLASTIC, EMERALD, SLATE
} MaterialType;
enum {
	TORUS_MATERIAL = 1, TEAPOT_MATERIAL = 2, ICO_MATERIAL = 3
} MaterialDisplayList;
enum {
	LIGHT_OFF, LIGHT_RED, LIGHT_WHITE, LIGHT_GREEN
} LightValues;

GLfloat red_light[] =
{ 1.0, 0.0, 0.0, 1.0 }, green_light[] =
{ 0.0, 1.0, 0.0, 1.0 }, white_light[] =
{ 1.0, 1.0, 1.0, 1.0 };
GLfloat left_light_position[] =
{ -1.0, 0.0, 1.0, 0.0 }, right_light_position[] =
{ 1.0, 0.0, 1.0, 0.0 };
GLfloat brass_ambient[] =
{ 0.33, 0.22, 0.03, 1.0 }, brass_diffuse[] =
{ 0.78, 0.57, 0.11, 1.0 }, brass_specular[] =
{ 0.99, 0.91, 0.81, 1.0 }, brass_shininess = 27.8;

int shade_model = GL_SMOOTH;
char *left_light, *right_light, *teapot_material;

void
output(GLfloat x, GLfloat y, char *format, ...)
{
	va_list args;
	char buffer[200], *p;

	va_start(args, format);
	vsprintf_s(buffer, format, args);
	va_end(args);
	glPushMatrix();
	glTranslatef(x, y, 0);
	for (p = buffer; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
}

void changeViewPort(int w, int h)
{
	glViewport(0, 0, w, h);
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
}

void renderBitmapString(float x, float y, void *font, const char *string) {
	const char *c;
	glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

void
light_select(GLenum which, int value, char **label)
{
	glEnable(which);
	switch (value) {
	case LIGHT_OFF:
		*label = "off";
		glDisable(which);
		break;
	case LIGHT_RED:
		*label = "red";
		glLightfv(which, GL_DIFFUSE, red_light);
		break;
	case LIGHT_WHITE:
		*label = "white";
		glLightfv(which, GL_DIFFUSE, white_light);
		break;
	case LIGHT_GREEN:
		*label = "green";
		glLightfv(which, GL_DIFFUSE, green_light);
		break;
	}
	glutPostRedisplay();
}

void
left_light_select(int value)
{
	light_select(GL_LIGHT0, value, &left_light);
}

void
right_light_select(int value)
{
	light_select(GL_LIGHT1, value, &right_light);
}

void
material(int dlist, GLfloat * ambient, GLfloat * diffuse,
	GLfloat * specular, GLfloat shininess)
{
	glNewList(dlist, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
	glEndList();
}

char *
material_select(int object, int value)
{
	glutPostRedisplay();
	switch (value) {
	case BRASS:
		material(object, brass_ambient,
			brass_diffuse, brass_specular, brass_shininess);
		return "brass";
	
	}
	return NULL; /* avoid bogus warning! */
}


void
teapot_select(int value)
{
	teapot_material = material_select(TEAPOT_MATERIAL, value);
}

void Initialize() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	// Lighting set up
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Set lighting intensity and color
	GLfloat qaAmbientLight[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat qaDiffuseLight[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat qaSpecularLight[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);

	// Set the light position
	GLfloat qaLightPosition[] = { .5, .5, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);
}

void draw( float angle) {
	GLint m_viewport[4];

	//static float angle = 0.1;
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(.2, .5, .9);
	//glEnable(GL_LIGHTING);
	////glShadeModel(GL_FLAT);
	//GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glLightModelfv(GL_FLAT, global_ambient);
	glShadeModel(GL_SMOOTH);

glMatrixMode(GL_PROJECTION);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glLoadIdentity();
//	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	gluPerspective(45,16/9,0.01,2000);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(.3, .3, .3);
		glRotatef(angle, 0, 1, 0);
		glutSolidTeapot(.5);
		glColor3f(.2, .5, 0);
		glutSolidCube(.65);
	
	glBegin(GL_TRIANGLES);
	//glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	//glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
	//glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
	//glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);

	glTexCoord2f(0.0f, 0.0f);	glVertex3f(0.5, 0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-.1, -0.5, -0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5, -2, -0.5);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0, .5, -0.5);

		glColor3f(.5, 1, 0);
		glColor3f(0, .8, .5);
		glRasterPos2f(-.1, -.8);
		glBegin(GL_TRIANGLES);
		glTexCoord2f(0.0f, .0f); glVertex3f(0.5, 0.0, 0.5);
		glTexCoord2f(1.0f, .0f); glVertex3f(-.1, 0.5, 0.5);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, -2, 0.5);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.9, .5, 0.5);
		glColor3f(0, .3, .7);
		glRasterPos2f(-.9, .9);
		glBegin(GL_POLYGON);
		glTexCoord2f(.0f, .0f); glVertex3f(-0.5, -0.8, 0.0);
		glTexCoord2f(1.0f, .0f); glVertex3f(-.1, 0.5, 0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5, -2, 0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.4, .5, 0.0);
		glColor3f(1, 1, 1);
		glRasterPos2f(.5, 0.5);
		glBegin(GL_LINE);
		glVertex2f(150, 145);
		glVertex2f(156, 189);
		glEnd();
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, 54);
		glColor3f(1, 1, 1);
		renderBitmapString(.5, .5, GLUT_BITMAP_TIMES_ROMAN_24, "HELLO WORLD!!!!!!!!!!!!	");

	glPopMatrix();
	//glColor3f(.5, 0, 0);
	//
	//glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 54);
	//glColor3f(1, 1, 1);
	//renderBitmapString(.1, .5, GLUT_BITMAP_TIMES_ROMAN_24, "hello world!!!!!!!!!!!!	");
	glPopAttrib();
	
	//angle+=.05;
}

void displayStero(void) {
	static int vtmp = 0;
	static float angle = 0.1;

	if (vtmp == 0) {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	switch (vtmp) {
	case 0:
		glViewport(0, 0, 1920, 2160);
		vtmp++;
		break;
	case 1:
		glViewport(1920, 0, 1920, 2160);
		vtmp = 0;
		break;
	}

	if (vtmp == 0) {
		glLoadIdentity();
		if (Switch == false) {

		gluLookAt(camraPositive, 0, 1, 0, 0.0, 0.0, 0, 1.0, 0.0);
		}else if(Switch == true){
			gluLookAt(camraNegative, 0, 1, 0, 0, 0.0, 0, 1.0, 0.0);
		}
		draw(angle);
		if (pause == true) {
		}
		else {
		angle += (.05 + speed);
		}
	}
	else {
		glLoadIdentity();
		if (Switch == false) {

			gluLookAt(camraNegative, 0, 1, 0, 0.0, 0.0, 0, 1.0, 0.0);
		}
		else if (Switch == true) {
			gluLookAt(camraPositive, 0, 1, 0, 0, 0.0, 0, 1.0, 0.0);
		}

		draw(angle);
	}

	
	//glViewport(0, 0, 3840, 1080);
	if (vtmp == 0) {
		glutSwapBuffers();
		glFlush();
	}
}

void displayMe(void) {


	static int vtmp = 0;


	if (vtmp == 0) {

	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	}
	
	switch (vtmp) {
	case 0:
		glViewport(0, 1080, 960, 1080);
		vtmp++;
		break;
	case 1:
		glViewport(0,0, 960, 1080);
		vtmp++;
		break;
	case 2:
		glViewport(960, 1080, 960, 1080);
		vtmp++;
		break;
	case 3:
		glViewport(960, 0, 960, 1080);
		vtmp++;
		break;
	case 4:
		glViewport(1920, 1080, 960, 1080);
		vtmp++;
		break;
	case 5:
		glViewport(1920, 0, 960, 1080);
		vtmp++;
		break;
	case 6:
		glViewport(2880, 1080, 960, 1080);
		vtmp++;
		break;
	case 7:
		glViewport(2880, 0, 960, 1080);
		vtmp = 0;
		break;
	
	}

	if (vtmp == 0) {
		glLoadIdentity();
		gluLookAt(.1, -.1, .1, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0);
	}

	else if (vtmp <= 4) {
		glLoadIdentity();
		gluLookAt(.1, 0, 0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0);
	}
	else if(vtmp > 4 && vtmp < 8){
		glLoadIdentity();
		gluLookAt(.1, -.1, .1, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0);
	}

	//draw();
	//glViewport(0, 0, 3840, 1080);
	if (vtmp == 0) {
		glutSwapBuffers();
	glFlush();
	}
	//glDisable(GL_TEXTURE_3D);
}




void initgl() {
	glClearColor(1.0, .5, .0, 1.0);
	//glColor3f(.5, 1, 0);
}
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
	case 111:
		camraPositive -= .01;
		break;
	case 79:
		camraPositive += .01;
		break;
	case 49:
		pause = true;
		break;
	case 50:
		pause = false;
		break;
	case 51:
		speed += .01;
		break;
	case 52:
		speed -= .01;
		break;
	case 114:
		camraPositive = -.06;
		speed = 0;
		break;
	case 116:
		if (Switch == true) {
			Switch = false;
		}
		else if (Switch == false) {
			Switch = true;
		}		
		break;
	}


}

int main(int argc, char* argv[]) {
	static int mtmp = 0;
	int left_light_m, right_light_m, torus_m, teapot_m, ico_m;
	// Initialize GLUT
	glutInit(&argc, argv);
	// Set up some memory buffers for our display
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	//glutInitDisplayMode( GLUT_RGB | GLUT_SINGLE);
	glutInitDisplayMode(GLUT_SINGLE);
	/*glutSetColor(0, .5, .5, .5);*/
	// Set the window size
	
//	glutInitWindowSize(3840, 2160);
	

	// Create the window with the title "Hello,GL"
	glutCreateWindow("Hello world, GL");
	glutGameModeString("1920x1080");
	glutEnterGameMode();
	//glutFullScreen();

int width = glutGet(GLUT_WINDOW_WIDTH);
int height = glutGet(GLUT_WINDOW_HEIGHT);
printf(width + "              " + height);
glutKeyboardFunc(keyboard);

GLint m_viewport[4];

glGetIntegerv(GL_VIEWPORT, m_viewport);

	//initRendering();
	initgl();
	//Initialize();

	loadTextureFromFile(filename);
	
	glutDisplayFunc(displayStero);

	glutIdleFunc(displayStero);
	//glutReshapeFunc(changeViewPort);
	//glutDisplayFunc(displayMe);
	//glutIdleFunc(displayMe);
	// Bind the two functions (above) to respond when necessary

	//glutDisplayFunc(render);

	// Very important!  This initializes the entry points in the OpenGL driver so we can 
	// call all the functions in the API.
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return 1;
	}

	
	glutMainLoop();
	return 0;
}