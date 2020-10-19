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

Token* Parser::peek() {
	return input_.at((size_t)pos_ + 1);
}

ParserResult Parser::parse() {
	return parseLogicOr();
}


ParserResult Parser::parseBinary(ParserResult (Parser::*toCall)(), bool isType(Token::Type)) {
	ParserResult left = (this->*toCall)();
	if (left.error.isError())
		return ParserResult{ left.error, nullptr };
	ParserResult right;
	while (isType(currentToken_->type())) {
		Token* op = currentToken_;
		advance();
		right = (this->*toCall)();
		if (right.error.isError())
			return ParserResult{ right.error, nullptr };
		left.node = (Node*)new BinaryNode(left.node, op, right.node);
	}
	return ParserResult{ Error(), left.node };
}





ParserResult Parser::parseLogicOr() {
	return parseBinary(&Parser::parseLogicAnd, Token::isOrType);
}

ParserResult Parser::parseLogicAnd() {
	return parseBinary(&Parser::parseCompEq, Token::isAndType);
}

ParserResult Parser::parseCompEq() {
	return parseBinary(&Parser::parseCompRela, Token::isCompEqType);
}

ParserResult Parser::parseCompRela() {
	return parseBinary(&Parser::parseMathAdd, Token::isCompRelaType);
}

ParserResult Parser::parseMathAdd() {
	return parseBinary(&Parser::parseMathMul, Token::isAddType);
}

ParserResult Parser::parseMathMul() {
	return parseBinary(&Parser::parseUnary, Token::isMulType);
}

ParserResult Parser::parseUnary() {
	if (Token::isAddType(currentToken_->type())) {
		Token* op = currentToken_;
		advance();
		ParserResult inner = parseUnary();
		if (inner.error.isError())
			return ParserResult{ inner.error, nullptr };
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
		ParserResult inner = parseMathAdd();
		if (inner.error.isError())
			return ParserResult{ inner.error, nullptr };
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
