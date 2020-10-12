#include "fgpch.h"
#include "Parser.h"


namespace fenge {

Parser::Parser(const std::vector<Token*>& input) : input_(input) {
	pos_ = -1;
	advance();
}


void Parser::advance() {
	pos_++;
	currentToken_ = input_.at(pos_);
}

ParserResult* Parser::parse() {
}

}
