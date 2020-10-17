#include "fgpch.h"

#include "Instruction.h"

namespace fenge {


Instruction::Instruction(int value) : value_(value) { }

int Instruction::value() const {
	return value_;
}

std::string Instruction::toString() const
{
	return "";
}

}
