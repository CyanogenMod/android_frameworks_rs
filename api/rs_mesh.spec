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
summary: Mesh routines
description:
end:

function: rsgMeshGetIndexAllocation
version: 16
ret: rs_allocation, "allocation containing index data"
arg: rs_mesh m, "mesh to get data from"
arg: uint32_t index, "index of the index allocation"
summary:
description:
 Returns an allocation containing index data or a null
 allocation if only the primitive is specified
test: none
end:

function: rsgMeshGetPrimitive
version: 16
ret: rs_primitive, "primitive describing how the mesh is rendered"
arg: rs_mesh m, "mesh to get data from"
arg: uint32_t index, "index of the primitive"
summary:
description:
 Returns the primitive describing how a part of the mesh is
 rendered
test: none
end:

function: rsgMeshGetPrimitiveCount
version: 16
ret: uint32_t, "number of primitive groups in the mesh. This would include simple primitives as well as allocations containing index data"
arg: rs_mesh m, "mesh to get data from"
summary:
description:
 Meshes could have multiple index sets, this function returns
 the number.
test: none
end:

function: rsgMeshGetVertexAllocation
version: 16
ret: rs_allocation, "allocation containing vertex data"
arg: rs_mesh m, "mesh to get data from"
arg: uint32_t index, "index of the vertex allocation"
summary:
description:
 Returns an allocation that is part of the mesh and contains
 vertex data, e.g. positions, normals, texcoords
test: none
end:

function: rsgMeshGetVertexAllocationCount
version: 16
ret: uint32_t, "number of allocations in the mesh that contain vertex data"
arg: rs_mesh m, "mesh to get data from"
summary:
description:
 Returns the number of allocations in the mesh that contain
 vertex data
test: none
end:
