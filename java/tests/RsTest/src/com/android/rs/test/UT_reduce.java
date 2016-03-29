/*
 * Copyright (C) 2016 The Android Open Source Project
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

/* Same as UT_reduce_backward.java, except this test case exercises
 * pragmas before the functions (forward reference), and the other
 * test case exercises the pragmas after the functions (backward
 * reference).
 */

package com.android.rs.test;

import android.content.Context;
import android.content.res.Resources;
import android.renderscript.*;
import android.util.Log;
import java.lang.Float;
import java.util.Arrays;
import java.util.Random;

public class UT_reduce extends UnitTest {
    private static final String TAG = "reduce";

    protected UT_reduce(RSTestCore rstc, Resources res, Context ctx) {
        super(rstc, "reduce", ctx);
    }

    private static class timing {
        timing(long myJavaStart, long myJavaEnd, long myRsStart,
               long myCopyStart, long myKernelStart, long myRsEnd,
               Allocation... myInputs) {
            javaStart = myJavaStart;
            javaEnd = myJavaEnd;
            rsStart = myRsStart;
            copyStart = myCopyStart;
            kernelStart = myKernelStart;
            rsEnd = myRsEnd;

            inputBytes = 0;
            for (Allocation input : myInputs)
                inputBytes += input.getBytesSize();

            inputCells = (myInputs.length > 0) ? myInputs[0].getType().getCount() : 0;
        }

        timing(long myInputCells) {
            inputCells = myInputCells;
        }

        private long javaStart = -1;
        private long javaEnd = -1;
        private long rsStart = -1;
        private long copyStart = -1;
        private long kernelStart = -1;
        private long rsEnd = -1;
        private long inputBytes = -1;
        private long inputCells = -1;

        public long javaTime() { return javaEnd - javaStart; }
        public long rsTime() { return rsEnd - rsStart; }
        public long kernelTime() { return rsEnd - kernelStart; }
        public long overheadTime() { return kernelStart - rsStart; }
        public long allocationTime() { return copyStart - rsStart; }
        public long copyTime() { return kernelStart - copyStart; }

        public static String string(long myJavaStart, long myJavaEnd, long myRsStart,
                                    long myCopyStart, long myKernelStart, long myRsEnd,
                                    Allocation... myInputs) {
            return (new timing(myJavaStart, myJavaEnd, myRsStart,
                               myCopyStart, myKernelStart, myRsEnd, myInputs)).string();
        }

        public static String string(long myInputCells) {
            return (new timing(myInputCells)).string();
        }

        public String string() {
            String result;
            if (javaStart >= 0) {
                result = "(java " + javaTime() + "ms, rs " + rsTime() + "ms = overhead " +
                         overheadTime() + "ms (alloc " + allocationTime() + "ms + copy " +
                         copyTime() + "ms) + kernel+get() " + kernelTime() + "ms)";
                if (inputCells > 0)
                    result += " ";
            } else {
                result = "";
            }
            if (inputCells > 0) {
                result += "(" + fmt.format(inputCells) + " cells";
                if (inputBytes > 0)
                    result += ", " + fmt.format(inputBytes) + " bytes";
                result += ")";
            }
            return result;
        }

        private static java.text.DecimalFormat fmt;
        static {
            fmt = new java.text.DecimalFormat("###,###");
        }
    };

    private byte[] createInputArrayByte(int len, int seed) {
        byte[] array = new byte[len];
        (new Random(seed)).nextBytes(array);
        return array;
    }

    private float[] createInputArrayFloat(int len, int seed) {
        Random rand = new Random(seed);
        float[] array = new float[len];
        for (int i = 0; i < len; ++i)
            array[i] = rand.nextFloat();
        return array;
    }

    private int[] createInputArrayInt(int len, int seed) {
        Random rand = new Random(seed);
        int[] array = new int[len];
        for (int i = 0; i < len; ++i)
            array[i] = rand.nextInt();
        return array;
    }

    private int[] createInputArrayInt(int len, int seed, int eltRange) {
        Random rand = new Random(seed);
        int[] array = new int[len];
        for (int i = 0; i < len; ++i)
            array[i] = rand.nextInt(eltRange);
        return array;
    }

    private <T extends Number> boolean result(String testName, final timing t,
                                              T javaRslt, T rsRslt) {
        final boolean success = javaRslt.equals(rsRslt);
        String status = (success ? "PASSED" : "FAILED");
        if (success && (t != null))
            status += " " + t.string();
        Log.i(TAG, testName + ": java " + javaRslt + ", rs " + rsRslt + ": " + status);
        return success;
    }

    private boolean result(String testName, final timing t,
                           final long[] javaRslt, final long[] rsRslt) {
        if (javaRslt.length != rsRslt.length) {
            Log.i(TAG, testName + ": java length " + javaRslt.length +
                       ", rs length " + rsRslt.length + ": FAILED");
            return false;
        }
        for (int i = 0; i < javaRslt.length; ++i) {
            if (javaRslt[i] != rsRslt[i]) {
                Log.i(TAG, testName + "[" + i + "]: java " + javaRslt[i] +
                           ", rs " + rsRslt[i] + ": FAILED");
                return false;
            }
        }
        String status = "PASSED";
        if (t != null)
            status += " " + t.string();
        Log.i(TAG, testName + ": " + status);
        return true;
    }

    private boolean result(String testName, final timing t, Int2 javaRslt, Int2 rsRslt) {
        final boolean success = (javaRslt.x == rsRslt.x) && (javaRslt.y == rsRslt.y);
        String status = (success ? "PASSED" : "FAILED");
        if (success && (t != null))
            status += " " + t.string();
        Log.i(TAG,
                testName +
                ": java (" + javaRslt.x + ", " + javaRslt.y + ")" +
                ", rs (" + rsRslt.x + ", " + rsRslt.y + ")" +
                ": " + status);
        return success;
    }

    private boolean result(String testName, final timing t, Float2 javaRslt, Float2 rsRslt) {
        final boolean success = (javaRslt.x == rsRslt.x) && (javaRslt.y == rsRslt.y);
        String status = (success ? "PASSED" : "FAILED");
        if (success && (t != null))
            status += " " + t.string();
        Log.i(TAG,
                testName +
                ": java (" + javaRslt.x + ", " + javaRslt.y + ")" +
                ", rs (" + rsRslt.x + ", " + rsRslt.y + ")" +
                ": " + status);
        return success;
    }

    ///////////////////////////////////////////////////////////////////

    private int addint(int[] input) {
        int rslt = 0;
        for (int idx = 0; idx < input.length; ++idx)
            rslt += input[idx];
        return rslt;
    }

    private boolean addint1D_array(RenderScript RS, ScriptC_reduce s, int seed, int[] size) {
        final int[] input = createInputArrayInt(size[0], seed, Integer.MAX_VALUE / size[0]);

        final int javaRslt = addint(input);
        final int rsRslt = s.reduce_addint(input).get();

        return result("addint1D_array", new timing(size[0]), javaRslt, rsRslt);
    }

    private boolean addint1D(RenderScript RS, ScriptC_reduce s, int seed, int[] size) {
        final int[] inputArray = createInputArrayInt(size[0], seed, Integer.MAX_VALUE / size[0]);

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final int javaRslt = addint(inputArray);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Allocation inputAllocation = Allocation.createSized(RS, Element.I32(RS), inputArray.length);

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocation.copyFrom(inputArray);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final int rsRslt = s.reduce_addint(inputAllocation).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();

        return result("addint1D",
                new timing(javaTimeStart, javaTimeEnd, rsTimeStart,
                           copyTimeStart, kernelTimeStart, rsTimeEnd, inputAllocation),
                javaRslt, rsRslt);
    }

    private boolean addint2D(RenderScript RS, ScriptC_reduce s, int seed, int[] size) {
        final int dimX = size[0];
        final int dimY = size[1];

        final int[] inputArray = createInputArrayInt(dimX * dimY, seed, Integer.MAX_VALUE / (dimX * dimY));

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final int javaRslt = addint(inputArray);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Type.Builder typeBuilder = new Type.Builder(RS, Element.I32(RS));
        typeBuilder.setX(dimX).setY(dimY);
        Allocation inputAllocation = Allocation.createTyped(RS, typeBuilder.create());

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocation.copy2DRangeFrom(0, 0, dimX, dimY, inputArray);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final int rsRslt = s.reduce_addint(inputAllocation).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();

        return result("addint2D",
                new timing(javaTimeStart, javaTimeEnd, rsTimeStart,
                           copyTimeStart, kernelTimeStart, rsTimeEnd, inputAllocation),
                javaRslt, rsRslt);
    }

    ///////////////////////////////////////////////////////////////////

