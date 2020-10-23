#pragma once

namespace fenge {

class Context;

class Function {
public:
	Function(std::string name, Token::Keyword returnType, Context* context)
		: name(name), returnType(returnType), context(context) { }
	Function(std::string name, Token::Keyword returnType, Context* context, CADDR addr)
		: name(name), returnType(returnType), context(context), addr(addr) { }

	std::string name;
	Token::Keyword returnType;
	Context* context;
	std::vector<Variable> varbiables;
	CADDR addr = 0;
};

}

