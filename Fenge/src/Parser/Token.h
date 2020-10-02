#pragma once

namespace fenge {

class Token
{
public:
	enum class Type;
	enum class Keyword;

	Token(Type type, void* value);
	~Token();

	const bool hasValue() const;
	const size_t valueSize() const;

	const Type type() const;

	const std::string toString() const;
private:
	const Type type_;
	const void* value_;
};

#define CHAR_TOKEN_LIST(T) \
	T(PLUS, '+') \
	T(MINUS, '-') \
	T(MUL, '*') \
	T(DIV, '/') \
	T(LPAREN, '(') \
	T(RPAREN, ')') \

#define TOKEN_TO_STRING_LIST(T) \
	T(ILLEGAL, "ILLEGAL") \
	T(IDENTIFIER, "IDENTIFIER") \
	T(KEYWORD, "KEYWORD") \
	T(INT, "INT") \
	T(FLOAT, "FLOAT") \
	T(PLUS, "PLUS") \
	T(MINUS, "MINUS") \
	T(MUL, "MUL") \
	T(DIV, "DIV") \
	T(LPAREN, "LPAREN") \
	T(RPAREN, "RPAREN") \
	T(EOS, "EOS")


enum class Token::Type {
#define T(name, val) name,
	TOKEN_TO_STRING_LIST(T)
#undef T
};

enum class Token::Keyword {
	VAR
};


} // namespace fenge