    private Int2 findMinAndMax(float[] input) {
        float minVal = Float.POSITIVE_INFINITY;
        int minIdx = -1;
        float maxVal = Float.NEGATIVE_INFINITY;
        int maxIdx = -1;

        for (int idx = 0; idx < input.length; ++idx) {
            if (input[idx] < minVal) {
                minVal = input[idx];
                minIdx = idx;
            }
            if (input[idx] > maxVal) {
                maxVal = input[idx];
                maxIdx = idx;
            }
        }

        return new Int2(minIdx, maxIdx);
    }

    private boolean findMinAndMax_array(RenderScript RS, ScriptC_reduce s, int seed, int[] size) {
        final float[] input = createInputArrayFloat(size[0], seed);

        final Int2 javaRslt = findMinAndMax(input);
        final Int2 rsRslt = s.reduce_findMinAndMax(input).get();

        // Note that the Java and RenderScript algorithms are not
        // guaranteed to find the same cells -- but they should
        // find cells of the same value.
        final Float2 javaVal = new Float2(input[javaRslt.x], input[javaRslt.y]);
        final Float2 rsVal = new Float2(input[rsRslt.x], input[rsRslt.y]);

        return result("findMinAndMax_array", new timing(size[0]), javaVal, rsVal);
    }

    private boolean findMinAndMax(RenderScript RS, ScriptC_reduce s, int seed, int[] size) {
        final float[] inputArray = createInputArrayFloat(size[0], seed);

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final Int2 javaRslt = findMinAndMax(inputArray);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Allocation inputAllocation = Allocation.createSized(RS, Element.F32(RS), inputArray.length);

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocation.copyFrom(inputArray);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final Int2 rsRslt = s.reduce_findMinAndMax(inputAllocation).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();

        // Note that the Java and RenderScript algorithms are not
        // guaranteed to find the same cells -- but they should
        // find cells of the same value.
        final Float2 javaVal = new Float2(inputArray[javaRslt.x], inputArray[javaRslt.y]);
        final Float2 rsVal = new Float2(inputArray[rsRslt.x], inputArray[rsRslt.y]);

        return result("findMinAndMax",
                new timing(javaTimeStart, javaTimeEnd, rsTimeStart,
                           copyTimeStart, kernelTimeStart, rsTimeEnd, inputAllocation),
                javaVal, rsVal);
    }

    ///////////////////////////////////////////////////////////////////

    private int fz(final int[] input) {
        for (int i = 0; i < input.length; ++i)
            if (input[i] == 0)
                return i;
        return -1;
    }

    private boolean fz_array(RenderScript RS, ScriptC_reduce s, int seed, int size[]) {
        final int inputLen = size[0];
        int[] input = createInputArrayInt(inputLen, seed+0);
        // just in case we got unlucky
        input[(new Random(seed+1)).nextInt(inputLen)] = 0;

        final int rsRslt = s.reduce_fz(input).get();

        final boolean success = (input[rsRslt] == 0);
        Log.i(TAG,
                "fz_array: input[" + rsRslt + "] == " + input[rsRslt] + ": " +
                (success ? "PASSED " + timing.string(size[0]) : "FAILED"));
        return success;
    }

    private boolean fz(RenderScript RS, ScriptC_reduce s, int seed, int size[]) {
        final int inputLen = size[0];
        int[] inputArray = createInputArrayInt(inputLen, seed+0);
        // just in case we got unlucky
        inputArray[(new Random(seed+1)).nextInt(inputLen)] = 0;

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final int javaRslt = fz(inputArray);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Allocation inputAllocation = Allocation.createSized(RS, Element.I32(RS), inputArray.length);

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocation.copyFrom(inputArray);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final int rsRslt = s.reduce_fz(inputAllocation).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();

        final boolean success = (inputArray[rsRslt] == 0);
        String status = (success ? "PASSED" : "FAILED");
        if (success)
            status += " " + timing.string(javaTimeStart, javaTimeEnd, rsTimeStart,
                                          copyTimeStart, kernelTimeStart, rsTimeEnd, inputAllocation);
        Log.i(TAG,
                "fz: java input[" + javaRslt + "] == " + inputArray[javaRslt] +
                ", rs input[" + rsRslt + "] == " + inputArray[javaRslt] + ": " + status);
        return success;
    }

    ///////////////////////////////////////////////////////////////////

    private boolean fz2(RenderScript RS, ScriptC_reduce s, int seed, int size[]) {
        final int dimX = size[0], dimY = size[1];
        final int inputLen = dimX * dimY;

        int[] inputArray = createInputArrayInt(inputLen, seed+0);
        // just in case we got unlucky
        inputArray[(new Random(seed+1)).nextInt(inputLen)] = 0;

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final int javaRsltLinear = fz(inputArray);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();

        final Int2 javaRslt = new Int2(javaRsltLinear % dimX, javaRsltLinear / dimX);
        final int javaCellVal = inputArray[javaRslt.x + dimX * javaRslt.y];

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Type.Builder typeBuilder = new Type.Builder(RS, Element.I32(RS));
        typeBuilder.setX(dimX).setY(dimY);
        Allocation inputAllocation = Allocation.createTyped(RS, typeBuilder.create());

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocation.copy2DRangeFrom(0, 0, dimX, dimY, inputArray);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final Int2 rsRslt = s.reduce_fz2(inputAllocation).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();

        final int rsCellVal = inputArray[rsRslt.x + dimX * rsRslt.y];
        final boolean success = (rsCellVal == 0);
        String status = (success ? "PASSED" : "FAILED");
        if (success)
            status += " " + timing.string(javaTimeStart, javaTimeEnd, rsTimeStart,
                                          copyTimeStart, kernelTimeStart, rsTimeEnd, inputAllocation);
        Log.i(TAG,
                "fz2: java input[" + javaRslt.x + ", " + javaRslt.y + "] == " + javaCellVal +
                ", rs input[" + rsRslt.x + ", " + rsRslt.y + "] == " + rsCellVal + ": " + status);
        return success;
    }

    ///////////////////////////////////////////////////////////////////

    private boolean fz3(RenderScript RS, ScriptC_reduce s, int seed, int[] size) {
        final int dimX = size[0], dimY = size[1], dimZ = size[2];
        final int inputLen = dimX * dimY * dimZ;

        int[] inputArray = createInputArrayInt(inputLen, seed+0);
        // just in case we got unlucky
        inputArray[(new Random(seed+1)).nextInt(inputLen)] = 0;

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final int javaRsltLinear = fz(inputArray);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();

        final Int3 javaRslt = new Int3(
            javaRsltLinear % dimX,
            (javaRsltLinear / dimX) % dimY,
            javaRsltLinear / (dimX * dimY));
        final int javaCellVal = inputArray[javaRslt.x + dimX * javaRslt.y + dimX * dimY * javaRslt.z];

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Type.Builder typeBuilder = new Type.Builder(RS, Element.I32(RS));
        typeBuilder.setX(dimX).setY(dimY).setZ(dimZ);
        Allocation inputAllocation = Allocation.createTyped(RS, typeBuilder.create());

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocation.copy3DRangeFrom(0, 0, 0, dimX, dimY, dimZ, inputArray);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final Int3 rsRslt = s.reduce_fz3(inputAllocation).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();

        final int rsCellVal = inputArray[rsRslt.x + dimX * rsRslt.y + dimX * dimY * rsRslt.z];
        final boolean success = (rsCellVal == 0);
        String status = (success ? "PASSED" : "FAILED");
        if (success)
            status += " " + timing.string(javaTimeStart, javaTimeEnd, rsTimeStart,
                                          copyTimeStart, kernelTimeStart, rsTimeEnd, inputAllocation);
        Log.i(TAG,
                "fz3: java input[" + javaRslt.x + ", " + javaRslt.y + ", " + javaRslt.z + "] == " + javaCellVal +
                ", rs input[" + rsRslt.x + ", " + rsRslt.y + ", " + rsRslt.z + "] == " + rsCellVal + ": " + status);
        return success;
    }

    ///////////////////////////////////////////////////////////////////

    private static final int histogramBucketCount = 256;

    private long[] histogram(RenderScript RS, final byte[] inputArray) {
        Allocation inputAllocation = Allocation.createSized(RS, Element.U8(RS), inputArray.length);
        inputAllocation.copyFrom(inputArray);

        Allocation outputAllocation = Allocation.createSized(RS, Element.U32(RS), histogramBucketCount);

        ScriptIntrinsicHistogram scriptHsg = ScriptIntrinsicHistogram.create(RS, Element.U8(RS));
        scriptHsg.setOutput(outputAllocation);
        scriptHsg.forEach(inputAllocation);

        int[] outputArrayMistyped = new int[histogramBucketCount];
        outputAllocation.copyTo(outputArrayMistyped);

        long[] outputArray = new long[histogramBucketCount];
        for (int i = 0; i < histogramBucketCount; ++i)
            outputArray[i] = outputArrayMistyped[i] & (long)0xffffffff;
        return outputArray;
    }

    private boolean histogram_array(RenderScript RS, ScriptC_reduce s, int seed, int size[]) {
        final byte[] inputArray = createInputArrayByte(size[0], seed);

        final long[] javaRslt = histogram(RS, inputArray);
        _RS_ASSERT("javaRslt unexpected length: " + javaRslt.length, javaRslt.length == histogramBucketCount);
        final long[] rsRslt = s.reduce_histogram(inputArray).get();
        _RS_ASSERT("rsRslt unexpected length: " + rsRslt.length, rsRslt.length == histogramBucketCount);

        return result("histogram_array", new timing(size[0]), javaRslt, rsRslt);
    }

    private boolean histogram(RenderScript RS, ScriptC_reduce s, int seed, int size[]) {
        final byte[] inputArray = createInputArrayByte(size[0], seed);

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final long[] javaRslt = histogram(RS, inputArray);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();
        _RS_ASSERT("javaRslt unexpected length: " + javaRslt.length, javaRslt.length == histogramBucketCount);

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Allocation inputAllocation = Allocation.createSized(RS, Element.U8(RS), inputArray.length);

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocation.copyFrom(inputArray);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final long[] rsRslt = s.reduce_histogram(inputAllocation).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();
        _RS_ASSERT("rsRslt unexpected length: " + rsRslt.length, rsRslt.length == histogramBucketCount);

        // NOTE: The "java time" is actually for the RenderScript histogram intrinsic
        return result("histogram",
                new timing(javaTimeStart, javaTimeEnd, rsTimeStart,
                           copyTimeStart, kernelTimeStart, rsTimeEnd, inputAllocation),
                javaRslt, rsRslt);
    }

    //-----------------------------------------------------------------

    private Int2 mode(RenderScript RS, final byte[] inputArray) {
        long[] hsg = histogram(RS, inputArray);

        int modeIdx = 0;
        for (int i = 1; i < hsg.length; ++i)
            if (hsg[i] > hsg[modeIdx]) modeIdx =i;
        return new Int2(modeIdx, (int)hsg[modeIdx]);
    }

    private boolean mode_array(RenderScript RS, ScriptC_reduce s, int seed, int size[]) {
        final byte[] inputArray = createInputArrayByte(size[0], seed);

        final Int2 javaRslt = mode(RS, inputArray);
        final Int2 rsRslt = s.reduce_mode(inputArray).get();

        return result("mode", new timing(size[0]), javaRslt, rsRslt);
    }

    ///////////////////////////////////////////////////////////////////

    private long sumgcd(final int in1[], final int in2[]) {
        _RS_ASSERT("sumgcd input length mismatch", in1.length == in2.length);

        long sum = 0;
        for (int i = 0; i < in1.length; ++i) {
            int a = in1[i], b = in2[i];

            while (b != 0) {
                final int aNew = b;
                final int bNew = a % b;

                a = aNew;
                b = bNew;
            }

            sum += a;
        }
        return sum;
    }

    private boolean sumgcd(RenderScript RS, ScriptC_reduce s, int seed, int size[]) {
        final int len = size[0];

        final int[] inputArrayA = createInputArrayInt(len, seed+0);
        final int[] inputArrayB = createInputArrayInt(len, seed+1);

        final long javaTimeStart = java.lang.System.currentTimeMillis();
        final long javaRslt = sumgcd(inputArrayA, inputArrayB);
        final long javaTimeEnd = java.lang.System.currentTimeMillis();

        final long rsTimeStart = java.lang.System.currentTimeMillis();

        Allocation inputAllocationA = Allocation.createSized(RS, Element.I32(RS), len);
        Allocation inputAllocationB = Allocation.createSized(RS, Element.I32(RS), len);

        final long copyTimeStart = java.lang.System.currentTimeMillis();

        inputAllocationA.copyFrom(inputArrayA);
        inputAllocationB.copyFrom(inputArrayB);

        final long kernelTimeStart = java.lang.System.currentTimeMillis();
        final long rsRslt = s.reduce_sumgcd(inputAllocationA, inputAllocationB).get();
        final long rsTimeEnd = java.lang.System.currentTimeMillis();

        return result("sumgcd",
                new timing(javaTimeStart, javaTimeEnd, rsTimeStart, copyTimeStart, kernelTimeStart, rsTimeEnd,
                        inputAllocationA, inputAllocationB),
                javaRslt, rsRslt);
    }

    ///////////////////////////////////////////////////////////////////

    public static final int maxSeedsPerTest = 10;

    static interface Test {
        // A test execution is characterized by two properties: A seed
        // and a size.
        //
        // The seed is used for generating pseudorandom input data.
        // Ideally, we use different seeds for different tests and for
        // different executions of the same test at different sizes.
        // A test with multiple blocks of input data (i.e., for a
        // reduction with multiple inputs) may want multiple seeds; it
        // may use the seeds seed..seed+maxSeedsPerTest-1.
        //
        // The size indicates the amount of input data.  It is the number
        // of cells in a particular dimension of the iteration space.
        boolean run(RenderScript RS, ScriptC_reduce s, int seed, int[] size);
    };

    static class TestDescription {
        public TestDescription(String myTestName, Test myTest, int mySeed, int[] myDefSize, int[] myLog2MaxSize) {
            testName    = myTestName;
            test        = myTest;
            seed        = mySeed;
            defSize     = myDefSize;
            log2MaxSize = myLog2MaxSize;
        };

        public TestDescription(String myTestName, Test myTest, int mySeed, int[] myDefSize) {
            testName    = myTestName;
            test        = myTest;
            seed        = mySeed;
            defSize     = myDefSize;
            log2MaxSize = null;
        };

        public final String testName;

        public final Test test;

        // When executing the test, scale this up by maxSeedsPerTest.
        public final int seed;

        // If we're only going to run the test once, what size should
        // we use?
        public final int[] defSize;

        // If we're going to run the test over a range of sizes, what
        // is the maximum size to use?
        public final int[] log2MaxSize;
    };

    private boolean run(TestDescription td, RenderScript RS, ScriptC_reduce s, int seed, int[] size) {
        String arrayContent = "";
        for (int i = 0; i < size.length; ++i) {
            if (i != 0)
                arrayContent += ", ";
            arrayContent += size[i];
        }
        Log.i(TAG, "Running " + td.testName + "(seed = " + seed + ", size[] = {" + arrayContent + "})");
        return td.test.run(RS, s, seed, size);
    }

    private final TestDescription[] correctnessTests = {
        // alloc and array variants of the same test will use the same
        // seed, in case results need to be compared.

        new TestDescription("addint1D", this::addint1D, 0, new int[]{100000}, new int[]{20}),
        new TestDescription("addint1D_array", this::addint1D_array, 0, new int[]{100000}, new int[]{20}),
        new TestDescription("addint2D", this::addint2D, 1, new int[]{450, 225}),
        new TestDescription("findMinAndMax", this::findMinAndMax, 3, new int[]{100000}, new int[]{20}),
        new TestDescription("findMinAndMaxArray", this::findMinAndMax_array, 3, new int[]{100000}, new int[]{20}),
        new TestDescription("fz", this::fz, 4, new int[]{100000}, new int[]{20}),
        new TestDescription("fz_array", this::fz_array, 4, new int[]{100000}, new int[]{20}),
        new TestDescription("fz2", this::fz2, 5, new int[]{225, 450}),
        new TestDescription("fz3", this::fz3, 6, new int[]{59, 48, 37}),
        new TestDescription("histogram", this::histogram, 7, new int[]{100000}, new int[]{20}),
        new TestDescription("histogram_array", this::histogram_array, 7, new int[]{100000}, new int[]{20}),
        // might want to add: new TestDescription("mode", this::mode, 8, new int[]{100000}, new int[]{20}),
        new TestDescription("mode_array", this::mode_array, 8, new int[]{100000}, new int[]{20}),
        new TestDescription("sumgcd", this::sumgcd, 9, new int[]{1 << 16}, new int[]{20})
    };

    private boolean runCorrectnessQuick(RenderScript RS, ScriptC_reduce s) {
        boolean pass = true;

        for (TestDescription td : correctnessTests) {
            pass &= run(td, RS, s, maxSeedsPerTest * td.seed, td.defSize);
        }

        return pass;
    }

    private boolean runCorrectness(RenderScript RS, ScriptC_reduce s) {
        boolean pass = true;

        for (TestDescription td : correctnessTests) {
            if (td.log2MaxSize == null)  // TODO: Eventually this should never happen?
                continue;

            if (td.log2MaxSize.length == 1) {
                final int log2MaxSize = td.log2MaxSize[0];
                // We will execute the test with the following sizes:
                // (a) Each power of 2 from zero (2**0) up to log2MaxSize (2**log2MaxSize)
                // (b) Each size from (a) +/-1
                // (c) 2 random sizes between adjacent points in (a)
                int[] testSizes = new int[
                    /* a */ (1 + log2MaxSize) +
                    /* b */ 2*(1 + log2MaxSize) +
                    /* c */ 2*log2MaxSize];

                // NOTE: Each test execution gets maxSeedsPerTest, and
                // there are up to 3 + 5*log2MaxSize test executions
                // of a test, and we need a seed for (c).  Assuming
                // log2MaxSize does not exceed 32, then it should be
                // sufficient to reserve 1 + 5*32*maxSeedsPerTest seeds
                // per TestDescription.
                final int seedForPickingTestSizes = td.seed * (1 + 5*32*maxSeedsPerTest);

                int nextTestIdx = 0;

                // Fill in (a) and (b)
                for (int i = 0; i <= log2MaxSize; ++i) {
                    final int pwrOf2 = 1 << i;
                    testSizes[nextTestIdx++] = pwrOf2;      /* a */
                    testSizes[nextTestIdx++] = pwrOf2 - 1;  /* b */
                    testSizes[nextTestIdx++] = pwrOf2 + 1;  /* b */
                }

                // Fill in (c)
                Random r = new Random(seedForPickingTestSizes);
                for (int i = 0; i < log2MaxSize; ++i) {
                    final int lo = (1 << i) + 1;
                    final int hi = 1 << (i + 1);

                    if (lo < hi) {
                        for (int j = 0; j < 2; ++j) {
                            testSizes[nextTestIdx++] = r.nextInt(hi - lo) + lo;
                        }
                    }
                }

                Arrays.sort(testSizes);

                int[] lastTestSizeArg = new int[]{-1};
                for (int i = 0; i < testSizes.length; ++i) {
                    if ((testSizes[i] > 0) && (testSizes[i] != lastTestSizeArg[0])) {
                        lastTestSizeArg[0] = testSizes[i];
                        final int seedForTestExecution = seedForPickingTestSizes + 1 + i*maxSeedsPerTest;
                        pass &= run(td, RS, s, seedForTestExecution, lastTestSizeArg);
                    }
                }
            }
            // TODO: lengths 2 and 3, and assert otherwise
        }

        return pass;
    }

    private final TestDescription[] performanceTests = {
        new TestDescription("addint1D", this::addint1D, 0, new int[]{100000 << 10}),
        new TestDescription("addint2D", this::addint2D, 1, new int[]{450 << 5, 225 << 5}),
        new TestDescription("findMinAndMax", this::findMinAndMax, 3, new int[]{100000 << 9}),
        new TestDescription("fz", this::fz, 4, new int[]{100000 << 10}),
        new TestDescription("fz2", this::fz2, 5, new int[]{225 << 5, 450 << 5}),
        new TestDescription("fz3", this::fz3, 6, new int[]{59 << 3, 48 << 3, 37 << 3}),
        new TestDescription("histogram", this::histogram, 7, new int[]{100000 << 10}),
        // might want to add: new TestDescription("mode", this::mode, 8, new int[]{100000}),
        new TestDescription("sumgcd", this::sumgcd, 9, new int[]{1 << 21})
    };

    private boolean runPerformanceQuick(RenderScript RS, ScriptC_reduce s) {
        boolean pass = true;

        for (TestDescription td : performanceTests) {
            pass &= run(td, RS, s, maxSeedsPerTest * td.seed, td.defSize);
        }

        return pass;
    }


    public void run() {
        RenderScript pRS = RenderScript.create(mCtx);
        ScriptC_reduce s = new ScriptC_reduce(pRS);
        s.set_negInf(Float.NEGATIVE_INFINITY);
        s.set_posInf(Float.POSITIVE_INFINITY);

        boolean pass = true;

        pass &= runCorrectnessQuick(pRS, s);
        pass &= runCorrectness(pRS, s);
        // pass &= runPerformanceQuick(pRS, s);

        pRS.finish();
        pRS.destroy();

        Log.i(TAG, pass ? "PASSED" : "FAILED");
        if (pass)
            passTest();
        else
            failTest();
    }
}
