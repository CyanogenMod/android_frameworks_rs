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
summary: RenderScript graphics API
description:
 A set of graphics functions used by RenderScript.
include:
 #ifdef __LP64__
 // TODO We need to fix some of the builds before enabling this error:
 // #error "RenderScript graphics is deprecated and not supported in 64bit mode."
 #else
 #include "rs_mesh.rsh"
 #include "rs_program.rsh"
 #endif
end:

function: rsgAllocationSyncAll
size: 32
ret: void
arg: rs_allocation alloc
summary:
description:
 Sync the contents of an allocation.

 If the source is specified, sync from memory space specified by source.

 If the source is not specified, sync from its SCRIPT memory space to its HW
 memory spaces.
test: none
end:

function: rsgAllocationSyncAll
version: 14
size: 32
ret: void
arg: rs_allocation alloc
arg: rs_allocation_usage_type source
test: none
end:

function: rsgBindColorTarget
version: 14
size: 32
ret: void
arg: rs_allocation colorTarget
arg: uint slot
summary:
description:
 Set the color target used for all subsequent rendering calls
test: none
end:

function: rsgBindConstant
size: 32
ret: void
arg: rs_program_fragment ps, "program fragment object"
arg: uint slot, "index of the constant buffer on the program"
arg: rs_allocation c, "constants to bind"
summary:
description:
 Bind a new Allocation object to a ProgramFragment or ProgramVertex.
 The Allocation must be a valid constant input for the Program.
test: none
end:

function: rsgBindConstant
size: 32
ret: void
arg: rs_program_vertex pv, "program vertex object"
arg: uint slot
arg: rs_allocation c
test: none
end:

function: rsgBindDepthTarget
version: 14
size: 32
ret: void
arg: rs_allocation depthTarget
summary:
description:
 Set the depth target used for all subsequent rendering calls
test: none
end:

function: rsgBindFont
size: 32
ret: void
arg: rs_font font, "object to bind"
summary:
description:
 Binds the font object to be used for all subsequent font rendering calls
test: none
end:

function: rsgBindProgramFragment
size: 32
ret: void
arg: rs_program_fragment pf
summary:
description:
 Bind a new ProgramFragment to the rendering context.
test: none
end:

function: rsgBindProgramRaster
size: 32
ret: void
arg: rs_program_raster pr
summary:
description:
 Bind a new ProgramRaster to the rendering context.
test: none
end:

function: rsgBindProgramStore
size: 32
ret: void
arg: rs_program_store ps
summary:
description:
 Bind a new ProgramStore to the rendering context.
test: none
end:

function: rsgBindProgramVertex
size: 32
ret: void
arg: rs_program_vertex pv
summary:
description:
 Bind a new ProgramVertex to the rendering context.
test: none
end:

function: rsgBindSampler
size: 32
ret: void
arg: rs_program_fragment fragment
arg: uint slot
arg: rs_sampler sampler
summary:
description:
 Bind a new Sampler object to a ProgramFragment.  The sampler will
 operate on the texture bound at the matching slot.
test: none
end:

function: rsgBindTexture
size: 32
ret: void
arg: rs_program_fragment v
arg: uint slot
arg: rs_allocation alloc
summary:
description:
 Bind a new Allocation object to a ProgramFragment.  The
 Allocation must be a valid texture for the Program.  The sampling
 of the texture will be controled by the Sampler bound at the
 matching slot.
test: none
end:

function: rsgClearAllRenderTargets
version: 14
size: 32
ret: void
summary:
description:
 Clear all color and depth targets and resume rendering into
 the framebuffer
test: none
end:

function: rsgClearColor
size: 32
ret: void
arg: float r
arg: float g
arg: float b
arg: float a
summary:
description:
 Clears the rendering surface to the specified color.
test: none
end:

function: rsgClearColorTarget
version: 14
size: 32
ret: void
arg: uint slot
summary:
description:
 Clear the previously set color target
test: none
end:

function: rsgClearDepth
size: 32
ret: void
arg: float value
summary:
description:
 Clears the depth suface to the specified value.
test: none
end:

function: rsgClearDepthTarget
version: 14
size: 32
ret: void
summary:
description:
 Clear the previously set depth target
test: none
end:

function: rsgDrawMesh
size: 32
ret: void
arg: rs_mesh ism, "mesh object to render"
summary:
description:
 Draw a mesh using the current context state.

 If primitiveIndex is specified, draw part of a mesh using the current context state.

 If start and len are also specified, draw specified index range of part of a mesh using the current context state.

 Otherwise the whole mesh is rendered.
test: none
end:

function: rsgDrawMesh
size: 32
ret: void
arg: rs_mesh ism
arg: uint primitiveIndex, "for meshes that contain multiple primitive groups this parameter specifies the index of the group to draw."
test: none
end:

function: rsgDrawMesh
size: 32
ret: void
arg: rs_mesh ism
arg: uint primitiveIndex
arg: uint start, "starting index in the range"
arg: uint len, "number of indices to draw"
test: none
end:

function: rsgDrawQuad
size: 32
ret: void
arg: float x1
arg: float y1
arg: float z1
arg: float x2
arg: float y2
arg: float z2
arg: float x3
arg: float y3
arg: float z3
arg: float x4
arg: float y4
arg: float z4
summary:
description:
 Low performance utility function for drawing a simple quad.  Not intended for
 drawing large quantities of geometry.
test: none
end:

function: rsgDrawQuadTexCoords
size: 32
ret: void
arg: float x1
arg: float y1
arg: float z1
arg: float u1
arg: float v1
arg: float x2
arg: float y2
arg: float z2
arg: float u2
arg: float v2
arg: float x3
arg: float y3
arg: float z3
arg: float u3
arg: float v3
arg: float x4
arg: float y4
arg: float z4
arg: float u4
arg: float v4
summary:
description:
 Low performance utility function for drawing a textured quad.  Not intended
 for drawing large quantities of geometry.
test: none
end:

function: rsgDrawRect
size: 32
ret: void
arg: float x1
arg: float y1
arg: float x2
arg: float y2
arg: float z
summary:
description:
 Low performance utility function for drawing a simple rectangle.  Not
 intended for drawing large quantities of geometry.
test: none
end:

function: rsgDrawSpriteScreenspace
size: 32
ret: void
arg: float x
arg: float y
arg: float z
arg: float w
arg: float h
summary:
description:
 Low performance function for drawing rectangles in screenspace.  This
 function uses the default passthough ProgramVertex.  Any bound ProgramVertex
 is ignored.  This function has considerable overhead and should not be used
 for drawing in shipping applications.
test: none
end:

function: rsgDrawText
size: 32
ret: void
arg: const char* text
arg: int x
arg: int y
summary:
description:
 Draws text given a string and location
test: none
end:

function: rsgDrawText
size: 32
ret: void
arg: rs_allocation alloc
arg: int x
arg: int y
test: none
end:

function: rsgFinish
version: 14
size: 32
ret: uint
summary:
description:
 Force RenderScript to finish all rendering commands
test: none
end:

function: rsgFontColor
size: 32
ret: void
arg: float r, "red component"
arg: float g, "green component"
arg: float b, "blue component"
arg: float a, "alpha component"
summary:
description:
 Sets the font color for all subsequent rendering calls
test: none
end:

function: rsgGetHeight
size: 32
ret: uint
summary:
description:
 Get the height of the current rendering surface.
test: none
end:

function: rsgGetWidth
size: 32
ret: uint
summary:
description:
 Get the width of the current rendering surface.
test: none
end:

function: rsgMeasureText
size: 32
ret: void
arg: const char* text
arg: int* left
arg: int* right
arg: int* top
arg: int* bottom
summary:
description:
 Returns the bounding box of the text relative to (0, 0)
 Any of left, right, top, bottom could be NULL
test: none
end:

function: rsgMeasureText
size: 32
ret: void
arg: rs_allocation alloc
arg: int* left
arg: int* right
arg: int* top
arg: int* bottom
test: none
end:

function: rsgMeshComputeBoundingBox
size: 32
ret: void
arg: rs_mesh mesh
arg: float* minX
arg: float* minY
arg: float* min
arg: float* maxX
arg: float* maxY
arg: float* maxZ
summary:
description:
 Computes an axis aligned bounding box of a mesh object
test: none
end:

function: rsgMeshComputeBoundingBox
size: 32
attrib: always_inline
ret: void
arg: rs_mesh mesh
arg: float3* bBoxMin
arg: float3* bBoxMax
inline:
 float x1, y1, z1, x2, y2, z2;
 rsgMeshComputeBoundingBox(mesh, &x1, &y1, &z1, &x2, &y2, &z2);
 bBoxMin->x = x1;
 bBoxMin->y = y1;
 bBoxMin->z = z1;
 bBoxMax->x = x2;
 bBoxMax->y = y2;
 bBoxMax->z = z2;
test: none
end:


function: rsgProgramFragmentConstantColor
size: 32
ret: void
arg: rs_program_fragment pf
arg: float r
arg: float g
arg: float b
arg: float a
summary:
description:
 Set the constant color for a fixed function emulation program.
test: none
end:

function: rsgProgramVertexGetProjectionMatrix
size: 32
ret: void
arg: rs_matrix4x4* proj, "matrix to store the current projection matrix into"
summary:
description:
 Get the projection matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

function: rsgProgramVertexLoadModelMatrix
size: 32
ret: void
arg: const rs_matrix4x4* model, "model matrix"
summary:
description:
 Load the model matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

function: rsgProgramVertexLoadProjectionMatrix
size: 32
ret: void
arg: const rs_matrix4x4* proj, "projection matrix"
summary:
description:
 Load the projection matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

function: rsgProgramVertexLoadTextureMatrix
size: 32
ret: void
arg: const rs_matrix4x4* tex, "texture matrix"
summary:
description:
 Load the texture matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

#endif //__LP64__
