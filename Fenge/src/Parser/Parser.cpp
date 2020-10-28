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

Token* Parser::goBackTo(int pos) {
	pos_ = pos;
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
	} else if (Token::isSemicolonType(currentToken_->type())) {
		return parseEmpty();
	} else {
		return parseAssign();
	}
}

ParserResult Parser::parseEmpty() {
	return ParserResult{ Error(), (Node*)new EmptyNode() };
}

ParserResult Parser::parseFuncDef() {
	Token* datatype = currentToken_;
	advance();
	Token* identifier = currentToken_;
	advance();
	ParserResult paramList = parseParamList();
	if (paramList.error.isError())
		return paramList;
	ParserResult content = parseStatement();
	if (content.error.isError())
		return content;
	return ParserResult{ Error(), (Node*)new FuncDefNode(datatype, identifier, paramList.node, content.node) };
}

ParserResult Parser::parseParamList() {
	if (currentToken_->type() != Token::Type::LPAREN)
		return ParserResult{ Error(ErrorCode::ILLEGAL_TOKEN) };
	advance();

	ParserResult result;
	if (currentToken_->type() != Token::Type::RPAREN)
		result = parseBinary(&Parser::parseParam, Token::isCommaType);

	if (currentToken_->type() != Token::Type::RPAREN)
		return ParserResult{ Error(ErrorCode::ILLEGAL_TOKEN) };
	advance();

	return result;
}

ParserResult Parser::parseParam() {
	Token* datatype = currentToken_;
	advance();
	Token* identifier = currentToken_;
	advance();
	return ParserResult{ Error(), (Node*)new ParameterNode(datatype, identifier) };
}

ParserResult Parser::parseBlock() {
	advance();
	ParserResult result = parseStatementList();
	advance();
	return result;
}

ParserResult Parser::parseAssign(Token* datatype) {
	int startPos = pos_;
	ParserResult leftSide = parseLeftSide();
	if (leftSide.error.isError() || currentToken_->type() != Token::Type::EQ_ASSIGN) {
		delete leftSide.node;
		goBackTo(startPos);
		return parseLogOr();
	} else {
		advance(); // EQ_ASSIGN
		ParserResult right = parseAssign();
		if (right.error.isError())
			return right;
		return ParserResult{ Error(), (Node*)new AssignNode(datatype, leftSide.node, right.node) };
	}
}

ParserResult Parser::parseVarDef() {
	if (currentToken_->type() != Token::Type::KEYWORD)
		return ParserResult{ Error(ErrorCode::ILLEGAL_TOKEN) };
	Token* datatype = currentToken_;
	advance();
	return parseAssign(datatype);
}

ParserResult Parser::parseAssign() {
	return parseAssign(nullptr);
}

ParserResult Parser::parseReturn() {
	return parseUnary();
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
	if (Token::isUnaryType(currentToken_)) {
		Token* op = currentToken_;
		advance();
		ParserResult inner = parseUnary();
		if (inner.error.isError())
			return inner;
		return ParserResult{ Error(), (Node*)new UnaryNode(op, inner.node) };
	} else {
		return parseLeftSide();
	}
}

ParserResult Parser::parseLeftSide() {
	ParserResult left = parseSimple();
	if (left.error.isError())
		return left;

	while (currentToken_->type() == Token::Type::LPAREN) {
		ParserResult right = parseArgList();
		left = ParserResult{ Error(), (Node*)new FuncCallNode(left.node, right.node) };
	}
	return left;
}

ParserResult Parser::parseArgList() {
	if (currentToken_->type() != Token::Type::LPAREN)
		return ParserResult{ Error(ErrorCode::ILLEGAL_TOKEN) };
	advance();

	ParserResult result;
	if (currentToken_->type() != Token::Type::RPAREN)
		result = parseBinary(&Parser::parseAssign, Token::isCommaType);

	if (currentToken_->type() != Token::Type::RPAREN)
		return ParserResult{ Error(ErrorCode::ILLEGAL_TOKEN) };
	advance();

	return result;
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
