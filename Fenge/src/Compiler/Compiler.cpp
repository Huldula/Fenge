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
		if (Token::isLogOrType(binaryNode->op->type())) {
			return visitLogOr(binaryNode, targetReg);
		} else if (Token::isLogXorType(binaryNode->op->type())) {
			return visitLogXor(binaryNode, targetReg);
		} else if (Token::isLogAndType(binaryNode->op->type())) {
			return visitLogAnd(binaryNode, targetReg);
		} else if (Token::isCompEqType(binaryNode->op->type())) {
			return visitCompEq(binaryNode, targetReg);
		} else if(Token::isCompRelaType(binaryNode->op->type())) {
			return visitCompRela(binaryNode, targetReg);
		} else if (Token::isAddType(binaryNode->op->type())) {
			return visitMathAdd(binaryNode, targetReg);
		} else if (Token::isMulType(binaryNode->op->type())) {
			return visitMathMul(binaryNode, targetReg);
		} else if (Token::isBitOrType(binaryNode->op->type())) {
			return visitBitOr(binaryNode, targetReg);
		} else if (Token::isBitXorType(binaryNode->op->type())) {
			return visitBitXor(binaryNode, targetReg);
		} else if (Token::isBitAndType(binaryNode->op->type())) {
			return visitBitAnd(binaryNode, targetReg);
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

CompilerResult Compiler::compileBool(const Node* node, BYTE targetReg) {
	CompilerResult result = compile(node, targetReg);
	convertToBoolIfNecessary(result.instructions, node, targetReg);
	return result;
}


CompilerResult Compiler::visitBinaryExprConvert(
	const BinaryNode* node,
	BYTE targetReg,
	Instruction::Function func,
	CompilerResult(Compiler::* converter)(const Node*, BYTE)
) {
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
	CompilerResult left = (this->*converter)(first, reg0);

	// occupy targetReg for inner nodes
	const BYTE reg1 = occupyReg(nextFreeGPReg());
	CompilerResult right = (this->*converter)(second, reg1);
	freeReg(reg1);

	// free after compiling second, so second does not override
	if (!isGPReg(targetReg))
		freeReg(reg0);

	left.instructions.insert(left.instructions.end(), right.instructions.begin(), right.instructions.end());
	left.instructions.push_back(InstructionFactory::REG(func, targetReg, reg0, reg1));

	return left;
}

CompilerResult Compiler::visitBinaryExpr(const BinaryNode* node, BYTE targetReg, const Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compile);
}


CompilerResult Compiler::visitLogExpr(const BinaryNode* node, BYTE targetReg, Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compileBool);
}


CompilerResult Compiler::visitLogOr(const BinaryNode* node, BYTE targetReg) {
	return visitLogExpr(node, targetReg, Instruction::Function::OR);
}


CompilerResult Compiler::visitLogXor(const BinaryNode* node, BYTE targetReg) {
	return visitLogExpr(node, targetReg, Instruction::Function::XOR);
}


CompilerResult Compiler::visitLogAnd(const BinaryNode* node, BYTE targetReg) {
	return visitLogExpr(node, targetReg, Instruction::Function::AND);
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


CompilerResult Compiler::visitBitOr(const BinaryNode* node, BYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::OR);
}


CompilerResult Compiler::visitBitXor(const BinaryNode* node, BYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::XOR);
}


CompilerResult Compiler::visitBitAnd(const BinaryNode* node, BYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::AND);
}


CompilerResult Compiler::visitUnaryExpr(const UnaryNode* node, BYTE targetReg) {
	if (node->op->type() == Token::Type::PLUS)
		return compile(node->node, targetReg);

	const BYTE reg0 = isGPReg(targetReg) ? targetReg : occupyReg(nextFreeGPReg());
	if (node->op->type() == Token::Type::MINUS) {
		CompilerResult inner = compile(node->node, reg0);
		inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::SUB, targetReg, 0x0, reg0));
		return inner;
	} else if (node->op->type() == Token::Type::LOG_NOT || node->op->type() == Token::Type::BIT_NOT) {
		CompilerResult inner = node->op->type() == Token::Type::LOG_NOT
			? compileBool(node->node, reg0)
			: compile(node->node, reg0);
		inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::NOT, targetReg, 0x0, reg0));
		return inner;
	} else {
		return CompilerResult::generateError();
	}
}


CompilerResult Compiler::visitIntLiteralExpr(const LiteralNode* node, BYTE targetReg) {
	auto instructions = std::vector<Instruction>({
		InstructionFactory::LI(targetReg, *(int*)node->token->value())
		});
	return CompilerResult(instructions);
}



void Compiler::convertToBoolIfNecessary(std::vector<Instruction>& instructions, const Node* node, BYTE targetReg) const {
	bool isBool = false;
	if (node->type() == Node::Type::BINARY) {
		if (Token::isLogType(((BinaryNode*)node)->op->type()))
			isBool = true;
	} else if (node->type() == Node::Type::UNARY) {
		if (Token::isLogType(((UnaryNode*)node)->op->type()))
			isBool = true;
	}
	if (!isBool)
		instructions.push_back(InstructionFactory::REG(Instruction::Function::GR, targetReg, targetReg, 0x0));
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
