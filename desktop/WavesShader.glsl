///////////////////////////////////////////
// Code Common to the wave and the image //
///////////////////////////////////////////

// this will associate a complex number to a color by the evaluation of a polynomial
// the domain is the 5 unit square
// f(z) = z(z-3+0.5i)(z+1-i)(z+2+3i)^2
//		a_5 = 1
// 		a_4 = 2 	+ 5.5i
//		a_3 = -12.5 + 1.5i
//		a_2 = -15 	- 22.5i
// 		a_1 = -29.5 - 47.5i
//		a_0 = 0
// the hue is the argument of f(z)
#beginglsl CodeBlock complex_function
vec3 color_func(in vec2 complex) {
	vec3 color;
	if (abs(complex.x) > 5 || abs(complex.y) > 5) {
		color = vec3(0, 0, 0);
		return color;
	}
	// a_0
	float real = 0;
	float img  = 0;
	// a_1
	float x = complex.x;
	float y = complex.y;
	float t;
	real = real + (-29.5) * x - (-47.5) * y;
	img = img + (-29.5) * y + (-47.5) * x;
	// a_2
	t = x * complex.x - y * complex.y;
	y = x * complex.y + y * complex.x;
	x = t;
	real = real + (-15) * x - (-22.5) * y;
	img = img + (-15) * y + (-22.5) * x;
	// a_3
	t = x * complex.x - y * complex.y;
	y = x * complex.y + y * complex.x;
	x = t;
	real = real + (-12.5) * x - (1.5) * y;
	img = img + (-12.5) * y + (1.5) * x;
	// a_4
	t = x * complex.x - y * complex.y;
	y = x * complex.y + y * complex.x;
	x = t;
	real = real + (2) * x - (5.5) * y;
	img = img + (2) * y + (5.5) * x;
	// a_5
	t = x * complex.x - y * complex.y;
	y = x * complex.y + y * complex.x;
	x = t;
	real = real + x;
	img = img + y;
	// find color based off hue
	if (real == 0 && img == 0) {
		color = vec3(0, 0, 0);
	} else {
		t = 3.0 * (atan(real, img) + 3.1415926535897932384626) / 3.1415926535897932384626;
		x = 1 - abs(t - 2.0 * floor(t / 2.0) - 1);
		if (t < 1) {
			color = vec3(1, x, 0);
		} else if (t < 2) {
			color = vec3(x, 1, 0);
		} else if (t < 3) {
			color = vec3(0, 1, x);
		} else if (t < 4) {
			color = vec3(0, x, 1);
		} else if (t < 5) {
			color = vec3(x, 0, 1);
		} else {
			color = vec3(1, 0, x);
		}
	}
	return color;
}
#endglsl

/////////////////////////
// Code for the shadow //
/////////////////////////
#beginglsl VertexShader vertexShader_shadow
#version 330 core
layout (location = 0) in vec3 wave;
uniform float time;
out float r;

void main() {
    r = wave.y;
	// find the position and gradient of the wave
	float d1 = sqrt((wave.z + 6) * (wave.z + 6) + (wave.x - 6) * (wave.x - 6));
	float d2 = sqrt((wave.z - 6) * (wave.z - 6) + (wave.x - 6) * (wave.x - 6));
	float y = .15 * sin(2 * d1 + 2 * time) + .07 * sin(3 * d2 + 3 * time);
	float c1 = .15 * cos(2 * d1 + 2 * time);
    float c2 = .07 * cos(3 * d2 + 3 * time);
    float nx = 3 * (wave.x - 6) * c2 / d2 + 2 * (wave.x - 6) * c1 / d1;
    float nz = 3 * (wave.z - 6) * c2 / d2 + 2 * (wave.z + 6) * c1 / d1;

    vec3 norm = normalize(vec3(-nx, 1, -nz));
	vec3 light = normalize(vec3(-wave.x, 5.5 - y, -wave.z));

	// compute the output ray from refraction
	vec3 vlat = (dot(light, norm) * norm - light) / 1.13;
	vec3 rayOut = vlat - sqrt(1 - dot(vlat, vlat)) * norm;

	// set the position
	vec3 pos = vec3(wave.x + rayOut.x * (y + 3) / rayOut.y, wave.z + rayOut.z * (y + 3) / rayOut.y, 0);
	gl_Position = vec4(pos, 5.0);
}
#endglsl

