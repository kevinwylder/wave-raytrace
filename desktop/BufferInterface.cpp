//
//  MySurfaces.cpp
//
//   Sets up and renders 
//     - the ground plane, and
//     - the surface of rotation
//   for the Math 155A project #4.
//


// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "LinearR4.h"		// Adjust path as needed.

#include "BufferInterface.h"

extern int meshRes;
extern int imageShadowLocation;
extern bool check_for_opengl_errors();

unsigned int vertPos_loc = 0;

// ************************
// General data helping with setting up VAO (Vertex Array Objects)
//    and Vertex Buffer Objects.
// ***********************
const int NumObjects = 4;
const int iFloor = 0;
const int iImage = 1;
const int iLight = 2;
const int iShadow = 3;

// shadow variables 
GLuint intermediateBuffer = 0;
extern GLuint shadowTexture;
GLenum drawType = GL_COLOR_ATTACHMENT0;
const float shadowBoxSize = 8.0f;
const int shadowBoxSteps = 170;
const float shadowGaussRad = 50.0f;

unsigned int myVBO[NumObjects];  // a Vertex Buffer Object holds an array of data
unsigned int myVAO[NumObjects];  // a Vertex Array Object - holds info about an array of vertex data;
unsigned int myEBO[NumObjects];  // a Element Array Buffer Object - holds an array of elements (vertex indices)

// **********************
// This sets up geometries needed for the "Initial" (the 3-D alphabet letter)
//  It is called only once.
// **********************
void MySetupSurfaces() {
    // Initialize the VAO's, VBO's and EBO's for the ground plane,
    // No data is loaded into the VBO's or EBO's until the "Remesh"
    //   routines are called.

    // For the floor:
    // Allocate the needed Vertex Array Objects (VAO's),
    //      Vertex Buffer Objects (VBO's) and Element Array Buffer Objects (EBO's)
    // Since the floor has normal (0,1,0) everywhere, it will be given as
    // generic vertex attribute, not loaded in the VBO as a per-vertex value.
    // The color is also a generic vertex attribute.
    glGenVertexArrays(NumObjects, &myVAO[0]);
    glGenBuffers(NumObjects, &myVBO[0]);
    glGenBuffers(NumObjects, &myEBO[0]);

    glBindVertexArray(myVAO[iFloor]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iFloor]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[iFloor]);

    glVertexAttribPointer(vertPos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(vertPos_loc);
 
    // load the light shape 
    glBindVertexArray(myVAO[iLight]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iLight]);
    glVertexAttribPointer(vertPos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(vertPos_loc);

    RemeshFloor();

    // load the corners of the image to be rendered
    glBindVertexArray(myVAO[iImage]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iImage]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[iImage]);
    glVertexAttribPointer(vertPos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);

    float floorVerts[] = {
        -5.0f, -3.0f, -5.0f,
        5.0f, -3.0f, -5.0f,
        -5.0f, -3.0f, 5.0f,
        5.0f, -3.0f, 5.0f
    };

    unsigned int floorElements[] = {
        0, 1, 2, 3
    };

    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iImage]);
    glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), floorVerts, GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[iImage]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned int), floorElements, GL_STATIC_DRAW); 
    
    glEnableVertexAttribArray(vertPos_loc);

	// put the render locations of the shadow into a buffer
	float *data = new float[shadowBoxSteps * shadowBoxSteps * 3];

	for (int ix = 0; ix < shadowBoxSteps; ix++) {
		for (int iz = 0; iz < shadowBoxSteps; iz++) {
			float x = -shadowBoxSize + (float)(ix * 2 * shadowBoxSize) / (float)shadowBoxSteps;
			float z = -shadowBoxSize + (float)(iz * 2 * shadowBoxSize) / (float)shadowBoxSteps;
			data[3 * (ix + shadowBoxSteps * iz) + 0] = x;
			data[3 * (ix + shadowBoxSteps * iz) + 1] = sqrtf(x * x + z * z) + 5;
			data[3 * (ix + shadowBoxSteps * iz) + 2] = z;
		}
	}

	glBindVertexArray(myVAO[iShadow]);
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[iShadow]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glBufferData(GL_ARRAY_BUFFER, shadowBoxSteps * shadowBoxSteps * 3 * sizeof(float), data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertPos_loc);
	delete[] data;

    check_for_opengl_errors();      // Watch the console window for error messages!

	///////////////////// Create a framebuffer to render into

	glGenFramebuffers(1, &intermediateBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateBuffer);

	glGenTextures(1, &shadowTexture);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 500, 500, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	check_for_opengl_errors();
}


