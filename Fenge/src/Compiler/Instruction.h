#pragma once

#include "Instructions.h"

namespace fenge {

class Instruction {
public:
	enum class Opcode {
#define T(name, val) name = (char)val,
		OPCODE_LIST
#undef T
	};
	enum class Function {
#define T(name, val) name = (char)val,
		FUNCTION_LIST
#undef T
	};

	Instruction(int value);

	Opcode getOpcode() const;
	Function getFunction() const;
	Function getFunctionSafe() const;

	bool hasFunction() const;

	int value() const;

	[[nodiscard]] std::string toString() const;
private:
	int value_;
};

}
