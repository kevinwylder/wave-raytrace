
*****************************************************

SHADERS FOR PHONG LIGHTING, NO TEXTURES

vertexShaderPhongPhong
fragmentShader_PhongPhongBase  - must be combined with shaderCalcPhong code block

vertexShader_PhongGouraudBase  - must be combined with shaderCalcPhong code block
fragmentShader_PhongGouraud

shaderCalcPhong - subroutine for calculating Phong lighting

This code is made available "as is" with warranties of any kind.
Please send bug reports to the author at sambuss@gmail.com.

Author: Sam Buss
March 2018.  All rights reserved.
*****************************************************

#beginglsl VertexShader vertexShader_Wave
#version 330 core
layout (location = 0) in vec3 vertPos;	     // Position in attribute location 0
layout (location = 1) in vec3 vertNormal;	 // Surface normal in attribute location 1
layout (location = 2) in vec3 EmissiveColor; // Surface material properties 
layout (location = 3) in vec3 AmbientColor; 
layout (location = 4) in vec3 DiffuseColor; 
layout (location = 5) in vec3 SpecularColor; 
layout (location = 6) in float SpecularExponent; 

out vec3 mvPos;         // Vertex position in modelview coordinates
out vec3 mvNormalFront; // Normal vector to vertex in modelview coordinates
out vec3 matEmissive;
out vec3 matAmbient;
out vec3 matDiffuse;
out vec3 matSpecular;
out float matSpecExponent;

uniform mat4 projectionMatrix;		// The projection matrix
uniform mat4 modelviewMatrix;		// The modelview matrix
uniform float time;

void main()
{
	float d = sqrt(vertPos.z * vertPos.z + (vertPos.x - 6) * (vertPos.x - 6));
	float y = .5 * sin(d - 2 * time);
	float n = (.5 * cos(d - 2 * time));
    vec4 mvPos4 = modelviewMatrix * vec4(vertPos.x, y, vertPos.z, 1.0); 
    gl_Position = projectionMatrix * mvPos4; 
    mvPos = vec3(mvPos4.x,mvPos4.y,mvPos4.z)/mvPos4.w; 
	vec3 vn = -1 * normalize(vec3(n * (vertPos.x-6) / d, -1, n * (vertPos.z) / d));
    mvNormalFront = normalize(inverse(transpose(mat3(modelviewMatrix)))*vn); // Unit normal from the surface 
    matEmissive = EmissiveColor;
    matAmbient = AmbientColor;
    matDiffuse = DiffuseColor;
    matSpecular = SpecularColor;
    matSpecExponent = SpecularExponent;
}
#endglsl

*******************************************
// The base code for the fragment shader for Phong lighting with Phong shading.
//   This does all the hard work of the Phong lighting
#beginglsl FragmentShader fragmentShader_Project
#version 330 core

in vec3 mvPos;   // Vertex position in modelview coordinates
in vec3 mvNormalFront; // Normal vector to vertex (front facing) in modelview coordinates
in vec3 matEmissive;
in vec3 matAmbient;
in vec3 matDiffuse;
in vec3 matSpecular;
in float matSpecExponent;

layout (std140) uniform phGlobal { 
    vec3 GlobalAmbientColor;        // Global ambient light color 
    int NumLights;                  // Number of lights 
    bool LocalViewer;               // true for local viewer; false for directional viewer 
    bool EnableEmissive;            // Control whether emissive colors are rendered 
    bool EnableDiffuse;             // Control whether diffuse colors are rendered 
    bool EnableAmbient;             // Control whether ambient colors are rendered 
    bool EnableSpecular;            // Control whether specular colors are rendered 
};

const int MaxLights = 8; 
struct phLight { 
    bool IsEnabled;             // True if light is turned on 
    bool IsAttenuated;          // True if attenuation is active 
    bool IsSpotLight;           // True if spotlight 
    bool IsDirectional;         // True if directional 
    vec3 Position; 
    vec3 AmbientColor; 
    vec3 DiffuseColor; 
    vec3 SpecularColor; 
    vec3 SpotDirection;     // Should be unit vector! 
    float SpotCosCutoff;        // Cosine of cutoff angle 
    float SpotExponent; 
    float ConstantAttenuation; 
    float LinearAttenuation; 
    float QuadraticAttenuation; 
};
layout (std140) uniform phLightArray { 
    phLight Lights[MaxLights];
};

