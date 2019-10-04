package com.wylder.openglproject;

import android.content.Context;
import android.graphics.PointF;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by kevin on 5/14/18.
 */

public class GraphicsRenderer extends GLSurfaceView implements GLSurfaceView.Renderer, View.OnTouchListener {

    Wave wave;
    Complex complex;

    private float model[] = new float[16];
    private float persp[] = new float[16];
    private float viewer[] = new float[4];
    private float time = 0;

    private float Xmax = 6.0f;                // Control x dimensions of viewable scene
    private float Ymax = 4.0f;                // Control y dimensions of viewable scene
    private float Zmin = -8.0f, Zmax = 8.0f;   // Control z dimensions of the viewable scene
    private float zNear = 15.0f;              // Make this value larger or smaller to affect field of view.

    public GraphicsRenderer(Context context) {
        super(context);
        setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(RENDERMODE_WHEN_DIRTY);
        this.setOnTouchListener(this);

        setModel();
        handler.post(timer);
    }

    public void setModel() {
        Matrix.setIdentityM(model, 0);
        Matrix.translateM(model, 0, 0, 0, -Zmax - zNear);
        Matrix.rotateM(model, 0, azimuth, 1, 0, 0);
        Matrix.rotateM(model, 0, direction, 0, 1, 0);
        Matrix.translateM(model, 0, 0, .5f, 0);

        float r = (Zmax + zNear);
        viewer[0] = (float) (r * Math.sin(-direction * Math.PI / 180) * Math.cos(azimuth * Math.PI / 180));
        viewer[1] = (float) (r * Math.sin(azimuth * Math.PI / 180));
        viewer[2] = (float) (r * Math.cos(-direction * Math.PI / 180) * Math.cos(azimuth * Math.PI / 180));
        viewer[3] = 1;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0, 0, 0, 1);

        GLES20.glEnable( GLES20.GL_DEPTH_TEST );
        GLES20.glDepthFunc(GLES20.GL_LEQUAL);


        wave = new Wave();
        complex = new Complex();

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
        float w = (width == 0) ? 1.0f : (float)width;
        float h = (height == 0) ? 1.0f : (float)height;
        width = width / 2;
        height = height / 2;
        float windowXmax, windowYmax;
        float aspectFactor = w * Ymax / (h * Xmax);   // == (w/h)/(Xmax/Ymax), ratio of aspect ratios
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
        float zFar = zNear + Zmax - Zmin;
        Matrix.frustumM(persp, 0, -windowXmax, windowXmax, -windowYmax, windowYmax, zNear, zFar);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        complex.draw(model, persp);
        wave.draw(model, persp, viewer, time);

    }

    PointF position = new PointF();
    float direction = 0f;
    float azimuth = .25f;

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (event.getPointerCount() >= 2) {
            return false;
        }
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            position.set(event.getX(), event.getY());
        } else if (event.getAction() == MotionEvent.ACTION_MOVE) {
            float dy = event.getY() - position.y;
            float dx = event.getX() - position.x;

            azimuth += dy / 10f;
            direction += dx / 10f;
            setModel();

            position.set(event.getX(), event.getY());
        }
        return true;
    }


    Handler handler = new Handler();
    Runnable timer = new Runnable() {
        @Override
        public void run() {
            handler.postDelayed(timer, 1000 / 60);
            time -= 1.0f / 60.0f;
            requestRender();
        }
    };
}
