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
summary: Input/output functions
description:
 TODO Add documentation
end:

function: rsAllocationIoReceive
version: 16
ret: void
arg: rs_allocation a, "allocation to work on"
summary: Receive new content from the queue
description:
 Receive a new set of contents from the queue.
test: none
end:

function: rsAllocationIoSend
version: 16
ret: void
arg: rs_allocation a, "allocation to work on"
summary: Send new content to the queue
description:
 Send the contents of the Allocation to the queue.
test: none
end:

function: rsSendToClient
ret: bool
arg: int cmdID
summary:
description:
 Send a message back to the client.  Will not block and returns true
 if the message was sendable and false if the fifo was full.
 A message ID is required.  Data payload is optional.
test: none
end:

function: rsSendToClient
ret: bool
arg: int cmdID
arg: const void* data
arg: uint len
test: none
end:

function: rsSendToClientBlocking
ret: void
arg: int cmdID
summary:
description:
 Send a message back to the client, blocking until the message is queued.
 A message ID is required.  Data payload is optional.
test: none
end:

function: rsSendToClientBlocking
ret: void
arg: int cmdID
arg: const void* data
arg: uint len
test: none
end:
