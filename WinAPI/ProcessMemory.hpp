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
	DWORD pid = NULL;	 //Process id
public:
	const bool OpenByWindowTitle(const std::string title)
	{
		//Find window by title
		HWND hwnd = FindWindow(NULL, title.c_str());
		if (hwnd == NULL)
			return false;

		//Obtain process id
		pid = NULL;
		GetWindowThreadProcessId(hwnd, &pid);
		if (pid == NULL)
			return false;

		//Open process
		handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid);
		return handle == NULL;
	}

	const uint64_t GetModuleAddr(const std::string module) const
	{
		uint64_t result = NULL;
		//Query list of modules
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
		if (snap == INVALID_HANDLE_VALUE)
			return NULL;

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

	template <typename T>
	inline const bool ReadMemoryValue(T &result, uint64_t address, const std::initializer_list<uint64_t> pointers = {}) const
	{
		//Simple read if no multi level pointers
		if (pointers.size() == 0)
			return ReadProcessMemory(handle, (void *)address, &result, sizeof(result), NULL);

		//Read start address
		if (!ReadProcessMemory(handle, (void *)address, &address, sizeof(address), NULL))
			return false;

		//Loop through the pointers
		for (auto it = pointers.begin(); it != std::prev(pointers.end()); it++)
		{
			address += *it;
			if (!ReadProcessMemory(handle, (void *)address, &address, sizeof(address), NULL))
				return false;
		}

		//Read from last address to the result
		address += *std::prev(pointers.end());
		return ReadProcessMemory(handle, (void *)address, &result, sizeof(result), NULL);
	}

	const bool IsValid() const
	{
		DWORD ec;
		GetExitCodeProcess(handle, &ec);
		return ec == STILL_ACTIVE;
	}
};