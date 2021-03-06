#pragma once

#include "Core/Error.h"
#include "Parser/Nodes.h"
#include "Instruction.h"
#include "Variable.h"
#include "Register.h"
#include "Context.h"
#include "Function.h"
#include "InstructionFactory.h"
#include "MemoryManager.h"

namespace fenge {

class CompilerResult {
public:
	Error error;
	std::vector<Instruction*> instructions;
	BYTE actualTarget = Register::ZERO;

	CompilerResult(std::vector<Instruction*> instructions, CBYTE actualTarget)
		: instructions(instructions), actualTarget(actualTarget) { };
	CompilerResult(Error error) : error(error) { };
	CompilerResult() : error(Error()) { };

	static CompilerResult generateError() {
		return generateError(ErrorCode::COMPILE_ERROR);
	}

	static CompilerResult generateError(ErrorCode errorCode) {
		return CompilerResult(Error(errorCode), std::vector<Instruction*>());
	}

	[[nodiscard]] std::string toString() const;
	[[nodiscard]] std::string toReadableString() const;
	[[nodiscard]] std::string toOutputString() const;
private:
	CompilerResult(Error error, std::vector<Instruction*> instructions) : error(error), instructions(instructions) { };
};


class Compiler {
public:
	Compiler();

	CompilerResult compile(const Node* node);
private:
	CompilerResult compile(const Node* node, CBYTE targetReg);

	CompilerResult visitStatementList(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitFuncDef(const FuncDefNode* node, CBYTE targetReg);
	CompilerResult visitParamList(const BinaryNode* node);
	CompilerResult visitParam(const ParameterNode* node);
	CompilerResult visitDefOrAssign(const AssignNode* node, CBYTE targetReg);
	CompilerResult visitVarDef(const AssignNode* node, CBYTE targetReg);
	CompilerResult visitVarAssign(const AssignNode* node, CBYTE targetReg);
	CompilerResult visitIf(const IfNode* node, CBYTE targetReg);
	CompilerResult visitWhile(const WhileNode* node, CBYTE targetReg);
	CompilerResult visitExtern(const ExternNode* node, CBYTE targetReg);
	CompilerResult visitFuncCall(const FuncCallNode* node);
	CompilerResult visitArgList(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitArg(const Node* node, CBYTE targetReg);
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
	CompilerResult visitUnaryMinus(const UnaryNode* node, CBYTE targetReg);
	CompilerResult visitUnaryNot(const UnaryNode* node, CBYTE targetReg);
	CompilerResult visitUnaryReturn(const UnaryNode* node);
	CompilerResult visitSimple(const LiteralNode* node, CBYTE targetReg);

	CompilerResult visitBinaryExprConvert(const BinaryNode* node, BYTE targetReg,
		const Instruction::Function func, CompilerResult(Compiler::* converter)(const Node*, CBYTE));
	CompilerResult visitBinaryExpr(const BinaryNode* node, CBYTE targetReg, const Instruction::Function func);
	CompilerResult visitBinaryLogExpr(const BinaryNode* node, CBYTE targetReg, const Instruction::Function func);

	CompilerResult compileBool(const Node* node, CBYTE targetReg);
	void convertToBoolIfNecessary(std::vector<Instruction*>& instructions, const Node* node, CBYTE targetReg) const;

	Function& findFunction(const std::string& name);

	void insertPushPopInstructions(CompilerResult& result, CompilerResult& statementRes);

	CBYTE getTarget(CBYTE targetReg);
	CBYTE getTargetStoreVar(CBYTE targetReg, std::vector<Instruction*>& instructions);
	CBYTE targetRegValid(CBYTE targetReg);

	CBYTE nextFreeGPReg() const;
	CBYTE nextFreeArgReg() const;
	CBYTE nextFreeableReg() const;
	bool isRegFree(CBYTE reg) const;
	CBYTE freeReg(CBYTE reg);
	CBYTE freeRegIfNoVar(CBYTE reg);
	CBYTE occupyReg(CBYTE reg);
	void freeArgs();
	void setRegVar(CBYTE reg, Variable* var);
	Instruction* movVar(CBYTE reg, Variable* var);
	void delRegVar(CBYTE reg);


	Context* globalContext_;
	Context* currContext_;
	MemoryManager memManager = MemoryManager();
	std::vector<Function> functions = std::vector<Function>();

};

}
