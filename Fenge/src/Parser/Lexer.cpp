#include "fgpch.h"

#include "Lexer.h"

namespace fenge {

Lexer::Lexer(const std::string& input) : input_(input) {
	pos_ = Position();
	advance();
}

Lexer::~Lexer() = default;

void Lexer::advance() {
	pos_.advance(currentChar_);
	currentChar_ = input_[currentPos()];
}

inline void Lexer::pushBackAdvance(LexerResult& result, Token* t) {
	result.tokens.push_back(t);
	advance();
}

inline int Lexer::currentPos() const {
	return pos_.index();
}

LexerResult Lexer::generateTokens() {
	LexerResult result = LexerResult();
	while (currentChar_ != '\0') {
		if (IS_WHITESPACE_CHAR(currentChar_)) {
			advance();
		} else if (IS_LETTER_CHAR(currentChar_)) {
			result.tokens.push_back(makeIdentifier());
		} else if (IS_DECIMAL_DOT_CHAR(currentChar_)) {
			result.tokens.push_back(makeNumber());
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
		} else if (currentChar_ == '=') {
			result.tokens.push_back(makeEE_EQASSIGN());
		} else if (currentChar_ == '!') {
			result.tokens.push_back(makeNOT_NE());
		} else if (currentChar_ == '<') {
			result.tokens.push_back(makeLT_LTE_SHIFT());
		} else if (currentChar_ == '>') {
			result.tokens.push_back(makeGT_GTE_SHIFT());
		} else if (currentChar_ == '|') {
			result.tokens.push_back(makeOR());
		} else if (currentChar_ == '^') {
			result.tokens.push_back(makeXOR());
		} else if (currentChar_ == '&') {
			result.tokens.push_back(makeAND());
		} else if (currentChar_ == '~') {
			pushBackAdvance(result, new Token(Token::Type::BIT_NOT, nullptr));
		} else if (currentChar_ == ';') {
			pushBackAdvance(result, new Token(Token::Type::SEMICOLON, nullptr));
		} else {
			result.error = Error(ErrorCode::ILLEGAL_CHAR, pos_);
			return result;
		}

		if (result.tokens.size() > 0 &&
			result.tokens[result.tokens.size() - 1]->type() == Token::Type::ILLEGAL) {
			result.error = Error(ErrorCode::ILLEGAL_CHAR, pos_);
			return result;
		}
	}

	result.tokens.push_back(new Token(Token::Type::EOS, nullptr));
	return result;
}

Token* Lexer::makeNumber() {
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
		int startIndex = currentPos() - 1;
		do {
			floatValue += ((double)currentChar_ - '0') / pow(10, currentPos() - startIndex);
			advance();
		} while (IS_DECIMAL_CHAR(currentChar_));
		return new Token(Token::Type::FLOAT, new double(floatValue));
	} else {
		return new Token(Token::Type::INT, new int(intValue));
	}
}

Token* Lexer::makeIdentifier() {
	const int startPos = currentPos();
	while (IS_LETTER_DECIMAL_CHAR(currentChar_)) {
		advance();
	}
	const std::string_view identifier(&input_[startPos], (size_t)currentPos() - startPos);
	const Token::Keyword keywordType = Token::keywordType(identifier);
	if (keywordType == Token::Keyword::NO_KEYWORD) {
		return new Token(Token::Type::IDENTIFIER, new std::string(identifier));
	} else {
		return new Token(Token::Type::KEYWORD, new Token::Keyword(keywordType));
	}
}

Token* Lexer::makeEE_EQASSIGN() {
	advance();
	if (currentChar_ == '=') {
		advance();
		return new Token(Token::Type::EE, nullptr);
	} else {
		return new Token(Token::Type::EQ_ASSIGN, nullptr);
	}
}

Token* Lexer::makeNOT_NE() {
	advance();
	if (currentChar_ == '=') {
		advance();
		return new Token(Token::Type::NE, nullptr);
	} else {
		return new Token(Token::Type::LOG_NOT, nullptr);
	}
}

Token* Lexer::makeLT_LTE_SHIFT() {
	advance();
	if (currentChar_ == '=') {
		advance();
		return new Token(Token::Type::LTE, nullptr);
	} else if (currentChar_ == '<') {
		advance();
		if (currentChar_ == '<') {
			advance();
			return new Token(Token::Type::ROTL, nullptr);
		} else {
			return new Token(Token::Type::SHL, nullptr);
		}
	} else {
		return new Token(Token::Type::LT, nullptr);
	}
}

Token* Lexer::makeGT_GTE_SHIFT() {
	advance();
	if (currentChar_ == '=') {
		advance();
		return new Token(Token::Type::GTE, nullptr);
	} else if (currentChar_ == '>') {
		advance();
		if (currentChar_ == '>') {
			advance();
			return new Token(Token::Type::ROTR, nullptr);
		} else {
			return new Token(Token::Type::SHR, nullptr);
		}
	} else {
		return new Token(Token::Type::GT, nullptr);
	}
}

Token* Lexer::makeOR() {
	advance();
	if (currentChar_ == '|') {
		advance();
		return new Token(Token::Type::LOG_OR, nullptr);
	} else {
		return new Token(Token::Type::BIT_OR, nullptr);
	}
}

Token* Lexer::makeXOR() {
	advance();
	if (currentChar_ == '^') {
		advance();
		return new Token(Token::Type::LOG_XOR, nullptr);
	} else {
		return new Token(Token::Type::BIT_XOR, nullptr);
	}
}

Token* Lexer::makeAND() {
	advance();
	if (currentChar_ == '&') {
		advance();
		return new Token(Token::Type::LOG_AND, nullptr);
	} else {
		return new Token(Token::Type::BIT_AND, nullptr);
	}
}



LexerResult::LexerResult() = default;
LexerResult::~LexerResult() {
	//for (const Token* t : tokens)
		//delete t;
}

std::string LexerResult::toString() const {
	std::string out;
	if (error.isError()) {
		out = ErrorMessageGenerator::fromError(error);
	} else {
		for (const Token* t : tokens) {
			out += t->toString();
		}
	}
	return out;
}

} // namespace fenge
