#pragma once

namespace fenge {

class Context {
public:
	Context() { }
	Context(Context* parent) : parent(parent) { }

	Context* parent = nullptr;
	std::vector<Variable> variables = std::vector<Variable>();

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
};

}
