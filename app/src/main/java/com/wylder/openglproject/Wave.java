package com.wylder.openglproject;

import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

/**
 * Created by kevin on 5/14/18.
 */

public class Wave {

    FloatBuffer vertices;
    ShortBuffer elements;

    int meshRes = 50;
    int numFloorVerts = (meshRes + 1)*(meshRes + 1);
    int numFloorElts = meshRes * 2 * (meshRes + 1);

    int program;
    int timeLoc;
    int perspMatrixLoc;
    int modelMatrixLoc;
    int viewerLoc;

    public Wave() {

        int fragmentShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(fragmentShader, fragShader);
        GLES20.glCompileShader(fragmentShader);

        int vertexShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        GLES20.glShaderSource(vertexShader, vertShader);
        GLES20.glCompileShader(vertexShader);

        program = GLES20.glCreateProgram();
        GLES20.glAttachShader(program, vertexShader);
        GLES20.glAttachShader(program, fragmentShader);
        GLES20.glLinkProgram(program);

        perspMatrixLoc = GLES20.glGetUniformLocation(program, "projectionMatrix");
        modelMatrixLoc = GLES20.glGetUniformLocation(program, "modelviewMatrix");
        timeLoc = GLES20.glGetUniformLocation(program, "time");
        viewerLoc = GLES20.glGetUniformLocation(program, "viewer");

        // Floor vertices.
        float floorVerts[] = new float[3 * numFloorVerts];
        // Floor elements
        short floorElements[] = new short[numFloorElts];

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
                floorElements[elt] = (short) (i * (meshRes + 1) + j);
                floorElements[elt + 1] = (short) ((i + 1) * (meshRes + 1) + j);
            }
        }

        ByteBuffer floorVertData = ByteBuffer.allocateDirect(numFloorVerts * 3 * 4);
        floorVertData.order(ByteOrder.nativeOrder());
        vertices = floorVertData.asFloatBuffer();
        vertices.put(floorVerts);
        vertices.position(0);

        ByteBuffer elementData = ByteBuffer.allocateDirect(numFloorElts * 4);
        elementData.order(ByteOrder.nativeOrder());
        elements = elementData.asShortBuffer();
        elements.put(floorElements);
        elements.position(0);

    }

    public void draw(float[] model, float[] persp, float[] viewer, float time) {
        GLES20.glUseProgram(program);

        GLES20.glUniformMatrix4fv(modelMatrixLoc, 1, false, model, 0);
        GLES20.glUniformMatrix4fv(perspMatrixLoc, 1, false, persp, 0);
        GLES20.glUniform1f(timeLoc, time);
        GLES20.glUniform4f(viewerLoc, viewer[0], viewer[1], viewer[2], viewer[3]);

        // position is location = 0
        GLES20.glEnableVertexAttribArray(0);
        GLES20.glVertexAttribPointer(0, 3, GLES20.GL_FLOAT, false, 12, vertices);

        for (int i = 0; i < meshRes; i++) {
            elements.position(i * 2 * (meshRes + 1));
            GLES20.glDrawElements(GLES20.GL_TRIANGLE_STRIP, 2 * (meshRes + 1), GLES20.GL_UNSIGNED_SHORT, elements);
        }
    }


    String vertShader =
            "#version 300 es\n" +
                    "layout (location = 0) in vec3 vertPos;\t     // Position in attribute location 0\n" +
                    "\n" +
                    "out vec3 mvPos;\n" +
                    "\n" +
                    "uniform mat4 projectionMatrix;\t\t// The projection matrix\n" +
                    "uniform mat4 modelviewMatrix;\t\t// The modelview matrix\n" +
                    "uniform mediump float time;\n" +
                    "uniform vec4 viewer;\n" +
                    "\n" +
                    "void main()\n" +
                    "{\n" +
                    "    // set the y value to be the sum of two sine waves with different origins\n" +
                    "\tfloat d1 = sqrt((vertPos.z + 6.0) * (vertPos.z + 6.0) + (vertPos.x - 6.0) * (vertPos.x - 6.0));\n" +
                    "\tfloat d2 = sqrt((vertPos.z - 6.0) * (vertPos.z - 6.0) + (vertPos.x - 6.0) * (vertPos.x - 6.0));\n" +
                    "\tfloat y = .15 * sin(2.0 * d1 + 2.0 * time) + .07 * sin(3.0 * d2 + 3.0 * time);\n" +
                    "    vec4 mvPos4 = modelviewMatrix * vec4(vertPos.x, y, vertPos.z, 1.0); \n" +
                    "    gl_Position = projectionMatrix * mvPos4; \n" +
                    "    mvPos = vec3(vertPos.x, y, vertPos.z);\n" +
                    "}\n";


    String fragShader =
            "#version 300 es\n" +
                    "\n" +
                    "uniform mat4 projectionMatrix;\t\t// The projection matrix\n" +
                    "uniform mat4 modelviewMatrix;\t\t// The modelview matrix\n" +
                    "uniform mediump float time;\n" +
                    "uniform vec4 viewer;\n" +
                    "\n" +
                    "vec3 color_func(in vec2 complex) {\n" +
                    "\tvec3 color;\n" +
                    "\tif (abs(complex.x) > 5.0 || abs(complex.y) > 5.0) {\n" +
                    "\t\tcolor = vec3(0, 0, 0);\n" +
                    "\t\treturn color;\n" +
                    "\t}\n" +
                    "\t// a_0\n" +
                    "\tfloat real = 0.0;\n" +
                    "\tfloat img  = 0.0;\n" +
                    "\t// a_1\n" +
                    "\tfloat x = complex.x;\n" +
                    "\tfloat y = complex.y;\n" +
                    "\tfloat t;\n" +
                    "\treal = real + (-29.5) * x - (-47.5) * y;\n" +
                    "\timg = img + (-29.5) * y + (-47.5) * x;\n" +
                    "\t// a_2\n" +
                    "\tt = x * complex.x - y * complex.y;\n" +
                    "\ty = x * complex.y + y * complex.x;\n" +
                    "\tx = t;\n" +
                    "\treal = real + (-15.0) * x - (-22.5) * y;\n" +
                    "\timg = img + (-15.0) * y + (-22.5) * x;\n" +
                    "\t// a_3\n" +
                    "\tt = x * complex.x - y * complex.y;\n" +
                    "\ty = x * complex.y + y * complex.x;\n" +
                    "\tx = t;\n" +
                    "\treal = real + (-12.5) * x - (1.5) * y;\n" +
                    "\timg = img + (-12.5) * y + (1.5) * x;\n" +
                    "\t// a_4\n" +
                    "\tt = x * complex.x - y * complex.y;\n" +
                    "\ty = x * complex.y + y * complex.x;\n" +
                    "\tx = t;\n" +
                    "\treal = real + (2.0) * x - (5.5) * y;\n" +
                    "\timg = img + (2.0) * y + (5.5) * x;\n" +
                    "\t// a_5\n" +
                    "\tt = x * complex.x - y * complex.y;\n" +
                    "\ty = x * complex.y + y * complex.x;\n" +
                    "\tx = t;\n" +
                    "\treal = real + x;\n" +
                    "\timg = img + y;\n" +
                    "\t// find color based off hue\n" +
                    "\tif (real == 0.0 && img == 0.0) {\n" +
                    "\t\tcolor = vec3(0, 0, 0);\n" +
                    "\t} else {\n" +
                    "\t\tt = 3.0 * (atan(real, img) + 3.14159265358) / 3.14159265358;\n" +
                    "\t\tx = 1.0 - abs(t - 2.0 * floor(t / 2.0) - 1.0);\n" +
                    "\t\tif (t < 1.0) {\n" +
                    "\t\t\tcolor = vec3(1, x, 0);\n" +
                    "\t\t} else if (t < 2.0) {\n" +
                    "\t\t\tcolor = vec3(x, 1, 0);\n" +
                    "\t\t} else if (t < 3.0) {\n" +
                    "\t\t\tcolor = vec3(0, 1, x);\n" +
                    "\t\t} else if (t < 4.0) {\n" +
                    "\t\t\tcolor = vec3(0, x, 1);\n" +
                    "\t\t} else if (t < 5.0) {\n" +
                    "\t\t\tcolor = vec3(x, 0, 1);\n" +
                    "\t\t} else {\n" +
                    "\t\t\tcolor = vec3(1, 0, x);\n" +
                    "\t\t}\n" +
                    "\t}\n" +
                    "\treturn color;\n" +
                    "}\n" +

                    "in vec3 mvPos;\n" +
                    "out vec4 fragmentColor;\n" +
                    "\n" +
                    "void main() {\n" +
                    "    vec4 specularColor = vec4(0, 0, 0, 0);\n" +
                    "    vec4 diffuseColor = vec4(0, 0, 0, 0);\n" +
                    "    vec4 refractiveColor = vec4(0, 0, 0, 0);\n" +
                    "\n" +
                    "    // get light relative to the surface\n" +
                    "    vec3 light = normalize(vec3(0, 1.5, 0) - mvPos);\n" +
                    "\n" +
                    "    // find the normal\n" +
                    "\tfloat d1 = sqrt((mvPos.z + 6.0) * (mvPos.z + 6.0) + (mvPos.x - 6.0) * (mvPos.x - 6.0));\n" +
                    "\tfloat d2 = sqrt((mvPos.z - 6.0) * (mvPos.z - 6.0) + (mvPos.x - 6.0) * (mvPos.x - 6.0));\n" +
                    "    float c1 = .15 * cos(2.0 * d1 + 2.0 * time);\n" +
                    "    float c2 = .07 * cos(3.0 * d2 + 3.0 * time);\n" +
                    "    float nx = 3.0 * (mvPos.x - 6.0) * c2 / d2 + 2.0 * (mvPos.x - 6.0) * c1 / d1;\n" +
                    "    float nz = 3.0 * (mvPos.z - 6.0) * c2 / d2 + 2.0 * (mvPos.z + 6.0) * c1 / d1;\n" +
                    "    vec3 norm = -1.0 * normalize(vec3(nx, -1, nz));\n" +
                    "\n" +
                    "    // find the viewer position relative to fragment\n" +
                    "    vec3 vr = normalize(viewer.xyz / viewer.w);\n" +
                    "\n" +
                    "    if (gl_FrontFacing) { // render into the water\n" +
                    "        // diffuse lighting\n" +
                    "        diffuseColor = dot(light, norm) * vec4(.5, .5, 1, 0);\n" +
                    "    \n" +
                    "        // specular lighting\n" +
                    "        vec3 reflection = normalize(light - 2.0 * dot(light, norm) * norm);\n" +
                    "float base = dot(reflection, -vr);" +
                    " if (base < 0.0) {base = 0.0;}" +
                    "        specularColor = pow(base, 40.0) * vec4(1, 1, 1, 0);\n" +
                    "    \n" +
                    "        // refraction image lookup\n" +
                    "        vec3 vlat = (dot(vr, norm) * norm - vr) / 1.13;\n" +
                    "        float size = vlat.x * vlat.x + vlat.y * vlat.y + vlat.z * vlat.z;\n" +
                    "        vec3 rayOut = vlat - sqrt(1.0 - size) * norm;\n" +
                    "        // compute collision with image\n" +
                    "        if (rayOut.y == 0.0) {\n" +
                    "            refractiveColor = vec4(0, 0, 0, 0);\n" +
                    "        } else {\n" +
                    "            rayOut = rayOut / rayOut.y; // now y = 1, x and z are d/dy because light is linear\n" +
                    "            vec2 collision = vec2(mvPos.x + rayOut.x * (mvPos.y-3.0), mvPos.z + rayOut.z * (mvPos.y-3.0));\n" +
                    "            refractiveColor = vec4(color_func(collision), 0);\n" +
                    "        }\n" +
                    "\n" +
                    "    } else { // render towards the light\n" +
                    "        norm = norm * -1.0;\n" +
                    "\n" +
                    "        // compute reflection of the image\n" +
                    "        vec3 reflection = vr - 2.0 * dot(vr, norm) * norm;\n" +
                    "        if (reflection.y == 0.0) {\n" +
                    "            diffuseColor = vec4(0, 0, 0, 0);\n" +
                    "        } else {\n" +
                    "            reflection = reflection / reflection.y; // now y = 1, x and z are d/dt\n" +
                    "            vec2 collision = vec2(mvPos.x + reflection.x * (mvPos.y-3.0), mvPos.z + reflection.z * (mvPos.y-3.0));\n" +
                    "            diffuseColor = .2 * vec4(color_func(collision), 0);\n" +
                    "        }\n" +
                    "\n" +
                    "        // compute the refraction of the light\n" +
                    "        vec3 vlat = (dot(vr, norm) * norm - vr) * 1.13;\n" +
                    "        float size = vlat.x * vlat.x + vlat.y * vlat.y + vlat.z * vlat.z;\n" +
                    "        if (size <= 1.0) {\n" +
                    "            vec3 rayOut = vlat - sqrt(1.0 - size) * norm;\n" +
                    "            // compute collision with light plane\n" +
                    "            if (rayOut.y == 0.0) {\n" +
                    "                specularColor = vec4(0,0,0.2,0);\n" +
                    "            } else {\n" +
                    "                rayOut = rayOut / rayOut.y;\n" +
                    "                vec2 collision = vec2(mvPos.x + (1.5 - mvPos.y) * rayOut.x, mvPos.z + (1.5 - mvPos.y) * rayOut.z);\n" +
                    "                if (collision.x * collision.x + collision.y * collision.y < .04) {\n" +
                    "                    specularColor = vec4(1, 1, 1, 0);\n" +
                    "                } else {\n" +
                    "                    specularColor = vec4(0, 0, .2, 0);\n" +
                    "                }\n" +
                    "            }\n" +
                    "        } else {\n" +
                    "            // total internal refraction (blue)\n" +
                    "            specularColor = vec4(0,0,.2,0);\n" +
                    "        }\n" +
                    "\n" +
                    "    }\n" +
                    "\n" +
                    "\tfragmentColor = vec4(0, 0, 0, 1.0) + specularColor + .5 * diffuseColor + .5 * refractiveColor;\n" +
                    " if (fragmentColor.x > 1.0) {fragmentColor.x = 1.0;}" +
                    " if (fragmentColor.y > 1.0) {fragmentColor.y = 1.0;}" +
                    " if (fragmentColor.z > 1.0) {fragmentColor.z = 1.0;}" +
                    " if (fragmentColor.w > 1.0) {fragmentColor.w = 1.0;}" +
                    "}\n";

}
