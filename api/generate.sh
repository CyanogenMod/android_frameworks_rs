#!/bin/bash
#
# Copyright (C) 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e
g++ Generator.cpp Specification.cpp GenerateHtmlDocumentation.cpp GenerateHeaderFiles.cpp GenerateTestFiles.cpp Scanner.cpp Utilities.cpp -g -std=c++11 -Wall -o generator

mkdir -p test
mkdir -p scriptc
mkdir -p html

# Because rsIs/Clear/SetObject is documented in rs_object_info but also found in rs_graphics, the latter must appear
# after the former.
./generator rs_core.spec rs_value_types.spec rs_object_types.spec rs_allocation_data.spec rs_atomic.spec rs_convert.spec rs_debug.spec rs_for_each.spec rs_io.spec rs_math.spec rs_matrix.spec rs_object_info.spec rs_quaternion.spec rs_time.spec rs_vector_math.spec rs_graphics.spec

rm -f ../../../cts/tests/tests/renderscript/src/android/renderscript/cts/generated/*
mv test/* ../../../cts/tests/tests/renderscript/src/android/renderscript/cts/generated/
rmdir test

mv scriptc/*.rsh ../scriptc
rmdir scriptc

# TODO handle the documentation files.
rm html/*
rmdir html

rm generator
