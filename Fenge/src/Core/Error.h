#pragma once

#include "fgpch.h"

#include "Position.h"

namespace fenge {

enum class ErrorCode {
	NO_ERROR,
	ILLEGAL_CHAR,
	ILLEGAL_TOKEN
};

struct Error {
public:
	Error(ErrorCode code, Position pos) : code(code), pos(pos) {}
	Error(ErrorCode code) : code(code) {}
	ErrorCode code;
	Position pos;

	inline bool isError() const {
		return code != ErrorCode::NO_ERROR;
	}
};

class ErrorMessageGenerator {
public:
	static std::string fromError(Error error) {
		switch (error.code) {
		case ErrorCode::NO_ERROR:
			return std::string();
		case ErrorCode::ILLEGAL_CHAR:
			return std::string("Illegal character at index ") + std::to_string(error.pos.index());
		case ErrorCode::ILLEGAL_TOKEN:
			return std::string("Illegal token at index ") + std::to_string(error.pos.index());
		default:
			return std::string();
		}
	}
};

}
