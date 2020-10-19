#include "fgpch.h"

#include "Compiler.h"
#include "InstructionFactory.h"

namespace fenge {


Compiler::Compiler() {
	for (size_t reg = 0; reg <= 0xF; reg++) {
		registers[reg] = false;
	}
}



CompilerResult Compiler::compile(const Node* node, BYTE targetReg) {
	if (node == nullptr)
		return CompilerResult::generateError();
	switch (node->type()) {
	case Node::Type::BINARY:
		#define binaryNode ((BinaryNode*)node)
		if (Token::isOrType(binaryNode->op->type())) {
			return visitLogicOr(binaryNode, targetReg);
		} else if (Token::isAndType(binaryNode->op->type())) {
			return visitLogicAnd(binaryNode, targetReg);
		} else if (Token::isCompEqType(binaryNode->op->type())) {
			return visitCompEq(binaryNode, targetReg);
		} else if(Token::isCompRelaType(binaryNode->op->type())) {
			return visitCompRela(binaryNode, targetReg);
		} else if (Token::isAddType(binaryNode->op->type())) {
			return visitMathAdd(binaryNode, targetReg);
		} else if (Token::isMulType(binaryNode->op->type())) {
			return visitMathMul(binaryNode, targetReg);
		} else {
			return CompilerResult::generateError();
		}
	case Node::Type::UNARY:
		return visitUnaryExpr((UnaryNode*)node, targetReg);
	case Node::Type::LITERAL:
		return visitIntLiteralExpr((LiteralNode*)node, targetReg);
	}
	return CompilerResult::generateError();
}


CompilerResult Compiler::visitBinaryExpr(const BinaryNode* node, BYTE targetReg, Instruction::Function func) {
	Node* first;
	Node* second;
	// doing literal last requires less registers
	if (node->left->type() == Node::Type::LITERAL) {
		first = node->right;
		second = node->left;
	} else {
		first = node->left;
		second = node->right;
	}

	const BYTE reg0 = isGPReg(targetReg) ? targetReg : occupyReg(nextFreeGPReg());
	CompilerResult left = compile(first, reg0);

	// occupy targetReg for inner nodes
	const BYTE reg1 = occupyReg(nextFreeGPReg());
	CompilerResult right = compile(second, reg1);
	freeReg(reg1);

	// free after compiling second, so second does not override
	if (!isGPReg(targetReg))
		freeReg(reg0);

	left.instructions.insert(left.instructions.end(), right.instructions.begin(), right.instructions.end());
	left.instructions.push_back(InstructionFactory::REG(func, targetReg, reg0, reg1));

	return left;
}


CompilerResult Compiler::visitLogicOr(const BinaryNode* node, BYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::OR);
}


CompilerResult Compiler::visitLogicAnd(const BinaryNode* node, BYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::AND);
}


CompilerResult Compiler::visitCompEq(const BinaryNode* node, BYTE targetReg) {
	Instruction::Function func;
	switch (node->op->type())
	{
	case Token::Type::EE:
		func = Instruction::Function::EQ;
		break;
	case Token::Type::NE:
		func = Instruction::Function::NEQ;
		break;
	default:
		return CompilerResult::generateError();
	}
	return visitBinaryExpr(node, targetReg, func);
}


CompilerResult Compiler::visitCompRela(const BinaryNode* node, BYTE targetReg) {
	Instruction::Function func;
	switch (node->op->type())
	{
	case Token::Type::LT:
		func = Instruction::Function::LS;
		break;
	case Token::Type::LTE:
		func = Instruction::Function::LSEQ;
		break;
	case Token::Type::GT:
		func = Instruction::Function::GR;
		break;
	case Token::Type::GTE:
		func = Instruction::Function::GREQ;
		break;
	default:
		return CompilerResult::generateError();
	}
	return visitBinaryExpr(node, targetReg, func);
}


CompilerResult Compiler::visitMathAdd(const BinaryNode* node, BYTE targetReg) {
	Instruction::Function func;
	switch (node->op->type())
	{
	case Token::Type::PLUS:
		func = Instruction::Function::ADD;
		break;
	case Token::Type::MINUS:
		func = Instruction::Function::SUB;
		break;
	default:
		return CompilerResult::generateError();
	}
	return visitBinaryExpr(node, targetReg, func);
}


CompilerResult Compiler::visitMathMul(const BinaryNode* node, BYTE targetReg) {
	Instruction::Function func;
	switch (node->op->type())
	{
	case Token::Type::MUL:
		func = Instruction::Function::MUL;
		break;
	case Token::Type::DIV:
		func = Instruction::Function::DIV;
		break;
	case Token::Type::MOD:
		func = Instruction::Function::MOD;
		break;
	default:
		return CompilerResult::generateError();
	}
	return visitBinaryExpr(node, targetReg, func);
}


CompilerResult Compiler::visitUnaryExpr(const UnaryNode* node, BYTE targetReg) {
	if (node->op->type() == Token::Type::MINUS) {
		CompilerResult inner = compile(node->node, targetReg);
		inner.instructions.push_back(InstructionFactory::REG(
			Instruction::Function::SUB,
			targetReg,
			0x0,
			targetReg
			));
		return inner;
	} else if (node->op->type() == Token::Type::PLUS) {
		return compile(node->node, targetReg);
	} else {
		return CompilerResult::generateError();
	}
}


CompilerResult Compiler::visitIntLiteralExpr(const LiteralNode* node, BYTE targetReg) {
	//freeReg(targetReg);
	auto instructions = std::vector<Instruction>({
		InstructionFactory::LI(targetReg, *(int*)node->token->value())
		});
	return CompilerResult(instructions);
}




size_t Compiler::nextFreeGPReg() const {
	for (size_t reg = 0x8; reg <= 0xF; reg++) {
		if (!registers[reg])
			return reg;
	}
	return 0;
}

const size_t Compiler::freeReg(const size_t reg) {
	registers[reg] = false;
	return reg;
}

const size_t Compiler::occupyReg(const size_t reg) {
	registers[reg] = true;
	return reg;
}





std::string CompilerResult::toString() const
{
	return std::string();
}
}
