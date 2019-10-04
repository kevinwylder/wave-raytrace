/*
 * LetterProj.cpp - Version 0.2 - February 5, 2018
 *
 * Starting code for Math 155A, Project #3,
 * 
 * Author: Sam Buss
 *
 * Software accompanying POSSIBLE SECOND EDITION TO the book
 *		3D Computer Graphics: A Mathematical Introduction with OpenGL,
 *		by S. Buss, Cambridge University Press, 2003.
 *
 * Software is "as-is" and carries no warranty.  It may be used without
 *   restriction, but if you modify it, please change the filenames to
 *   prevent confusion between different versions.
 * Bug reports: Sam Buss, sbuss@ucsd.edu.
 * Web page: http://math.ucsd.edu/~sbuss/MathCG2
 */

// These libraries are needed to link the program.
// First five are usually provided by the system.
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"glew32s.lib")
#pragma comment(lib,"glew32.lib")


// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

//#include "LinearR3.h"		// Adjust path as needed.
#include "LinearR4.h"		// Adjust path as needed.
#include "GlShaderMgr.h"

// Enable standard input and output via printf(), etc.
// Put this include *after* the includes for glew and GLFW!
#include <stdio.h>

#include "GlslWaves.h"
#include "BufferInterface.h"



// ********************
// Animation controls and state infornation
// ********************

// These variables control the view direction.
//    The arrow keys are used to change these values.
double viewAzimuth = 0.25;	// Angle of view up/down (in radians)
double viewDirection = 0.0; // Rotation of view around y-axis (in radians)
double deltaAngle = 0.01;	// Change in view angle for each up/down/left/right arrow key press
LinearMapR4 viewMatrix;		// The current view matrix, based on viewAzimuth and viewDirection.

//  The Projection matrix: Controls the "camera view/field-of-view" transformation
//     Generally is the same for all objects in the scene.
LinearMapR4 theProjectionMatrix;		//  The Projection matrix: Controls the "camera/view" transformation

float animateIncrement = 1;
float currentTime = 0;

// The next variable controls the resoluton of the meshes for the water plane.
int meshRes=40;

int shadowProgram;
int shadowTimeLocation;
GLuint shadowTexture;

int waveProgram;
int waveProjMatLocation;
int waveModelviewMatLocation;
int waveViewerLocation;
int waveTimeLocation;
int waveShadowLocation;

int imageProgram;
int imageProjMatLocation;
int imageModelviewMatLocation;
int imageShadowLocation;


// A ModelView matrix controls the placement of a particular object in 3-space.
//     It is generally different for each object.
// The array matEntries holds the matrix values as floats to be loaded into the shader program. 
float matEntries[16];		// Holds 16 floats (since cannot load doubles into a shader that uses floats)

const double Xmax = 6.0;                // Control x dimensions of viewable scene
const double Ymax = 4.0;                // Control y dimensions of viewable scene
const double Zmin = -8.0, Zmax = 8.0;   // Control z dimensions of the viewable scene
// zNear equals the distance from the camera to the z = Zmax plane
const double zNear = 15.0;              // Make this value larger or smaller to affect field of view.


void mySetViewMatrix() {
    viewMatrix.Set_glTranslate(0.0, 0.0, -(Zmax + zNear));      // Translate to be in front of the camera
    viewMatrix.Mult_glRotate(viewAzimuth, 1.0, 0.0, 0.0);	    // Rotate viewAzimuth radians around x-axis
    viewMatrix.Mult_glRotate(-viewDirection, 0.0, 1.0, 0.0);    // Rotate -viewDirection radians around y-axis
    viewMatrix.Mult_glTranslate(0.0, 0.5, 0.0);                // Translate the view is above the water plane
    viewMatrix.DumpByColumns(matEntries);

    // set the view for program 1
    glUseProgram(imageProgram);
    glUniformMatrix4fv(imageModelviewMatLocation, 1, false, matEntries);

    glUseProgram(waveProgram);
    // set the view for program 2
    glUniformMatrix4fv(waveModelviewMatLocation, 1, false, matEntries);

    // set the viewer position (hack)
    float r = Zmax + zNear;

    VectorR4 viewer = VectorR4(r * sin(viewDirection) * cos(viewAzimuth), r * sin(viewAzimuth), r * cos(viewDirection) * cos(viewAzimuth), 1);
    viewer.Dump(matEntries);
    glUniform4f(waveViewerLocation, matEntries[0], matEntries[1], matEntries[2], matEntries[3]);
}

