#pragma once

#include "fgpch.h"

#include "Token.h"
#include <Core\Log.h>

namespace fenge {

class Node {
public:
	enum class Type {
		NODE,
		LITERAL,
		UNARY,
		ASSIGN,
		BINARY,
		FUNC_DEF,
		PARAM,
		FUNC_CALL,
		IF,
		WHILE,
		EMPTY
	};

	virtual ~Node() {}
	inline virtual std::string toString() const {
		return "( )";
	}

	virtual Type type() const {
		return Type::NODE;
	}
};


class LiteralNode : Node {
public:
	Token* token;

	LiteralNode(Token* token) : token(token) { }

	~LiteralNode() {
	}

	inline std::string toString() const override {
		return token->toString();
	}

	Type type() const override {
		return Type::LITERAL;
	}
};


class UnaryNode : Node {
public:
	Token* op;
	Node* node;

	UnaryNode(Token* op, Node* node) : op(op), node(node) { }

	~UnaryNode() {
		delete node;
	}

	inline std::string toString() const override {
		return std::string("( ") + op->toString() + node->toString() + " )";
	}

	Type type() const override {
		return Type::UNARY;
	}
};

class AssignNode : Node {
public:
	Token* dt = nullptr;
	Node* leftSide;
	Node* right;

	AssignNode(Token* dt, Node* leftSide, Node* right) : dt(dt), leftSide(leftSide), right(right) { }

	~AssignNode() {
		delete leftSide;
		delete right;
	}

	inline std::string toString() const override {
		return std::string("( ") + (dt == nullptr ? "" : dt->toString()) + leftSide->toString()
			+ " = " + right->toString() + " )";
	}

	Type type() const override {
		return Type::ASSIGN;
	}

	const Token::Keyword datatype() const {
		return *(Token::Keyword*)dt->value();
	}

	const std::string& nameOfVar() const {
		return *(std::string*)((LiteralNode*)leftSide)->token->value();
	}
};

class BinaryNode : Node {
public:
	Node* left;
	Token* op;
	Node* right;

	BinaryNode(Node* left, Token* op, Node* right) : left(left), op(op), right(right) { }

	~BinaryNode() {
		delete left;
		delete right;
	}

	inline std::string toString() const override {
		return std::string("( ") + left->toString()
			+ (op->type() == Token::Type::SEMICOLON ? "\n" : op->toString())+ right->toString() + " )";
	}

	Type type() const override {
		return Type::BINARY;
	}

	bool isArgList() const {
		return type() == Node::Type::BINARY
			&& op->type() == Token::Type::COMMA;
	}
};

class FuncDefNode : Node {
public:
	Token* dt;
	Token* id;
	Node* paramList;
	Node* statement;

	FuncDefNode(Token* dt, Token* id, Node* paramList, Node* statement)
		: dt(dt), id(id), paramList(paramList), statement(statement) { }

	~FuncDefNode() {
		delete paramList;
		delete statement;
	}

	inline std::string toString() const override {
		return std::string("( ") + (dt == nullptr ? "void" : dt->toString()) + id->toString()
			+ (paramList == nullptr ? "" : paramList->toString()) + "{\n" + statement->toString() + "}\n" + " )";
	}

	Type type() const override {
		return Type::FUNC_DEF;
	}

	const std::string& name() const {
		return *(std::string*)id->value();
	}

	const Token::Keyword datatype() const {
		return *(Token::Keyword*)dt->value();
	}
};

class ParameterNode : Node {
public:
	Token* dt;
	Token* id;

	ParameterNode(Token* dt, Token* id) : dt(dt), id(id) { }

	~ParameterNode() {
	}

	inline std::string toString() const override {
		return std::string("( ") + (dt == nullptr ? "void" : dt->toString()) + id->toString() + " )";
	}

	Type type() const override {
		return Type::PARAM;
	}

	const std::string& name() const {
		return *(std::string*)id->value();
	}

	const Token::Keyword datatype() const {
		return *(Token::Keyword*)dt->value();
	}
};

class FuncCallNode : Node {
public:
	Node* leftSide;
	Node* argList;

	FuncCallNode(Node* leftSide, Node* argList) : leftSide(leftSide), argList(argList) { }

	~FuncCallNode() {
		delete leftSide;
		delete argList;
	}

	inline std::string toString() const override {
		return std::string("( ") + leftSide->toString() + "(" + (argList == nullptr ? "" : argList->toString()) + ")" + " )";
	}

	Type type() const override {
		return Type::FUNC_CALL;
	}

	const std::string& nameOfVar() const {
		return *(std::string*)((LiteralNode*)leftSide)->token->value();
	}
};

class EmptyNode : Node {
public:

	EmptyNode() = default;
	~EmptyNode() = default;

	inline std::string toString() const override {
		return std::string("()");
	}

	Type type() const override {
		return Type::EMPTY;
	}
};

class IfNode : Node {
public:
	Node* condition;
	Node* statement;
	Node* elseBlock = nullptr;

	IfNode(Node* leftSide, Node* argList)
		: condition(leftSide), statement(argList) { }
	IfNode(Node* leftSide, Node* argList, Node* elseBlock)
		: condition(leftSide), statement(argList), elseBlock(elseBlock) { }

	~IfNode() {
		delete condition;
		delete statement;
		delete elseBlock;
	}

	inline std::string toString() const override {
		return std::string("( IF (") + condition->toString() + ") {" + statement->toString() + "}" + " )";
	}

	Type type() const override {
		return Type::IF;
	}
};

class WhileNode : Node {
public:
	Node* condition;
	Node* statement;

	WhileNode(Node* leftSide, Node* argList) : condition(leftSide), statement(argList) { }

	~WhileNode() {
		delete condition;
		delete statement;
	}

	inline std::string toString() const override {
		return std::string("( WHILE (") + condition->toString() + ") {" + statement->toString() + "}" + " )";
	}

	Type type() const override {
		return Type::WHILE;
	}
};





class Nodes {
public:

	static bool isLiteral(const Node* node){
		return node->type() == Node::Type::LITERAL
			|| node->type() == Node::Type::ASSIGN
			&& ((AssignNode*)node)->right->type() == Node::Type::LITERAL;
	}

	// for binary expressions
	static int rankNode(const Node* node) {
		if (node->type() == Node::Type::LITERAL
			&& ((LiteralNode*)node)->token->type() == Token::Type::IDENTIFIER
			|| node->type() == Node::Type::ASSIGN
			&& ((AssignNode*)node)->right->type() == Node::Type::LITERAL) {
			return 3; // Variable
		} else if (node->type() == Node::Type::LITERAL) {
			return 1; // int literal
		} else {
			return 2; // anything else
		}
	}

	static CBYTE getTargetFromNodeType(const Node* left, const Node* right, CBYTE firstRes, CBYTE secondRes, bool leftFirst) {
		leftFirst = leftFirst
			? (Nodes::rankNode(left) < Nodes::rankNode(right))
			: (Nodes::rankNode(right) < Nodes::rankNode(left));
		if (leftFirst) {
			return firstRes;
		} else {
			return secondRes;
		}
	}
};

}
