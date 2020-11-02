#include "fgpch.h"

#include "Compiler.h"
#include "Linker.h"

namespace fenge {

#define RETURN_IF_ERROR(compilerResult) if (compilerResult.error.isError()) return compilerResult

Compiler::Compiler() {
	globalContext_ = new Context();
	currContext_ = globalContext_;
}


CompilerResult Compiler::compile(const Node* node) {
	CompilerResult res = compile(node, Register::CHOOSE);
	auto start = std::vector<Instruction*>({
		InstructionFactory::NOP(),
		InstructionFactory::LI(Register::SP, globalContext_->stackMemPointer)
		});
	INSERT_TO_BEGIN(res.instructions, start);
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
		#define unaryNode ((UnaryNode*)node)
		if (unaryNode->op->type() == Token::Type::PLUS) {
			return compile(unaryNode->node, targetReg);
		} else if (unaryNode->op->type() == Token::Type::MINUS) {
			return visitUnaryMinus(unaryNode, targetReg);
		} else if (Token::isNotType(unaryNode->op->type())) {
			return visitUnaryNot(unaryNode, targetReg);
		} else if (Token::isReturnKeyword(unaryNode->op)) {
			return visitUnaryReturn(unaryNode);
		} else {
			return CompilerResult::generateError();
		}
		break;
	case Node::Type::LITERAL:
		return visitSimple((LiteralNode*)node, targetReg);
	case Node::Type::ASSIGN:
		return visitDefOrAssign((AssignNode*)node, targetReg);
	case Node::Type::FUNC_DEF:
		return visitFuncDef((FuncDefNode*)node, targetReg);
	case Node::Type::FUNC_CALL:
		return visitFuncCall((FuncCallNode*)node);
	case Node::Type::IF:
		return visitIf((IfNode*)node, targetReg);
	case Node::Type::WHILE:
		return visitWhile((WhileNode*)node, targetReg);
	case Node::Type::EMPTY:
		return CompilerResult();
	}
	return CompilerResult::generateError();
}



CompilerResult Compiler::visitStatementList(const BinaryNode* node, CBYTE targetReg) {
	CompilerResult left = compile(node->left, Register::ZERO);
	RETURN_IF_ERROR(left);

	CompilerResult right = compile(node->right, Register::ZERO);
	RETURN_IF_ERROR(right);

	INSERT_TO_END(left.instructions, right.instructions);
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
	RETURN_IF_ERROR(result);

	CompilerResult statementRes = compile(node->statement, targetReg);
	RETURN_IF_ERROR(statementRes);

	insertPushPopInstructions(result, statementRes);

	Function& olderDef = findFunction(*(std::string*)identifier->value());
	if (olderDef.exists()) {
		olderDef.instructions = result.instructions;
		olderDef.context = context;
	} else {
		functions.push_back(
			Function(identifier->value(), returnType->value(), context, result.instructions)
		);
	}
	currContext_ = context.parent;

	return CompilerResult();
}

CompilerResult Compiler::visitParamList(const BinaryNode* node) {
	CompilerResult left = node->left->type() == Node::Type::PARAM
		? visitParam((ParameterNode*)node->left)
		: visitParamList((BinaryNode*)node->left);
	RETURN_IF_ERROR(left);

	CompilerResult right = node->right->type() == Node::Type::PARAM
		? visitParam((ParameterNode*)node->right)
		: visitParamList((BinaryNode*)node->right);
	RETURN_IF_ERROR(right);

	INSERT_TO_END(left.instructions, right.instructions);
	return left;
}

CompilerResult Compiler::visitParam(const ParameterNode* node) {
	const std::string& name = node->name();

	CADDR addr0 = currContext_->stackMalloc();
	Variable* var = new Variable(name, node->datatype(), nextFreeArgReg(), addr0);
	currContext_->variables.push_back(var);

	auto instructions = std::vector<Instruction*>({
		InstructionFactory::ST(Register::SP, var->reg, var->addr)
		});
	setRegVar(var->reg, var);
	return CompilerResult(instructions, var->reg);
}


// optimizer: check if necessary storing to ram or just keep in reg
CompilerResult Compiler::visitDefOrAssign(const AssignNode* node, CBYTE targetReg) {
	if (node->dt) {
		return visitVarDef(node, targetReg);
	} else {
		return visitVarAssign(node, targetReg);
	}
}

