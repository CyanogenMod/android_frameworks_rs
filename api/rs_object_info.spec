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

header:
summary: Object Characteristics Functions
description:
 The functions below can be used to query the characteristics of an allocation,
 element, or sampler object.  These objects are created from Java.

 The term "element" is used a bit ambiguously in RenderScript, as both
 the type of an item of an allocation and the instantiation of that type:<ul>
 <li>@rs_element is a handle to a type specification, and</li>
 <li>In functions like @rsGetElementAt(), "element" means the instantiation
     of the type, i.e. an item of an allocation.</li></ul>

 The functions below let you query the characteristics of the type specificiation.

 To create complex elements, use the <a href='http://developer.android.com/reference/android/renderscript/Element.Builder.html'>Element.Builder</a> Java class.
 For common elements, in Java you can simply use one of the many predefined elements
 like <a href='http://developer.android.com/reference/android/renderscript/Element.html#F32_2(android.renderscript.RenderScript)'>F32_2</a>.  You can't create elements from a script.

 An element can be a simple data type as found in C/C++, a handle type,
 a structure, or a fixed size vector (of size 2, 3, or 4) of sub-elements.

 Elements can also have a kind, which is semantic information used mostly to
 interpret pixel data.
end:

function: rsAllocationGetDimFaces
ret: uint32_t, "Returns 1 if more than one face is present, 0 otherwise."
arg: rs_allocation a
summary: Presence of more than one face
description:
 If the allocation is a cubemap, this function returns 1 if there's more than
 one face present.  In all other cases, it returns 0.

 Use @rsGetDimHasFaces() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimLOD
ret: uint32_t, "Returns 1 if more than one LOD is present, 0 otherwise."
arg: rs_allocation a
summary: Presence of levels of detail
description:
 Query an allocation for the presence of more than one Level Of Detail.
 This is useful for mipmaps.

 Use @rsGetDimLod() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimX
ret: uint32_t, "The X dimension of the allocation."
arg: rs_allocation a
summary: Size of the X dimension
description:
 Returns the size of the X dimension of the allocation.

 Use @rsGetDimX() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimY
ret: uint32_t, "The Y dimension of the allocation."
arg: rs_allocation a
summary: Size of the Y dimension
description:
 Returns the size of the Y dimension of the allocation.
 If the allocation has less than two dimensions, returns 0.

 Use @rsGetDimY() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimZ
ret: uint32_t, "The Z dimension of the allocation."
arg: rs_allocation a
summary: Size of the Z dimension
description:
 Returns the size of the Z dimension of the allocation.
 If the allocation has less than three dimensions, returns 0.

 Use @rsGetDimZ() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetElement
ret: rs_element, "element describing allocation layout"
arg: rs_allocation a, "allocation to get data from"
summary:
description:
 Get the element object describing the allocation's layout
test: none
end:

function: rsClearObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: void
arg: #1* dst
summary: Release an object
description:
 Tells the run time that this handle will no longer be used to access the
 the related object.  If this was the last handle to that object, resource
 recovery may happen.

 After calling this function, *dst will be set to an empty handle.  See
 @rsIsObject().
test: none
end:

function: rsIsObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: bool
arg: #1 v
summary: Check for an empty handle
description:
 Returns true if the handle contains a non-null reference.

 This function does not validate that the internal pointer used in the handle
 points to an actual valid object; it only checks for null.

 This function can be used to check the element returned by
 @rsElementGetSubElement() or see if @rsClearObject() has been called on a
 handle.
test: none
end:

function: rsElementGetBytesSize
version: 16
ret: uint32_t
arg: rs_element e
summary: Return the size of an element
description:
 Returns the size in bytes that an instantiation of this element will occupy.
test: none
end:

function: rsElementGetDataKind
version: 16
ret: rs_data_kind
arg: rs_element e
summary: Return the kind of an element
description:
 Returns the element's data kind.  This is used to interpret pixel data.

 See @rs_data_kind.
test: none
end:

function: rsElementGetDataType
version: 16
ret: rs_data_type
arg: rs_element e
summary: Return the data type of an element
description:
 Returns the element's base data type.  This can be a type similar to C/C++ (e.g. RS_TYPE_UNSIGNED_8),
 a handle (e.g. RS_TYPE_ALLOCATION and RS_TYPE_ELEMENT), or a more complex numerical type
 (e.g.RS_TYPE_UNSIGNED_5_6_5 and RS_TYPE_MATRIX_4X4).

 If the element describes a vector, this function returns the data type of one of its items.

 If the element describes a structure, RS_TYPE_NONE is returned.

 See @rs_data_type.
test: none
end:

function: rsElementGetSubElement
version: 16
ret: rs_element, "Sub-element at the given index"
arg: rs_element e, "Element to query"
arg: uint32_t index, "Index of the sub-element to return"
summary: Return a sub element of a complex element
description:
 For the element represents a structure, this function returns the sub-element at
 the specified index.

 If the element is not a structure or the index is greater or equal to the number
 of sub-elements, an invalid handle is returned.
test: none
end:

function: rsElementGetSubElementArraySize
version: 16
ret: uint32_t, "Array size of the sub-element at the given index"
arg: rs_element e, "Element to query"
arg: uint32_t index, "Index of the sub-element"
summary: Return the array size of a sub element of a complex element
description:
 For complex elements, some sub-elements could be statically
 sized arrays. This function returns the array size of the
 sub-element at the index.
test: none
end:

function: rsElementGetSubElementCount
version: 16
ret: uint32_t, "Number of sub-elements in this element"
arg: rs_element e, "Element to get data from"
summary: Return the number of sub-elements
description:
 Elements could be simple, such as an int or a float, or a
 structure with multiple sub-elements, such as a collection of
 floats, float2, float4.  This function returns zero for simple
 elements or the number of sub-elements otherwise.
test: none
end:

function: rsElementGetSubElementName
version: 16
ret: uint32_t, "Number of characters actually written, excluding the null terminator"
arg: rs_element e, "Element to get data from"
arg: uint32_t index, "Index of the sub-element"
arg: char* name, "Array to store the name into"
arg: uint32_t nameLength, "Length of the provided name array"
summary: Return the name of a sub-element
description:
 For complex elements, this function returns the name of the sub-element
 at the specified index.
test: none
end:

function: rsElementGetSubElementNameLength
version: 16
ret: uint32_t, "Length of the sub-element name including the null terminator (size of buffer needed to write the name)"
arg: rs_element e, "Element to get data from"
arg: uint32_t index, "Index of the sub-element to return"
summary: Return the length of the name of a sub-element
description:
 For complex elements, this function will return the length of
 sub-element name at index
test: none
end:

function: rsElementGetSubElementOffsetBytes
version: 16
ret: uint32_t, "Offset in bytes of sub-element in this element at given index"
arg: rs_element e, "Element to get data from"
arg: uint32_t index, "Index of the sub-element"
summary:
description:
 This function specifies the location of a sub-element within
 the element
test: none
end:

function: rsElementGetVectorSize
version: 16
ret: uint32_t, "Length of the element vector (for float2, float3, etc.)"
arg: rs_element e, "Element to get data from"
summary:
description:
 Returns the element's vector size
test: none
end:

function: rsGetAllocation
ret: rs_allocation
arg: const void* p
deprecated: This function is deprecated and will be removed from the SDK in a future release.
summary: Returns the Allocation for a given pointer
description:
 Returns the Allocation for a given pointer.  The pointer should point within
 a valid allocation.  The results are undefined if the pointer is not from a
 valid allocation.
test: none
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