vec3 mvNormal;
vec3 nonspecColor;  
vec3 specularColor;  
out vec4 fragmentColor;	// Color that will be used for the fragment
void CalculatePhongLighting();  // Calculate: nonspecColor and specularColor. 

void main() { 
    if ( gl_FrontFacing ) {
        mvNormal = mvNormalFront;
    }
    else {
        mvNormal = -mvNormalFront;
    }
    CalculatePhongLighting();  // Calculate: nonspecColor and specularColor. 
    fragmentColor = vec4(nonspecColor+specularColor, 1.0f);   // Add alpha value of 1.0.
}
#endglsl


#beginglsl VertexShader vertexShader_PhongPhong
#version 330 core
layout (location = 0) in vec3 vertPos;	     // Position in attribute location 0
layout (location = 1) in vec3 vertNormal;	 // Surface normal in attribute location 1
layout (location = 2) in vec3 EmissiveColor; // Surface material properties 
layout (location = 3) in vec3 AmbientColor; 
layout (location = 4) in vec3 DiffuseColor; 
layout (location = 5) in vec3 SpecularColor; 
layout (location = 6) in float SpecularExponent; 

out vec3 mvPos;         // Vertex position in modelview coordinates
out vec3 mvNormalFront; // Normal vector to vertex in modelview coordinates
out vec3 matEmissive;
out vec3 matAmbient;
out vec3 matDiffuse;
out vec3 matSpecular;
out float matSpecExponent;

uniform mat4 projectionMatrix;		// The projection matrix
uniform mat4 modelviewMatrix;		// The modelview matrix
uniform float time;

void main()
{
		float d = sqrt(vertPos.z * vertPos.z + (vertPos.x - 6) * (vertPos.x - 6));
		float y = .5 * sin(d - 2 * time);
		float n = (.5 * cos(d - 2 * time));
    vec4 mvPos4 = modelviewMatrix * vec4(vertPos.x, y, vertPos.z, 1.0); 
    gl_Position = projectionMatrix * mvPos4; 
    mvPos = vec3(mvPos4.x,mvPos4.y,mvPos4.z)/mvPos4.w; 
		vec3 vn = -1 * normalize(vec3(n * (vertPos.x-6) / d, -1, n * (vertPos.z) / d));
    mvNormalFront = normalize(inverse(transpose(mat3(modelviewMatrix)))*vn); // Unit normal from the surface 
    matEmissive = EmissiveColor;
    matAmbient = AmbientColor;
    matDiffuse = DiffuseColor;
    matSpecular = SpecularColor;
    matSpecExponent = SpecularExponent;
}
#endglsl

*******************************************
// The base code for the fragment shader for Phong lighting with Phong shading.
//   This does all the hard work of the Phong lighting
#beginglsl FragmentShader fragmentShader_PhongPhongBase
#version 330 core

in vec3 mvPos;   // Vertex position in modelview coordinates
in vec3 mvNormalFront; // Normal vector to vertex (front facing) in modelview coordinates
in vec3 matEmissive;
in vec3 matAmbient;
in vec3 matDiffuse;
in vec3 matSpecular;
in float matSpecExponent;

layout (std140) uniform phGlobal { 
    vec3 GlobalAmbientColor;        // Global ambient light color 
    int NumLights;                  // Number of lights 
    bool LocalViewer;               // true for local viewer; false for directional viewer 
    bool EnableEmissive;            // Control whether emissive colors are rendered 
    bool EnableDiffuse;             // Control whether diffuse colors are rendered 
    bool EnableAmbient;             // Control whether ambient colors are rendered 
    bool EnableSpecular;            // Control whether specular colors are rendered 
};

const int MaxLights = 8; 
struct phLight { 
    bool IsEnabled;             // True if light is turned on 
    bool IsAttenuated;          // True if attenuation is active 
    bool IsSpotLight;           // True if spotlight 
    bool IsDirectional;         // True if directional 
    vec3 Position; 
    vec3 AmbientColor; 
    vec3 DiffuseColor; 
    vec3 SpecularColor; 
    vec3 SpotDirection;     // Should be unit vector! 
    float SpotCosCutoff;        // Cosine of cutoff angle 
    float SpotExponent; 
    float ConstantAttenuation; 
    float LinearAttenuation; 
    float QuadraticAttenuation; 
};
layout (std140) uniform phLightArray { 
    phLight Lights[MaxLights];
};

vec3 mvNormal;
vec3 nonspecColor;  
vec3 specularColor;  
out vec4 fragmentColor;	// Color that will be used for the fragment
void CalculatePhongLighting();  // Calculate: nonspecColor and specularColor. 

void main() { 
    if ( gl_FrontFacing ) {
        mvNormal = mvNormalFront;
    }
    else {
        mvNormal = -mvNormalFront;
    }
    CalculatePhongLighting();  // Calculate: nonspecColor and specularColor. 
    fragmentColor = vec4(nonspecColor+specularColor, 1.0f);   // Add alpha value of 1.0.
}
#endglsl


*******************************************************
// Shared code for calculating Phong light!
//    Needs as input the global variables (uniforms or attributes or general global):  
//            nonspecColor, specularColor, 
//            EnableEmissive, EnableAmbient, EnableDiffuse, EnableSpecular
//            matEmissive, matAmbient, matDiffuse, matSpecular, matSpecExponent,
//            mvPos, mvNormal, (position and normal in modelview coordinates)
//            Lights[i], phGlobal
//
// Calculates as outputs: nonspecColor and specularColor
//
#beginglsl CodeBlock shaderCalcPhong 
void CalculatePhongLighting() { 
    nonspecColor = vec3(0.0, 0.0, 0.0);  
    specularColor = vec3(0.0, 0.0, 0.0);  
    if ( EnableEmissive ) { 
       nonspecColor = matEmissive; 
    }
    if ( EnableAmbient ) { 
         nonspecColor += matAmbient*GlobalAmbientColor; 
    } 
    vec3 vVector = LocalViewer ? -mvPos : vec3(0.0, 0.0, 1.0); // Unit vector towards non-local viewer 
    vVector = normalize(vVector);
    for ( int i=0; i<NumLights; i++ ) {
        if ( Lights[i].IsEnabled ) { 
            vec3 nonspecColorLt = vec3(0.0, 0.0, 0.0);
            vec3 specularColorLt = vec3(0.0, 0.0, 0.0);
            vec3 lVector = -Lights[i].Position;   // Direction to the light 
            if ( !Lights[i].IsDirectional ) {
                lVector = -(lVector + mvPos);
            }
            lVector = normalize(lVector); // Unit vector to the light position.
            float dotEllNormal = dot(lVector, mvNormal); 
            if (dotEllNormal > 0 ) { 
                float spotCosine;
                if ( Lights[i].IsSpotLight ) {
                    spotCosine = -dot(lVector,Lights[i].SpotDirection);
                }
                if ( !Lights[i].IsSpotLight || spotCosine > Lights[i].SpotCosCutoff ) {
                    if ( EnableDiffuse ) { 
                        nonspecColorLt += matDiffuse*Lights[i].DiffuseColor*dotEllNormal; 
                    } 
                    if ( EnableSpecular ) { 
                        float rDotV = dot(vVector, 2.0*dotEllNormal*mvNormal - lVector); 
                        if ( rDotV>0.0 ) {
                            float specFactor = pow( rDotV, matSpecExponent);
                            specularColorLt += specFactor*matSpecular*Lights[i].SpecularColor; 
                        } 
                    } 
                    if ( Lights[i].IsSpotLight ) {
                        float spotAtten = pow(spotCosine,Lights[i].SpotExponent);
                        nonspecColorLt *= spotAtten; 
                       specularColorLt *= spotAtten;
                    } 
                }
            }
            if ( Lights[i].IsAttenuated ) { 
                float dist = distance(mvPos,Lights[i].Position); 
                float atten = 1.0/(Lights[i].ConstantAttenuation + (Lights[i].LinearAttenuation + Lights[i].QuadraticAttenuation*dist)*dist);
                nonspecColorLt *= atten; 
                specularColorLt *= atten;
            } 
            if ( EnableAmbient ) { 
                nonspecColorLt += matAmbient*Lights[i].AmbientColor; 
            } 
            nonspecColor += nonspecColorLt;
            specularColor += specularColorLt;
        }
    }
}
#endglsl


