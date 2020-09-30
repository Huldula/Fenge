#pragma once

#include <vector>
#include <string>
#include "../Core/Position.h"
#include "../Core/Log.h"
#include "../Core/Error.h"
#include "Token.h"

namespace fenge {

struct LexerResult {
	LexerResult() = default;
	~LexerResult() {
		for (Token* t : tokens) 
			delete t;
	}

	Error error = Error::NO_ERROR;
	std::vector<Token*> tokens;

	const inline std::string toString() const {
		std::string out;
		if (error != Error::NO_ERROR) {
			out = ErrorMessageGenerator::fromError(error);
		} else {
			for (const Token* t : tokens) {
				out += t->toString();
			}
		}
		return out;
	}
};


class Lexer
{
public:
	Lexer(const std::string& input);
	~Lexer();
	LexerResult* generateTokens();
private:
	Position pos_;
	void advance();
	char currentChar_;
	const std::string& input_;

	inline void pushBackAdvance(LexerResult* result, Token* t);
	inline Token* makeNumber();
};



} // namespace fenge
