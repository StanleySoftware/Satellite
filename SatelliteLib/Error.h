#pragma once
#include <string>
#include <CStringWrapper.h>

namespace Sat
{

enum class ErrorType : int
{
	UNSPECIFIED = -1,
	SUCCESS = 0,
	NOINIT,
	NOTACHECKOUT,
	BARECHECKOUT,
	FILEREAD,
	JSONPARSE,
	EMPTYQUERY,
	SEPARATEARGFAILURE,
	EMPTYVALUE,
	FILEDOESNOTEXIST,
	FILENOTJSON,
	INVALIDSTRHND,
	STRCOPYFAILURE
};

struct Error
{
	ErrorType m_errorType;
	CStringWrapper m_errorMessage;

	unsigned int errorCode() const { return static_cast<unsigned int>(m_errorType);  }
};

}