#beginglsl FragmentShader fragmentShader_shadow
#version 330 core
out vec4 fragColor;
in float r;
void main() {
	float ds = (gl_PointCoord.s - .5) * 25 / r;
	float dt = (gl_PointCoord.t - .5) * 25 / r;
	float intensity = exp(-(ds * ds + dt * dt)) / 50.0;
	fragColor = vec4(intensity, intensity, intensity, 1);
}
#endglsl

////////////////////////////
// Code for just the wave //
////////////////////////////
#beginglsl VertexShader vertexShader_Waves
#version 330 core
layout (location = 0) in vec3 vertPos;	     // Position in attribute location 0

out vec3 mvPos;

uniform mat4 projectionMatrix;		// The projection matrix
uniform mat4 modelviewMatrix;		// The modelview matrix
uniform float time;

void main()
{
    // set the y value to be the sum of two sine waves with different origins
	float d1 = sqrt((vertPos.z + 6) * (vertPos.z + 6) + (vertPos.x - 6) * (vertPos.x - 6));
	float d2 = sqrt((vertPos.z - 6) * (vertPos.z - 6) + (vertPos.x - 6) * (vertPos.x - 6));
	float y = .15 * sin(2 * d1 + 2 * time) + .07 * sin(3 * d2 + 3 * time);
    vec4 mvPos4 = modelviewMatrix * vec4(vertPos.x, y, vertPos.z, 1.0); 
    gl_Position = projectionMatrix * mvPos4; 
    mvPos = vec3(vertPos.x, y, vertPos.z);
}
#endglsl

#beginglsl FragmentShader fragmentShader_Waves
#version 330 core

uniform mat4 projectionMatrix;		// The projection matrix
uniform mat4 modelviewMatrix;		// The modelview matrix
uniform vec4 viewer;
uniform sampler2D shadow;
uniform float time;

vec3 color_func(in vec2 complex);
in vec3 mvPos;
out vec4 fragmentColor;

