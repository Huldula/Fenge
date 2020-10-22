#include "fgpch.h"
#include "Parser.h"


namespace fenge {

Parser::Parser(const std::vector<Token*>& input) : input_(input) {
	pos_ = -1;
	advance();
}


Token* Parser::advance() {
	pos_++;
	return currentToken_ = input_.at(pos_);
}

Token* Parser::peek(unsigned int count) {
	if (input_.size() <= (size_t)pos_ + count)
		return input_.back();
	return input_.at((size_t)pos_ + count);
}

Token* Parser::peek() {
	return peek(1);
}

ParserResult Parser::parse() {
	ParserResult result = parseStatementList();
	if (currentToken_->type() != Token::Type::EOS)
		return ParserResult{ Error(ErrorCode::ILLEGAL_TOKEN, pos_), nullptr };
	return result;
}

ParserResult Parser::parseMaybeBinary(ParserResult(Parser::* toCall)(), bool isType(Token::Type), bool maybe) {
	ParserResult left = (this->*toCall)();
	if (left.error.isError())
		return left;
	ParserResult right;
	while (isType(currentToken_->type())) {
		Token* op = currentToken_;
		advance();
		if (maybe && Token::isAfterSemicolonType(currentToken_->type()))
			break;
		right = (this->*toCall)();
		if (right.error.isError())
			return right;
		left.node = (Node*)new BinaryNode(left.node, op, right.node);
	}
	return ParserResult{ Error(), left.node };
}

ParserResult Parser::parseBinary(ParserResult (Parser::*toCall)(), bool isType(Token::Type)) {
	return parseMaybeBinary(toCall, isType, false);
}

ParserResult Parser::parseAssign(Token* datatype) {
	if (peek()->type() == Token::Type::EQ_ASSIGN) {
		Token* id = currentToken_;
		advance();
		advance();
		ParserResult right = parseAssign();
		if (right.error.isError())
			return right;
		return ParserResult{ Error(), (Node*)new VarAssignNode(datatype, id, right.node) };
	} else {
		return parseLogOr();
	}
}









ParserResult Parser::parseStatementList() {
	return parseMaybeBinary(&Parser::parseStatement, Token::isSemicolonType, true);
}

ParserResult Parser::parseStatement() {
	if (Token::isDTKeyword(currentToken_)) {
		if (peek()->type() == Token::Type::IDENTIFIER && peek(2)->type() == Token::Type::EQ_ASSIGN) {
			return parseVarDef();
		} else if (peek()->type() == Token::Type::IDENTIFIER && peek(2)->type() == Token::Type::LPAREN) {
			return parseFuncDef();
		} else {
			advance();
			return ParserResult{ Error(ErrorCode::ILLEGAL_TOKEN, pos_), nullptr };
		}
	} else if (Token::isReturnKeyword(currentToken_)) {
		return parseReturn();
	} else if (currentToken_->type() == Token::Type::LBRACE) {
		return parseBlock();
	} else {
		return parseAssign();
	}
}

ParserResult Parser::parseFuncDef() {
	Token* datatype = currentToken_;
	advance();
	Token* identifier = currentToken_;
	advance();
	ParserResult argList = parseArgList();
	if (argList.error.isError())
		return argList;
	ParserResult content = parseStatement();
	if (content.error.isError())
		return content;
	return ParserResult{ Error(), (Node*)new FuncDefNode(datatype, identifier, argList.node, content.node) };
}

ParserResult Parser::parseArgList() {
	advance();
	ParserResult result;
	if (currentToken_->type() != Token::Type::RPAREN)
		result = parseMaybeBinary(&Parser::parseArg, Token::isColonType, true);
	advance();
	return result;
}

ParserResult Parser::parseArg() {
	Token* datatype = currentToken_;
	advance();
	Token* identifier = currentToken_;
	advance();
	return ParserResult{ Error(), (Node*)new ArgumentNode(datatype, identifier) };
}

ParserResult Parser::parseBlock() {
	advance();
	ParserResult result = parseStatementList();
	advance();
	return result;
}

// TODO
ParserResult Parser::parseReturn() {
	advance();
	return parseAssign();
}

ParserResult Parser::parseVarDef() {
	Token* datatype = currentToken_;
	advance();
	return parseAssign(datatype);
}

ParserResult Parser::parseAssign() {
	return parseAssign(nullptr);
}

ParserResult Parser::parseLogOr() {
	return parseBinary(&Parser::parseLogXor, Token::isLogOrType);
}

ParserResult Parser::parseLogXor() {
	return parseBinary(&Parser::parseLogAnd, Token::isLogXorType);
}

ParserResult Parser::parseLogAnd() {
	return parseBinary(&Parser::parseCompEq, Token::isLogAndType);
}

ParserResult Parser::parseCompEq() {
	return parseBinary(&Parser::parseCompRela, Token::isCompEqType);
}

ParserResult Parser::parseCompRela() {
	return parseBinary(&Parser::parseBitOr, Token::isCompRelaType);
}

ParserResult Parser::parseBitOr() {
	return parseBinary(&Parser::parseBitXor, Token::isBitOrType);
}

ParserResult Parser::parseBitXor() {
	return parseBinary(&Parser::parseBitAnd, Token::isBitXorType);
}

ParserResult Parser::parseBitAnd() {
	return parseBinary(&Parser::parseBitShift, Token::isBitAndType);
}

ParserResult Parser::parseBitShift() {
	return parseBinary(&Parser::parseMathAdd, Token::isBitShiftType);
}

ParserResult Parser::parseMathAdd() {
	return parseBinary(&Parser::parseMathMul, Token::isAddType);
}

ParserResult Parser::parseMathMul() {
	return parseBinary(&Parser::parseUnary, Token::isMulType);
}

ParserResult Parser::parseUnary() {
	if (Token::isUnaryType(currentToken_->type())) {
		Token* op = currentToken_;
		advance();
		ParserResult inner = parseUnary();
		if (inner.error.isError())
			return inner;
		return ParserResult{ Error(), (Node*)new UnaryNode(op, inner.node) };
	} else {
		return parseSimple();
	}
}

ParserResult Parser::parseSimple() {
	ParserResult result = ParserResult();
	if (Token::isLiteralType(currentToken_->type())) {
		result.node = (Node*)new LiteralNode(currentToken_);
		advance();
	} else if (currentToken_->type() == Token::Type::LPAREN) {
		advance();
		ParserResult inner = parseAssign();
		if (inner.error.isError())
			return inner;
		if (currentToken_->type() != Token::Type::RPAREN)
			return ParserResult{ Error(ErrorCode::RPAREN_EXPECTED, pos_), nullptr };
		result.node = inner.node;
		advance();
	} else {
		result.error = Error(ErrorCode::ILLEGAL_TOKEN, Position(pos_));
	}
	return result;

}





std::string ParserResult::toString() const {
	std::string out;
	if (error.isError()) {
		out = ErrorMessageGenerator::fromError(error);
	} else {
		out = node->toString();
	}
	return out;
}
}
