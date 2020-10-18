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
		BINARY
	};

	virtual ~Node() {}
	inline virtual std::string toString() const {
		return "()";
	}

	virtual Type type() const {
		return Type::NODE;
	}
};


class LiteralNode : Node {
public:
	~LiteralNode() {
		delete token;
	}

	Token* token;

	LiteralNode(Token* token) : token(token) { }

	inline std::string toString() const override {
		return token->toString();
	}

	Type type() const override {
		return Type::LITERAL;
	}
};


class UnaryNode : Node {
public:
	~UnaryNode() {
		delete op;
		delete node;
	}

	Token* op;
	Node* node;

	UnaryNode(Token* op, Node* node) : op(op), node(node) { }

	inline std::string toString() const override {
		return std::string("(") + op->toString() + node->toString() + ")";
	}

	Type type() const override {
		return Type::UNARY;
	}
};


class BinaryNode : Node {
public:
	~BinaryNode() {
		delete left;
		delete op;
		delete right;
	}

	Node* left;
	Token* op;
	Node* right;

	BinaryNode(Node* left, Token* op, Node* right) : left(left), op(op), right(right) { }

	inline std::string toString() const override {
		return std::string("(") + left->toString() + op->toString() + right->toString() + ")";
	}

	Type type() const override {
		return Type::BINARY;
	}
};

}
