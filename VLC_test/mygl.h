/*
*
*	Simple example of combining OpenCvs camera interface and OpenGL
*
*
*	@author Gordon Wetzstein, now @ The University of British Columbia [wetzste1@cs.ubc.ca]
*	@date 10/15/06
*
*/


//-----------------------------------------------------------------------------
// includes

#ifdef WIN32
#include <windows.h>
#endif

#include "gl/glew.h"
#include "gl/glut.h"

#include "Header.h"
#include "Parameters.h"
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// GLUT callbacks and functions

void initGlut(int argc, char **argv);
void displayFunc(void);
void idleFunc(void);
void reshapeFunc(int width, int height);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);
void keyboardFunc(unsigned char key, int x, int y);
void specialFunc(int key, int x, int y);
//-----------------------------------------------------------------------------

// other [OpenGL] functions
void countFrames(void);
void renderBitmapString(float x, float y, float z, void *font, char *string);

//-----------------------------------------------------------------------------

bool bFullsreen = false;
bool pause = true;
int nWindowID;

//-----------------------------------------------------------------------------

// parameters for the framecounter
char pixelstring[30];
int cframe = 0;
int glut_time = 0;
int timebase = 0;

//-----------------------------------------------------------------------------

// OpenCV variables
//cv::VideoCapture cap;
GLuint cameraImageTextureID;

//-----------------------------------------------------------------------------

bool bInit = false;

//-----------------------------------------------------------------------------

void displayFunc(void) {
	if (Parameters::done && !Parameters::transmitterQueue.size())
	{
		long long milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - Parameters::transmissionStartTime).count();
		cout << "Time = " << milli << " ms" << endl;
		exit(0);
	}
	if (!bInit) {

		//cap.open("C:\\VLC\\Release\\HiLightLowFreqDynamicLive\\20150510_175957_978068469__HiLight_2x3rand_HiLightLowFreqsymbol_sideA3_sideB2_full1_200ms_levels_I420_test1mp4_output.mp4");

		// initialze OpenGL texture		
		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glGenTextures(1, &cameraImageTextureID);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, cameraImageTextureID);

		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		bInit = true;
	}
	// start processing whin the queue has 10 frames
	while (Parameters::outputFrameIndex == 0 && Parameters::transmitterQueue.size() < 10)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(50));
	}
	while (!Parameters::transmitterQueue.size())
	{
		std::this_thread::sleep_for(std::chrono::microseconds(50));
	}
	Mat newImage;
	Parameters::transmitterQueue.pop(newImage);
	
	if ((newImage.cols > 0) && (newImage.rows > 0)) {

		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, (GLdouble)newImage.cols, (GLdouble)newImage.rows, 0.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, cameraImageTextureID);

		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, newImage.cols, newImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, newImage.data);
		
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex2i(0, 0);
		glTexCoord2i(newImage.cols, 0);
		glVertex2i(newImage.cols, 0);
		glTexCoord2i(newImage.cols, newImage.rows);
		glVertex2i(newImage.cols, newImage.rows);
		glTexCoord2i(0, newImage.rows);
		glVertex2i(0, newImage.rows);
		glEnd();

	}

	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	//countFrames();
	long long expectedMicroseconds = Parameters::outputFrameIndex;
	expectedMicroseconds *= 1000000 / Parameters::fps;
	long long currentMicroseonds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - Parameters::transmissionStartTime).count();
	//printf("exp = %lld\tcurr = %lld\tind = %d\tQueue = %d\n",
		//expectedMicroseconds, currentMicroseonds, Parameters::outputFrameIndex, Parameters::transmitterQueue.size());
	if (Parameters::outputFrameIndex && currentMicroseonds < expectedMicroseconds)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(expectedMicroseconds - currentMicroseonds));
	}
	glutSwapBuffers();
	//currentMicroseonds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - Parameters::transmissionStartTime).count();
	//printf("%5d\tfps: %4.1f\n", Parameters::outputFrameIndex, Parameters::outputFrameIndex*1000000.0 / currentMicroseonds);
	if (!Parameters::outputFrameIndex)
	{
		/*while (pause)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}*/
		Parameters::transmissionStartTime = std::chrono::system_clock::now();
	}
	Parameters::outputFrameIndex++;
}

//-----------------------------------------------------------------------------

void initGlut(int argc, char **argv) {

	// GLUT Window Initialization:
	glutInit(&argc, argv);
	glutInitWindowSize(800, 500);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	nWindowID = glutCreateWindow(Parameters::displayName.c_str());
	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);
	// Register callbacks:
	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutKeyboardFunc(keyboardFunc);
	glutSpecialFunc(specialFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);
	glutIdleFunc(idleFunc);
}



//-----------------------------------------------------------------------------

void idleFunc(void) {
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------

void reshapeFunc(int width, int height) {
	glViewport(0, 0, width, height);
}

//-----------------------------------------------------------------------------


// mouse callback
void mouseFunc(int button, int state, int x, int y) {

}

//-----------------------------------------------------------------------------

void mouseMotionFunc(int x, int y) {

}

//-----------------------------------------------------------------------------

void keyboardFunc(unsigned char key, int x, int y) {

	switch (key) {

		// -----------------------------------------

#ifdef WIN32
		// exit on escape
	case '\033':

		if (bInit) {
			glDeleteTextures(1, &cameraImageTextureID);
			//cvReleaseCapture(&cvCapture);
			//cap.release();
		}
		exit(0);
		break;
#endif

		// -----------------------------------------

		// switch to fullscreen
		/*
	case 'f':

		bFullsreen = !bFullsreen;
		if (bFullsreen)
			glutFullScreen();
		else {
			glutSetWindow(nWindowID);
			glutPositionWindow(100, 100);
			glutReshapeWindow(640, 480);
		}
		break;

	case 's':
		pause = !pause;
		break;
		*/
		// -----------------------------------------
	}
}

//-----------------------------------------------------------------------------

void specialFunc(int key, int x, int y) {
	//printf("key pressed: %d\n", key);
}

//-----------------------------------------------------------------------------

void countFrames(void)  {

	glut_time = glutGet(GLUT_ELAPSED_TIME);
	cframe++;
	if (glut_time - timebase > 50) {
		sprintf(pixelstring, "fps: %4.2f", cframe*1000.0 / (glut_time - timebase));
		printf("fps: %4.2f\n", cframe*1000.0 / (glut_time - timebase));
		timebase = glut_time;
		cframe = 0;
		// Draw status text and uni-logo:
	}
	glDisable(GL_LIGHTING);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 200, 0, 200);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// render the string
	renderBitmapString(5, 5, 0.0, GLUT_BITMAP_HELVETICA_10, pixelstring);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------

void renderBitmapString(float x, float y, float z, void *font, char *string) {
	char *c;
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

//-----------------------------------------------------------------------------

void displayGlut(int argc, char **argv) {
	if (Parameters::liveTranmitter)
	{
		initGlut(argc, argv);
		glutMainLoop();
	}
}



