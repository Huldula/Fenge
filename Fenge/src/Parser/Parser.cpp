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

ParserResult* Parser::parse() {
	ParserResult* result = new ParserResult();

	while (currentToken_->type() != Token::Type::EOS) {

	}
}

ParserResult* Parser::parseLiteral() {
	ParserResult* result = new ParserResult();
	if (Token::isLiteralType(currentToken_->type())) {
		result->node = (Node*)new LiteralNode(currentToken_);
		advance();
	} else {
		result->error = Error(ErrorCode::ILLEGAL_TOKEN, Position(pos_));
	}
	return result;

}

ParserResult* Parser::parseUnary() {
	ParserResult* result = new ParserResult();
	if (Token::isLiteralType(currentToken_->type())) {
		//result->node = (Node*)new UnaryNode(currentToken_);
	}
	else {
		result->error = Error(ErrorCode::ILLEGAL_TOKEN, Position(pos_));
	}
	return result;
}

ParserResult* Parser::parseAddExpr() {
	ParserResult* left = parseMulExpr();
	if (left->error.isError())
		return new ParserResult{ left->error, nullptr };
	ParserResult* right;
	while (Token::isAddType(currentToken_->type())) {
		Token* op = currentToken_;
		advance();
		right = parseMulExpr();
		if (right->error.isError())
			return new ParserResult{ right->error, nullptr };
		left->node = (Node*)new BinaryNode(left->node, op, right->node);
	}
	return new ParserResult{ ErrorCode::NO_ERROR, left->node };
}

ParserResult* Parser::parseMulExpr() {
	ParserResult* left = parseUnary();
	if (left->error.isError())
		return new ParserResult{ left->error, nullptr };
	ParserResult* right;
	while (Token::isMulType(currentToken_->type())) {
		Token* op = currentToken_;
		advance();
		right = parseUnary();
		if (right->error.isError())
			return new ParserResult{ right->error, nullptr };
		left->node = (Node*)new BinaryNode(left->node, op, right->node);
	}
	return new ParserResult{ ErrorCode::NO_ERROR, left->node };
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
