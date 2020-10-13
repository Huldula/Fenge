#pragma once

namespace fenge {

class Token
{
public:
	enum class Type;
	enum class Keyword;

	Token(Type type, void* value);
	~Token();

	const inline bool hasValue() const;
	const size_t valueSize() const;

	static Keyword keywordType(const std::string_view in);

	static bool isLiteralType(Type type);
	static bool isAddType(Type type);
	static bool isMulType(Type type);

	const inline Type type() const;

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
	T(MOD, "%") \
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

