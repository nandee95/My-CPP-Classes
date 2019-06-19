/*
Licensed under the MIT license
Copyright 2019 Nandor Szalma

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
#include <string>
#include <initializer_list>

//Windows headers
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>

class ProcessMemory
{
protected:
	HANDLE handle = NULL; //Process handle
	DWORD pid = NULL; //Process id
public:

	/// <summary> Opens the process by window title </summary>
	/// <param name="title"> Title of the window </param>
	/// <returns> True on success </returns>
	const bool OpenByWindowTitle(const std::string title)
	{
		//Find window by title
		HWND hwnd=FindWindow(NULL, title.c_str());
		if (hwnd == NULL) return false;

		//Obtain process id
		pid = NULL;
		GetWindowThreadProcessId(hwnd, &pid);
		if (pid == NULL) return false;

		//Open process
		handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid);
		return handle == NULL;
	}

	/// <summary> Finds the address of the given module. </summary>
	/// <param name="module"> Name of the module </param>
	/// <returns> Module address, 0 on fail </returns>
	const uint64_t GetModuleAddr(const std::string module) const
	{
		uint64_t result = NULL;
		//Query list of modules
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
		if (snap == INVALID_HANDLE_VALUE) return NULL;
		
		MODULEENTRY32 entry;
		entry.dwSize = sizeof(entry);
		//Loop through each module
		if (Module32First(snap, &entry))
		{
			do
			{
				if (module.compare(entry.szModule) == 0)
				{
					result = (uint64_t)entry.modBaseAddr;
					break;
				}
			} while (Module32Next(snap, &entry));
		}
		
		//Close handle
		CloseHandle(snap);
		return result;
	}

	/// <summary> Reads a value from the process. </summary>
	/// <param name="result"> Variable to read into </param>
	/// <param name="address"> Base address </param>
	/// <param name="pointers"> List of multi-level pointers (Optional) </param>
	/// <returns> True on success </returns>
	template <typename T>
	inline const bool ReadMemoryValue(T& result, uint64_t address, const std::initializer_list<uint64_t> pointers = {}) const
{
		//Simple read if no multi level pointers
		if (pointers.size() == 0) return ReadProcessMemory(handle, (void*)address, &result, sizeof(result), NULL);
		
		//Read start address
		if (!ReadProcessMemory(handle, (void*)address, &address, sizeof(address), NULL)) return false;

		//Loop through the pointers
		for (auto it = pointers.begin(); it != std::prev(pointers.end()); it++)
		{
			address += *it;
			if (!ReadProcessMemory(handle, (void*)address, &address, sizeof(address), NULL)) return false;
		}

		//Read from last address to the result
		address += *std::prev(pointers.end());
		return ReadProcessMemory(handle, (void*)address, &result, sizeof(result), NULL);
	}

	/// <summary> Modifies the memory of the process. </summary>
	/// <param name="value"> Value to write </param>
	/// <param name="address"> Base address </param>
	/// <param name="pointers"> List of multi-level pointers (Optional) </param>
	/// <returns> True on success </returns>
	template <typename T>
	inline const bool WriteMemoryValue(const T value, uint64_t address, const std::initializer_list<uint64_t> pointers = {}) const
	{
		//Simple write if no multi level pointers
		if (pointers.size() == 0) return WriteProcessMemory(handle, (void*)address, &value, sizeof(value), NULL);

		//Read start address
		if (!ReadProcessMemory(handle, (void*)address, &address, sizeof(address), NULL)) return false;

		//Loop through the pointers
		for (auto it = pointers.begin(); it != std::prev(pointers.end()); it++)
		{
			address += *it;
			if (!ReadProcessMemory(handle, (void*)address, &address, sizeof(address), NULL)) return false;
		}

		//Write the value to the address
		address += *std::prev(pointers.end());
		return WriteProcessMemory(handle, (void*)address, &value, sizeof(value), NULL);
	}

	/// <summary> Checks if the hande still exists. </summary>
	/// <returns> State of the handle. </returns>
	inline const bool IsValid() const
	{
		DWORD ec;
		GetExitCodeProcess(handle, &ec);
		return ec == STILL_ACTIVE;
	}
};