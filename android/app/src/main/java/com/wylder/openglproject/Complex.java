package com.wylder.openglproject;

import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * Created by kevin on 5/14/18.
 */

public class Complex {

    int program;

    FloatBuffer lightVerts;
    FloatBuffer imageVerts;

    float imageCoords[] = {
            -5.0f, -3.0f, -5.0f,
            5.0f, -3.0f, -5.0f,
            -5.0f, -3.0f, 5.0f,
            5.0f, -3.0f, 5.0f
    };

    int modelMatrixLoc;
    int perspMatrixLoc;

    public Complex() {

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

        ByteBuffer imageData = ByteBuffer.allocateDirect(imageCoords.length * 4);
        imageData.order(ByteOrder.nativeOrder());
        imageVerts = imageData.asFloatBuffer();
        imageVerts.put(imageCoords);
        imageVerts.position(0);

        float lightCoords[] = new float[3 * (10 + 1)];
        lightCoords[0] = 0;
        lightCoords[1] = 1.75f;
        lightCoords[2] = 0;
        for (int i = 0; i < 10; i++) {
            lightCoords[3 * (i + 1) + 0] = 0.2f * (float) Math.cos(6.283185f * i / (10 - 1.0f));
            lightCoords[3 * (i + 1) + 1] = 1.5f;
            lightCoords[3 * (i + 1) + 2] = 0.2f * (float) Math.sin(6.283185f * i / (10 - 1.0f));
        }

        ByteBuffer lightData = ByteBuffer.allocateDirect(lightCoords.length * 4);
        lightData.order(ByteOrder.nativeOrder());
        lightVerts = lightData.asFloatBuffer();
        lightVerts.put(lightCoords);
        lightVerts.position(0);

    }

    public void draw(float[] model, float[] perspective) {
        GLES20.glUseProgram(program);

        GLES20.glUniformMatrix4fv(modelMatrixLoc, 1, false, model, 0);
        GLES20.glUniformMatrix4fv(perspMatrixLoc, 1, false, perspective, 0);

        GLES20.glEnableVertexAttribArray(0);
        GLES20.glVertexAttribPointer(0, 3, GLES20.GL_FLOAT, false, 12, imageVerts);
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glVertexAttribPointer(0, 3, GLES20.GL_FLOAT, false, 12, lightVerts);
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_FAN, 0, 10 + 1);

    }

    private String vertShader =
            "#version 300 es\nlayout (location = 0) in vec3 vertPos;\n" +
            "out vec2 complex;\n" +
            "uniform mat4 projectionMatrix;\t\t// The projection matrix\n" +
            "uniform mat4 modelviewMatrix;\t\t// The modelview matrix\n" +
            "void main() \n" +
                    "{\n" +
                    "\tvec4 mvPos = modelviewMatrix * vec4(vertPos, 1.0);\n" +
                    "    gl_Position = projectionMatrix * mvPos;\n" +
                    "\tcomplex = vec2(vertPos.x, vertPos.z);\n" +
            "}\n";

    private String fragShader =
            "#version 300 es\nin vec2 complex;\n" +
            "out vec4 fragmentColor;\n" +
                    "void main() {\n" +
                    "\tvec3 color;\n" +
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
                    "\t\tcolor = vec3(0.0, 0.0, 0.0);\n" +
                    "\t} else {\n" +
                    "\t\tt = 3.0 * (atan(real, img) + 3.1415926535897932384626) / 3.1415926535897932384626;\n" +
                    "\t\tx = 1.0 - abs(t - 2.0 * floor(t / 2.0) - 1.0);\n" +
                    "\t\tif (t < 1.0) {\n" +
                    "\t\t\tcolor = vec3(1.0, x, 0.0);\n" +
                    "\t\t} else if (t < 2.0) {\n" +
                    "\t\t\tcolor = vec3(x, 1.0, 0.0);\n" +
                    "\t\t} else if (t < 3.0) {\n" +
                    "\t\t\tcolor = vec3(0.0, 1.0, x);\n" +
                    "\t\t} else if (t < 4.0) {\n" +
                    "\t\t\tcolor = vec3(0.0, x, 1.0);\n" +
                    "\t\t} else if (t < 5.0) {\n" +
                    "\t\t\tcolor = vec3(x, 0.0, 1.0);\n" +
                    "\t\t} else {\n" +
                    "\t\t\tcolor = vec3(1.0, 0.0, x);\n" +
                    "\t}}\n" +
                    "if (abs(complex.x) > 5.0 || abs(complex.y) > 5.0) {\n" +
                    "\t\tcolor = vec3(0.0, 0.0, 0.0);\n" +
                    "\t}\n" +
                    "float d = real * real + img * img;\n" +
                    "color = color * (1.0 - 1.0 / (d*d/9.0 + 1.0));" +
                    "\tfragmentColor = vec4(color, 1.0);\n" +
                    "}\n";

}
