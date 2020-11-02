#include "fgpch.h"

#include "Token.h"

namespace fenge {

Token::Token(Type type, void* value) : type_(type), value_(value) { }

Token::~Token() {
	switch (type_)
	{
	case Type::INT:
		delete (int*)value_;
		break;
	case Type::FLOAT:
		delete (double*)value_;
		break;
	case Type::KEYWORD:
		delete (Keyword*)value_;
		break;
	case Type::IDENTIFIER:
		delete (std::string*)value_;
		break;
	}
}

const inline bool Token::hasValue() const {
	return value_ != nullptr;
}

const size_t Token::valueSize() const {
	switch (type_)
	{
	case Type::INT:
		return sizeof(int);
	case Type::FLOAT:
		return sizeof(double);
	case Type::KEYWORD:
		return sizeof(Keyword);
	case Type::IDENTIFIER:
		return ((std::string*)value_)->size();
	default:
		return 0;
	}
}
Token::Keyword Token::keywordType(const std::string_view in) {
#define T(name, str) if (in == str) { return Keyword::name; } else
	KEYWORD_STRING_LIST
#undef T
		return Keyword::NO_KEYWORD;
}

bool Token::isLiteralType(Type type) {
	return type == Type::INT || type == Type::FLOAT || type == Type::IDENTIFIER;
}

bool Token::isUnaryType(Token* token) {
	return token->type() == Type::LOG_NOT
		|| token->type() == Type::BIT_NOT
		|| isReturnKeyword(token)
		|| isAddType(token->type());
}

bool Token::isLogType(Type type) {
	return isLogOrType(type) || isLogXorType(type) || isLogAndType(type) || type == Type::LOG_NOT;
}


bool Token::isSemicolonType(Type type) {
	return type == Type::SEMICOLON;
}

bool Token::isCommaType(Type type) {
	return type == Type::COMMA;
}

bool Token::isLogOrType(Type type) {
	return type == Type::LOG_OR;
}

bool Token::isLogXorType(Type type) {
	return type == Type::LOG_XOR;
}

bool Token::isLogAndType(Type type) {
	return type == Type::LOG_AND;
}

bool Token::isCompEqType(Type type) {
	return type == Type::EE || type == Type::NE;
}

bool Token::isCompRelaType(Type type) {
	return type == Type::LT || type == Type::LTE || type == Type::GT || type == Type::GTE;
}

bool Token::isAddType(Type type) {
	return type == Type::PLUS || type == Type::MINUS;
}

bool Token::isMulType(Type type) {
	return type == Type::MUL || type == Type::DIV || type == Type::MOD;
}

bool Token::isNotType(Type type) {
	return type == Type::LOG_NOT || type == Type::BIT_NOT;
}

bool Token::isBitOrType(Type type) {
	return type == Type::BIT_OR;
}

bool Token::isBitXorType(Type type) {
	return type == Type::BIT_XOR;
}

bool Token::isBitAndType(Type type) {
	return type == Type::BIT_AND;
}

bool Token::isBitShiftType(Type type) {
	return type == Type::SHL || type == Type::SHR || type == Type::ROTL || type == Type::ROTR;
}

bool Token::isDTKeyword(Token* token) {
	return token->type() == Type::KEYWORD && *(Keyword*)token->value() == Keyword::INT;
}

bool Token::isReturnKeyword(Token* token) {
	return token->type() == Type::KEYWORD && *(Keyword*)token->value() == Keyword::RETURN;
}

bool Token::isIfKeyword(Token* token) {
	return token->type() == Type::KEYWORD && *(Keyword*)token->value() == Keyword::IF;
}

bool Token::isElseKeyword(Token* token) {
	return token->type() == Type::KEYWORD && *(Keyword*)token->value() == Keyword::ELSE;
}

bool Token::isWhileKeyword(Token* token) {
	return token->type() == Type::KEYWORD && *(Keyword*)token->value() == Keyword::WHILE;
}

bool Token::isExternKeyword(Token* token) {
	return token->type() == Type::KEYWORD && *(Keyword*)token->value() == Keyword::EXTERN;
}

bool Token::isAfterSemicolonType(Type type)
{
	return type == Type::EOS || type == Type::RBRACE;
}



const Token::Type Token::type() const {
	return type_;
}

const void* Token::value() const {
	return value_;
}

[[nodiscard]] std::string Token::toString() const {
	std::string out = "[";
	switch (type_) {
#define T(name, str) \
	case Token::Type::name: \
		out += #name; \
		break;
	TOKEN_STRING_LIST
#undef T
	}
	switch (type_) {
	case Type::INT:
		out += ":" + std::to_string(*(int*)value_);
		break;
	case Type::FLOAT:
		out += ":" + std::to_string(*(double*)value_);
		break;
	case Type::KEYWORD:
		switch (*(Keyword*)value_) {
#define T(name, str) \
			case Keyword::name: \
				out += ":" + std::string(#name); \
				break;
		KEYWORD_STRING_LIST
#undef T
		}
		break;
	case Type::IDENTIFIER:
		out += ":" + *((std::string*)value_);
		break;
	}
	out += "]";
	return out;
}

} // namespace fenge
