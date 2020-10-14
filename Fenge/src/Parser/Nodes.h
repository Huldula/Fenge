#pragma once

#include "fgpch.h"

#include "Token.h"
#include <Core\Log.h>

namespace fenge {

class Node {
public:
	virtual ~Node() {}
	inline virtual std::string toString() const {
		return "()";
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
};

}
