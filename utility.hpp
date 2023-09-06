#ifndef _utility_hpp
#define _utility_hpp

#if defined _DEBUG
	static auto constexpr DEBUG_MODE = true;
#else
	static auto constexpr DEBUG_MODE = false;
#endif

#include <windows.h>

#include <vector>
#include <string>
#include <iostream>

namespace utility
{
	inline bool showConsole()
	{
		if constexpr (DEBUG_MODE)
		{
			if (AllocConsole())
			{
				SetConsoleTitleA("DEBUG CONSOLE");

				return !freopen_s(
					reinterpret_cast<FILE**>(stdout),
					"CONOUT$",
					"w",
					stdout);
			}
		}

		return false;
	}

	//Helper
	inline void debugMsg(std::string const& s)
	{
		if constexpr (DEBUG_MODE)
			std::cout << s;
	}

	inline uintptr_t getBaseAddress()
	{
		return reinterpret_cast<uintptr_t>(
			GetModuleHandleA(NULL));
	}

	inline bool readMemory(
		uintptr_t const address,
		uintptr_t buffer,
		size_t const size)
	{
		return ReadProcessMemory(
			GetCurrentProcess(),
			reinterpret_cast<LPVOID>(address),
			reinterpret_cast<LPVOID>(buffer),
			size,
			nullptr);
	}

	inline bool makeWritable(
		uintptr_t const address,
		size_t const size)
	{
		DWORD oldp;

		return VirtualProtect(
			reinterpret_cast<LPVOID>(address),
			size,
			PAGE_EXECUTE_READWRITE,
			&oldp);
	}

	inline bool doHooking(
		uintptr_t address,
		uintptr_t const callback,
		size_t size = 5,
		bool isCall = false)
	{
		auto offset = callback - address - 5;
		auto p = reinterpret_cast<uint8_t*>(address);

		if (size < 5)
			size = 5;

		if (makeWritable(address, size))
		{
			*(p++) = isCall ? 0xE8 : 0xE9;

			for (auto i = 0u; i < sizeof(decltype(offset)); ++i)
				*(p++) = reinterpret_cast<uint8_t*>(&offset)[i];

			for (auto i = 5u; i < size; ++i)
				*(p++) = 0x90;

			return true;
		}

		return false;
	}

	template <typename T>
	bool doHooking(
		T& address,
		uintptr_t const callback,
		size_t size = 5,
		bool isCall = false)
	{
		return doHooking(
			reinterpret_cast<uintptr_t>(address),
			callback,
			size,
			isCall);
	}

	template <typename T>
	bool doHooking(
		uintptr_t address,
		T const& callback,
		size_t size = 5,
		bool isCall = false)
	{
		return doHooking(
			address,
			reinterpret_cast<uintptr_t const>(callback),
			size,
			isCall);
	}

	inline uintptr_t getPointer(
		uintptr_t const base,
		std::vector<uint32_t> const& offsets)
	{
		auto p = getBaseAddress() + base;

		for (auto i = 0u; i < offsets.size(); ++i)
		{
			if (!ReadProcessMemory(
				GetCurrentProcess(),
				reinterpret_cast<uint8_t*>(p),
				&p,
				sizeof(decltype(p)),
				0))
				return 0u;

			p += offsets[i];
		}

		return p;
	}
}

#endif /* _utility_hpp */