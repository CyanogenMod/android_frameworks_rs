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
import android.view.View;
import android.util.Log;

public class IPTestListJB {
    private final String TAG = "Img";
    public final String RESULT_FILE = "image_processing_result.csv";

    /**
     * Define enum type for test names
     */
    public enum TestName {
        // totally there are 38 test cases
        LEVELS_VEC3_RELAXED ("Levels Vec3 Relaxed", 1),
        LEVELS_VEC4_RELAXED ("Levels Vec4 Relaxed", 1),
        LEVELS_VEC3_FULL ("Levels Vec3 Full", 0),
        LEVELS_VEC4_FULL ("Levels Vec4 Full", 0),
        BLUR_RADIUS_25 ("Blur radius 25", 1),
        INTRINSIC_BLUE_RADIUS_25 ("Intrinsic Blur radius 25", 2),
        GREYSCALE ("Greyscale", 1),
        GRAIN ("Grain", 1),
        FISHEYE_FULL ("Fisheye Full", 0),
        FISHEYE_RELAXED ("Fisheye Relaxed", 1),
        FISHEYE_APPROXIMATE_FULL ("Fisheye Approximate Full", 0),
        FISHEYE_APPROXIMATE_RELAXED ("Fisheye Approximate Relaxed", 1),
        VIGNETTE_FULL ("Vignette Full", 0),
        VIGNETTE_RELAXED ("Vignette Relaxed", 1),
        VIGNETTE_APPROXIMATE_FULL ("Vignette Approximate Full", 0),
        VIGNETTE_APPROXIMATE_RELAXED ("Vignette Approximate Relaxed", 1),
        GROUP_TEST_EMULATED ("Group Test (emulated)", 2),
        GROUP_TEST_NATIVE ("Group Test (native)", 2),
        CONVOLVE_3X3 ("Convolve 3x3", 1),
        INTRINSICS_CONVOLVE_3X3 ("Intrinsics Convolve 3x3", 2),
        COLOR_MATRIX ("ColorMatrix", 1),
        INTRINSICS_COLOR_MATRIX ("Intrinsics ColorMatrix", 2),
        INTRINSICS_COLOR_MATRIX_GREY ("Intrinsics ColorMatrix Grey", 2),
        COPY ("Copy", 1),
        CROSS_PROCESS_USING_LUT ("CrossProcess (using LUT)", 2),
        CONVOLVE_5X5 ("Convolve 5x5", 1),
        INTRINSICS_CONVOLVE_5X5 ("Intrinsics Convolve 5x5", 2),
        MANDELBROT ("Mandelbrot", 0),
        INTRINSICS_BLEND ("Intrinsics Blend", 2),
        VIBRANCE ("Vibrance", 1),
        BW_FILTER ("BW Filter", 1),
        SHADOWS ("Shadows", 1),
        CONTRAST ("Contrast", 1),
        EXPOSURE ("Exposure", 1),
        WHITE_BALANCE ("White Balance", 1);


        private final String name;
        public final int group;

        private TestName(String s, int g) {
            name = s;
            group = g;
        }

        // return quoted string as displayed test name
        public String toString() {
            return name;
        }
    }

    static TestBase newTest(TestName testName) {
        switch(testName) {
        case LEVELS_VEC3_RELAXED:
            return new LevelsV4(false, false);
        case LEVELS_VEC4_RELAXED:
            return new LevelsV4(false, true);
        case LEVELS_VEC3_FULL:
            return new LevelsV4(true, false);
        case LEVELS_VEC4_FULL:
            return new LevelsV4(true, true);
        case BLUR_RADIUS_25:
            return new Blur25(false);
        case INTRINSIC_BLUE_RADIUS_25:
            return new Blur25(true);
        case GREYSCALE:
            return new Greyscale();
        case GRAIN:
            return new Grain();
        case FISHEYE_FULL:
            return new Fisheye(false, false);
        case FISHEYE_RELAXED:
            return new Fisheye(false, true);
        case FISHEYE_APPROXIMATE_FULL:
            return new Fisheye(true, false);
        case FISHEYE_APPROXIMATE_RELAXED:
            return new Fisheye(true, true);
        case VIGNETTE_FULL:
            return new Vignette(false, false);
        case VIGNETTE_RELAXED:
            return new Vignette(false, true);
        case VIGNETTE_APPROXIMATE_FULL:
            return new Vignette(true, false);
        case VIGNETTE_APPROXIMATE_RELAXED:
            return new Vignette(true, true);
        case GROUP_TEST_EMULATED:
            return new GroupTest(false);
        case GROUP_TEST_NATIVE:
            return new GroupTest(true);
        case CONVOLVE_3X3:
            return new Convolve3x3(false);
        case INTRINSICS_CONVOLVE_3X3:
            return new Convolve3x3(true);
        case COLOR_MATRIX:
            return new ColorMatrix(false, false);
        case INTRINSICS_COLOR_MATRIX:
            return new ColorMatrix(true, false);
        case INTRINSICS_COLOR_MATRIX_GREY:
            return new ColorMatrix(true, true);
        case COPY:
            return new Copy();
        case CROSS_PROCESS_USING_LUT:
            return new CrossProcess();
        case CONVOLVE_5X5:
            return new Convolve5x5(false);
        case INTRINSICS_CONVOLVE_5X5:
            return new Convolve5x5(true);
        case MANDELBROT:
            return new Mandelbrot();
        case INTRINSICS_BLEND:
            return new Blend();
        case VIBRANCE:
            return new Vibrance();
        case BW_FILTER:
            return new BWFilter();
        case SHADOWS:
            return new Shadows();
        case CONTRAST:
            return new Contrast();
        case EXPOSURE:
            return new Exposure();
        case WHITE_BALANCE:
            return new WhiteBalance();
        }
        return null;
    }

}

