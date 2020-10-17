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
	Instruction(char opcode, char agr1, char agr2, char agr3, char agr4, char agr5);

	void setOpcode(Opcode opcode);
	void setRW(char reg);

	int value() const;

	[[nodiscard]] std::string toString() const;
private:
	int value_;
};

}
