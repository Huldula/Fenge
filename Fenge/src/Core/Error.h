#pragma once

#include "fgpch.h"

#include "Position.h"

namespace fenge {

enum class ErrorCode {
	NO_ERROR,
	ILLEGAL_CHAR,

	ILLEGAL_TOKEN,
	RPAREN_EXPECTED,

	COMPILE_ERROR,
	VAR_NOT_FOUND,
	VAR_ALREADY_EXISTS,
	SEMICOLON_EXPECTED
};

struct Error {
public:
	Error() : code(ErrorCode::NO_ERROR) {}
	Error(ErrorCode code, Position pos) : code(code), pos(pos) {}
	explicit Error(ErrorCode code) : code(code) {}
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
		case ErrorCode::RPAREN_EXPECTED:
			return std::string("Expected ')' at index ") + std::to_string(error.pos.index());
		case ErrorCode::COMPILE_ERROR:
			return std::string("Compile Error");
		case ErrorCode::VAR_NOT_FOUND:
			return std::string("Variable not found");
		case ErrorCode::VAR_ALREADY_EXISTS:
			return std::string("Variable already exists");
		case ErrorCode::SEMICOLON_EXPECTED:
			return std::string("Seimcolon expected");
		default:
			return std::string("Internal Error");
		}
	}
};

}
