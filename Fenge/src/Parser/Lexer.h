#pragma once

#include "Core/Position.h"
#include "Core/Error.h"
#include "Token.h"

namespace fenge {

struct LexerResult {
	LexerResult();
	~LexerResult();

	Error error;
	std::vector<Token*> tokens;

	[[nodiscard]] std::string toString() const;
};


class Lexer
{
public:
	explicit Lexer(const std::string& input);
	~Lexer();
	LexerResult generateTokens();
private:
	Position pos_;
	void advance();
	char currentChar_;
	const std::string& input_;

	inline void pushBackAdvance(LexerResult& result, Token* t);
	inline int currentPos() const;
	Token* makeNumber();
	Token* makeIdentifier();
	Token* makeEE_EQASSIGN();
	Token* makeNOT_NE();
	Token* makeLT_LTE();
	Token* makeGT_GTE();
	Token* makeOR();
	Token* makeXOR();
	Token* makeAND();
};



} // namespace fenge
