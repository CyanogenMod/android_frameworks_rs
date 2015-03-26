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
summary: Sampler routines
description:
end:

function: rsSamplerGetAnisotropy
version: 16
ret: float, "anisotropy"
arg: rs_sampler s, "sampler to query"
summary:
description:
  Get sampler anisotropy
test: none
end:

function: rsSamplerGetMagnification
version: 16
ret: rs_sampler_value, "magnification value"
arg: rs_sampler s, "sampler to query"
summary:
description:
 Get sampler magnification value
test: none
end:

function: rsSamplerGetMinification
version: 16
ret: rs_sampler_value, "minification value"
arg: rs_sampler s, "sampler to query"
summary:
description:
 Get sampler minification value
test: none
end:

function: rsSamplerGetWrapS
version: 16
ret: rs_sampler_value, "wrap S value"
arg: rs_sampler s, "sampler to query"
summary:
description:
 Get sampler wrap S value
test: none
end:

function: rsSamplerGetWrapT
version: 16
ret: rs_sampler_value, "wrap T value"
arg: rs_sampler s, "sampler to query"
summary:
description:
 Get sampler wrap T value
test: none
end:
