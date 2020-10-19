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

	CompilerResult visitBinaryExprConvert(const BinaryNode* node, BYTE targetReg,
		const Instruction::Function func, CompilerResult (Compiler::* converter)(const Node*, BYTE));
	CompilerResult visitBinaryExpr(const BinaryNode* node, BYTE targetReg, const Instruction::Function func);
	CompilerResult visitLogExpr(const BinaryNode* node, BYTE targetReg, const Instruction::Function func);
	CompilerResult compileBool(const Node* node, BYTE targetReg);

	CompilerResult visitLogOr(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitLogXor(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitLogAnd(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitCompEq(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitCompRela(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitBitOr(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitBitXor(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitBitAnd(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitBitShift(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitMathAdd(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitMathMul(const BinaryNode* node, BYTE targetReg);
	CompilerResult visitUnaryExpr(const UnaryNode* node, BYTE targetReg);
	CompilerResult visitIntLiteralExpr(const LiteralNode* node, BYTE targetReg);

	void convertToBoolIfNecessary(std::vector<Instruction>& instructions, const Node* node, BYTE targetReg) const;

	bool registers[16];
	size_t nextFreeGPReg() const;
	const size_t freeReg(const size_t reg);
	const size_t occupyReg(const size_t reg);
	bool isGPReg(BYTE reg) const { return reg > 0x7; };
};

}
