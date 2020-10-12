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

	static Keyword keywordType(const std::string_view in);

	const Type type() const;

	[[nodiscard]] std::string toString() const;
private:
	const Type type_;
	const void* value_;
};

#define TOKEN_STRING_LIST \
	T(ILLEGAL, "") \
	T(IDENTIFIER, "") \
	T(KEYWORD, "") \
	T(INT, "") \
	T(FLOAT, "") \
	T(PLUS, "+") \
	T(MINUS, "-") \
	T(MUL, "*") \
	T(DIV, "/") \
	T(LPAREN, "(") \
	T(RPAREN, ")") \
	T(EOS, "\0")

#define KEYWORD_STRING_LIST \
	T(NO_KEYWORD, "") \
	T(VAR, "var")


enum class Token::Type {
#define T(name, str) name,
	TOKEN_STRING_LIST
#undef T
};

enum class Token::Keyword {
#define T(name, str) name,
	KEYWORD_STRING_LIST
#undef T
};


} // namespace fenge