CompilerResult Compiler::visitVarDef(const AssignNode* node, CBYTE targetReg) {
	const std::string& name = node->nameOfVar();
	if (currContext_->findVariableInContext(name)->exists())
		return CompilerResult::generateError(ErrorCode::VAR_ALREADY_EXISTS);

	CompilerResult inner = compile(node->right, targetReg);
	RETURN_IF_ERROR(inner);

	CADDR addr0 = currContext_->stackMalloc();
	Variable* var = new Variable(name, node->datatype(), inner.actualTarget, addr0);
	currContext_->variables.push_back(var);

	inner.instructions.push_back(
		InstructionFactory::ST(Register::SP, var->reg, var->addr)
	);
	setRegVar(var->reg, var);
	return inner;
}

CompilerResult Compiler::visitVarAssign(const AssignNode* node, CBYTE targetReg) {
	const std::string& name = node->nameOfVar();

	Variable* var = currContext_->findVariable(name);
	if (!var->exists())
		return CompilerResult::generateError(ErrorCode::VAR_NOT_FOUND);

	CompilerResult inner = compile(node->right, var->reg);
	RETURN_IF_ERROR(inner);
	setRegVar(inner.actualTarget, var);
	inner.instructions.push_back(
		InstructionFactory::ST(Register::SP, var->reg, var->addr)
	);
	return inner;
}


CompilerResult Compiler::visitIf(const IfNode* node, CBYTE targetReg) {
	CompilerResult condition = compile(node->condition, Register::CHOOSE);
	RETURN_IF_ERROR(condition);
	freeReg(condition.actualTarget);

	CompilerResult statement = compile(node->statement, targetReg);
	RETURN_IF_ERROR(statement);

	condition.instructions.push_back(InstructionFactory::JMPC(
			Instruction::Function::EQ,
			Register::ZERO,
			condition.actualTarget,
			(CADDR)statement.instructions.size() + (node->elseBlock == nullptr ? 1 : 2) // 2 in case JMPC gets inserted below
	));

	INSERT_TO_END(condition.instructions, statement.instructions);

	if (node->elseBlock != nullptr) {
		CompilerResult elseBlock = compile(node->elseBlock, targetReg);
		RETURN_IF_ERROR(elseBlock);
		condition.instructions.push_back(InstructionFactory::JMPC( // end of if-block -> jmp over else-block
			Instruction::Function::EQ,
			Register::ZERO,
			Register::ZERO,
			(CADDR)elseBlock.instructions.size() + 1
		));
		INSERT_TO_END(condition.instructions, elseBlock.instructions);
	}

	return condition;
}


CompilerResult Compiler::visitWhile(const WhileNode* node, CBYTE targetReg) {
	CompilerResult condition = compile(node->condition, Register::CHOOSE);
	RETURN_IF_ERROR(condition);
	freeReg(condition.actualTarget);

	CompilerResult statement = compile(node->statement, targetReg);
	RETURN_IF_ERROR(statement);

	condition.instructions.push_back(InstructionFactory::JMPC(
		Instruction::Function::EQ,
		Register::ZERO,
		condition.actualTarget,
		(CADDR)statement.instructions.size() + 2
	));

	INSERT_TO_END(condition.instructions, statement.instructions);

	condition.instructions.push_back(
		InstructionFactory::JMPC(
			Instruction::Function::EQ,
			Register::ZERO,
			Register::ZERO,
			0x100 - ((CADDR)condition.instructions.size() + 1) // condition already includes statement instructions
		)
	);

	return condition;
}


CompilerResult Compiler::visitFuncCall(const FuncCallNode* node) {
	CompilerResult result = CompilerResult();
	const std::string& name = node->nameOfVar();

	if (node->argList == nullptr) {
	} else if (((BinaryNode*)node->argList)->isArgList()) {
		result = visitArgList((BinaryNode*)node->argList, occupyReg(Register::RET));
	} else {
		result = visitArg(node->argList, Register::RET);
	}
	RETURN_IF_ERROR(result);
	freeArgs();

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
	CompilerResult left = ((BinaryNode*)node->left)->isArgList()
		? visitArgList((BinaryNode*)node->left, targetReg)
		: compile(node->left, targetReg);
	RETURN_IF_ERROR(left);

	CompilerResult right = visitArg(node->right, occupyReg(nextFreeArgReg()));
	RETURN_IF_ERROR(right);

	INSERT_TO_END(left.instructions, right.instructions);
	return left;
}


CompilerResult Compiler::visitArg(const Node* node, CBYTE targetReg) {
	CompilerResult res = compile(node, targetReg);
	RETURN_IF_ERROR(res);

	if (res.actualTarget != targetReg) {
		res.instructions.push_back(
			InstructionFactory::REG(Instruction::Function::MOV, targetReg, res.actualTarget, res.actualTarget)
		);
	}

	return res;
}


CompilerResult Compiler::visitLogOr(const BinaryNode* node, CBYTE targetReg) {
	return visitBinaryLogExpr(node, targetReg, Instruction::Function::OR);
}


CompilerResult Compiler::visitLogXor(const BinaryNode* node, CBYTE targetReg) {
	return visitBinaryLogExpr(node, targetReg, Instruction::Function::XOR);
}


CompilerResult Compiler::visitLogAnd(const BinaryNode* node, CBYTE targetReg) {
	return visitBinaryLogExpr(node, targetReg, Instruction::Function::AND);
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


CompilerResult Compiler::visitUnaryMinus(const UnaryNode* node, CBYTE targetReg) {
	CompilerResult inner = compile(node->node, targetReg);
	RETURN_IF_ERROR(inner);

	inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::SUB, targetReg,
		Register::ZERO, inner.actualTarget));
	inner.actualTarget = targetReg;
	return inner;
}

CompilerResult Compiler::visitUnaryNot(const UnaryNode* node, CBYTE targetReg)
{
	CompilerResult inner = node->op->type() == Token::Type::LOG_NOT
		? compileBool(node->node, targetReg)
		: compile(node->node, targetReg);
	RETURN_IF_ERROR(inner);

	inner.instructions.push_back(InstructionFactory::REG(Instruction::Function::NOT,
		targetReg, Register::ZERO, inner.actualTarget));
	inner.actualTarget = targetReg;
	return inner;
}

CompilerResult Compiler::visitUnaryReturn(const UnaryNode* node) {
	CompilerResult inner = compile(node->node, Register::RET);
	RETURN_IF_ERROR(inner);

	Instruction* jmp = InstructionFactory::JMP(Register::ZERO, 0x0000);
	currContext_->returnStatements.push_back(jmp);
	inner.instructions.push_back(jmp);
	return inner;
}


CompilerResult Compiler::visitSimple(const LiteralNode* node, CBYTE targetReg) {
	if (node->token->type() == Token::Type::IDENTIFIER) {
		Variable* var = currContext_->findVariable(*(std::string*)node->token->value());
		if (!var->exists())
			return CompilerResult::generateError(ErrorCode::VAR_NOT_FOUND);

		if (var->reg) {
			return CompilerResult(std::vector<Instruction*>(), var->reg);
		} else {
			CBYTE actualTarget = getTarget(targetReg);
			return CompilerResult(std::vector<Instruction*>({
				InstructionFactory::LD(actualTarget, Register::SP, var->addr)
				}), actualTarget);
		}
	} else {
		CBYTE actualTarget = getTarget(targetReg);
		return CompilerResult(std::vector<Instruction*>({
			InstructionFactory::LI(actualTarget, *(int*)node->token->value())
			}), actualTarget);
	}
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
	if (Nodes::isLiteral(node->right)) {
		first = node->left;
		second = node->right;
	} else {
		first = node->right;
		second = node->left;
		leftFirst = false;
	}

	CompilerResult firstResult = (this->*converter)(first, targetReg);
	RETURN_IF_ERROR(firstResult);

	// occupy targetReg for inner nodes
	CompilerResult secondResult = (this->*converter)(second, Register::CHOOSE);
	RETURN_IF_ERROR(secondResult);

	freeRegIfNoVar(firstResult.actualTarget);
	freeRegIfNoVar(secondResult.actualTarget);

	INSERT_TO_END(firstResult.instructions, secondResult.instructions);

	CBYTE actualTarget = targetReg != Register::CHOOSE
		? targetReg
		: Nodes::getTargetFromNodeType(node->left, node->right,
			firstResult.actualTarget, secondResult.actualTarget, leftFirst);

	occupyReg(actualTarget);

	if (leftFirst)
		firstResult.instructions.push_back(
			InstructionFactory::REG(func, actualTarget, firstResult.actualTarget, secondResult.actualTarget));
	else
		firstResult.instructions.push_back(
			InstructionFactory::REG(func, actualTarget, secondResult.actualTarget, firstResult.actualTarget));

	firstResult.actualTarget = actualTarget;
	return firstResult;
}

CompilerResult Compiler::visitBinaryExpr(const BinaryNode* node, CBYTE targetReg, const Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compile);
}

CompilerResult Compiler::visitBinaryLogExpr(const BinaryNode* node, CBYTE targetReg, Instruction::Function func) {
	return visitBinaryExprConvert(node, targetReg, func, &Compiler::compileBool);
}



CompilerResult Compiler::compileBool(const Node* node, CBYTE targetReg) {
	CompilerResult result = compile(node, targetReg);
	convertToBoolIfNecessary(result.instructions, node, targetReg);
	return result;
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




Function& Compiler::findFunction(const std::string& name) {
	for (Function& func : functions) {
		if (name.compare(func.name) == 0) {
			return func;
		}
	}
	return Function::notFound();
}



void Compiler::insertPushPopInstructions(CompilerResult& result, CompilerResult& statementRes)
{
	std::vector<Instruction*> push = memManager.pushForContext(currContext_);
	std::vector<Instruction*> pop = memManager.popForContext(currContext_);
	INSERT_TO_BEGIN(result.instructions, push);
	INSERT_TO_END(result.instructions, statementRes.instructions);
	currContext_->endIndex = result.instructions.size();
	INSERT_TO_END(result.instructions, pop);
	result.instructions.push_back(InstructionFactory::RET());
}



CBYTE Compiler::getTarget(CBYTE targetReg) {
	BYTE reg = targetRegValid(targetReg); // 0 if no free reg
	if (reg != Register::CHOOSE)
		return reg;
	return occupyReg(freeReg(nextFreeableReg()));
}

// Registers 0x0 to 0x2 (IM) cannot be written to, thus they have to be invalid targets
// still returns 0 when no Register is free
CBYTE Compiler::targetRegValid(CBYTE targetReg) {
	return targetReg > Register::IM && targetReg < Register::GP_MAX ? targetReg : occupyReg(nextFreeGPReg());
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

CBYTE Compiler::nextFreeableReg() const {
	for (BYTE reg = Register::GP_MIN; reg <= Register::GP_MAX; reg++) {
		if (currContext_->registers[reg].containedVar != nullptr) {
			return reg;
		}
	}
	return 0;
}

bool Compiler::isRegFree(CBYTE reg) const {
	return !currContext_->registers[reg].isOccupied;
}

CBYTE Compiler::freeReg(CBYTE reg) {
	currContext_->registers[reg].isOccupied = false;
	delRegVar(reg);
	return reg;
}

CBYTE Compiler::freeRegIfNoVar(CBYTE reg) {
	if (currContext_->registers[reg].containedVar != nullptr)
		return Register::ZERO;
	currContext_->registers[reg].isOccupied = false;
	delRegVar(reg);
	return reg;
}

CBYTE Compiler::occupyReg(CBYTE reg) {
	currContext_->registers[reg].isOccupied = true;
	currContext_->registers[reg].isGenerallyUsed = true;
	return reg;
}

void Compiler::freeArgs() {
	for (BYTE reg = Register::ARG_MIN; reg <= Register::ARG_MAX; reg++) {
		freeReg(reg);
	}
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
		for (const Instruction* instr : instructions)
			out += instr->toHexString() + "\n";
		return out;
	}
}

std::string CompilerResult::toReadableString() const
{
	if (error.isError()) {
		return ErrorMessageGenerator::fromError(error);
	} else {
		std::string out;
		for (const Instruction* instr : instructions)
			out += instr->toString() + "\n";
		return out;
	}
}

std::string CompilerResult::toOutputString() const
{
	if (error.isError()) {
		return ErrorMessageGenerator::fromError(error);
	} else {
		std::string out;
		for (const Instruction* instr : instructions)
			out += instr->toOutString();
		return out;
	}
}

}
