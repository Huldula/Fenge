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
	static bool isUnaryType(Type type);
	static bool isLogType(Type type);

	static bool isSemicolonType(Type type);
	static bool isLogOrType(Type type);
	static bool isLogXorType(Type type);
	static bool isLogAndType(Type type);
	static bool isCompEqType(Type type);
	static bool isCompRelaType(Type type);
	static bool isBitOrType(Type type);
	static bool isBitXorType(Type type);
	static bool isBitAndType(Type type);
	static bool isBitShiftType(Type type);
	static bool isAddType(Type type);
	static bool isMulType(Type type);

	static bool isDTKeyword(Token* type);

	const Type type() const; 
	const void* value() const; 

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
	T(EE, "==") \
	T(NE, "!=") \
	T(LT, "<") \
	T(LTE, "<=") \
	T(GT, ">") \
	T(GTE, ">=") \
	T(LOG_OR, "||") \
	T(LOG_XOR, "^^") \
	T(LOG_AND, "&&") \
	T(LOG_NOT, "!") \
	T(BIT_OR, "|") \
	T(BIT_XOR, "^") \
	T(BIT_AND, "&") \
	T(BIT_NOT, "~") \
	T(SHL, "<<") \
	T(SHR, ">>") \
	T(ROTL, "<<<") \
	T(ROTR, ">>>") \
	T(EQ_ASSIGN, "=") \
	T(SEMICOLON, ";") \
	T(EOS, "\0")

#define KEYWORD_STRING_LIST \
	T(NO_KEYWORD, "") \
	T(INT, "int")


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