void MyRenderShadow() {
	// bind the intermediate buffer
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateBuffer);

	// tell the framebuffer to render to the texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowTexture, 0);
	glDrawBuffers(1, &drawType);

	// clear the old data
	glClear(GL_COLOR_BUFFER_BIT);
	
	// make circular points
	glEnable(GL_BLEND);
	check_for_opengl_errors();      // Watch the console window for error messages
	glBlendFunc(GL_ONE, GL_ONE);
	check_for_opengl_errors();      // Watch the console window for error messages
	glPointSize(shadowGaussRad);
	check_for_opengl_errors();      // Watch the console window for error messages

	// render the points using shadow shader
	glBindVertexArray(myVAO[iShadow]);
	//glBindBuffer(GL_ARRAY_BUFFER, myVBO[iShadow]);
	glDrawArrays(GL_POINTS, 0, shadowBoxSteps * shadowBoxSteps);

	// bind the OG framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
	check_for_opengl_errors();      // Watch the console window for error messages!
}

void MyRenderWave() {
    // render the floor
    glBindVertexArray(myVAO[iFloor]);

    // Draw the the triangle strips
    for (int i = 0; i < meshRes; i++) {
        glDrawElements(GL_TRIANGLE_STRIP, 2 * (meshRes + 1), GL_UNSIGNED_INT, 
            (void*)(i * 2 * (meshRes + 1) * sizeof(unsigned int)));
    }

    check_for_opengl_errors();      // Watch the console window for error messages!
}

void MyRenderImage() {
    glBindVertexArray(myVAO[iImage]);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (void*) 0);
}

void MyRenderLight() {
    glBindVertexArray(myVAO[iLight]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, meshRes + 1);
}


// *********************************************
// A water plane gridded as an array of rectangles (triangulated)
// All points stored here with y value equal to zero.
// Heights are changed by the vertex shader.
// *********************************************************

void RemeshFloor()
{
    // Floor vertices.
    int numFloorVerts = (meshRes + 1)*(meshRes + 1);    
    float* floorVerts = new float[3 * numFloorVerts];
    // Floor elements 
    int numFloorElts = meshRes * 2 * (meshRes + 1);     
    unsigned int* floorElements = new unsigned int[numFloorElts];
    // light vertices
    float *lightVerts = new float[(meshRes + 1) * 3];

    // define floor verts
    for (int i = 0; i <= meshRes; i++) {
        float z = (-5.0f*(float)(meshRes - i) + 5.0f*(float)i) / (float)meshRes;
        for (int j = 0; j <= meshRes; j++) {
            float x = (-5.0f*(float)(meshRes - j) + 5.0f*(float)j) / (float)meshRes;
            int vrtIdx = 3 * i * (meshRes + 1) + 3 * j;
            floorVerts[vrtIdx] = x;
            floorVerts[vrtIdx + 1] = 0.0f;
            floorVerts[vrtIdx + 2] = z;
        }
    }

    // define floor elems
    for (int i = 0; i < meshRes; i++) {
        for (int j = 0; j <= meshRes; j++) {
            int elt = 2 * i * (meshRes + 1) + 2 * j;
            floorElements[elt] = i * (meshRes + 1) + j;
            floorElements[elt + 1] = (i + 1) * (meshRes + 1) + j;
        }
    }

    // define light verts
    lightVerts[0] = 0;
    lightVerts[1] = 5.75;
    lightVerts[2] = 0;
    for (int i = 0; i < meshRes; i++) {
        lightVerts[3 * (i + 1) + 0] = 0.2f * cosf(6.283185f * i / (meshRes - 1.0f));
        lightVerts[3 * (i + 1) + 1] = 5.5f;
        lightVerts[3 * (i + 1) + 2] = 0.2f * sinf(6.283185f * i / (meshRes - 1.0f));
    }

    // load floor
    glBindVertexArray(myVAO[iFloor]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iFloor]);
    glBufferData(GL_ARRAY_BUFFER, 3 * numFloorVerts * sizeof(float), floorVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[iFloor]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numFloorElts * sizeof(unsigned int), floorElements, GL_STATIC_DRAW); 

    // load light
    glBindVertexArray(myVAO[iLight]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iLight]);
    glBufferData(GL_ARRAY_BUFFER, 3 * (meshRes + 1) * sizeof(float), lightVerts, GL_STATIC_DRAW);

    // Avoid a memory leak by deleting the arrays obtained with "new" above
    delete[] floorVerts;
    delete[] lightVerts;
    delete[] floorElements;
}

