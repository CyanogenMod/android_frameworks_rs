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
summary: Element functions
description:
 The term "element" is used a bit ambiguously in RenderScript, as both
 the type of an item of an allocation and the instantiation of that type:
 <ul>
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
