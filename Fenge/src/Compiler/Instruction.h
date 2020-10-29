#pragma once

#include "Core/Macros.h"
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
	Instruction(const Instruction& other);

	Opcode getOpcode() const;
	Function getFunction() const;
	Function getFunctionSafe() const;

	bool hasFunction() const;

	void setCallAddr(CADDR addr);

	int value() const;

	static void insertIfValid(std::vector<Instruction>& instructions, Instruction& ins);

	[[nodiscard]] std::string toHexString() const;
	[[nodiscard]] std::string toString() const;
	[[nodiscard]] std::string toOutString() const;
private:
	int value_;

	std::string opcodeAndFunctionToString() const;
	std::string opcodeToString() const;
	std::string functionToString() const;
	std::string rwToString() const;
	std::string rr1ToString() const;
	std::string rr2ToString() const;
	std::string imToString() const;
};

}
