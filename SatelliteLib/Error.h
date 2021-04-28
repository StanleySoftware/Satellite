#pragma once
#include <string>
#include "Export.h"

namespace Sat
{

enum class ErrorType : signed int
{
	SUCCESS = 0,
	NOTACHECKOUT,
	BARECHECKOUT,
	FILEREAD,
	JSONPARSE
};

struct Error
{
	ErrorType m_errorType;
	std::string m_errorMessage;

	unsigned int errorCode() const { return static_cast<unsigned int>(m_errorType);  }
};

}