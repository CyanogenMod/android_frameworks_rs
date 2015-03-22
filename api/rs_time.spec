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
summary: RenderScript time routines
description:
 This file contains RenderScript functions relating to time and date manipulation.
end:

type: rs_time_t
size: 32
simple: int
summary: Seconds since January 1, 1970
description:
 Calendar time interpreted as seconds elapsed since the Epoch (00:00:00 on
 January 1, 1970, Coordinated Universal Time (UTC)).
end:

type: rs_time_t
size: 64
simple: long
end:

type: rs_tm
struct:
field: int tm_sec, "Seconds after the minute. This ranges from 0 to 59, but possibly up to 60 for leap seconds."
field: int tm_min, "Minutes after the hour. This ranges from 0 to 59."
field: int tm_hour, "Hours past midnight. This ranges from 0 to 23."
field: int tm_mday, "Day of the month. This ranges from 1 to 31."
field: int tm_mon, "Months since January. This ranges from 0 to 11."
field: int tm_year, "Years since 1900."
field: int tm_wday, "Days since Sunday. This ranges from 0 to 6."
field: int tm_yday, "Days since January 1. This ranges from 0 to 365."
field: int tm_isdst, "Flag to indicate whether daylight saving time is in effect. The value is positive if it is in effect, zero if it is not, and negative if the information is not available."
summary: Date and time structure
description:
 Data structure for broken-down time components.
end:

function: rsGetDt
ret: float, "Time in seconds."
summary:
description:
 Returns the time in seconds since this function was last called in this
 script.
test: none
end:

function: rsLocaltime
ret: rs_tm*, "Pointer to broken-down time (same as input p local)."
arg: rs_tm* local, "Broken-down time."
arg: const rs_time_t* timer, "Input time as calendar time."
summary:
description:
 Converts the time specified by p timer into broken-down time and stores it
 in p local. This function also returns a pointer to p local. If p local
 is NULL, this function does nothing and returns NULL.
test: none
end:

function: rsTime
ret: rs_time_t, "Seconds since the Epoch."
arg: rs_time_t* timer, "Location to also store the returned calendar time."
summary:
description:
 Returns the number of seconds since the Epoch (00:00:00 UTC, January 1,
 1970). If p timer is non-NULL, the result is also stored in the memory
 pointed to by this variable. If an error occurs, a value of -1 is returned.
test: none
end:

function: rsUptimeMillis
ret: int64_t, "Uptime in milliseconds."
summary:
description:
 Returns the current system clock (uptime) in milliseconds.
test: none
end:

function: rsUptimeNanos
ret: int64_t, "Uptime in nanoseconds."
summary:
description:
 Returns the current system clock (uptime) in nanoseconds.
test: none
end:
