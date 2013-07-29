/*
 * Copyright (C) 2008-2012 The Android Open Source Project
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

#include <malloc.h>

#include "RenderScript.h"

using namespace android;
using namespace RSC;

ScriptIntrinsic::ScriptIntrinsic(sp<RS> rs, int id, sp<const Element> e)
    : Script(NULL, rs) {
    mID = RS::dispatch->ScriptIntrinsicCreate(rs->getContext(), id, e->getID());
}

ScriptIntrinsic3DLUT::ScriptIntrinsic3DLUT(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_3DLUT, e) {

}
void ScriptIntrinsic3DLUT::forEach(sp<Allocation> ain, sp<Allocation> aout) {
    Script::forEach(0, ain, aout, NULL, 0);
}
void ScriptIntrinsic3DLUT::setLUT(sp<Allocation> lut) {
    Script::setVar(0, lut);
}

ScriptIntrinsicBlend::ScriptIntrinsicBlend(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_BLEND, e) {
}

void ScriptIntrinsicBlend::blendClear(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(0, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendSrc(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(1, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendDst(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(2, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendSrcOver(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(3, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendDstOver(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(4, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendSrcIn(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(5, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendDstIn(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(6, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendSrcOut(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(7, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendDstOut(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(8, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendSrcAtop(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(9, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendDstAtop(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(10, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendXor(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(11, in, out, NULL, 0);
}

// Numbering jumps here
void ScriptIntrinsicBlend::blendMultiply(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(14, in, out, NULL, 0);
}

// Numbering jumps here
void ScriptIntrinsicBlend::blendAdd(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(34, in, out, NULL, 0);
}

void ScriptIntrinsicBlend::blendSubtract(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(35, in, out, NULL, 0);
}




ScriptIntrinsicBlur::ScriptIntrinsicBlur(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_BLUR, e) {

}

void ScriptIntrinsicBlur::blur(sp<Allocation> in, sp<Allocation> out) {
    Script::setVar(1, in);
    Script::forEach(0, NULL, out, NULL, 0);
}

void ScriptIntrinsicBlur::setRadius(float radius) {
    Script::setVar(0, &radius, sizeof(float));
}



ScriptIntrinsicColorMatrix::ScriptIntrinsicColorMatrix(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_COLOR_MATRIX, e) {

}

void ScriptIntrinsicColorMatrix::forEach(sp<Allocation> in, sp<Allocation> out) {
    Script::forEach(0, in, out, NULL, 0);
}


void ScriptIntrinsicColorMatrix::setColorMatrix3(float* m) {
    Script::setVar(0, (void*)m, sizeof(float) * 9);
}


void ScriptIntrinsicColorMatrix::setColorMatrix4(float* m) {
    Script::setVar(0, (void*)m, sizeof(float) * 16);
}


void ScriptIntrinsicColorMatrix::setGreyscale() {
    float matrix[] = {0.299f, 0.587f,  0.114f, 0.299f, 0.587f, 0.114f, 0.299f, 0.587f, 0.114f};
    setColorMatrix3(matrix);
}


void ScriptIntrinsicColorMatrix::setRGBtoYUV() {
    float matrix[] = {0.299f,0.587f,0.114f,-0.14713f,-0.28886f,0.436f,0.615f,-0.51499f,-0.10001f};
    setColorMatrix3(matrix);
}


void ScriptIntrinsicColorMatrix::setYUVtoRGB() {
    float matrix[] = {1.f,0.f,1.13983f,1.f,-0.39465f,-0.5806f,1.f,2.03211f,0.f};
    setColorMatrix3(matrix);
}

ScriptIntrinsicConvolve3x3::ScriptIntrinsicConvolve3x3(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_CONVOLVE_3x3, e) {

}

void ScriptIntrinsicConvolve3x3::setInput(sp<Allocation> in) {
    Script::setVar(1, in);
}

void ScriptIntrinsicConvolve3x3::forEach(sp<Allocation> out) {
    Script::forEach(0, NULL, out, NULL, 0);
}

void ScriptIntrinsicConvolve3x3::setCoefficients(float* v) {
    Script::setVar(0, (void*)v, sizeof(float) * 9);
}

ScriptIntrinsicConvolve5x5::ScriptIntrinsicConvolve5x5(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_CONVOLVE_5x5, e) {

}

void ScriptIntrinsicConvolve5x5::setInput(sp<Allocation> in) {
    Script::setVar(1, in);
}

void ScriptIntrinsicConvolve5x5::forEach(sp<Allocation> out) {
    Script::forEach(0, NULL, out, NULL, 0);
}

void ScriptIntrinsicConvolve5x5::setCoefficients(float* v) {
    Script::setVar(0, (void*)v, sizeof(float) * 25);
}

/*ScriptIntrinsicLUT::ScriptIntrinsicLUT(sp<RS> rs, sp<const Element> e)
    : ScriptIntrinsic(rs, RS_SCRIPT_INTRINSIC_ID_LUT, e) {

}

void ScriptIntrinsicLUT::forEach(sp<Allocation> ain, sp<Allocation> aout) {

}

void ScriptIntrinsicLUT::setLUT(sp<Allocation> lut) {

}*/

