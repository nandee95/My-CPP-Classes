#pragma once

template <typename T>
class ScopedPtr
{
public:
	T* ptr;
	ScopedPtr(T* ptr) : ptr(ptr)
	{

	}

	ScopedPtr() : ptr(nullptr)
	{

	}
	inline operator T* () const noexcept
	{
		return ptr;
	}

	void operator = (T* ptr) noexcept
	{
		if (ptr != nullptr) delete this->ptr;
		this->ptr = ptr;
	}
	inline T& operator *() const noexcept
	{
		return *ptr;
	}

	inline T* operator ->() const noexcept
	{
		return ptr;
	}

	inline operator bool() const noexcept
	{
		return ptr == nullptr;
	}

	~ScopedPtr()
	{
		if (ptr != nullptr) delete ptr;
	}
};