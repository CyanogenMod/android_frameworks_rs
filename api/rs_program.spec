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
summary: Program object routines
description:
end:

function: rsgProgramRasterGetCullMode
version: 16
ret: rs_cull_mode
arg: rs_program_raster pr, "program raster to query"
summary:
description:
 Get program raster cull mode
test: none
end:

function: rsgProgramRasterIsPointSpriteEnabled
version: 16
ret: bool
arg: rs_program_raster pr, "program raster to query"
summary:
description:
 Get program raster point sprite state
test: none
end:

function: rsgProgramStoreGetBlendDstFunc
version: 16
ret: rs_blend_dst_func
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store blend destination function
test: none
end:

function: rsgProgramStoreGetBlendSrcFunc
version: 16
ret: rs_blend_src_func
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store blend source function
test: none
end:

function: rsgProgramStoreGetDepthFunc
version: 16
ret: rs_depth_func
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store depth function
test: none
end:

function: rsgProgramStoreIsColorMaskAlphaEnabled
version: 16
ret: bool
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store alpha component color mask
test: none
end:

function: rsgProgramStoreIsColorMaskBlueEnabled
version: 16
ret: bool
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store blur component color mask
test: none
end:

function: rsgProgramStoreIsColorMaskGreenEnabled
version: 16
ret: bool
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store green component color mask
test: none
end:

function: rsgProgramStoreIsColorMaskRedEnabled
version: 16
ret: bool
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store red component color mask
test: none
end:

function: rsgProgramStoreIsDepthMaskEnabled
version: 16
ret: bool
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store depth mask
test: none
end:

function: rsgProgramStoreIsDitherEnabled
version: 16
ret: bool
arg: rs_program_store ps, "program store to query"
summary:
description:
 Get program store dither state
test: none
end:
