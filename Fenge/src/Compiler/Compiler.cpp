#include "fgpch.h"

#include "Compiler.h"
#include "Linker.h"

namespace fenge {


Compiler::Compiler() {
	globalContext_ = Context();
	currContext_ = &globalContext_;
}


CompilerResult Compiler::compile(const Node* node) {
	CompilerResult res = compile(node, Register::ZERO);
	res.instructions.push_back(InstructionFactory::HLT());
	Linker::link(res.instructions, functions);
	return res;
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
		return visitUnary((UnaryNode*)node, targetReg);
	case Node::Type::LITERAL:
		return visitSimple((LiteralNode*)node, targetReg);
	case Node::Type::ASSIGN:
		return visitDefOrAssign((AssignNode*)node, targetReg);
	case Node::Type::FUNC_DEF:
		return visitFuncDef((FuncDefNode*)node, targetReg);
	case Node::Type::FUNC_CALL:
		return visitFuncCall((FuncCallNode*)node);
	case Node::Type::EMPTY:
		return CompilerResult();
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
	BYTE targetReg,
	Instruction::Function func,
	CompilerResult(Compiler::* converter)(const Node*, CBYTE)
) {
	Node* first;
	Node* second;
	bool leftFirst = true;
	// doing literal last requires less registers
	if (node->right->type() == Node::Type::LITERAL
		|| node->right->type() == Node::Type::ASSIGN
		&& ((AssignNode*)node->right)->right->type() == Node::Type::LITERAL) {
		first = node->left;
		second = node->right;
	} else {
		first = node->right;
		second = node->left;
		leftFirst = false;
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
	//if (!isGPReg(targetReg))
		//freeReg(reg0);

	firstResult.instructions.insert(firstResult.instructions.end(),
		secondResult.instructions.begin(), secondResult.instructions.end());
	if (leftFirst)
		firstResult.instructions.push_back(
			InstructionFactory::REG(func, reg0, firstResult.actualTarget, secondResult.actualTarget));
	else
		firstResult.instructions.push_back(
			InstructionFactory::REG(func, reg0, secondResult.actualTarget, firstResult.actualTarget));

	firstResult.actualTarget = reg0;
	return firstResult;
}

CompilerResult Compiler::visitBinaryExpr(const BinaryNode* node, CBYTE targetReg, const Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compile);
}

CompilerResult Compiler::visitLogExpr(const BinaryNode* node, CBYTE targetReg, Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compileBool);
}

CompilerResult Compiler::visitVarDef(const AssignNode* node, CBYTE targetReg) {
	const std::string& name = node->nameOfVar();

	if (currContext_->findVariableInContext(name)->datatype != Token::Keyword::NO_KEYWORD)
		return CompilerResult::generateError(ErrorCode::VAR_ALREADY_EXISTS);
	CBYTE reg0 = targetRegOrNextFree(targetReg);
	CompilerResult inner = compile(node->right, reg0);
	if (inner.error.isError())
		return inner;

	CADDR addr0 = memManager.malloc();
	Variable* var = new Variable(name, node->datatype(), inner.actualTarget, addr0);
	currContext_->variables.push_back(var);

	inner.instructions.push_back(
		InstructionFactory::ST(Register::ZERO, var->reg, var->addr)
	);
	setRegVar(var->reg, var);
	return inner;
}

CompilerResult Compiler::visitAssign(const AssignNode* node, CBYTE targetReg) {
	return visitVarAssign(node, targetReg);
}

CompilerResult Compiler::visitVarAssign(const AssignNode* node, CBYTE targetReg) {
	const std::string& name = node->nameOfVar();

	Variable* var = currContext_->findVariable(name);
	if (var->datatype == Token::Keyword::NO_KEYWORD)
		return CompilerResult::generateError(ErrorCode::VAR_NOT_FOUND);
	//CBYTE reg0 = var.reg ? var.reg : targetRegOrNextFree(targetReg);

	CompilerResult inner = compile(node->right, Register::ZERO);
	if (inner.error.isError())
		return inner;
	//Instruction::insertIfValid(inner.instructions, movVar(inner.actualTarget, &var));
	setRegVar(inner.actualTarget, var);
	inner.instructions.push_back(
		InstructionFactory::ST(Register::ZERO, var->reg, var->addr)
	);
	return inner;
}

CompilerResult Compiler::visitAddrAssign(const AssignNode* node, CBYTE targetReg) {
	return CompilerResult();
}






CompilerResult Compiler::visitStatementList(const BinaryNode* node, CBYTE targetReg) {
	CompilerResult left = compile(node->left, Register::ZERO);
	if (left.error.isError())
		return left;
	CompilerResult right = compile(node->right, Register::ZERO);
	if (right.error.isError())
		return right;
	left.instructions.insert(left.instructions.end(), right.instructions.begin(), right.instructions.end());
	return left;
}

CompilerResult Compiler::visitFuncDef(const FuncDefNode* node, CBYTE targetReg) {
	Token* returnType = node->dt;
	Token* identifier = node->id;

	Context context = Context(currContext_);
	currContext_ = &context;

	CompilerResult result = CompilerResult();
	if (node->paramList == nullptr) {
	} else if (node->paramList->type() == Node::Type::BINARY) {
		result = visitParamList((BinaryNode*)node->paramList);
	} else if (node->paramList->type() == Node::Type::PARAM) {
		result = visitParam((ParameterNode*)node->paramList);
	}
	if (result.error.isError())
		return result;

	CompilerResult statementRes = compile(node->statement, targetReg);
	if (statementRes.error.isError())
		return statementRes;

	std::vector<Instruction*> push = memManager.pushForContext(currContext_);
	std::vector<Instruction*> pop = memManager.popForContext(currContext_);
	result.instructions.insert(result.instructions.end(), push.begin(), push.end());
	result.instructions.insert(result.instructions.end(), statementRes.instructions.begin(), statementRes.instructions.end());
	result.instructions.insert(result.instructions.end(), pop.begin(), pop.end());
	result.instructions.push_back(InstructionFactory::RET());

	Function& olderDef = findFunction(*(std::string*)identifier->value());
	if (olderDef.exists()) {
		olderDef.instructions = result.instructions;
	} else {
		functions.push_back(
			Function(*(std::string*)identifier->value(), *(Token::Keyword*)returnType->value(), context, result.instructions)
		);
	}
	currContext_ = context.parent;

	return CompilerResult();
}

CompilerResult Compiler::visitParamList(const BinaryNode* node) {
	if (node->left->type() != Node::Type::PARAM)
		LOG("komisch");
	CompilerResult left = node->left->type() == Node::Type::PARAM
		? visitParam((ParameterNode*)node->left)
		: visitParamList((BinaryNode*)node->left);
	if (left.error.isError())
		return left;

	CompilerResult right = node->right->type() == Node::Type::PARAM
		? visitParam((ParameterNode*)node->right)
		: visitParamList((BinaryNode*)node->right);
	if (right.error.isError())
		return right;

	left.instructions.insert(left.instructions.end(), right.instructions.begin(), right.instructions.end());
	return left;
}

CompilerResult Compiler::visitParam(const ParameterNode* node) {
	const std::string& name = node->name();
	CADDR addr0 = memManager.malloc();
	Variable* var = new Variable(name, node->datatype(), nextFreeArgReg(), addr0);
	currContext_->variables.push_back(var);

	auto instructions = std::vector<Instruction*>({
		InstructionFactory::ST(Register::ZERO, var->reg, var->addr)
		});
	setRegVar(var->reg, var);
	return CompilerResult(instructions, var->reg);
}

// optimizer: check if necessary storing to ram or just keep in reg
CompilerResult Compiler::visitDefOrAssign(const AssignNode* node, CBYTE targetReg) {
	if (node->dt) {
		return visitVarDef(node, targetReg);
	} else {
		return visitAssign(node, targetReg);
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


CompilerResult Compiler::visitUnary(const UnaryNode* node, CBYTE targetReg) {
	if (node->op->type() == Token::Type::PLUS)
		return compile(node->node, targetReg);

	CBYTE reg0 = targetRegOrNextFree(targetReg);
	if (node->op->type() == Token::Type::MINUS) {
		CompilerResult inner = compile(node->node, reg0);
		if (inner.error.isError())
			return inner;
		inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::SUB, targetReg,
			Register::ZERO, inner.actualTarget));
		inner.actualTarget = targetReg;
		return inner;
	} else if (node->op->type() == Token::Type::LOG_NOT || node->op->type() == Token::Type::BIT_NOT) {
		CompilerResult inner = node->op->type() == Token::Type::LOG_NOT
			? compileBool(node->node, reg0)
			: compile(node->node, reg0);
		if (inner.error.isError())
			return inner;
		inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::NOT,
			targetReg, Register::ZERO, inner.actualTarget));
		inner.actualTarget = targetReg;
		return inner;
	} else if (Token::isReturnKeyword(node->op)) {
		return compile(node->node, Register::RET);
	} else {
		return CompilerResult::generateError();
	}
}


CompilerResult Compiler::visitFuncCall(const FuncCallNode* node) {
	CompilerResult result = CompilerResult();
	const std::string& name = node->nameOfVar();

	freeReg(Register::RET);
	if (node->argList == nullptr) {
	} else if (node->argList->type() == Node::Type::BINARY) {
		result = visitArgList((BinaryNode*)node->argList, Register::RET);
	} else {
		result = compile(node->argList, Register::RET);
	}
	if (result.error.isError())
		return result;

	result.actualTarget = Register::RET;
	Instruction* ins = InstructionFactory::CALL(Register::ZERO, 0 /* Linker will set this value after compilation */);
	result.instructions.push_back(ins);
	Function& func = findFunction(name);
	if (!func.exists())
		return CompilerResult::generateError(ErrorCode::FUNC_NOT_FOUND);
	func.calledFrom.push_back(ins);

	return result;
}


CompilerResult Compiler::visitArgList(const BinaryNode* node, CBYTE targetReg) {
	CompilerResult left = node->left->type() == Node::Type::ASSIGN
		? compile(node->left, targetReg)
		: visitArgList((BinaryNode*)node->left, targetReg);
	if (left.error.isError())
		return left;

	CBYTE reg0 = nextFreeArgReg();
	freeReg(reg0);
	CompilerResult right = node->right->type() == Node::Type::ASSIGN
		? compile(node->right, reg0)
		: visitArgList((BinaryNode*)node->right, reg0);
	if (right.error.isError())
		return right;

	left.instructions.insert(left.instructions.end(), right.instructions.begin(), right.instructions.end());
	return left;
}


CompilerResult Compiler::visitSimple(const LiteralNode* node, CBYTE targetReg) {
	if (node->token->type() == Token::Type::IDENTIFIER) {
		Variable* var = currContext_->findVariable(*(std::string*)node->token->value());
		if (var->datatype == Token::Keyword::NO_KEYWORD)
			return CompilerResult::generateError(ErrorCode::VAR_NOT_FOUND);
		if (var->reg) {
			return CompilerResult(std::vector<Instruction*>(), var->reg);
		} else {
			return CompilerResult(std::vector<Instruction*>({
				InstructionFactory::LD(targetReg, Register::ZERO, var->addr)
				}), targetReg);
		}
	} else {
		return CompilerResult(std::vector<Instruction*>({
			InstructionFactory::LI(targetReg, *(int*)node->token->value())
			}), targetReg);
	}
}



void Compiler::convertToBoolIfNecessary(std::vector<Instruction*>& instructions, const Node* node, CBYTE targetReg) const {
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


Function& Compiler::findFunction(const std::string& name) {
	for (Function& func : functions) {
		if (name.compare(func.name) == 0) {
			return func;
		}
	}
	return Function::notFound();
}


bool Compiler::isRegFree(CBYTE reg) const {
	return !currContext_->registers[reg].isOccupied;
}

CBYTE Compiler::nextFreeGPReg() const {
	for (BYTE reg = Register::GP_MIN; reg <= Register::GP_MAX; reg++) {
		if (isRegFree(reg))
			return reg;
	}
	return 0;
}

CBYTE Compiler::nextFreeArgReg() const {
	for (BYTE reg = Register::ARG_MIN; reg <= Register::ARG_MAX; reg++) {
		if (isRegFree(reg))
			return reg;
	}
	return 0;
}

CBYTE Compiler::freeReg(CBYTE reg) {
	currContext_->registers[reg].isOccupied = false;
	delRegVar(reg);
	return reg;
}

CBYTE Compiler::freeParentReg(CBYTE reg) {
	currContext_->parent->registers[reg].isOccupied = false;
	currContext_->parent->registers[reg].containedVar = nullptr;
	return reg;
}

void Compiler::freeArgs() {
	for (BYTE reg = Register::ARG_MIN; reg <= Register::ARG_MAX; reg++) {
		freeReg(reg);
	}
}

CBYTE Compiler::occupyReg(CBYTE reg) {
	currContext_->registers[reg].isOccupied = true;
	currContext_->registers[reg].isGenerallyUsed = true;
	return reg;
}

void Compiler::setRegVar(CBYTE reg, Variable* var) {
	freeReg(var->reg);
	occupyReg(reg);
	currContext_->registers[reg].containedVar = var;
	var->reg = reg;
}

Instruction* Compiler::movVar(CBYTE reg, Variable* var) {
	freeReg(var->reg);
	occupyReg(reg);
	currContext_->registers[reg].containedVar = var;
	if (var->reg == reg)
		return new Instruction(0);
	auto out = InstructionFactory::REG(Instruction::Function::MOV, reg, var->reg, var->reg);
	var->reg = reg;
	return out;
}

void Compiler::delRegVar(CBYTE reg) {
	if (currContext_->registers[reg].containedVar == nullptr)
		return;
	currContext_->registers[reg].containedVar->reg = Register::ZERO;
	currContext_->registers[reg].containedVar = nullptr;
}



std::string CompilerResult::toString() const
{
	if (error.isError()) {
		return ErrorMessageGenerator::fromError(error);
	} else {
		std::string out;
		for (const Instruction* instr : instructions) {
			out += instr->toHexString() + "\n";
		}
		return out;
	}
}

std::string CompilerResult::toReadableString() const
{
	if (error.isError()) {
		return ErrorMessageGenerator::fromError(error);
	} else {
		std::string out;
		for (const Instruction* instr : instructions) {
			out += instr->toString() + "\n";
		}
		return out;
	}
}
}
