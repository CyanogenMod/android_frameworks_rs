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

package com.android.rs.sgtest;

import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.Sampler;
import android.renderscript.Script;
import android.renderscript.Type;
import android.renderscript.Matrix3f;
import android.renderscript.Matrix4f;
import android.renderscript.ScriptGroup;
import android.renderscript.ScriptGroup2;
import android.util.Log;

import java.lang.reflect.Constructor;
import java.lang.Math;
import java.util.HashMap;

public class Filters extends TestBase {

  interface FilterInterface {
    public void init();
    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b);
    public Script.KernelID getKernelID();
      public ScriptGroup2.Closure asyncLaunch(ScriptGroup2.Builder builder,
                                              Object in, Type outputType);
    public void forEach(Allocation in, Allocation out);
  }

    abstract class FilterBase implements FilterInterface {
        public ScriptGroup2.Closure asyncLaunch(ScriptGroup2.Builder builder,
                                                Object in, Type outputType) {
            return builder.addKernel(getKernelID(), outputType, new Object[] { in },
                                     new HashMap<Script.FieldID, Object>());
        }
    }

  /*

    Template for a subclass that implements Filter.

  class Filter implements Filter {
    Filter(RenderScript RS) { s = new ScriptC_(RS); }

    void init() {}

    Script.KernelID getKernelID() { return s.getKernelID_(); }

    void forEach(Allocation in, Allocation out) { s.forEach_(in, out); }

    private ScriptC_ s;
  }
  */

  class ColorMatrixFilter extends FilterBase {
    public ColorMatrixFilter(RenderScript RS) { s_mat = new ScriptC_colormatrix_f(RS); }

    public void init() { }

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) { /* TODO */ return null; }

    public Script.KernelID getKernelID() { return s_mat.getKernelID_colormatrix(); }

    public void forEach(Allocation in, Allocation out) { s_mat.forEach_colormatrix(in, out); }

    private ScriptC_colormatrix_f s_mat;
  }

  class ContrastFilter extends FilterBase {
    public ContrastFilter(RenderScript RS) { s = new ScriptC_contrast_f(RS); }

    public void init() {}

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) { return null; }

    public Script.KernelID getKernelID() { return s.getKernelID_contrast(); }

    public void forEach(Allocation in, Allocation out) { s.forEach_contrast(in, out); }

    private ScriptC_contrast_f s;
  }

  class ExposureFilter extends FilterBase {
    public ExposureFilter(RenderScript RS) { s = new ScriptC_exposure_f(RS); }

    public void init() {}

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) { return null; }

    public Script.KernelID getKernelID() { return s.getKernelID_exposure(); }

    public void forEach(Allocation in, Allocation out) { s.forEach_exposure(in, out); }

    private ScriptC_exposure_f s;
  }

  class FisheyeFilter extends FilterBase {
    public FisheyeFilter(RenderScript RS) {
        mRS = RS;
        s = new ScriptC_fisheye_approx_relaxed_f(RS);
    }

    public void init() {
        // s.set_sampler(Sampler.CLAMP_LINEAR(mRS));
    }

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) {
        return b.addInvoke(s.getInvokeID_init_filter(),
                           new Object[] { new Integer(dimX), new Integer(dimY),
                                          new Float(0.5f), new Float(0.5f), new Float(0.5f),
                                          Sampler.CLAMP_LINEAR(mRS)},
                           new HashMap<Script.FieldID, Object>());
    }

    public Script.KernelID getKernelID() { return s.getKernelID_fisheye(); }

    public ScriptGroup2.Closure asyncLaunch(ScriptGroup2.Builder builder,
                                            Object in, Type outputType) {
        HashMap<Script.FieldID, Object> globals = new HashMap<Script.FieldID, Object>();
        globals.put(s.getFieldID_in_alloc(), in);
        //globals.put(s.getFieldID_sampler(), Sampler.CLAMP_LINEAR(mRS));
        return builder.addKernel(getKernelID(), outputType, new Object[0], globals);
    }

    public void forEach(Allocation in, Allocation out) {
        s.set_in_alloc(in);
        s.forEach_fisheye(out);
    }

      private RenderScript mRS;
      private ScriptC_fisheye_approx_relaxed_f s;
      private final int dimX=1067, dimY=1600;
  }

  class GreyFilter extends FilterBase {
    public GreyFilter(RenderScript RS) { s = new ScriptC_greyscale_f(RS); }

    public void init() {}

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) { return null; }

    public Script.KernelID getKernelID() { return s.getKernelID_greyscale(); }

    public void forEach(Allocation in, Allocation out) { s.forEach_greyscale(in, out); }

    private ScriptC_greyscale_f s;
  }

  class LevelsFilter extends FilterBase {
    public LevelsFilter(RenderScript RS) { s = new ScriptC_levels_relaxed_f(RS); }
    private final float mSaturation = 1.0f;
    private final float mInBlack = 0.0f; // 0-255
    private final float mOutBlack = 0.0f; // 0-255
    private final float mInWhite = 255.0f; // 0-255
    private final float mOutWhite = 255.0f; // 0-255
    private final float mInWMinInB = mInWhite - mInBlack;
    private final float mOutWMinOutB = mOutWhite - mOutBlack;
    private final float mOverInWMinInB = 1.f / mInWMinInB;
    private Matrix3f mSatMatrix;

    private void setLevels() {
      s.set_inBlack(mInBlack);
      s.set_outBlack(mOutBlack);
      s.set_inWMinInB(mInWMinInB);
      s.set_outWMinOutB(mOutWMinOutB);
      s.set_overInWMinInB(mOverInWMinInB);
    }

    private void setSaturation() {
      Matrix3f satMatrix = new Matrix3f();
      float rWeight = 0.299f;
      float gWeight = 0.587f;
      float bWeight = 0.114f;
      float oneMinusS = 1.0f - mSaturation;

      satMatrix.set(0, 0, oneMinusS * rWeight + mSaturation);
      satMatrix.set(0, 1, oneMinusS * rWeight);
      satMatrix.set(0, 2, oneMinusS * rWeight);
      satMatrix.set(1, 0, oneMinusS * gWeight);
      satMatrix.set(1, 1, oneMinusS * gWeight + mSaturation);
      satMatrix.set(1, 2, oneMinusS * gWeight);
      satMatrix.set(2, 0, oneMinusS * bWeight);
      satMatrix.set(2, 1, oneMinusS * bWeight);
      satMatrix.set(2, 2, oneMinusS * bWeight + mSaturation);
      s.set_colorMat(satMatrix);

      mSatMatrix = satMatrix;
    }

    public void init() {
      setSaturation();
      setLevels();
    }

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) {
        return b.addInvoke(s.getInvokeID_initialize(),
                           new Object[] { new Float(mInBlack), new Float(mOutBlack),
                                          new Float(mInWMinInB), new Float(mOutWMinOutB),
                                          new Float(mOverInWMinInB), mSatMatrix },
                           new HashMap<Script.FieldID, Object>());
    }

    public Script.KernelID getKernelID() { return s.getKernelID_levels_v4(); }

    public void forEach(Allocation in, Allocation out) { s.forEach_levels_v4(in, out); }

    private ScriptC_levels_relaxed_f s;
  }

  class ShadowsFilter extends FilterBase {
    public ShadowsFilter(RenderScript RS) { s = new ScriptC_shadows_f(RS); }

    public void init() { s.invoke_prepareShadows(50.f); }

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) {
      cInit = b.addInvoke(s.getInvokeID_prepareShadows(),
          new Object[] { new Float(50.f) },
          new HashMap<Script.FieldID, Object>());
      return cInit;
    }

    public Script.KernelID getKernelID() { return s.getKernelID_shadowsKernel(); }

    public void forEach(Allocation in, Allocation out) { s.forEach_shadowsKernel(in, out); }

    private ScriptC_shadows_f s;
    private ScriptGroup2.Closure cInit;
  }

  class VibranceFilter extends FilterBase {
    public VibranceFilter(RenderScript RS) { s = new ScriptC_vibrance_f(RS); }

    public void init() {}

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) { return null; }

    public Script.KernelID getKernelID() { return s.getKernelID_vibranceKernel(); }

    public void forEach(Allocation in, Allocation out) { s.forEach_vibranceKernel(in, out); }

    private ScriptC_vibrance_f s;
  }

  class VignetteFilter extends FilterBase {
    public VignetteFilter(RenderScript RS) { s = new ScriptC_vignette_approx_relaxed_f(RS); }
    private final float center_x = 0.5f;
    private final float center_y = 0.5f;
    private final float scale = 0.5f;
    private final float shade = 0.5f;
    private final float slope = 20.0f;
    private ScriptGroup2.Closure cInit;

    public void init() {
      s.invoke_init_vignette(
          mInPixelsAllocation.getType().getX(),
          mInPixelsAllocation.getType().getY(), center_x,
          center_y, scale, shade, slope);
    }

    public ScriptGroup2.Closure prepInit(ScriptGroup2.Builder b) {
      cInit = b.addInvoke(s.getInvokeID_init_vignette(),
          new Object[] {
            new Integer(mInPixelsAllocation.getType().getX()),
            new Integer(mInPixelsAllocation.getType().getY()),
            new Float(center_x),
            new Float(center_y),
            new Float(scale), new Float(shade), new Float(slope) },
          new HashMap<Script.FieldID, Object>());
      return cInit;
    }

    public Script.KernelID getKernelID() { return s.getKernelID_vignette(); }

    public void forEach(Allocation in, Allocation out) { s.forEach_vignette(in, out); }

    private ScriptC_vignette_approx_relaxed_f s;
  }

  public final static Class[] mFilterClasses = {
    ColorMatrixFilter.class,
    ContrastFilter.class,
    ExposureFilter.class,
    FisheyeFilter.class,
    GreyFilter.class,
    LevelsFilter.class,
    ShadowsFilter.class,
    VibranceFilter.class,
    VignetteFilter.class
  };
  private FilterInterface[] mFilters;
  private int[] mIndices;

  ScriptC_uc4tof4 s_uc2f;
  ScriptC_f4touc4 s_f2uc;

  private Allocation[] mScratchPixelsAllocation = new Allocation[2];
  private ScriptGroup mGroup;
  private ScriptGroup2 mGroup2;

  private int mWidth;
  private int mHeight;
  private int mMode;

  public static final int EMULATED = 0;
  public static final int NATIVE2 = 1;
  public static final int NATIVE1 = 2;
  public static final int MANUAL = 3;

  public Filters(int mode, int[] filter) {
    mMode = mode;
    mIndices = new int[filter.length];
    System.arraycopy(filter, 0, mIndices, 0, filter.length);
    mFilters = new FilterInterface[filter.length+2];
  }

  public void createTest(android.content.res.Resources res) {
    s_uc2f = new ScriptC_uc4tof4(mRS);
    s_f2uc = new ScriptC_f4touc4(mRS);
    for (int i = 0; i < mIndices.length; i++) {
      try {
        /*
        Constructor[] constructors = mFilterClasses[mIndices[i]].getConstructors();
        for (Constructor ctr : constructors) {
          Log.i("Filters", "constructor " + ctr);
        }
        */
        Constructor constructor =
            // mFilterClasses[i].getConstructor(new Class[]{ RenderScript.class });
            //mFilterClasses[i].getConstructor(RenderScript.class);
            mFilterClasses[mIndices[i]].getConstructors()[0];
        try {
          mFilters[i] = (FilterInterface)constructor.newInstance(this, mRS);
        } catch (Exception e) {
          Log.e("Filters", "newInstance caught " + e);
          System.exit(-2);
        }
        mFilters[i].init();
      } catch (Exception e) {
        Log.e("Filters", "getConstructor caught " + e + " for " +
              mFilterClasses[mIndices[i]].getName());
        System.exit(-1);
      }

    }

    mWidth = mInPixelsAllocation.getType().getX();
    mHeight = mInPixelsAllocation.getType().getY();

    Type.Builder tb = new Type.Builder(mRS, Element.F32_4(mRS));
    tb.setX(mWidth);
    tb.setY(mHeight);
    Type connect = tb.create();

    switch (mMode) {
      case NATIVE1:
          ScriptGroup.Builder b = new ScriptGroup.Builder(mRS);
          b.addKernel(s_uc2f.getKernelID_uc4tof4());
          b.addKernel(mFilters[0].getKernelID());
          b.addConnection(connect, s_uc2f.getKernelID_uc4tof4(),
              mFilters[0].getKernelID());

          for (int i = 0; i < mIndices.length; i++) {
            b.addKernel(mFilters[i].getKernelID());
            b.addConnection(connect, mFilters[i-1].getKernelID(),
                mFilters[i].getKernelID());
          }

          b.addKernel(s_f2uc.getKernelID_f4touc4());
          b.addConnection(mOutPixelsAllocation.getType(),
              mFilters[0].getKernelID(), s_f2uc.getKernelID_f4touc4());

          mGroup = b.create();
          break;
      case NATIVE2: {
        ScriptGroup2.Builder b2 = new ScriptGroup2.Builder(mRS);

        for (int i = 0; i < mIndices.length; i++) {
          mFilters[i].prepInit(b2);
        }

        ScriptGroup2.UnboundValue in = b2.addInput();

        HashMap<Script.FieldID, Object> emptyMap =
            new HashMap<Script.FieldID, Object>();

        ScriptGroup2.Closure c = b2.addKernel(s_uc2f.getKernelID_uc4tof4(),
            connect, new Object[]{ in }, emptyMap);

        for (int i = 0; i < mIndices.length; i++) {
//          c = b2.addKernel(mFilters[i].getKernelID(), connect,
//              new Object[]{ c.getReturn() }, emptyMap);
            c = mFilters[i].asyncLaunch(b2, c.getReturn(), connect);
        }

        c = b2.addKernel(s_f2uc.getKernelID_f4touc4(),
            mOutPixelsAllocation.getType(),
            new Object[]{ c.getReturn() }, emptyMap);

        final String name = mFilters[0].getClass().getSimpleName() + "-" +
                mFilters[1].getClass().getSimpleName();
        mGroup2 = b2.create(name, c.getReturn());
      }
        break;
      case EMULATED:
        mScratchPixelsAllocation[0] = Allocation.createTyped(mRS, connect);
        mScratchPixelsAllocation[1] = Allocation.createTyped(mRS, connect);
        break;
    }
  }

    public void runTest() {
        switch (mMode) {
          case NATIVE1:
            // mGroup.setInput(mFilters[0].getKernelID(), mInPixelsAllocation);
            mGroup.setInput(s_uc2f.getKernelID_uc4tof4(), mInPixelsAllocation);
            // mGroup.setOutput(mFilters[mIndices.length - 1].getKernelID(), mOutPixelsAllocation);
            mGroup.setOutput(s_f2uc.getKernelID_f4touc4(), mOutPixelsAllocation);
            mGroup.execute();
            break;
          case NATIVE2:
            mOutPixelsAllocation = (Allocation)mGroup2.execute(mInPixelsAllocation)[0];
            break;
          case EMULATED:
            s_uc2f.forEach_uc4tof4(mInPixelsAllocation, mScratchPixelsAllocation[0]);
            for (int i = 0; i < mIndices.length; i++) {
              mFilters[i].forEach(mScratchPixelsAllocation[i % 2],
                  mScratchPixelsAllocation[(i+1) % 2]);
            }
            s_f2uc.forEach_f4touc4(mScratchPixelsAllocation[mIndices.length % 2],
                mOutPixelsAllocation);
            break;
        }
    }

}
