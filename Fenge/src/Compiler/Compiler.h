#pragma once

#include "Core/Error.h"
#include "Parser/Nodes.h"
#include "Instruction.h"
#include "Variable.h"
#include "Register.h"
#include "Context.h"
#include "Function.h"

namespace fenge {

class CompilerResult {
public:
	Error error;
	std::vector<Instruction> instructions;
	BYTE actualTarget = Register::ZERO;

	CompilerResult(std::vector<Instruction> instructions, CBYTE actualTarget)
		: instructions(instructions), actualTarget(actualTarget) { };
	CompilerResult(Error error) : error(error) { };
	CompilerResult() : error(Error()) { };

	static CompilerResult generateError() {
		return generateError(ErrorCode::COMPILE_ERROR);
	}

	static CompilerResult generateError(ErrorCode errorCode) {
		return CompilerResult(Error(errorCode), std::vector<Instruction>());
	}

	[[nodiscard]] std::string toString() const;
private:
	CompilerResult(Error error, std::vector<Instruction> instructions) : error(error), instructions(instructions) { };
};

class Compiler {
public:
	Compiler();

	CompilerResult compile(const Node* node);
	CompilerResult compile(const Node* node, CBYTE targetReg);
private:

	CompilerResult visitBinaryExprConvert(const BinaryNode* node, BYTE targetReg,
		const Instruction::Function func, CompilerResult (Compiler::* converter)(const Node*, CBYTE));
	CompilerResult visitBinaryExpr(const BinaryNode* node, CBYTE targetReg, const Instruction::Function func);
	CompilerResult visitLogExpr(const BinaryNode* node, CBYTE targetReg, const Instruction::Function func);
	CompilerResult compileBool(const Node* node, CBYTE targetReg);
	CompilerResult visitVarDef(const VarAssignNode* node, CBYTE targetReg);
	CompilerResult visitVarAss(const VarAssignNode* node, CBYTE targetReg);

	CompilerResult visitStatementList(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitFuncDef(const FuncDefNode* node, CBYTE targetReg);
	CompilerResult visitParamList(const BinaryNode* node);
	CompilerResult visitParam(const ParameterNode* node);
	//CompilerResult visitReturn(const UnaryNode* node, CBYTE targetReg);
	CompilerResult visitAssign(const VarAssignNode* node, CBYTE targetReg);
	CompilerResult visitLogOr(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitLogXor(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitLogAnd(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitCompEq(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitCompRela(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitBitOr(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitBitXor(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitBitAnd(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitBitShift(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitMathAdd(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitMathMul(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitUnaryExpr(const UnaryNode* node, CBYTE targetReg);
	CompilerResult visitSimple(const LiteralNode* node, CBYTE targetReg);

	void convertToBoolIfNecessary(std::vector<Instruction>& instructions, const Node* node, CBYTE targetReg) const;
	CBYTE targetRegOrNextFree(CBYTE targetReg);

	ADDR addrPointer = 0x0001;
	Context globalContext_;
	Context* currContext_;

	std::vector<Function> functions = std::vector<Function>();
	Function findFunction(const std::string& name) const;

	bool isRegFree(CBYTE reg) const;
	CBYTE nextFreeGPReg() const;
	CBYTE nextFreeArgReg() const;
	CBYTE freeReg(CBYTE reg);
	CBYTE occupyReg(CBYTE reg);
	void setRegVar(CBYTE reg, Variable* var);
	Instruction movVar(CBYTE reg, Variable* var);
	void delRegVar(CBYTE reg);
	bool isGPReg(CBYTE reg) const { return reg >= Register::GP_MIN && reg <= Register::GP_MAX; };

	CADDR occupyNextAddr();
};

}
