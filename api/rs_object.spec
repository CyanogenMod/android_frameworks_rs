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
summary: Object routines
description:
end:

function: rsClearObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: void
arg: #1* dst
hidden:
summary: For internal use.
description:
test: none
end:

function: rsClearObject
size: 32
t: rs_mesh, rs_program_fragment, rs_program_vertex, rs_program_raster, rs_program_store, rs_font
ret: void
arg: #1* dst
test: none
end:

function: rsIsObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: bool
arg: #1 v
hidden:
summary: For internal use.
description:
test: none
end:

function: rsIsObject
size: 32
t: rs_mesh, rs_program_fragment, rs_program_vertex, rs_program_raster, rs_program_store, rs_font
ret: bool
arg: #1 v
test: none
end:

function: rsSetObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: void
arg: #1* dst
arg: #1 src
hidden:
summary: For internal use.
description:
test: none
end:

function: rsSetObject
size: 32
t: rs_mesh, rs_program_fragment, rs_program_vertex, rs_program_raster, rs_program_store, rs_font
ret: void
arg: #1* dst
arg: #1 src
test: none
end:
