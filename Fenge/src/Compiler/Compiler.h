#pragma once

#include "Core/Error.h"
#include "Parser/Nodes.h"
#include "Instruction.h"

namespace fenge {

class CompilerResult {
public:
	Error error{ ErrorCode::NO_ERROR };
	std::vector<Instruction> instructions;

	[[nodiscard]] std::string toString() const;
};

class Compiler {
public:
	explicit Compiler(const Node* node);

	CompilerResult compile();
private:


	CompilerResult visitAddExpr();
	CompilerResult visitMulExpr();
	CompilerResult visitUnaryExpr();
	CompilerResult visitSimpleExpr();
};

}
