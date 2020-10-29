#include "fgpch.h"

#include "Core/Macros.h"
#include "Core/Log.h"
#include "Instruction.h"
#include "Core/Helper.h"

namespace fenge {


Instruction::Instruction(int value) : value_(value) { }
Instruction::Instruction(const Instruction& other) {
	value_ = other.value_;
	LOG("copy");
}

Instruction::Opcode Instruction::getOpcode() const {
	return (Opcode)(value_ % 16);
}

// does not check if there is actually a function
Instruction::Function Instruction::getFunction() const {
	if (getOpcode() == Opcode::REG) {
		return (Function)(((value_ & 0xF00) >> 8) + ((value_ & 0xF00000) >> 16));
	} else {
		return (Function)((value_ & 0xF00) >> 8);
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
		|| opcode == Opcode::LD || opcode == Opcode::ST || opcode == Opcode::RET
		|| opcode == Opcode::HLT || opcode == Opcode::NOP);
}

void Instruction::setCallAddr(CADDR addr) {
	if (getOpcode() == Opcode::CALL || getOpcode() == Opcode::JMP) {
		value_ = value_ & 0x000FF;
		value_ += (addr & 0xFF00) + ((addr & 0xFF) << 16);
	}
}

int Instruction::value() const {
	return value_;
}

void Instruction::insertIfValid(std::vector<Instruction>& instructions, Instruction& ins) {
	if (ins.value())
		instructions.push_back(ins);
}

std::string Instruction::toHexString() const {
	return Helper::toHexString(value_, 6);
}

std::string Instruction::toString() const {
	if (getOpcode() == Opcode::NOP || getOpcode() == Opcode::RET || getOpcode() == Opcode::HLT)
		return opcodeAndFunctionToString();
	return opcodeAndFunctionToString()
		+ " " + rwToString()
		+ " " + rr1ToString()
		+ " " + rr2ToString()
		+ " " + imToString();
}

std::string Instruction::toOutString() const {
	return Helper::toHexString(value_ & 0xFF, 2)
		+ Helper::toHexString((value_ & 0xFF00) >> 8, 2)
		+ Helper::toHexString((value_ & 0xFF0000) >> 16, 2);
}

std::string Instruction::opcodeAndFunctionToString() const {
	if (getOpcode() == Opcode::REG) {
		return functionToString();
	} else if (hasFunction()) {
		return opcodeToString() + " " + functionToString();
	} else {
		return opcodeToString();
	}
}

std::string Instruction::opcodeToString() const {
	switch ((int)getOpcode()) {
#define T(name, val) case val: return #name;
		OPCODE_LIST
#undef T
	default:
		return "";
	}
}

std::string Instruction::functionToString() const {
	switch ((int)getFunction()) {
#define T(name, val) case val: return #name;
		FUNCTION_LIST
#undef T
	default:
		return "";
	}
}

std::string Instruction::rwToString() const {
	if (getOpcode() != Opcode::JMPC && getOpcode() != Opcode::ST) {
		return Helper::toHexString((value_ & 0xF0) >> 4);
	} else {
		return "";
	}
}

std::string Instruction::rr1ToString() const {
	if (getOpcode() != Opcode::LI && getOpcode() != Opcode::JMP && getOpcode() != Opcode::CALL) {
		return Helper::toHexString((value_ & 0xF000) >> 12);
	} else {
		return "";
	}
}

std::string Instruction::rr2ToString() const {
	if (getOpcode() == Opcode::REG || getOpcode() == Opcode::JMPC || getOpcode() == Opcode::ST) {
		return Helper::toHexString((value_ & 0xF0000) >> 16);
	} else {
		return "";
	}
}

std::string Instruction::imToString() const {
	const Opcode opcode = getOpcode();
	if (opcode == Opcode::REG) {
		return "";
	} else if (opcode == Opcode::LWI || opcode == Opcode::UPI) {
		return Helper::toHexString((value_ & 0xFF0000) >> 16);
	} else if (opcode == Opcode::LI || opcode == Opcode::JMP || opcode == Opcode::CALL) {
		return Helper::toHexString(((value_ & 0xFF0000) >> 16) + (value_ & 0x00FF00));
	} else if (opcode == Opcode::JMPC) {
		return Helper::toHexString(((value_ & 0xF00000) >> 16) + ((value_ & 0x0000F0) >> 4));
	} else if (opcode == Opcode::LD) {
		return Helper::toHexString(((value_ & 0xFF0000) >> 16) + (value_ & 0x000F00));
	} else if (opcode == Opcode::ST) {
		return Helper::toHexString(((value_ & 0xF00000) >> 16) + (value_ & 0x000F00) + ((value_ & 0x0000F0) >> 4));
	} else {
		return "";
	}
}

}
