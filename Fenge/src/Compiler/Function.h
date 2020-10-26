#pragma once

namespace fenge {

class Context;

class Function {
public:
	Function(std::string name, Token::Keyword returnType, Context context, std::vector<Instruction> instructions)
		: name(name), returnType(returnType), context(context), instructions(instructions) { }
	Function(std::string name, Token::Keyword returnType, Context context, std::vector<Instruction> instructions, CADDR addr)
		: name(name), returnType(returnType), context(context), instructions(instructions), addr(addr) { }

	static Function notFound() {
		return Function("", Token::Keyword::NO_KEYWORD, nullptr, std::vector<Instruction>());
	}

	std::string name;
	Token::Keyword returnType;
	Context context;
	CADDR addr = 0;
	std::vector<Instruction> instructions;
	std::vector<Instruction> calledFrom;
};

}

