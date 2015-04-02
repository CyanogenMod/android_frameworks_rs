#
# Copyright (C) 2015 The Android Open Source Project
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

header:
summary: TODO Add documentation
description:
 TODO Add documentation
end:

type: rs_for_each_strategy_t
enum: rs_for_each_strategy
value: RS_FOR_EACH_STRATEGY_SERIAL = 0
value: RS_FOR_EACH_STRATEGY_DONT_CARE = 1
value: RS_FOR_EACH_STRATEGY_DST_LINEAR = 2
value: RS_FOR_EACH_STRATEGY_TILE_SMALL = 3
value: RS_FOR_EACH_STRATEGY_TILE_MEDIUM = 4
value: RS_FOR_EACH_STRATEGY_TILE_LARGE = 5
summary: Launch order hint for rsForEach calls
description:
 Launch order hint for rsForEach calls.  This provides a hint to the system to
 determine in which order the root function of the target is called with each
 cell of the allocation.

 This is a hint and implementations may not obey the order.
end:

type: rs_kernel_context
version: 23
simple: const struct rs_kernel_context_t *
summary: Opaque handle to RenderScript kernel invocation context
description:
 TODO
end:

type: rs_script_call_t
struct: rs_script_call
field: rs_for_each_strategy_t strategy
field: uint32_t xStart
field: uint32_t xEnd
field: uint32_t yStart
field: uint32_t yEnd
field: uint32_t zStart
field: uint32_t zEnd
field: uint32_t arrayStart
field: uint32_t arrayEnd
summary: Provides extra information to a rsForEach call
description:
 Structure to provide extra information to a rsForEach call.  Primarly used to
 restrict the call to a subset of cells in the allocation.
end:

function: rsForEach
version: 9 13
ret: void
arg: rs_script script, "The target script to call"
arg: rs_allocation input, "The allocation to source data from"
arg: rs_allocation output, "the allocation to write date into"
arg: const void* usrData, "The user defined params to pass to the root script.  May be NULL."
arg: const rs_script_call_t* sc, "Extra control infomation used to select a sub-region of the allocation to be processed or suggest a walking strategy.  May be NULL."
summary:
description:
 Make a script to script call to launch work. One of the input or output is
 required to be a valid object. The input and output must be of the same
 dimensions.
test: none
end:

function: rsForEach
version: 9 13
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
test: none
end:

function: rsForEach
version: 14 20
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
arg: size_t usrDataLen, "The size of the userData structure.  This will be used to perform a shallow copy of the data if necessary."
arg: const rs_script_call_t* sc
test: none
end:

function: rsForEach
version: 14 20
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
arg: size_t usrDataLen
test: none
end:

function: rsForEach
version: 14
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
test: none
end:

function: rsGetArray0
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Index in the Array0 dimension for the specified context
description:
 Returns the index in the Array0 dimension of the cell being processed,
 as specified by the supplied context.

 This context is created when a kernel is launched and updated at each
 iteration.  It contains common characteristics of the allocations being
 iterated over and rarely used indexes, like the Array0 index.

 You can access the context by adding a rs_kernel_context argument to your
 kernel function.  E.g.<br/>
 <code>short RS_KERNEL myKernel(short value, uint32_t x, rs_kernel_context context) {<br/>
 &nbsp;&nbsp;// The current index in the common x, y, z, w dimensions are accessed by<br/>
 &nbsp;&nbsp;// adding these variables as arguments.  For the more rarely used indexes<br/>
 &nbsp;&nbsp;// to the other dimensions, extract them from the context:<br/>
 &nbsp;&nbsp;uint32_t index_a0 = rsGetArray0(context);<br/>
 &nbsp;&nbsp;//...<br/>
 }<br/></code>

 This function returns 0 if the Array0 dimension is not present.
test: none
end:

function: rsGetArray1
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Index in the Array1 dimension for the specified context
description:
 Returns the index in the Array1 dimension of the cell being processed,
 as specified by the supplied context.  See @rsGetArray0() for an explanation
 of the context.

 Returns 0 if the Array1 dimension is not present.
test: none
end:

function: rsGetArray2
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Index in the Array2 dimension for the specified context
description:
 Returns the index in the Array2 dimension of the cell being processed,
 as specified by the supplied context.  See @rsGetArray0() for an explanation
 of the context.

 Returns 0 if the Array2 dimension is not present.
test: none
end:

function: rsGetArray3
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Index in the Array3 dimension for the specified context
description:
 Returns the index in the Array3 dimension of the cell being processed,
 as specified by the supplied context.  See @rsGetArray0() for an explanation
 of the context.

 Returns 0 if the Array3 dimension is not present.
test: none
end:

function: rsGetDimArray0
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Size of the Array0 dimension for the specified context
description:
 Returns the size of the Array0 dimension for the specified context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array0 dimension is not present.
#TODO Add an hyperlink to something that explains Array0/1/2/3
# for the relevant functions.
test: none
end:

function: rsGetDimArray1
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Size of the Array1 dimension for the specified context
description:
 Returns the size of the Array1 dimension for the specified context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array1 dimension is not present.
test: none
end:

function: rsGetDimArray2
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Size of the Array2 dimension for the specified context
description:
 Returns the size of the Array2 dimension for the specified context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array2 dimension is not present.
test: none
end:

function: rsGetDimArray3
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Size of the Array3 dimension for the specified context
description:
 Returns the size of the Array3 dimension for the specified context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array3 dimension is not present.
test: none
end:

function: rsGetDimHasFaces
version: 23
ret: bool, "Returns true if more than one face is present, false otherwise."
arg: rs_kernel_context ctxt
summary: Presence of more than one face for the specified context
description:
 If the context refers to a cubemap, this function returns true if there's
 more than one face present.  In all other cases, it returns false.
 See @rsGetDimX() for an explanation of the context.

 @rsAllocationGetDimFaces() is similar but returns 0 or 1 instead of a bool.
test: none
end:

function: rsGetDimLod
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Number of levels of detail for the specified context
description:
 Returns the number of levels of detail for the specified context.
 This is useful for mipmaps.  See @rsGetDimX() for an explanation of the context.
 Returns 0 if Level of Detail is not used.

 @rsAllocationGetDimLOD() is similar but returns 0 or 1 instead the actual
 number of levels.
test: none
end:

function: rsGetDimX
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Size of the X dimension for the specified context
description:
 Returns the size of the X dimension for the specified context.

 This context is created when a kernel is launched.  It contains common
 characteristics of the allocations being iterated over by the kernel in
 a very efficient structure.  It also contains rarely used indexes.

 You can access it by adding a rs_kernel_context argument to your kernel
 function.  E.g.<br/>
 <code>int4 RS_KERNEL myKernel(int4 value, rs_kernel_context context) {<br/>
 &nbsp;&nbsp;uint32_t size = rsGetDimX(context); //...<br/></code>
test: none
end:

function: rsGetDimY
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Size of the Y dimension for the specified context
description:
 Returns the size of the X dimension for the specified context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Y dimension is not present.
test: none
end:

function: rsGetDimZ
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Size of the Z dimension for the specified context
description:
 Returns the size of the Z dimension for the specified context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Z dimension is not present.
test: none
end:

function: rsGetFace
version: 23
ret: rs_allocation_cubemap_face
arg: rs_kernel_context ctxt
summary: Coordinate of the Face for the specified context
description:
 Returns the face on which the cell being processed is found, as specified
 by the supplied context.  See @rsGetArray0() for an explanation of the context.

 Returns RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X if the face dimension is not
 present.
test: none
end:

function: rsGetLod
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary: Index in the Levels of Detail dimension for the specified context.
description:
 Returns the index in the Levels of Detail dimension of the cell being
 processed, as specified by the supplied context.  See @rsGetArray0() for
 an explanation of the context.

 Returns 0 if the Levels of Detail dimension is not present.
test: none
end:
