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

package com.android.rs.image2;

import android.app.Activity;

import android.content.Intent;
import android.os.Bundle;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.support.v8.renderscript.*;
import android.os.Handler;
import android.os.Message;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.view.View;
import android.graphics.Point;

import android.util.Log;


public class ImageProcessingActivity2 extends Activity
                                       implements SeekBar.OnSeekBarChangeListener {
    private final String TAG = "Img";

    private Spinner mSpinner;
    private SeekBar mBar1;
    private SeekBar mBar2;
    private SeekBar mBar3;
    private SeekBar mBar4;
    private SeekBar mBar5;

    private int mBars[] = new int[5];
    private int mBarsOld[] = new int[5];

    private TextView mText1;
    private TextView mText2;
    private TextView mText3;
    private TextView mText4;
    private TextView mText5;
    private ImageView mDisplayView;

    private int mTestList[];
    private float mTestResults[];

    private boolean mToggleLong;
    private boolean mTogglePause;
    private boolean mToggleAnimate;
    private boolean mToggleDisplay;
    private int mBitmapWidth;
    private int mBitmapHeight;
    private boolean mDemoMode;

    // Updates pending is a counter of how many kernels have been
    // sent to RS for processing
    //
    // In benchmark this is incremented each time a kernel is launched and
    // decremented each time a kernel completes
    //
    // In demo mode, each UI input increments the counter and it is zeroed
    // when the latest settings are sent to RS for processing.
    private int mUpdatesPending;

    // In demo mode this is used to count updates in the pipeline.  It's
    // incremented when work is submitted to RS and decremented when invalidate is
    // called to display a result.
    private int mShowsPending;



    /////////////////////////////////////////////////////////////////////////

    // Message processor to handle notifications for when kernel completes
    private class MessageProcessor extends RenderScript.RSMessageHandler {
        MessageProcessor() {
        }

        public void run() {
            synchronized(mProcessor) {
                // In demo mode, decrement the pending displays and notify the
                // UI processor it can now enqueue more work if additional updates
                // are blocked by a full pipeline.
                if (mShowsPending > 0) {
                    mShowsPending --;
                    mProcessor.notifyAll();
                    if (mToggleDisplay) {
                        mProcessor.mHandler.sendMessage(Message.obtain());
                    }
                }
            }
        }
    }


    /////////////////////////////////////////////////////////////////////////
    // Processor is a helper thread for running the work without
    // blocking the UI thread.
    class Processor extends Thread {
        RenderScript mRS;
        ScriptC_util mScriptUtils;
        Allocation mInPixelsAllocation;
        Allocation mInPixelsAllocation2;
        Allocation mOutDisplayAllocation1;
        Allocation mOutDisplayAllocation2;
        Bitmap mBitmapOut1;
        Bitmap mBitmapOut2;
        int mActiveBitmap;

        private float mLastResult;
        private boolean mRun = true;
        private boolean mDoingBenchmark;
        private TestBase mTest;

        private boolean mBenchmarkMode;

        // We don't want to call the "changed" methods excessively as this
        // can cause extra work for drivers.  Before running a test update
        // any bars which have changed.
        void runTest() {
            if (mBars[0] != mBarsOld[0]) {
                mTest.onBar1Changed(mBars[0]);
                mBarsOld[0] = mBars[0];
            }
            if (mBars[1] != mBarsOld[1]) {
                mTest.onBar2Changed(mBars[1]);
                mBarsOld[1] = mBars[1];
            }
            if (mBars[2] != mBarsOld[2]) {
                mTest.onBar3Changed(mBars[2]);
                mBarsOld[2] = mBars[2];
            }
            if (mBars[3] != mBarsOld[3]) {
                mTest.onBar4Changed(mBars[3]);
                mBarsOld[3] = mBars[3];
            }
            if (mBars[4] != mBarsOld[4]) {
                mTest.onBar5Changed(mBars[4]);
                mBarsOld[4] = mBars[4];
            }
            mTest.runTest();
        }

        Processor(RenderScript rs, boolean benchmarkMode) {
            mRS = rs;
            mRS.setMessageHandler(new MessageProcessor());
            mScriptUtils = new ScriptC_util(mRS);

            switch(mBitmapWidth) {
            case 1920:
                mInPixelsAllocation = Allocation.createFromBitmapResource(
                        mRS, getResources(), R.drawable.img1920x1080a);
                mInPixelsAllocation2 = Allocation.createFromBitmapResource(
                        mRS, getResources(), R.drawable.img1920x1080b);
                mBitmapOut1 = Bitmap.createBitmap(1920, 1080, Bitmap.Config.ARGB_8888);
                mBitmapOut2 = Bitmap.createBitmap(1920, 1080, Bitmap.Config.ARGB_8888);
                break;
            case 1280:
                mInPixelsAllocation = Allocation.createFromBitmapResource(
                        mRS, getResources(), R.drawable.img1280x720a);
                mInPixelsAllocation2 = Allocation.createFromBitmapResource(
                        mRS, getResources(), R.drawable.img1280x720b);
                mBitmapOut1 = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
                mBitmapOut2 = Bitmap.createBitmap(1280, 720, Bitmap.Config.ARGB_8888);
                break;
            case 800:
                mInPixelsAllocation = Allocation.createFromBitmapResource(
                        mRS, getResources(), R.drawable.img800x450a);
                mInPixelsAllocation2 = Allocation.createFromBitmapResource(
                        mRS, getResources(), R.drawable.img800x450b);
                mBitmapOut1 = Bitmap.createBitmap(800, 450, Bitmap.Config.ARGB_8888);
                mBitmapOut2 = Bitmap.createBitmap(800, 450, Bitmap.Config.ARGB_8888);
                break;
            }

            mBitmapOut1.setHasAlpha(false);
            mBitmapOut2.setHasAlpha(false);
            mOutDisplayAllocation1 = Allocation.createFromBitmap(mRS, mBitmapOut1);
            mOutDisplayAllocation2 = Allocation.createFromBitmap(mRS, mBitmapOut2);
            mBenchmarkMode = benchmarkMode;
            start();
        }

        class Result {
            float totalTime;
            int itterations;
        }

        // Run one loop of kernels for at least the specified minimum time.
        // The function returns the average time in ms for the test run
        private Result runBenchmarkLoop(float minTime) {
            mUpdatesPending = 0;
            Result r = new Result();

            long t = java.lang.System.currentTimeMillis();
            do {
                synchronized(this) {
                    // Shows pending is used to track the number of kernels in the RS pipeline
                    // We throttle it to 2.  This provide some buffering to allow a kernel to be started
                    // before we are nofitied the previous finished.  However, larger numbers are uncommon
                    // in interactive apps as they introduce 'lag' between user input and display.
                    mShowsPending++;
                    if (mShowsPending > 2) {
                        try {
                            this.wait();
                        } catch(InterruptedException e) {
                        }
                    }
                }

                // If animations are enabled update the test state.
                if (mToggleAnimate) {
                    mTest.animateBars(r.totalTime);
                }

                // Run the kernel
                if (mActiveBitmap == 0) {
                    mTest.mOutPixelsAllocation = mOutDisplayAllocation1;
                } else {
                    mTest.mOutPixelsAllocation = mOutDisplayAllocation2;
                }
                mTest.runTest();
                r.itterations ++;

                if (mToggleDisplay) {
                    if (mActiveBitmap == 0) {
                        mOutDisplayAllocation1.copyTo(mBitmapOut1);
                    } else {
                        mOutDisplayAllocation2.copyTo(mBitmapOut2);
                    }
                }

                // Send our RS message handler a message so we know when this work has completed
                //mRS.sendMessage(mActiveBitmap, null);
                mScriptUtils.invoke_utilSendMessage(mActiveBitmap);
                mActiveBitmap ^= 1;

                long t2 = java.lang.System.currentTimeMillis();
                r.totalTime += (t2 - t) / 1000.f;
                t = t2;
            } while (r.totalTime < minTime);

            // Wait for any stray operations to complete and update the final time
            mRS.finish();
            long t2 = java.lang.System.currentTimeMillis();
            r.totalTime += (t2 - t) / 1000.f;
            t = t2;
            return r;
        }


        // Get a benchmark result for a specific test
        private float getBenchmark() {
            mDoingBenchmark = true;
            mUpdatesPending = 0;

            long result = 0;
            float runtime = 1.f;
            if (mToggleLong) {
                runtime = 10.f;
            }

            // We run a short bit of work before starting the actual test
            // this is to let any power management do its job and respond
            runBenchmarkLoop(0.3f);

            // Run the actual benchmark
            Result r = runBenchmarkLoop(runtime);

            Log.v("rs", "Test: time=" + r.totalTime +"s,  frames=" + r.itterations +
                  ", avg=" + r.totalTime / r.itterations * 1000.f);

            mDoingBenchmark = false;
            return r.totalTime / r.itterations * 1000.f;
        }

        private int mDisplayedBitmap;
        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                if (mDisplayedBitmap == 0) {
                    mDisplayView.setImageBitmap(mBitmapOut1);
                } else {
                    mDisplayView.setImageBitmap(mBitmapOut2);
                }
                mDisplayedBitmap ^= 1;
                mDisplayView.invalidate();
            }
        };

        public void run() {
            while (mRun) {
                // Our loop for launching tests or benchmarks
                synchronized(this) {
                    // If we have no work to do, or we have displays pending, wait
                    if ((mUpdatesPending == 0) || (mShowsPending != 0)) {
                        try {
                            this.wait();
                        } catch(InterruptedException e) {
                        }
                    }

                    // We may have been asked to exit while waiting
                    if (!mRun) return;
                }

                if (mBenchmarkMode) {
                    // Loop over the tests we want to benchmark
                    for (int ct=0; (ct < mTestList.length) && mRun; ct++) {

                        // For reproducibility we wait a short time for any sporadic work
                        // created by the user touching the screen to launch the test to pass.
                        // Also allows for things to settle after the test changes.
                        mRS.finish();
                        try {
                            sleep(250);
                        } catch(InterruptedException e) {
                        }

                        // If we just ran a test, we destroy it here to relieve some memory pressure
                        if (mTest != null) {
                            mTest.destroy();
                        }

                        // Select the next test
                        mTest = changeTest(mTestList[ct], false);

                        // If the user selected the "long pause" option, wait
                        if (mTogglePause) {
                            for (int i=0; (i < 100) && mRun; i++) {
                                try {
                                    sleep(100);
                                } catch(InterruptedException e) {
                                }
                            }
                        }

                        // Run the test
                        mTestResults[ct] = getBenchmark();
                    }
                    onBenchmarkFinish(mRun);
                    return;
                } else {
                    boolean update = false;
                    synchronized(this) {
                        // If we have updates to process and are not blocked by pending shows,
                        // start the next kernel
                        if ((mUpdatesPending > 0) && (mShowsPending == 0)) {
                            mUpdatesPending = 0;
                            update = true;
                            mShowsPending++;
                        }
                    }

                    if (update) {
                        // Run the kernel
                        if (mActiveBitmap == 0) {
                            mTest.mOutPixelsAllocation = mOutDisplayAllocation1;
                        } else {
                            mTest.mOutPixelsAllocation = mOutDisplayAllocation2;
                        }
                        runTest();

                        if (mToggleDisplay) {
                            if (mActiveBitmap == 0) {
                                mOutDisplayAllocation1.copyTo(mBitmapOut1);
                            } else {
                                mOutDisplayAllocation2.copyTo(mBitmapOut2);
                            }
                        }

                        // Send our RS message handler a message so we know when this work has completed
                        //mRS.sendMessage(mActiveBitmap, null);
                        mScriptUtils.invoke_utilSendMessage(mActiveBitmap);
                        mActiveBitmap ^= 1;
                    }
                }
            }

        }

        public void update() {
            // something UI related has changed, enqueue an update if one is not
            // already pending.  Wake the worker if needed
            synchronized(this) {
                if (mUpdatesPending < 2) {
                    mUpdatesPending++;
                    notifyAll();
                }
            }
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

            if (mTest != null) {
                mTest.destroy();
                mTest = null;
            }
            mOutDisplayAllocation1.destroy();
            mOutDisplayAllocation2.destroy();
            mRS.destroy();

            mInPixelsAllocation = null;
            mInPixelsAllocation2 = null;
            mOutDisplayAllocation1 = null;
            mOutDisplayAllocation2 = null;
            mRS = null;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    private boolean mDoingBenchmark;
    public Processor mProcessor;

    TestBase changeTest(IPTestList.TestName t, boolean setupUI) {
        TestBase tb = IPTestList.newTest(t);

        tb.createBaseTest(this);
        if (setupUI) {
            setupBars(tb);
        }
        return tb;
    }

    TestBase changeTest(int id, boolean setupUI) {
        IPTestList.TestName t = IPTestList.TestName.values()[id];
        return changeTest(t, setupUI);
    }

    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            if (seekBar == mBar1) {
                mBars[0] = progress;
            } else if (seekBar == mBar2) {
                mBars[1] = progress;
            } else if (seekBar == mBar3) {
                mBars[2] = progress;
            } else if (seekBar == mBar4) {
                mBars[3] = progress;
            } else if (seekBar == mBar5) {
                mBars[4] = progress;
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

    void hideBars() {
        mSpinner.setVisibility(View.INVISIBLE);

        mBar1.setVisibility(View.INVISIBLE);
        mText1.setVisibility(View.INVISIBLE);

        mBar2.setVisibility(View.INVISIBLE);
        mText2.setVisibility(View.INVISIBLE);

        mBar3.setVisibility(View.INVISIBLE);
        mText3.setVisibility(View.INVISIBLE);

        mBar4.setVisibility(View.INVISIBLE);
        mText4.setVisibility(View.INVISIBLE);

        mBar5.setVisibility(View.INVISIBLE);
        mText5.setVisibility(View.INVISIBLE);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mDisplayView = (ImageView) findViewById(R.id.display);
        //mDisplayView.setImageBitmap(mBitmapOut);

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
        mProcessor.exit();
    }

    public void onBenchmarkFinish(boolean ok) {
        if (ok) {
            Intent intent = new Intent();
            intent.putExtra("tests", mTestList);
            intent.putExtra("results", mTestResults);
            setResult(RESULT_OK, intent);
        } else {
            setResult(RESULT_CANCELED);
        }
        finish();
    }


    void startProcessor() {
        if (!mDemoMode) {
            hideBars();
        }

        Point size = new Point();
        getWindowManager().getDefaultDisplay().getSize(size);

        int mScreenWidth = size.x;
        int mScreenHeight = size.y;

        int tw = mBitmapWidth;
        int th = mBitmapHeight;

        if (tw > mScreenWidth || th > mScreenHeight) {
            float s1 = (float)tw / (float)mScreenWidth;
            float s2 = (float)th / (float)mScreenHeight;

            if (s1 > s2) {
                tw /= s1;
                th /= s1;
            } else {
                tw /= s2;
                th /= s2;
            }
        }

        android.util.Log.v("rs", "TV sizes " + tw + ", " + th);

        mProcessor = new Processor(RenderScript.create(this), !mDemoMode);
        if (mDemoMode) {
            mProcessor.mTest = changeTest(mTestList[0], true);
        }
        mProcessor.update();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Intent i = getIntent();
        mTestList = i.getIntArrayExtra("tests");
        mToggleLong = i.getBooleanExtra("enable long", false);
        mTogglePause = i.getBooleanExtra("enable pause", false);
        mToggleAnimate = i.getBooleanExtra("enable animate", false);
        mToggleDisplay = i.getBooleanExtra("enable display", false);
        mBitmapWidth = i.getIntExtra("resolution X", 0);
        mBitmapHeight = i.getIntExtra("resolution Y", 0);
        mDemoMode = i.getBooleanExtra("demo", false);

        mTestResults = new float[mTestList.length];

        startProcessor();
    }

    protected void onDestroy() {
        super.onDestroy();
    }

}
