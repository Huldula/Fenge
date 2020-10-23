#pragma once

namespace fenge {

class Context {
public:
	Context() { }
	Context(Context* parent) : parent(parent) { }

	Context* parent = nullptr;
	std::vector<Variable> variables = std::vector<Variable>();
	std::vector<Function> functions = std::vector<Function>();

	Variable findVariable(const std::string& name) {
		for (Variable& var : variables) {
			if (name.compare(var.name) == 0) {
				return var;
			}
		}
		if (parent == nullptr) {
			return Variable("", Token::Keyword::NO_KEYWORD); // Error
		}
		return parent->findVariable(name);
	}

	Variable findVariableInContext(const std::string& name) {
		for (Variable& var : variables) {
			if (name.compare(var.name) == 0) {
				return var;
			}
		}
		return Variable("", Token::Keyword::NO_KEYWORD);
	}

	Function findFunction(const std::string& name) {
		for (Function& func : functions) {
			if (name.compare(func.name) == 0) {
				return func;
			}
		}
		if (parent == nullptr) {
			return Function("", Token::Keyword::NO_KEYWORD, this); // Error
		}
		return parent->findFunction(name);
	}

	Function findFunctionInContext(const std::string& name) {
		for (Function& func : functions) {
			if (name.compare(func.name) == 0) {
				return func;
			}
		}
		return Function("", Token::Keyword::NO_KEYWORD, this);
	}
};

}
