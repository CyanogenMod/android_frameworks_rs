/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.rs.imagejb;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.view.SurfaceView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.view.View;
import android.view.TextureView;
import android.view.Surface;
import android.graphics.SurfaceTexture;
import android.util.Log;
import android.renderscript.ScriptC;
import android.renderscript.RenderScript;
import android.renderscript.Type;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.Script;

import android.os.Environment;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class ImageProcessingActivityJB extends Activity
                                       implements SeekBar.OnSeekBarChangeListener,
                                                  TextureView.SurfaceTextureListener {
    private final String TAG = "Img";
    public final String RESULT_FILE = "image_processing_result.csv";

    Bitmap mBitmapIn;
    Bitmap mBitmapIn2;

    private Spinner mSpinner;
    private SeekBar mBar1;
    private SeekBar mBar2;
    private SeekBar mBar3;
    private SeekBar mBar4;
    private SeekBar mBar5;
    private TextView mText1;
    private TextView mText2;
    private TextView mText3;
    private TextView mText4;
    private TextView mText5;
    private TextureView mDisplayView;

    private int mTestList[];
    private float mTestResults[];

    private boolean mToggleIO;
    private boolean mToggleDVFS;
    private boolean mToggleLong;
    private boolean mTogglePause;


    /////////////////////////////////////////////////////////////////////////

    class Processor extends Thread {
        RenderScript mRS;
        Allocation mInPixelsAllocation;
        Allocation mInPixelsAllocation2;
        Allocation mOutPixelsAllocation;

        private Surface mOutSurface;
        private float mLastResult;
        private boolean mRun = true;
        private int mOp = 0;
        private boolean mDoingBenchmark;
        private TestBase mTest;
        private TextureView mDisplayView;

        private boolean mBenchmarkMode;


        Processor(RenderScript rs, TextureView v, boolean benchmarkMode) {
            mRS = rs;
            mDisplayView = v;
            mInPixelsAllocation = Allocation.createFromBitmap(mRS, mBitmapIn);
            mInPixelsAllocation2 = Allocation.createFromBitmap(mRS, mBitmapIn2);
            mOutPixelsAllocation = Allocation.createTyped(mRS, mInPixelsAllocation.getType(),
                                                               Allocation.MipmapControl.MIPMAP_NONE,
                                                               Allocation.USAGE_SCRIPT |
                                                               Allocation.USAGE_IO_OUTPUT);
            mBenchmarkMode = benchmarkMode;
            start();
        }

        private float getBenchmark() {
            mDoingBenchmark = true;

            mTest.setupBenchmark();
            long result = 0;
            long runtime = 1000;
            if (mToggleLong) {
                runtime = 10000;
            }

            if (mToggleDVFS) {
                mDvfsWar.go();
            }

            Log.v("rs", "Warming");
            long t = java.lang.System.currentTimeMillis() + 250;
            do {
                mTest.runTest();
                mTest.finish();
            } while (t > java.lang.System.currentTimeMillis());
            //mHandler.sendMessage(Message.obtain());

            Log.v("rs", "Benchmarking");
            int ct = 0;
            t = java.lang.System.currentTimeMillis();
            do {
                mTest.runTest();
                mTest.finish();
                ct++;
            } while ((t + runtime) > java.lang.System.currentTimeMillis());
            t = java.lang.System.currentTimeMillis() - t;
            float ft = (float)t;
            ft /= ct;

            mTest.exitBenchmark();
            mDoingBenchmark = false;

            android.util.Log.v("rs", "bench " + ft);
            return ft;
        }

        private Handler mHandler = new Handler() {
            // Allow the filter to complete without blocking the UI
            // thread.  When the message arrives that the op is complete
            // we will either mark completion or start a new filter if
            // more work is ready.  Either way, display the result.
            @Override
            public void handleMessage(Message msg) {
                synchronized(this) {
                    if (mRS == null || mOutPixelsAllocation == null) {
                        return;
                    }
                    mOutPixelsAllocation.ioSend();
                    mDisplayView.invalidate();
                    //mTest.runTestSendMessage();
                }
            }
        };

        public void run() {
            Surface lastSurface = null;
            while (mRun) {
                synchronized(this) {
                    try {
                        this.wait();
                    } catch(InterruptedException e) {
                    }
                    if (!mRun) return;

                    if ((mOutSurface == null) || (mOutPixelsAllocation == null)) {
                        continue;
                    }

                    if (lastSurface != mOutSurface) {
                        mOutPixelsAllocation.setSurface(mOutSurface);
                        lastSurface = mOutSurface;
                    }
                }

                if (mBenchmarkMode) {
                    for (int ct=0; ct < mTestList.length; ct++) {
                        mRS.finish();

                        try {
                            sleep(1000);
                        } catch(InterruptedException e) {
                        }

                        if (mTest != null) {
                            mTest.destroy();
                        }

                        mTest = changeTest(mTestList[ct]);
                        if (mTogglePause) {
                            try {
                                sleep(10000);
                            } catch(InterruptedException e) {
                            }
                        }

                        mTestResults[ct] = getBenchmark();
                        mHandler.sendMessage(Message.obtain());
                    }
                    onBenchmarkFinish();
                }
            }

        }

        public void update() {
            synchronized(this) {
                if (mOp == 0) {
                    mOp = 2;
                }
                notifyAll();
            }
        }

        public void setSurface(Surface s) {
            synchronized(this) {
                mOutSurface = s;
                notifyAll();
            }
            //update();
        }

        public void exit() {
            mRun = false;

            synchronized(this) {
                notifyAll();
            }

            try {
                this.join();
            } catch(InterruptedException e) {
            }

            mInPixelsAllocation.destroy();
            mInPixelsAllocation2.destroy();
            mOutPixelsAllocation.destroy();
            mRS.destroy();

            mInPixelsAllocation = null;
            mInPixelsAllocation2 = null;
            mOutPixelsAllocation = null;
            mRS = null;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    static class DVFSWorkaround {
        static class spinner extends Thread {
            boolean mRun = true;
            long mNextSleep;

            spinner() {
                setPriority(MIN_PRIORITY);
                start();
            }

            public void run() {
                while (mRun) {
                    Thread.yield();
                    synchronized(this) {
                        long t = java.lang.System.currentTimeMillis();
                        if (t > mNextSleep) {
                            try {
                                this.wait();
                            } catch(InterruptedException e) {
                            }
                        }
                    }
                }
            }

            public void go(long t) {
                synchronized(this) {
                    mNextSleep = t;
                    notifyAll();
                }
            }
        }

        spinner s1;
        DVFSWorkaround() {
            s1 = new spinner();
        }

        void go() {
            long t = java.lang.System.currentTimeMillis() + 2000;
            s1.go(t);
        }

        void destroy() {
            synchronized(this) {
                s1.mRun = false;
                notifyAll();
            }
        }
    }
    DVFSWorkaround mDvfsWar = new DVFSWorkaround();

    ///////////////////////////////////////////////////////////


    private boolean mDoingBenchmark;
    public Processor mProcessor;


    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            mDisplayView.invalidate();
        }
    };

    public void updateDisplay() {
        mHandler.sendMessage(Message.obtain());
        //mProcessor.update();
    }

    TestBase changeTest(int id) {
        IPTestListJB.TestName t = IPTestListJB.TestName.values()[id];
        TestBase tb = IPTestListJB.newTest(t);
        tb.createBaseTest(this);
        //setupBars(tb);
        return tb;
    }

    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            if (seekBar == mBar1) {
                mProcessor.mTest.onBar1Changed(progress);
            } else if (seekBar == mBar2) {
                mProcessor.mTest.onBar2Changed(progress);
            } else if (seekBar == mBar3) {
                mProcessor.mTest.onBar3Changed(progress);
            } else if (seekBar == mBar4) {
                mProcessor.mTest.onBar4Changed(progress);
            } else if (seekBar == mBar5) {
                mProcessor.mTest.onBar5Changed(progress);
            }
            mProcessor.update();
        }
    }

    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    public void onStopTrackingTouch(SeekBar seekBar) {
    }

    void setupBars(TestBase t) {
        mSpinner.setVisibility(View.VISIBLE);
        t.onSpinner1Setup(mSpinner);

        mBar1.setVisibility(View.VISIBLE);
        mText1.setVisibility(View.VISIBLE);
        t.onBar1Setup(mBar1, mText1);

        mBar2.setVisibility(View.VISIBLE);
        mText2.setVisibility(View.VISIBLE);
        t.onBar2Setup(mBar2, mText2);

        mBar3.setVisibility(View.VISIBLE);
        mText3.setVisibility(View.VISIBLE);
        t.onBar3Setup(mBar3, mText3);

        mBar4.setVisibility(View.VISIBLE);
        mText4.setVisibility(View.VISIBLE);
        t.onBar4Setup(mBar4, mText4);

        mBar5.setVisibility(View.VISIBLE);
        mText5.setVisibility(View.VISIBLE);
        t.onBar5Setup(mBar5, mText5);
    }


    void cleanup() {
        synchronized(this) {
            mProcessor.exit();
        }

        mBitmapIn = null;
        mBitmapIn2 = null;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mBitmapIn = loadBitmap(R.drawable.img1600x1067);
        mBitmapIn2 = loadBitmap(R.drawable.img1600x1067b);

        mDisplayView = (TextureView) findViewById(R.id.display);

        mSpinner = (Spinner) findViewById(R.id.spinner1);

        mBar1 = (SeekBar) findViewById(R.id.slider1);
        mBar2 = (SeekBar) findViewById(R.id.slider2);
        mBar3 = (SeekBar) findViewById(R.id.slider3);
        mBar4 = (SeekBar) findViewById(R.id.slider4);
        mBar5 = (SeekBar) findViewById(R.id.slider5);

        mBar1.setOnSeekBarChangeListener(this);
        mBar2.setOnSeekBarChangeListener(this);
        mBar3.setOnSeekBarChangeListener(this);
        mBar4.setOnSeekBarChangeListener(this);
        mBar5.setOnSeekBarChangeListener(this);

        mText1 = (TextView) findViewById(R.id.slider1Text);
        mText2 = (TextView) findViewById(R.id.slider2Text);
        mText3 = (TextView) findViewById(R.id.slider3Text);
        mText4 = (TextView) findViewById(R.id.slider4Text);
        mText5 = (TextView) findViewById(R.id.slider5Text);
    }

    @Override
    protected void onPause() {
        super.onPause();

        cleanup();
    }

    public void onBenchmarkFinish() {
        Intent intent = new Intent();
        intent.putExtra("tests", mTestList);
        intent.putExtra("results", mTestResults);
        setResult(RESULT_OK, intent);
        finish();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Intent i = getIntent();
        mTestList = i.getIntArrayExtra("tests");

        mToggleIO = i.getBooleanExtra("enable io", true);
        mToggleDVFS = i.getBooleanExtra("enable dvfs", true);
        mToggleLong = i.getBooleanExtra("enable long", false);
        mTogglePause = i.getBooleanExtra("enable pause", false);

        mTestResults = new float[mTestList.length];

        mProcessor = new Processor(RenderScript.create(this), mDisplayView, true);
        mDisplayView.setSurfaceTextureListener(this);
    }

    protected void onDestroy() {
        super.onDestroy();
    }

    private Bitmap loadBitmap(int resource) {
        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeResource(getResources(), resource, options);
    }



    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mProcessor.setSurface(new Surface(surface));
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        mProcessor.setSurface(new Surface(surface));
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mProcessor.setSurface(null);
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }
}
