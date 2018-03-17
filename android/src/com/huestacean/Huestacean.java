package com.huestacean;

import android.content.Intent;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.view.Surface;
import android.util.DisplayMetrics;
import com.android.grafika.gles.*;
import android.media.projection.*;
import android.graphics.SurfaceTexture;
import java.nio.*;
import android.graphics.Bitmap;
import android.hardware.display.*;
import android.app.Activity;
import android.util.Log;

public class Huestacean extends org.qtproject.qt5.android.bindings.QtActivity implements SurfaceTexture.OnFrameAvailableListener
{
    private static final int REQUEST_MEDIA_PROJECTION = 1;

    private static Huestacean m_instance;
    private static MediaProjectionManager mMediaProjectionManager;
    private static MediaProjection mMediaProjection;

    private static int mResultCode;
    private static Intent mResultData;

    private static EglCore eglCore;

    private static Surface producer;
    private static SurfaceTexture texture;
    private static int textureId;

    private static Texture2dProgram shader;
    private static FullFrameRect screen;

    private static OffscreenSurface consumer;

    private static int width;
    private static int height;

    private static final String TAG = "Huestacean";

    public native void handleFrame(int w, int h);

    public Huestacean()
    {
        m_instance = this;
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_MEDIA_PROJECTION) {
            if (resultCode != Activity.RESULT_OK) {
                return;
            }

            mMediaProjection = mMediaProjectionManager.getMediaProjection(resultCode, data);
            StartCapture();
        }
    }

    public static void StartCapture()
    {
        Log.v(TAG, "StartCapture");

        if(mMediaProjectionManager == null)
        {
            Log.v(TAG, "getSystemService");
            mMediaProjectionManager = (MediaProjectionManager)
                            m_instance.getSystemService(Context.MEDIA_PROJECTION_SERVICE);
            m_instance.startActivityForResult(mMediaProjectionManager.createScreenCaptureIntent(),
                                REQUEST_MEDIA_PROJECTION);
            return;
        }

        if(mMediaProjection == null
            || eglCore != null)
        {
            Log.v(TAG, "Bail out, already running");
            return;
        }

        Log.v(TAG, "Starting java side of cap");

        DisplayMetrics metrics = new DisplayMetrics();
        m_instance.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int screenDensity = metrics.densityDpi;

        // dimensions of the Display, or whatever you wanted to read from
        width = 100;
        height = 100;

        // feel free to try FLAG_RECORDABLE if you want
        eglCore = new EglCore(null, EglCore.FLAG_TRY_GLES3);

        consumer = new OffscreenSurface(eglCore, width, height);
        consumer.makeCurrent();

        shader = new Texture2dProgram(Texture2dProgram.ProgramType.TEXTURE_EXT);
        screen = new FullFrameRect(shader);

        texture = new SurfaceTexture(textureId = screen.createTextureObject(), false);
        texture.setDefaultBufferSize(width, height);
        producer = new Surface(texture);
        texture.setOnFrameAvailableListener(m_instance);

        mMediaProjection.createVirtualDisplay("huestacean", width, height, screenDensity, DisplayManager.VIRTUAL_DISPLAY_FLAG_AUTO_MIRROR, producer, null, null);
    }

    public static void StopCapture()
    {
        //TODO: cleanup
    }

    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
      if(eglCore == null) return;

      Log.v(TAG, "onFrameAvailable");

      consumer.makeCurrent();

      texture.updateTexImage();
      float[] matrix = new float[4*4];
      texture.getTransformMatrix(matrix);

      consumer.makeCurrent();

      screen.drawFrame(textureId, matrix);
      consumer.swapBuffers();

      // call into C++ to have it glReadPixels
      handleFrame(width, height);
    }
}
