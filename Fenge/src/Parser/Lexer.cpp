#include "Lexer.h"

namespace fenge {

Lexer::Lexer(const std::string& input) : input_(input) {
	pos_ = Position();
	advance();
}

Lexer::~Lexer() = default;

void Lexer::advance() {
	pos_.advance(currentChar_);
	currentChar_ = input_[pos_.index()];
}

inline void Lexer::pushBackAdvance(LexerResult* result, Token* t) {
	result->tokens.push_back(t);
	advance();
}

LexerResult* Lexer::generateTokens() {
	LexerResult* result = new LexerResult();
	while (currentChar_ != '\0') {
		if (IS_WHITESPACE_CHAR(currentChar_)) {
			advance();
		} else if (IS_LETTER_CHAR(currentChar_)) {
			advance();
		} else if (IS_DECIMAL_DOT_CHAR(currentChar_)) {
			result->tokens.push_back(makeNumber());
		} else if (currentChar_ == '+') {
			pushBackAdvance(result, new Token(Token::Type::PLUS, nullptr));
		} else if (currentChar_ == '-') {
			pushBackAdvance(result, new Token(Token::Type::MINUS, nullptr));
		} else if (currentChar_ == '*') {
			pushBackAdvance(result, new Token(Token::Type::MUL, nullptr));
		} else if (currentChar_ == '/') {
			pushBackAdvance(result, new Token(Token::Type::DIV, nullptr));
		} else if (currentChar_ == '(') {
			pushBackAdvance(result, new Token(Token::Type::LPAREN, nullptr));
		} else if (currentChar_ == ')') {
			pushBackAdvance(result, new Token(Token::Type::RPAREN, nullptr));
		} else {
			result->error = Error(Error::ILLEGAL_CHAR);
			return result;
		}

		if (result->tokens.size() > 0 &&
			result->tokens[result->tokens.size() - 1]->type() == Token::Type::ILLEGAL) {
			result->error = Error(Error::ILLEGAL_CHAR);
			return result;
		}
	}
	return result;
}

inline Token* Lexer::makeNumber() {
	int intValue = 0;
	while (IS_DECIMAL_CHAR(currentChar_)) {
		intValue = intValue * 10 + (currentChar_ - '0');
		advance();
	}
	if (currentChar_ == '.') {
		double floatValue = (double)intValue;
		advance();
		if (!(IS_DECIMAL_CHAR(currentChar_))) {
			return new Token(Token::Type::ILLEGAL, nullptr);
		}
		int startIndex = pos_.index() - 1;
		do {
			floatValue += (currentChar_ - '0') / pow(10, pos_.index() - startIndex);
			advance();
		} while (IS_DECIMAL_CHAR(currentChar_));
		return new Token(Token::Type::FLOAT, new double(floatValue));
	} else {
		return new Token(Token::Type::INT, new int(intValue));
	}
}

} // namespace fenge
