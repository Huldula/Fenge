#pragma once

namespace fenge {

class Context {
public:
	Context() {
		init();
	}
	Context(Context* parent) : parent(parent) {
		init();
	}

	void init() {
		for (size_t reg = Register::ZERO; reg <= Register::GP_MAX; reg++) {
			registers[reg] = Register();
		}
	}

	Context* parent = nullptr;
	std::vector<Variable*> variables = std::vector<Variable*>();
	Register registers[16];

	size_t endIndex;
	std::vector<Instruction*> returnStatements = std::vector<Instruction*>();

	Variable* findVariable(const std::string& name) {
		for (Variable* var : variables) {
			if (name.compare(var->name) == 0) {
				return var;
			}
		}
		if (parent == nullptr) {
			return &Variable::notFound();
		}
		return parent->findVariable(name);
	}

	Variable* findVariableInContext(const std::string& name) {
		for (Variable* var : variables) {
			if (name.compare(var->name) == 0) {
				return var;
			}
		}
		return &Variable::notFound();
	}

	ADDR stackMemPointer = 0;
	CADDR stackMalloc() {
		return 0x1000 - (++stackMemPointer);
	}
};

}
