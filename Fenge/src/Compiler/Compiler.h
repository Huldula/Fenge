#pragma once

#include "Core/Error.h"
#include "Parser/Nodes.h"
#include "Instruction.h"

namespace fenge {

class CompilerResult {
public:
	Error error;
	std::vector<Instruction> instructions;

	CompilerResult(std::vector<Instruction> instructions) : instructions(instructions) { };
	CompilerResult(Error error, std::vector<Instruction> instructions) : error(error), instructions(instructions) { };

	static CompilerResult generateError() {
		return CompilerResult(Error(), std::vector<Instruction>());
	}

	static CompilerResult generateError(ErrorCode errorCode) {
		return CompilerResult(Error(errorCode), std::vector<Instruction>());
	}

	[[nodiscard]] std::string toString() const;
};

class Compiler {
public:
	Compiler();

	CompilerResult compile(const Node* node, BYTE targetReg);
private:

	CompilerResult visitBinaryExpr(const BinaryNode* node, BYTE targetReg, const Instruction::Function func);
	CompilerResult visitAddExpr(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitMulExpr(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitUnaryExpr(const UnaryNode* node, BYTE targetReg);
	CompilerResult visitIntLiteralExpr(const LiteralNode* node, BYTE targetReg);

	bool registers[16];
	size_t nextFreeGPReg() const;
	const size_t freeReg(const size_t reg);
	const size_t occupyReg(const size_t reg);
	bool isGPReg(BYTE reg) const { return reg > 0x7; };
};

}
