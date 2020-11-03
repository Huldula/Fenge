#pragma once

#include "Instruction.h"

namespace fenge {

class InstructionFactory {
public:

	static Instruction* NOP() {
		return new Instruction((int)Instruction::Opcode::NOP);
	}

	static Instruction* REG(const Instruction::Function func, CBYTE rw, CBYTE rr1, CBYTE rr2) {
		if (func == Instruction::Function::MOV) {
			return InstructionFactory::REG(Instruction::Function::ADD, rw, rr1, 0);
		}
		return new Instruction(
			(int)Instruction::Opcode::REG
			+ (rw << 4)
			+ (((CBYTE)func & 0xF) << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ (((CBYTE)func >> 4) << 20)
		);
	}

	static Instruction* LWI(const Instruction::Function func, CBYTE rw, CBYTE rr1, CBYTE im) {
		return LWI_UPI(Instruction::Opcode::LWI, func, rw, rr1, im);
	}

	static Instruction* UPI(const Instruction::Function func, CBYTE rw, CBYTE rr1, CBYTE im) {
		return LWI_UPI(Instruction::Opcode::UPI, func, rw, rr1, im);
	}

	static Instruction* LI(CBYTE rw, CSHORT im) {
		return LI_JMP_CALL(Instruction::Opcode::LI, rw, im);
	}

	static Instruction* JMP(CBYTE rw, CSHORT im) {
		return LI_JMP_CALL(Instruction::Opcode::JMP, rw, im);
	}

	static Instruction* JMPC(const Instruction::Function func, CBYTE rr1, CBYTE rr2, CBYTE im) {
		return new Instruction(
			(int)Instruction::Opcode::JMPC
			+ ((im & 0xF) << 4)
			+ ((CBYTE)func << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ (((CBYTE)im >> 4) << 20)
		);
	}

	static Instruction* LD(CBYTE rw, CBYTE rr1, CSHORT im) {
		return new Instruction(
			(int)Instruction::Opcode::LD
			+ (rw << 4)
			+ (((im & 0xF00) >> 8) << 8)
			+ (rr1 << 12)
			+ ((im & 0xFF) << 16)
		);
	}

	static Instruction* ST(CBYTE rr1, CBYTE rr2, CSHORT im) {
		return new Instruction(
			(int)Instruction::Opcode::ST
			+ ((im & 0x00F) << 4)
			+ (((im & 0xF00) >> 8) << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ ((im & 0x0F0) << 16) // only 16, because it has 4 0s at the end
		);
	}

	static Instruction* CALL(CBYTE rw, CSHORT im) {
		return LI_JMP_CALL(Instruction::Opcode::CALL, rw, im);
	}

	static Instruction* RET() {
		return new Instruction((int)Instruction::Opcode::RET);
	}

	static Instruction* HLT() {
		return new Instruction((int)Instruction::Opcode::HLT);
	}

private:
	static Instruction* LWI_UPI(const Instruction::Opcode op, const Instruction::Function func, CBYTE rw, CBYTE rr1, CBYTE im) {
		return new Instruction(
			(int)op
			+ (rw << 4)
			+ ((CBYTE)func << 8)
			+ (rr1 << 12)
			+ ((int)im << 16)
		);
	}

	static Instruction* LI_JMP_CALL(const Instruction::Opcode op, CBYTE rw, CSHORT im) {
		return new Instruction(
			(int)op
			+ (rw << 4)
			+ ((im >> 8) << 8)
			+ ((im & 0xFF) << 16)
		);
	}
};
}
