#pragma once

namespace fenge {

enum class Error {
	NO_ERROR,
	ILLEGAL_CHAR
};

static class ErrorMessageGenerator {
public:
	static std::string fromError(Error error) {
		switch (error) {
		case Error::NO_ERROR:
			return std::string();
		case Error::ILLEGAL_CHAR:
			return std::string("Illegal character");
		default:
			return std::string();
		}
	}
};

}
