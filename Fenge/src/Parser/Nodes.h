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
		BINARY,
		VAR_ASSIGN
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
		delete token;
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
		delete op;
		delete node;
	}

	inline std::string toString() const override {
		return std::string("( ") + op->toString() + node->toString() + " )";
	}

	Type type() const override {
		return Type::UNARY;
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
		delete op;
		delete right;
	}

	inline std::string toString() const override {
		return std::string("( ") + left->toString() + op->toString() + right->toString() + " )";
	}

	Type type() const override {
		return Type::BINARY;
	}
};

class VarAssignNode : Node {
public:
	Token* dt = nullptr;
	Token* id;
	Node* right;

	VarAssignNode(Token* dt, Token* id, Node* right) : dt(dt), id(id), right(right) { }

	~VarAssignNode() {
		delete dt;
		delete id;
		delete right;
	}

	inline std::string toString() const override {
		return std::string("( ") + (dt == nullptr ? "" : dt->toString()) + id->toString()
			+ " = " + right->toString() + " )";
	}

	Type type() const override {
		return Type::VAR_ASSIGN;
	}

	const std::string& name() const {
		return *(std::string*)id->value();
	}

	const Token::Keyword datatype() const {
		return *(Token::Keyword*)dt->value();
	}
};

}
