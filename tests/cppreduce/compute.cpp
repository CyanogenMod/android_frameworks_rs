/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "RenderScript.h"

#include "ScriptC_reduce.h"

using namespace android;
using namespace RSC;

bool checkForFailure(Int3 output, Int3 expected) {
  static bool first = true;
  if (first) {
    printf("========================\n");
    first = false;
  }
  printf("output  : %4d %4d %4d\n", output.x, output.y, output.z);
  printf("expected: %4d %4d %4d\n", expected.x, expected.y, expected.z);
  printf("========================\n");
  return output.x != expected.x || output.y != expected.y || output.z != expected.z;
}

bool checkForFailure(int32_t output[3], Int3 expected) {
  return checkForFailure(Int3(output[0], output[1], output[2]), expected);
}

// This tests the variants of the reduce kernel available to the C++ API.
//
// For this to work, the appropriate API level needs to be set so that ScriptReduce is
// available to the dispatch table (frameworks/rs/cpp/rsDispatch.cpp).
int test_reduce() {
  bool failed = false;
  {
    sp<RS> rs = new RS();

    // only legitimate because this is a standalone executable
    bool r = rs->init("/system/bin");

    // Input array
    const int SIZE = 100;
    int input[SIZE * 3];
    for (int i = 0; i < SIZE * 3; ++i) {
      input[i] = (i % 3);
    }

    sp<ScriptC_reduce> sc = new ScriptC_reduce(rs);

    // Create input and output allocations.
    sp<const Element> e = Element::I32_3(rs);
    sp<Allocation> ain = Allocation::createSized(rs, e, SIZE);
    // Set auto-padding so that we don't have to pad arrays in copyTo() / copyFrom().
    ain->setAutoPadding(true);
    ain->copy1DRangeFrom(0, SIZE, input);

    sp<Allocation> aout = Allocation::createSized(rs, e, 1);
    aout->setAutoPadding(true);

    // Demo of all reduce variants.

    // Reduce by passing input and output allocations.
    sc->reduce_add(ain, aout);
    int32_t output[3];
    aout->copy1DTo(output);
    failed |= checkForFailure(output, Int3(0, 100, 200));

    // Reduce by passing input and output allocations along with a sub-range.
    RsScriptCall bounds{};
    bounds.xStart = 1;
    bounds.xEnd = 10;
    sc->reduce_add(ain, aout, &bounds);
    aout->copy1DTo(output);
    failed |= checkForFailure(output, Int3(0, 9, 18));

    // Helper variant with explicit size
    Int3 result = sc->reduce_add(input, sizeof(input) / sizeof(input[0]));
    failed |= checkForFailure(result, Int3(0, 100, 200));

    // Helper variant with inferred size
    result = sc->reduce_add(input);
    failed |= checkForFailure(result, Int3(0, 100, 200));

    // Helper variant with sub-range
    result = sc->reduce_add(input, 0, 1, sizeof(input) / sizeof(input[0]));
    failed |= checkForFailure(result, Int3(0, 1, 2));

    // Helper variant with sub-range and inferred size
    result = sc->reduce_add(input, 2, 11);
    failed |= checkForFailure(result, Int3(0, 9, 18));

    failed |= rs->getError() != RS_SUCCESS;

    // Helper variant with sub-range and out-of-bounds (should cause an error)
    result = sc->reduce_add(input, 0, 101);
    failed |= checkForFailure(result, Int3()) || (rs->getError() == RS_SUCCESS);
  }

  return failed;
}

int main() {
  bool failed = test_reduce();

  if (failed) {
    printf("TEST FAILED!\n");
  } else {
    printf("TEST PASSED!\n");
  }

  return failed;
}