// *************************************
// Main routine for rendering the scene
// myRenderScene() is called every time the scene needs to be redrawn.
// mySetupGeometries() has already created the vertex and buffer objects
//    and the model view matrices.
// The EduPhong shaders are already setup.
// *************************************
void myRenderScene() {
	currentTime -= (1.0f / 60.0f) * animateIncrement;

    // Clear the rendering window
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float clearDepth = 1.0f;
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_DEPTH, 0, &clearDepth);  // Must pass in a *pointer* to the depth

    mySetViewMatrix();
	glUseProgram(shadowProgram);
	glUniform1f(shadowTimeLocation, currentTime);
	
	MyRenderShadow();

    glUseProgram(waveProgram);
	glUniform1i(waveShadowLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glUniform1f(waveTimeLocation, currentTime);
    MyRenderWave();

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!

    glUseProgram(imageProgram);
	glUniform1i(imageShadowLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    MyRenderImage();
    MyRenderLight();

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

bool setup_shaders() {
    bool fileOK = GlShaderMgr::LoadShaderSource("WavesShader.glsl");
    if (!fileOK) {
        return false;
    }

    const char * fshaders[2] = {"fragmentShader_Waves", "complex_function"};
    
    // program for the wave
    unsigned int vertexShader = GlShaderMgr::CompileShader("vertexShader_Waves");
    unsigned int fragmentShader = GlShaderMgr::CompileShader(2, fshaders);
    unsigned int params[2] = {vertexShader, fragmentShader};
    params[0] = vertexShader;
    params[1] = fragmentShader;
    waveProgram = GlShaderMgr::LinkShaderProgram(2, params);
    check_for_opengl_errors();

    // program for the other things
    vertexShader = GlShaderMgr::CompileShader("vertexShader_Image");
    fshaders[0] = "fragmentShader_Image";
    fragmentShader = GlShaderMgr::CompileShader(2, fshaders);
    params[0] = vertexShader;
    params[1] = fragmentShader;
    imageProgram = GlShaderMgr::LinkShaderProgram(2, params);
    check_for_opengl_errors();

	// program for the shadow
	vertexShader = GlShaderMgr::CompileShader("vertexShader_shadow");
	fshaders[0] = "fragmentShader_shadow";
	fragmentShader = GlShaderMgr::CompileShader(1, fshaders);
	params[0] = vertexShader;
	params[1] = fragmentShader;
	shadowProgram = GlShaderMgr::LinkShaderProgram(2, params);


    GlShaderMgr::FinalizeCompileAndLink();
    check_for_opengl_errors();

    waveTimeLocation = glGetUniformLocation(waveProgram, "time");
    waveModelviewMatLocation = glGetUniformLocation(waveProgram, "modelviewMatrix");
    waveProjMatLocation = glGetUniformLocation(waveProgram, "projectionMatrix");
    waveViewerLocation = glGetUniformLocation(waveProgram, "viewer");
	waveShadowLocation = glGetUniformLocation(waveProgram, "shadow");
    imageModelviewMatLocation = glGetUniformLocation(imageProgram, "modelviewMatrix");
    imageProjMatLocation = glGetUniformLocation(imageProgram, "projectionMatrix");
	imageShadowLocation = glGetUniformLocation(imageProgram, "shadow");
	shadowTimeLocation = glGetUniformLocation(shadowProgram, "time");
    return true;
}

void my_setup_SceneData() {
    MySetupSurfaces();  // in BufferLoader

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!

	bool phongOK= setup_shaders();
    if (!phongOK) {
        return;
    }

    mySetViewMatrix();

	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

void my_setup_OpenGL() {
    
    glEnable(GL_DEPTH_TEST);    // Enable depth buffering
    glDepthFunc(GL_LEQUAL);     // Useful for multipass shaders

    // Set polygon drawing mode for front and back of each polygon
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //glEnable(GL_CULL_FACE);

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

double cx, cy = -500;
void cursor_pos_callback (GLFWwindow *window, double x, double y) {
    if (cy == -500) {
        cx = x;
        cy = y;
        return;
    }

    double dD = (cx - x) / 100.0;
    double dA = (y - cy) / 100.0;

    viewDirection += dD;
    viewAzimuth += dA;

    mySetViewMatrix();

    cx = x;
    cy = y;
}


void setup_callbacks(GLFWwindow* window) {
    // Set callback function for resizing the window
    glfwSetFramebufferSizeCallback(window, window_size_callback);

    // Set callback for key up/down/repeat events
    glfwSetKeyCallback(window, key_callback);

    // Set callbacks for mouse movement (cursor position) and mouse botton up/down events.
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    // glfwSetMouseButtonCallback(window, mouse_button_callback);
}

// *******************************************************
// Process all key press events.
// This routine is called each time a key is pressed or released.
// *******************************************************
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    static const double Pi = 3.1415926535897932384626f;
    if (action == GLFW_RELEASE) {
        return;			// Ignore key up (key release) events
    }
    bool viewChanged = false;
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        return;
    case 'M':
        if (mods & GLFW_MOD_SHIFT) {
            meshRes = meshRes < 79 ? meshRes + 1 : 80;  // Uppercase 'M'
        }
        else {
            meshRes = meshRes > 4 ? meshRes - 1 : 3;    // Lowercase 'm'
        }
        RemeshFloor();
        return;
    case GLFW_KEY_UP:
        viewAzimuth = Min(viewAzimuth + 0.02, 2.0);
        viewChanged = true;
        break;
    case GLFW_KEY_DOWN:
        viewAzimuth = Max(viewAzimuth - 0.02, -2.0);
        viewChanged = true;
        break;
    case GLFW_KEY_RIGHT:
        viewDirection += 0.01;
        if (viewDirection > PI) {
            viewDirection -= PI2;
        }
        viewChanged = true;
        break;
    case GLFW_KEY_LEFT:
        viewDirection -= 0.01;
        if (viewDirection < -PI) {
            viewDirection += PI2;
        }
        viewChanged = true;
        break;
    }

    if (viewChanged) {
        mySetViewMatrix();
    }
}


// *************************************************
// This function is called with the graphics window is first created,
//    and again whenever it is resized.
// The Projection View Matrix is typically set here.
//    But this program does not use any transformations or matrices.
// *************************************************
void window_size_callback(GLFWwindow* window, int width, int height) {
	// Define the portion of the window used for OpenGL rendering.
	glViewport(0, 0, width, height);

	// Setup the projection matrix as a perspective view.
	// The complication is that the aspect ratio of the window may not match the
	//		aspect ratio of the scene we want to view.
	double w = (width == 0) ? 1.0 : (double)width;
	double h = (height == 0) ? 1.0 : (double)height;
	double windowXmax, windowYmax;
    double aspectFactor = w * Ymax / (h * Xmax);   // == (w/h)/(Xmax/Ymax), ratio of aspect ratios
	if (aspectFactor>1) {
		windowXmax = Xmax * aspectFactor;
		windowYmax = Ymax;
	}
	else {
		windowYmax = Ymax / aspectFactor;
		windowXmax = Xmax;
	}

	// Using the max & min values for x & y & z that should be visible in the window,
	//		we set up the orthographic projection.
    double zFar = zNear + Zmax - Zmin;
	theProjectionMatrix.Set_glFrustum(-windowXmax, windowXmax, -windowYmax, windowYmax, zNear, zFar);

    // update the projection matrix
    theProjectionMatrix.DumpByColumns(matEntries);
    glUseProgram(waveProgram);
    glUniformMatrix4fv(waveProjMatLocation, 1, false, matEntries);
    glUseProgram(imageProgram);
    glUniformMatrix4fv(imageProjMatLocation, 1, false, matEntries);

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}


int main() {
	glfwSetErrorCallback(error_callback);	// Supposed to be called in event of errors. (doesn't work?)
    //glewExperimental = GL_TRUE;
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

	const int initWidth = 800;
	const int initHeight = 600;
	GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "Phong Demo", NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window!\n");
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (GLEW_OK != glewInit()) {
		printf("Failed to initialize GLEW!.\n");
		return -1;
	}

	// Print info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
#ifdef GL_SHADING_LANGUAGE_VERSION
	printf("Supported GLSL version is %s.\n", (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
    printf("Using GLEW version %s.\n", glewGetString(GLEW_VERSION));

	printf("------------------------------\n");
    printf("Press arrow keys to adjust the view direction.\n");
    printf("Press 'M' (mesh) to increase the mesh resolution.\n");
    printf("Press 'm' (mesh) to decrease the mesh resolution.\n");
    printf("Press ESCAPE or 'X' or 'x' to exit.\n");
	
    setup_callbacks(window);
   
	// Initialize OpenGL, the scene and the shaders
    my_setup_OpenGL();
	my_setup_SceneData();
 	window_size_callback(window, initWidth, initHeight);
    glfwSetWindowPos(window, 100, 100);

    // Loop while program is not terminated.
	while (!glfwWindowShouldClose(window)) {
	
	
		myRenderScene();				// Render into the current buffer
		glfwSwapBuffers(window);		// Displays what was just rendered (using double buffering).
        check_for_opengl_errors();

		// Poll events (key presses, mouse events)
		glfwWaitEventsTimeout(1.0/60.0);	    // Use this to animate at 60 frames/sec (timing is NOT reliable)
		// glfwWaitEvents();					// Or, Use this instead if no animation.
		// glfwPollEvents();					// Use this version when animating as fast as possible
	}

	glfwTerminate();
	return 0;
}

void error_callback(int error, const char* description)
{
    // Print error
    fputs(description, stderr);
}

// If an error is found, it could have been caused by any command since the
//   previous call to check_for_opengl_errors()
// To find what generated the error, you can try adding more calls to
//   check_for_opengl_errors().
char errNames[8][36] = {
	"Unknown OpenGL error",
	"GL_INVALID_ENUM", "GL_INVALID_VALUE", "GL_INVALID_OPERATION",
	"GL_INVALID_FRAMEBUFFER_OPERATION", "GL_OUT_OF_MEMORY",
	"GL_STACK_UNDERFLOW", "GL_STACK_OVERFLOW" };
bool check_for_opengl_errors() {
	int numErrors = 0;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		numErrors++;
		int errNum = 0;
		switch (err) {
		case GL_INVALID_ENUM:
			errNum = 1;
			break;
		case GL_INVALID_VALUE:
			errNum = 2;
			break;
		case GL_INVALID_OPERATION:
			errNum = 3;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errNum = 4;
			break;
		case GL_OUT_OF_MEMORY:
			errNum = 5;
			break;
		case GL_STACK_UNDERFLOW:
			errNum = 6;
			break;
		case GL_STACK_OVERFLOW:
			errNum = 7;
			break;
		}
		printf("OpenGL ERROR: %s.\n", errNames[errNum]);
	}
	return (numErrors != 0);
}