void main() {
    vec4 specularColor;
    vec4 diffuseColor;
    vec4 refractiveColor;

    // get light relative to the surface
    vec3 light = normalize(vec3(0, 5.5, 0) - mvPos);

    // find the normal
	float d1 = sqrt((mvPos.z + 6) * (mvPos.z + 6) + (mvPos.x - 6) * (mvPos.x - 6));
	float d2 = sqrt((mvPos.z - 6) * (mvPos.z - 6) + (mvPos.x - 6) * (mvPos.x - 6));
    float c1 = .15 * cos(2 * d1 + 2 * time);
    float c2 = .07 * cos(3 * d2 + 3 * time);
    float nx = 3 * (mvPos.x - 6) * c2 / d2 + 2 * (mvPos.x - 6) * c1 / d1;
    float nz = 3 * (mvPos.z - 6) * c2 / d2 + 2 * (mvPos.z + 6) * c1 / d1;
    vec3 norm = -1 * normalize(vec3(nx, -1, nz));

    // find the viewer position relative to fragment
    vec3 vr = normalize(viewer.xyz / viewer.w);

    if (gl_FrontFacing) { // render into the water
        // diffuse lighting
        diffuseColor = dot(light, norm) * vec4(.2, .2, .5, 0);
    
        // specular lighting
        vec3 reflection = light - 2 * dot(light, norm) * norm;
        float base = dot(reflection, vr);
        if (base > 0.0) base = 0;
        specularColor = pow(-base, 70) * vec4(1, 1, 1, 0);
    
        // refraction image lookup
        vec3 vlat = (dot(vr, norm) * norm - vr) / 1.13;
        float size = vlat.x * vlat.x + vlat.y * vlat.y + vlat.z * vlat.z;
        vec3 rayOut = vlat - sqrt(1 - size) * norm;
        // compute collision with image
        if (rayOut.y == 0) {
            refractiveColor = vec4(0, 0, 0, 0);
        } else {
            rayOut = rayOut / rayOut.y; // now y = 1, x and z are d/dy because light is linear
            vec2 collision = vec2(mvPos.x + rayOut.x * (mvPos.y-3), mvPos.z + rayOut.z * (mvPos.y-3));
			float shade = texture(shadow, (collision + 5.0) / 10.0).x / 2.0 + .5;
            refractiveColor = vec4(shade * color_func(collision), 0);
        }

    } else { // render towards the light
        norm = norm * -1;

        // compute reflection of the image
        vec3 reflection = vr - 2 * dot(vr, norm) * norm;
        if (reflection.y == 0) {
            diffuseColor = vec4(0, 0, 0, 0);
        } else {
            reflection = reflection / reflection.y; // now y = 1, x and z are d/dt
            vec2 collision = vec2(mvPos.x + reflection.x * (mvPos.y-3), mvPos.z + reflection.z * (mvPos.y-3));
            diffuseColor = .5 * vec4(color_func(collision), 0);
        }

        // compute the refraction of the light
        vec3 vlat = (dot(vr, norm) * norm - vr) * 1.13;
        float size = vlat.x * vlat.x + vlat.y * vlat.y + vlat.z * vlat.z;
        if (size <= 1) {
            vec3 rayOut = vlat - sqrt(1 - size) * norm;
            // compute collision with light plane
            if (rayOut.y == 0) {
                specularColor = vec4(0,0,0.2,0);
            } else {
                rayOut = rayOut / rayOut.y;
                vec2 collision = vec2(mvPos.x + (5.5 - mvPos.y) * rayOut.x, mvPos.z + (5.5 - mvPos.y) * rayOut.z);
                if (collision.x * collision.x + collision.y * collision.y < .04) {
                    specularColor = vec4(1, 1, 1, 0);
                } else {
                    specularColor = vec4(0, 0, .2, 0);
                }
            }
        } else {
            // total internal refraction (blue)
            specularColor = vec4(0,0,.2,0);
        }

        refractiveColor = vec4(0,0,0,0);
    }

	fragmentColor = vec4(0, 0, 0, 1.0) + specularColor + .5 * diffuseColor + .5 * refractiveColor;
}

#endglsl

/////////////////////////////
// Code for just the image //
/////////////////////////////

// this will not modify the vert position, and maps the model space to complex space
#beginglsl VertexShader vertexShader_Image
#version 330 core
layout (location = 0) in vec3 vertPos;

out vec2 complex;

uniform mat4 projectionMatrix;		// The projection matrix
uniform mat4 modelviewMatrix;		// The modelview matrix

void main() 
{
	vec4 mvPos = modelviewMatrix * vec4(vertPos, 1.0);
    gl_Position = projectionMatrix * mvPos;
	complex = vec2(vertPos.x, vertPos.z);
}

#endglsl

// this fragment shader will apply the function mapping to the object
#beginglsl FragmentShader fragmentShader_Image
#version 330 core
in vec2 complex;
out vec4 fragmentColor;
uniform sampler2D shadow;

vec3 color_func(in vec2 complex);

void main() {;
	float shade = texture(shadow, (complex + 5) / 10.0).x / 2.0 + .5;
	vec3 color = color_func(complex);
	if (!gl_FrontFacing) {
		fragmentColor = vec4(shade * color, 1);
	} else {
		fragmentColor = vec4(color, 1);
	}
}
#endglsl

