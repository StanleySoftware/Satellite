#include "DllLoad.h"
#include <cassert>

namespace LibLoad
{

#if defined(_WIN32)
LibHandle dlopen(LibString p_filename)
{
	return LoadLibraryA(p_filename);
}

int dlclose(LibHandle p_handle)
{
	return FreeLibrary(p_handle);
}

void* dlsym(LibHandle p_handle, LibString p_func_name)
{
	return GetProcAddress(p_handle, p_func_name);
}
#endif


LibObject::LibObject(LibString p_filename)
	:
	m_libName(p_filename)
{
	
}

LibObject::~LibObject()
{
	assert(!m_libHandle);
	assert(m_count == 0);
}

bool LibObject::load()
{
	if (!m_libHandle)
	{
		m_libHandle = dlopen(m_libName);
	}
	m_count++;
	return (bool)m_libHandle;
}

bool LibObject::is_loaded() const
{
	return m_libHandle != nullptr;
}

void LibObject::unload()
{
	--m_count;
	if (m_count == 0 && m_libHandle)
	{
		dlclose(m_libHandle);
		m_libHandle = nullptr;
	}
}

template<typename FuncPtr>
FuncPtr LibObject::get_proc(LibString p_func_name)
{
	if(m_libHandle)
	{
		return dlsym(m_libHandle, p_func_name);
	}
	else
	{
		return nullptr;
	}
}

constexpr LibString LibObject::get_lib_name() const
{
	return m_libName;
}

}