#pragma once

#include "Core/Error.h"
#include "Parser/Nodes.h"
#include "Instruction.h"
#include "Variable.h"
#include "Register.h"
#include "Context.h"
#include "Function.h"
#include "InstructionFactory.h"

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
private:
	CompilerResult(Error error, std::vector<Instruction*> instructions) : error(error), instructions(instructions) { };
};


class MemoryManager{
public:
	static CADDR STACK_SIZE = 0x100;

	std::vector<Instruction*> push(CBYTE reg) const {
		return std::vector<Instruction*>({
			InstructionFactory::ST(Register::SP, reg, 0x0000),
			InstructionFactory::LI(Register::ZERO, 0x0001),
			InstructionFactory::REG(Instruction::Function::ADD, Register::SP, Register::SP, Register::IM)
			});
	}

	std::vector<Instruction*> pop(CBYTE reg) const {
		return std::vector<Instruction*>({
			InstructionFactory::LD(reg, Register::SP, 0x0000),
			InstructionFactory::LI(Register::ZERO, 0x0001),
			InstructionFactory::REG(Instruction::Function::SUB, Register::SP, Register::SP, Register::IM)
			});
	}

	std::vector<Instruction*> pushForContext(Context* context) const {
		auto out = std::vector<Instruction*>();
		BYTE count = 0;
		for (BYTE reg = Register::GP_MIN; reg <= Register::GP_MAX; reg++) {
			if (context->registers[reg].isGenerallyUsed) {
				count++;
				out.push_back(InstructionFactory::ST(Register::SP, reg, count));
			}
		}
		if (count > 0) {
			out.push_back(InstructionFactory::LI(Register::ZERO, count));
			out.push_back(InstructionFactory::REG(Instruction::Function::ADD, Register::SP, Register::SP, Register::IM));
		}
		return out;
	}

	std::vector<Instruction*> popForContext(Context* context) const {
		auto out = std::vector<Instruction*>();
		BYTE count = 0;
		for (BYTE reg = Register::GP_MAX; reg >= Register::GP_MIN; reg--) {
			if (context->registers[reg].isGenerallyUsed) {
				out.push_back(InstructionFactory::LD(reg, Register::SP, -count));
				count++;
			}
		}
		if (count > 0) {
			out.push_back(InstructionFactory::LI(Register::ZERO, count));
			out.push_back(InstructionFactory::REG(Instruction::Function::SUB, Register::SP, Register::SP, Register::IM));
		}
		return out;
	}

	ADDR addrPointer = STACK_SIZE;
	CADDR malloc() {
		return ++addrPointer;
	}
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
	CompilerResult visitVarDef(const AssignNode* node, CBYTE targetReg);
	CompilerResult visitDefOrAssign(const AssignNode* node, CBYTE targetReg);
	CompilerResult visitVarAssign(const AssignNode* node, CBYTE targetReg);
	CompilerResult visitAddrAssign(const AssignNode* node, CBYTE targetReg);

	CompilerResult visitStatementList(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitFuncDef(const FuncDefNode* node, CBYTE targetReg);
	CompilerResult visitParamList(const BinaryNode* node);
	CompilerResult visitParam(const ParameterNode* node);
	CompilerResult visitAssign(const AssignNode* node, CBYTE targetReg);
	CompilerResult visitIf(const IfNode* node, CBYTE targetReg);
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
	CompilerResult visitUnary(const UnaryNode* node, CBYTE targetReg);
	CompilerResult visitFuncCall(const FuncCallNode* node);
	CompilerResult visitArgList(const BinaryNode* node, CBYTE targetReg);
	CompilerResult visitSimple(const LiteralNode* node, CBYTE targetReg);

	void convertToBoolIfNecessary(std::vector<Instruction*>& instructions, const Node* node, CBYTE targetReg) const;

	Context globalContext_;
	Context* currContext_;

	MemoryManager memManager = MemoryManager();

	std::vector<Function> functions = std::vector<Function>();
	Function& findFunction(const std::string& name);

	CBYTE targetRegOrNextFreeGP(CBYTE targetReg);
	CBYTE targetRegValid(CBYTE targetReg);

	bool isRegFree(CBYTE reg) const;
	CBYTE nextFreeGPReg() const;
	CBYTE nextFreeArgReg() const;
	CBYTE freeReg(CBYTE reg);
	CBYTE freeParentReg(CBYTE reg);
	void freeArgs();
	CBYTE occupyReg(CBYTE reg);
	void setRegVar(CBYTE reg, Variable* var);
	Instruction* movVar(CBYTE reg, Variable* var);
	void delRegVar(CBYTE reg);
	bool isGPReg(CBYTE reg) const { return reg >= Register::GP_MIN && reg <= Register::GP_MAX; };
};

}
