#pragma once

#include "fgpch.h"

#include "Token.h"

namespace fenge {

class Node {
public:
	inline virtual std::string toString() const {
		return "()";
	}
};


class LiteralNode : Node {
public:
	Token* token;

	LiteralNode(Token* token) : token(token) { }

	inline std::string toString() const override {
		return std::string("(") + token->toString() + ")";
	}
};


class UnaryNode : Node {
public:
	Token* op;
	Node* node;

	UnaryNode(Token* op, Node* node) : op(op), node(node) { }

	inline std::string toString() const override {
		return std::string("(") + op->toString() + node->toString() + ")";
	}
};


class BinaryNode : Node {
public:
	Node* left;
	Token* op;
	Node* right;

	BinaryNode(Node* left, Token* op, Node* right) : left(left), op(op), right(right) { }

	inline std::string toString() const override {
		return std::string("(") + left->toString() + op->toString() + right->toString() + ")";
	}
};

}
