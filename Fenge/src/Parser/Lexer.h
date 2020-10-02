#pragma once

#include "Core/Position.h"
#include "Core/Error.h"
#include "Token.h"

namespace fenge {

struct LexerResult {
	LexerResult();
	~LexerResult();

	Error error = Error::NO_ERROR;
	std::vector<Token*> tokens;

	std::string toString() const;
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
