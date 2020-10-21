#include "fgpch.h"

#include "Compiler.h"
#include "InstructionFactory.h"

namespace fenge {


Compiler::Compiler() {
	for (size_t reg = 0; reg <= 0xF; reg++) {
		registers_[reg] = Register();
	}
	globalContext_ = Context();
	currContext_ = &globalContext_;
}



CompilerResult Compiler::compile(const Node* node, CBYTE targetReg) {
	if (node == nullptr)
		return CompilerResult::generateError();
	switch (node->type()) {
	case Node::Type::BINARY:
		#define binaryNode ((BinaryNode*)node)
		if (Token::isSemicolonType(binaryNode->op->type())) {
			return visitStatementList(binaryNode, targetReg);
		} else if (Token::isLogOrType(binaryNode->op->type())) {
			return visitLogOr(binaryNode, targetReg);
		} else if (Token::isLogXorType(binaryNode->op->type())) {
			return visitLogXor(binaryNode, targetReg);
		} else if (Token::isLogAndType(binaryNode->op->type())) {
			return visitLogAnd(binaryNode, targetReg);
		} else if (Token::isCompEqType(binaryNode->op->type())) {
			return visitCompEq(binaryNode, targetReg);
		} else if(Token::isCompRelaType(binaryNode->op->type())) {
			return visitCompRela(binaryNode, targetReg);
		} else if (Token::isBitOrType(binaryNode->op->type())) {
			return visitBitOr(binaryNode, targetReg);
		} else if (Token::isBitXorType(binaryNode->op->type())) {
			return visitBitXor(binaryNode, targetReg);
		} else if (Token::isBitAndType(binaryNode->op->type())) {
			return visitBitAnd(binaryNode, targetReg);
		} else if (Token::isBitShiftType(binaryNode->op->type())) {
			return visitBitShift(binaryNode, targetReg);
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
		return visitSimple((LiteralNode*)node, targetReg);
	case Node::Type::VAR_ASSIGN:
		return visitAssign((VarAssignNode*)node, targetReg);
	}
	return CompilerResult::generateError();
}

CompilerResult Compiler::compileBool(const Node* node, CBYTE targetReg) {
	CompilerResult result = compile(node, targetReg);
	convertToBoolIfNecessary(result.instructions, node, targetReg);
	return result;
}


CompilerResult Compiler::visitBinaryExprConvert(
	const BinaryNode* node,
	CBYTE targetReg,
	Instruction::Function func,
	CompilerResult(Compiler::* converter)(const Node*, CBYTE)
) {
	Node* first;
	Node* second;
	// doing literal last requires less registers
	if (node->left->type() == Node::Type::LITERAL
		|| node->left->type() == Node::Type::VAR_ASSIGN
		&& ((VarAssignNode*)node->left)->right->type() == Node::Type::LITERAL) {
		first = node->right;
		second = node->left;
	} else {
		first = node->left;
		second = node->right;
	}

	CBYTE reg0 = targetRegOrNextFree(targetReg);
	CompilerResult firstResult = (this->*converter)(first, reg0);
	if (firstResult.error.isError())
		return firstResult;

	// occupy targetReg for inner nodes
	BYTE reg1 = occupyReg(nextFreeGPReg());
	CompilerResult secondResult = (this->*converter)(second, reg1);
	if (secondResult.error.isError())
		return secondResult;
	freeReg(reg1);

	// free after compiling second, so second does not override
	if (!isGPReg(targetReg))
		freeReg(reg0);

	firstResult.instructions.insert(firstResult.instructions.end(),
		secondResult.instructions.begin(), secondResult.instructions.end());
	firstResult.instructions.push_back(InstructionFactory::REG(func, targetReg, reg0, reg1));

	return firstResult;
}

CompilerResult Compiler::visitBinaryExpr(const BinaryNode* node, CBYTE targetReg, const Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compile);
}

CompilerResult Compiler::visitLogExpr(const BinaryNode* node, CBYTE targetReg, Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compileBool);
}

CompilerResult Compiler::visitVarDef(const VarAssignNode* node, CBYTE targetReg) {
	const std::string& name = node->name();

	if (currContext_->findVariableInContext(name).datatype != Token::Keyword::NO_KEYWORD)
		return CompilerResult::generateError(ErrorCode::VAR_ALREADY_EXISTS);
	CBYTE reg0 = targetRegOrNextFree(targetReg);
	CompilerResult inner = compile(node->right, reg0);
	if (inner.error.isError())
		return inner;

	CADDR addr0 = occupyNextAddr();
	Variable var = Variable(name, node->datatype(), reg0, addr0);
	currContext_->variables.push_back(var);

	inner.instructions.push_back(
		InstructionFactory::ST(Register::ZERO, var.reg, var.addr)
	);
	setRegVar(var.reg, &var);
	return inner;
}

CompilerResult Compiler::visitVarAss(const VarAssignNode* node, BYTE targetReg) {
	const std::string& name = node->name();

	Variable var = currContext_->findVariable(name);
	if (var.datatype == Token::Keyword::NO_KEYWORD)
		return CompilerResult::generateError(ErrorCode::VAR_NOT_FOUND);
	CBYTE reg0 = var.reg ? var.reg : targetRegOrNextFree(targetReg);
	var.reg = reg0;

	CompilerResult inner = compile(node->right, reg0);
	if (inner.error.isError())
		return inner;
	inner.instructions.push_back(
		InstructionFactory::ST(Register::ZERO, var.reg, var.addr)
	);

	setRegVar(var.reg, &var);
	if (var.reg != targetReg) {
		inner.actualTarget = var.reg;
	}
	return inner;
}






