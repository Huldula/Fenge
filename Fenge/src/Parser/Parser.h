#pragma once

#include "Core/Error.h"
#include "Token.h"
#include "Nodes.h"

namespace fenge {

class ParserResult {
public:
	Error error;
	Node* node{ nullptr };

	[[nodiscard]] std::string toString() const;
};

class Parser
{
public:
	explicit Parser(const std::vector<Token*>& input);

	ParserResult parse();
	ParserResult parseMaybeBinary(ParserResult(Parser::* toCall)(), bool isType(Token::Type), bool maybe);
	ParserResult parseBinary(ParserResult(Parser::* toCall)(), bool isType(Token::Type));
	ParserResult parseAssign(Token* datatype);

	ParserResult parseStatementList();
	ParserResult parseStatement();
	ParserResult parseFuncDef();
	ParserResult parseArgList();
	ParserResult parseArg();
	ParserResult parseBlock();
	ParserResult parseReturn();
	ParserResult parseVarDef();
	ParserResult parseAssign();
	ParserResult parseLogOr();
	ParserResult parseLogXor();
	ParserResult parseLogAnd();
	ParserResult parseCompEq();
	ParserResult parseCompRela();
	ParserResult parseBitOr();
	ParserResult parseBitXor();
	ParserResult parseBitAnd();
	ParserResult parseBitShift();
	ParserResult parseMathAdd();
	ParserResult parseMathMul();
	ParserResult parseUnary();
	ParserResult parseSimple();
	//ParserResult* parseBinary(ParserResult* (Parser::* func)());

	//ParserResult* parseUnaryExpr();
private:
	int pos_;
	Token* advance();
	Token* peek(unsigned int count);
	Token* peek();
	Token* currentToken_;
	const std::vector<Token*>& input_;
};

}
