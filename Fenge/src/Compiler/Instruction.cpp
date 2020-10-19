#include "fgpch.h"

#include "Instruction.h"

namespace fenge {


Instruction::Instruction(int value) : value_(value) { }

Instruction::Opcode Instruction::getOpcode() const {
	return (Opcode)(value_ % 16);
}

// does not check if there is actually a function
Instruction::Function Instruction::getFunction() const {
	if (getOpcode() == Opcode::REG) {
		return (Function)(((value_ & 0xF00) >> 8) + ((value_ & 0xF00000) >> 16));
	} else {
		return (Function)(value_ & 0xF00 >> 8);
	}
}

Instruction::Function Instruction::getFunctionSafe() const {
	if (getOpcode() == Opcode::REG) {
		return (Function)(((value_ & 0xF00) >> 8) + ((value_ & 0xF00000) >> 16));
	} else if (!hasFunction()) {
		return Function::ILL;
	} else {
		return (Function)((value_ & 0xF00) >> 8);
	}
}

bool Instruction::hasFunction() const {
	const Opcode opcode = getOpcode();
	return !(opcode == Opcode::LI || opcode == Opcode::JMP || opcode == Opcode::CALL
		|| opcode == Opcode::LD || opcode == Opcode::ST || opcode == Opcode::RET);
}

int Instruction::value() const {
	return value_;
}

std::string Instruction::toHexString() const {
	std::stringstream stream;
	stream << std::hex << value_;
	std::string out = std::string(stream.str());
	while (out.length() < 6) {
		out = std::string("0") + out;
	}
	return out;
}

std::string Instruction::toString() const {
	return "";
}

}