CompilerResult Compiler::visitStatementList(const BinaryNode* node, CBYTE targetReg) {
	CompilerResult left = compile(node->left, targetReg);
	if (left.error.isError())
		return left;
	CompilerResult right = compile(node->right, targetReg);
	if (right.error.isError())
		return right;
	left.instructions.insert(left.instructions.end(), right.instructions.begin(), right.instructions.end());
	return left;
}

// optimizer: check if necessary loading to ram or just keep in reg
CompilerResult Compiler::visitAssign(const VarAssignNode* node, CBYTE targetReg) {
	if (node->dt) {
		return visitVarDef(node, targetReg);
	} else {
		return visitVarAss(node, targetReg);
	}
}


CompilerResult Compiler::visitLogOr(const BinaryNode* node, CBYTE targetReg) {
	return visitLogExpr(node, targetReg, Instruction::Function::OR);
}


CompilerResult Compiler::visitLogXor(const BinaryNode* node, CBYTE targetReg) {
	return visitLogExpr(node, targetReg, Instruction::Function::XOR);
}


CompilerResult Compiler::visitLogAnd(const BinaryNode* node, CBYTE targetReg) {
	return visitLogExpr(node, targetReg, Instruction::Function::AND);
}


CompilerResult Compiler::visitCompEq(const BinaryNode* node, CBYTE targetReg) {
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


CompilerResult Compiler::visitCompRela(const BinaryNode* node, CBYTE targetReg) {
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


CompilerResult Compiler::visitBitOr(const BinaryNode* node, CBYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::OR);
}


CompilerResult Compiler::visitBitXor(const BinaryNode* node, CBYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::XOR);
}


CompilerResult Compiler::visitBitAnd(const BinaryNode* node, CBYTE targetReg) {
	return visitBinaryExpr(node, targetReg, Instruction::Function::AND);
}

CompilerResult Compiler::visitBitShift(const BinaryNode* node, CBYTE targetReg) {
	Instruction::Function func;
	switch (node->op->type())
	{
	case Token::Type::SHL:
		func = Instruction::Function::SHL;
		break;
	case Token::Type::SHR:
		func = Instruction::Function::SHR;
		break;
	case Token::Type::ROTL:
		func = Instruction::Function::ROTL;
		break;
	case Token::Type::ROTR:
		func = Instruction::Function::ROTR;
		break;
	default:
		return CompilerResult::generateError();
	}
	return visitBinaryExpr(node, targetReg, func);
}


CompilerResult Compiler::visitMathAdd(const BinaryNode* node, CBYTE targetReg) {
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


CompilerResult Compiler::visitMathMul(const BinaryNode* node, CBYTE targetReg) {
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


CompilerResult Compiler::visitUnaryExpr(const UnaryNode* node, CBYTE targetReg) {
	if (node->op->type() == Token::Type::PLUS)
		return compile(node->node, targetReg);

	CBYTE reg0 = targetRegOrNextFree(targetReg);
	if (node->op->type() == Token::Type::MINUS) {
		CompilerResult inner = compile(node->node, reg0);
		if (inner.error.isError())
			return inner;
		inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::SUB, targetReg, Register::ZERO, reg0));
		return inner;
	} else if (node->op->type() == Token::Type::LOG_NOT || node->op->type() == Token::Type::BIT_NOT) {
		CompilerResult inner = node->op->type() == Token::Type::LOG_NOT
			? compileBool(node->node, reg0)
			: compile(node->node, reg0);
		if (inner.error.isError())
			return inner;
		inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::NOT, targetReg, Register::ZERO, reg0));
		return inner;
	} else {
		return CompilerResult::generateError();
	}
}


CompilerResult Compiler::visitSimple(const LiteralNode* node, CBYTE targetReg) {
	if (node->token->type() == Token::Type::IDENTIFIER) {
		Variable var = currContext_->findVariable(*(std::string*)node->token->value());
		if (var.datatype == Token::Keyword::NO_KEYWORD)
			return CompilerResult::generateError(ErrorCode::VAR_NOT_FOUND);
		if (var.reg) {
			return CompilerResult(std::vector<Instruction>(), var.reg);
		} else {
			return CompilerResult(std::vector<Instruction>({
				InstructionFactory::LD(targetReg, Register::ZERO, var.addr)
				}), targetReg);
		}
	} else {
		return CompilerResult(std::vector<Instruction>({
			InstructionFactory::LI(targetReg, *(int*)node->token->value())
			}), targetReg);
	}
}



void Compiler::convertToBoolIfNecessary(std::vector<Instruction>& instructions, const Node* node, CBYTE targetReg) const {
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

CBYTE Compiler::targetRegOrNextFree(CBYTE targetReg) {
	return isGPReg(targetReg) ? targetReg : occupyReg(nextFreeGPReg());
}


CBYTE Compiler::nextFreeGPReg() const {
	for (BYTE reg = Register::GP_MIN; reg <= Register::GP_MAX; reg++) {
		if (!registers_[reg].isOccupied)
			return reg;
	}
	return 0;
}

CBYTE Compiler::freeReg(CBYTE reg) {
	registers_[reg].isOccupied = false;
	return reg;
}

CBYTE Compiler::occupyReg(CBYTE reg) {
	registers_[reg].isOccupied = true;
	return reg;
}

CADDR Compiler::occupyNextAddr() {
	return ++addrPointer;
}

void Compiler::setRegVar(CBYTE reg, Variable* var) {
	registers_[reg].containedVar = var;
}

void Compiler::delRegVar(CBYTE reg) {
	registers_[reg].containedVar = nullptr;
}



std::string CompilerResult::toString() const
{
	if (error.isError()) {
		return ErrorMessageGenerator::fromError(error);
	} else {
		std::string out;
		for (auto instr : instructions) {
			out += instr.toHexString() + "\n";
		}
		return out;
	}
}
}
