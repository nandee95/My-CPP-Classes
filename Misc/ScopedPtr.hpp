/*
Licensed under the MIT license
Copyright (c) 2019 Nandor Szalma
Github: https://github.com/nandee95/My_CPP_Classes

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

/// <summary> Scoped pointer implementation </summary>
/// <template name="T"> Type of the pointer </template>
/// <template name="A"> True if the pointer is array </template>
template <typename T, bool A = false>
class ScopedPtr
{
public:
	T* ptr = nullptr; //Stored pointer
	
	/// <summary> Initializes the stored pointer to the given pointer </summary>
	ScopedPtr(T* ptr) : ptr(ptr)
	{ }

	/// <summary> Casting to the type of the stored pointer </summary>
	/// <returns> Stored pointer </returns>
	inline operator T* () const noexcept
	{
		return ptr;
	}

	/// <summary> Sets the stored pointer </summary>
	void operator = (T* ptr) noexcept
	{
		if (ptr != nullptr) delete this->ptr;
		this->ptr = ptr;
	}

	/// <summary> Returns the object the stored pointer points to </summary>
	/// <returns> Pointed object </returns>
	inline T& operator *() const noexcept
	{
		return *ptr;
	}

	/// <summary> References the functions of the stored pointer </summary>
	/// <returns> True on success </returns>
	inline T* operator ->() const noexcept
	{
		return ptr;
	}

	/// <summary> Returns true if the stored pointer is valid </summary>
	/// <returns> State of the pointer </returns>
	inline operator bool() const noexcept
	{
		return ptr == nullptr;
	}

	/// <summary> Frees the memory if necessary </summary>
	~ScopedPtr()
	{
		if (ptr != nullptr) delete ptr;
	}
};