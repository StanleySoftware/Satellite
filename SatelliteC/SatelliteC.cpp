// SatelliteC.cpp : Defines the exported functions for the DLL.

#define NOMINMAX

#include "SatelliteC.h"
#include "Utility.h"

#include <memory>
#include <map>
#include <cassert>
#include <limits>

#include <SatelliteLib.h>
#include <ISatellite.h>

namespace SatC
{

bool sat_init()
{
	assert(s_string_table == nullptr);
	s_string_table = new std::vector<CStringWrapper*>();
	return true;
}

bool sat_shutdown()
{
	if (s_string_table)
	{
		delete s_string_table;
		s_string_table = nullptr;
	}
	return true;
}

Error sat_create(ISatellite** p_out_satellite)
{
	if (s_string_table == nullptr)
	{
		Error err{};
		err.m_errorType = static_cast<int32_t>(Sat::ErrorType::NOINIT);
		err.m_errorMessage = c_string_table_no_init;
		return err;
	}

	std::unique_ptr<ISatellite> satellite = Sat::satellite_factory();
	*p_out_satellite = satellite.release();
	return Error{};
}

void sat_load(ISatellite* p_satellite)
{
	if (p_satellite)
	{
		p_satellite->load();
	}
}

void sat_unload(ISatellite* p_satellite)
{
	if (p_satellite)
	{
		p_satellite->unload();
	}
}

Error sat_workspace_info(ISatellite* p_satellite, char const* p_targetPath, WorkspaceInfo* p_out_WorkspaceInfo)
{
	Error errPod{};
	if (p_satellite)
	{
		Sat::WorkspaceInfo ci{};
		Sat::Error err = p_satellite->workspace_info(p_targetPath, ci);
		if(!err.errorCode())
		{
			*p_out_WorkspaceInfo = to_interface_type(std::move(ci));
		}
		
		errPod = to_interface_type(std::move(err));
		return errPod;
	}
	errPod = create_error(Sat::ErrorType::UNSPECIFIED, "Satellite: Invoking \'workspace_info\' method without a valid ISatellite object.");
	return errPod;
}

Error sat_relay(ISatellite* p_satellite, char const* p_originPath, char const* p_query, StrHnd* p_out_string)
{
	Error errPod{};
	if (p_satellite)
	{
		CStringWrapper cstring{};
		Sat::Error err = p_satellite->relay(p_originPath, p_query, cstring);
		if(!err.errorCode())
		{
			*p_out_string = to_interface_type(std::move(cstring));
		}

		errPod = to_interface_type(std::move(err));
		return errPod;
	}
	errPod = create_error(Sat::ErrorType::UNSPECIFIED, "Satellite: Invoking \'relay\' method without a valid ISatellite object.");
	return errPod;
}

void sat_release(ISatellite** p_satellite)
{
	if (p_satellite && *p_satellite)
	{
		delete (*p_satellite);
		(*p_satellite) = nullptr;
	}
}

uint32_t sat_length_from_strhnd(StrHnd p_strhnd)
{
	if (!s_string_table || p_strhnd == c_string_table_no_init)
	{
		return static_cast<uint32_t>(strlen(c_no_init_message));
	}
	else
	{
		char * ptr = s_string_table->at(p_strhnd)->get();
		if(ptr)
		{
			return static_cast<uint32_t>(strlen(ptr));
		}
		else
		{
			return 0u;
		}
	}
}

Error sat_copy_from_strhnd(StrHnd p_strhnd, uint32_t p_size, char* p_out_string)
{
	if (p_size == 0u)
	{
		return Error{};
	}

	if (!s_string_table || p_strhnd == c_string_table_no_init)
	{
		uint32_t len = sat_length_from_strhnd(p_strhnd);
		char const * ptr = c_no_init_message;
		errno_t errNum = strncpy_s(p_out_string, p_size * sizeof(char), ptr, len);
		if (errNum != 0)
		{
			Error err = create_error(Sat::ErrorType::STRCOPYFAILURE, "Satellite: Error during string copy '%i'.", errNum);
			return err;
		}

		return Error{};
	}

	//Out of range check
	if (s_string_table->size() <= p_strhnd)
	{
		Error err = create_error(Sat::ErrorType::STRCOPYFAILURE, "Satellite: Attempting to use an invalid String Handle -- out of bounds.");
		return err;
	}

	uint32_t len = sat_length_from_strhnd(p_strhnd);
	char const * ptr = s_string_table->at(p_strhnd)->get();
	errno_t errNum = strncpy_s(p_out_string, p_size * sizeof(char), ptr, len);

	if (errNum != 0)
	{
		Error err = create_error(Sat::ErrorType::STRCOPYFAILURE, "Satellite: Error during string copy '%i'.", errNum);
		return err;
	}

	return Error{};
}

bool sat_clear_string_table()
{
	if (s_string_table)
	{
		s_string_table->clear();
		return true;
	}
	return false;
}

}