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

#define TOKEN_STRING_LIST(T) \
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

#define KEYWORD_STRING_LIST(T) \
	T(NO_KEYWORD, "") \
	T(VAR, "var")


#define T(name, str) Z(name)
	#define TOKEN_LIST(Z) TOKEN_STRING_LIST(T)
#undef T

#define T(name, str) Z(name)
	#define KEYWORD_LIST(Z) KEYWORD_STRING_LIST(T)
#undef T

enum class Token::Type {
#define T(name) name,
	TOKEN_LIST(T)
#undef T
};

enum class Token::Keyword {
#define T(name) name,
	KEYWORD_LIST(T)
#undef T
};


} // namespace fenge