// Geometry shader: outputs the edges and normals of a triangle
// Vertices and normals have already been transformed into world
//     coordinates by the modelview matrix.
#beginglsl GeometryShader geomShaderNormals2
#version 330 core
layout(triangles) in;              // inputs are triangles
layout(line_strip, max_vertices = 12) out;   // outputs are line strips
in vec3 mvPos[];                    // input: array containing vertex position (in modelview position)
in vec3 mvNormalFront[];			// input: array containing normals (unit vectors) of the vertices
uniform mat4 projectionMatrix;		// The projection matrix
// uniform bool cullBackFaces = true; // cull back faces 
// uniform bool drawEdges = true;     // draw the outlines of triangles
bool cullBackFaces = true; // cull back faces 
bool drawEdges = true;     // draw the outlines of triangles
out vec3 theColor;                 // Vertex color output by the geometry shader
void main()
{
    vec3 edgeColor = vec3( 0.8, 0.2, 0.2 );  // Set the color to draw the edge outlines.
    vec3 normalColor = vec3(1.0, 1.0, 1.0);  // Set the color to draw the normals
    float normalLength = 0.4;             // Length to draw the normals
    vec4 projVertPos[3];                  // Holds vertices transformed by the projection matrix
    for (int i=0; i<3; i++ ) { 
        projVertPos[i] = projectionMatrix*vec4(mvPos[i],1.0); 
    }
    if ( cullBackFaces ) { 
        vec3 vert0 = vec3(projVertPos[0])/projVertPos[0].w; 
        vec3 vert1 = vec3(projVertPos[1])/projVertPos[1].w; 
        vec3 vert2 = vec3(projVertPos[2])/projVertPos[2].w; 
        if ( cross(vert1-vert0,vert2-vert1).z <=0 ) {
            return;
        }
    }
    if ( drawEdges ) { 
        for (int i=0; i<=3; i++ ) { 
            int j = i%3; 
            gl_Position = projVertPos[j];
            theColor = edgeColor;
            EmitVertex();
        }
    }
    EndPrimitive();
    for (int i=0; i<gl_in.length(); i++ ) { 
        theColor = normalColor;
        gl_Position = projVertPos[i];
        EmitVertex();
        vec3 vPos = mvPos[i] + normalLength*mvNormalFront[i]; // Here we use the normal in modelview coordinates
        theColor = normalColor;
        gl_Position = projectionMatrix*vec4(vPos,1.0);
        EmitVertex();
        EndPrimitive();
    }
}
#endglsl

// Set a general color using a fragment shader. (A "fragment" is a "pixel".)
//    The color value is passed in, obtained from the color(s) on the vertice(s).
//    Color values range from 0.0 to 1.0.
//    First three values are Red/Green/Blue (RGB).
//    Fourth color value (alpha) is 1.0, meaning there is no transparency.
#beginglsl FragmentShader fragmentShader_ColorOnly 
#version 330 core
in vec3 theColor;		// Color value came from the vertex shader (smoothed) 
out vec4 FragColor;	// Color that will be used for the fragment

layout (std140) uniform phGlobal { 
    vec3 GlobalAmbientColor;        // Global ambient light color 
    int NumLights;                  // Number of lights 
    bool LocalViewer;               // true for local viewer; false for directional viewer 
    bool EnableEmissive;            // Control whether emissive colors are rendered 
    bool EnableDiffuse;             // Control whether diffuse colors are rendered 
    bool EnableAmbient;             // Control whether ambient colors are rendered 
    bool EnableSpecular;            // Control whether specular colors are rendered 
};

const int MaxLights = 8; 
struct phLight { 
    bool IsEnabled;             // True if light is turned on 
    bool IsAttenuated;          // True if attenuation is active 
    bool IsSpotLight;           // True if spotlight 
    bool IsDirectional;         // True if directional 
    vec3 Position; 
    vec3 AmbientColor; 
    vec3 DiffuseColor; 
    vec3 SpecularColor; 
    vec3 SpotDirection;     // Should be unit vector! 
    float SpotCosCutoff;        // Cosine of cutoff angle 
    float SpotExponent; 
    float ConstantAttenuation; 
    float LinearAttenuation; 
    float QuadraticAttenuation; 
};
layout (std140) uniform phLightArray { 
    phLight Lights[MaxLights];
};


void main()
{
   FragColor = vec4(theColor, 1.0f);   // Add alpha value of 1.0.
}
#endglsl


