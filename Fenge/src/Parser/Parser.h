#pragma once

#include "Core/Error.h"
#include "Token.h"
#include "Nodes.h"

namespace fenge {

class ParserResult {
	Error error{ ErrorCode::NO_ERROR };
	Node* node{ nullptr };

	[[nodiscard]] std::string toString() const;
};

class Parser
{
public:
	explicit Parser(const std::vector<Token*>& input);
	~Parser();

	ParserResult* parse();
	ParserResult* parseLiteral();
	ParserResult* parseUnary();
	ParserResult* parseBinary();
private:
	int pos_;
	void advance();
	Token* currentToken_;
	const std::vector<Token*>& input_;
};

}
