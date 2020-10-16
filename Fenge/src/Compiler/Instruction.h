#pragma once

#include "Instructions.h"

namespace fenge {

class Instruction {
public:
	enum Opcode;
	enum Function;

	[[nodiscard]] std::string toString() const {
		return "";
	}
private:
	const Opcode opcode_;
	const Function func_;
};

enum Instruction::Opcode {
#define T(name, val) name = val,
	OPCODE_LIST
#undef T
};

enum Instruction::Function {
#define T(name, val) name = val,
	FUNCTION_LIST
#undef T
};

}
