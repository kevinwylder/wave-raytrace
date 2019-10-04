#pragma once

//
// Header file for LetterProg.cpp
//


// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

class LinearMapR4;      // Used in the function prototypes, declared in LinearMapR4.h

//
// External variables.  Can be be used by other .cpp files.
// These external variables are set mostly in LetterProj.cpp,
//    and many of them are used in MyInitial.cpp to control the rendering.
// The external declarations do not give the initial values:
//    Initial values are set with their declaration in LetterPrj.cpp
//


// The next variable controls the resoluton of the meshes for cylinders and spheres.
extern int meshRes;             // Resolution of the meshes (slices, stacks, and rings all equal)

extern LinearMapR4 viewMatrix;		// The current view matrix, based on viewAzimuth and viewDirection.
// Comment: It would be a better design to have two matrices --- a view matrix and model matrix;
//     and have the vertex shader multiply them together; however, it is left as a single matrix for 
//     simplicity and for consistency with Math 155A Project #2.

// We create one shader program: consisting of a vertex shader and a fragment shader
extern const unsigned int verPos_loc;         // Corresponds to "location = 0" in the verter shader definitions
extern int projMatLocation;						// Location of the projectionMatrix in the currently active shader program
extern int modelviewMatLocation;				// Location of the modelviewMatrix in the currently active shader program
extern int timeLocation;

extern float matEntries[16];	// Holds 16 floats (since cannot load doubles into a shader that uses floats)

// ***********************
// Function prototypes
// By declaring function prototypes here, they can be defined in any order desired in the .cpp file.
// ******
bool check_for_opengl_errors();     

bool setup_shaders();

void mySetViewMatrix();  

void myRenderScene();

void my_setup_SceneData();
void my_setup_OpenGL();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_size_callback(GLFWwindow* window, int width, int height);
void error_callback(int error, const char* description);
void setup_callbacks(GLFWwindow* window);
