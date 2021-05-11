#pragma once

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

namespace LibLoad
{

#if defined(_WIN32)
using LibHandle = HMODULE;
using LibString = LPCSTR;
#endif

class LibObject
{
public:
	LibObject(LibString p_filename);
	~LibObject();

	LibObject(const LibObject&) =delete;
	LibObject& operator=(const LibObject&) =delete;
	LibObject(LibObject&&) =delete;
	LibObject& operator=(LibObject&&) =delete;

	bool load();
	bool is_loaded() const;
	void unload();

	template<typename FuncPtr>
	FuncPtr get_proc(LibString p_func_name)
	{
		return static_cast<FuncPtr>(get_proc_inner(p_func_name));
	}

	LibString get_lib_name() const;

protected:
	void* get_proc_inner(LibString p_func_name);

	unsigned int m_count = 0;
	LibString const m_libName{};
	LibHandle m_libHandle{};
};

}