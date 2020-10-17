#pragma once

#include "Instruction.h"

namespace fenge {

class InstructionFactory {
public:

	static Instruction NOP() {
		return Instruction((int)Instruction::Opcode::NOP);
	}

	static Instruction REG(Instruction::Function func, unsigned char rw, unsigned char rr1, unsigned char rr2) {
		return Instruction(
			(int)Instruction::Opcode::REG
			+ (rw << 4)
			+ (((unsigned char)func & 0xF) << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ (((unsigned char)func >> 4) << 20)
		);
	}

	static Instruction LWI(Instruction::Function func, unsigned char rw, unsigned char rr1, unsigned char im) {
		return LWI_UPI(Instruction::Opcode::LWI, func, rw, rr1, im);
	}

	static Instruction UPI(Instruction::Function func, unsigned char rw, unsigned char rr1, unsigned char im) {
		return LWI_UPI(Instruction::Opcode::UPI, func, rw, rr1, im);
	}

	static Instruction LI(unsigned char rw, unsigned short im) {
		return LI_JMP_CALL(Instruction::Opcode::LI, rw, im);
	}

	static Instruction JMP(unsigned char rw, unsigned short im) {
		return LI_JMP_CALL(Instruction::Opcode::JMP, rw, im);
	}

	static Instruction JMPC(Instruction::Function func, unsigned char rr1, unsigned char rr2, unsigned char im) {
		return Instruction(
			(int)Instruction::Opcode::JMPC
			+ ((im & 0xF) << 4)
			+ ((unsigned char)func << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ (((unsigned char)im >> 4) << 20)
		);
	}

	static Instruction LD(unsigned char rw, unsigned char rr1, unsigned short im) {
		return Instruction(
			(int)Instruction::Opcode::LD
			+ (rw << 4)
			+ ((im >> 8) << 8)
			+ (rr1 << 12)
			+ ((im & 0xFF) << 16)
		);
	}

	static Instruction ST(unsigned char rr1, unsigned char rr2, unsigned short im) {
		return Instruction(
			(int)Instruction::Opcode::ST
			+ ((im & 0xF) << 4)
			+ ((im >> 8) << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ ((im & 0xF0) << 16) // only 16, because it has 4 0s at the end
		);
	}

	static Instruction CALL(unsigned char rw, unsigned short im) {
		return LI_JMP_CALL(Instruction::Opcode::CALL, rw, im);
	}

	static Instruction RET() {
		return Instruction((int)Instruction::Opcode::RET);
	}

private:
	static Instruction LWI_UPI(Instruction::Opcode op, Instruction::Function func, unsigned char rw, unsigned char rr1, unsigned char im) {
		return Instruction(
			(int)op
			+ (rw << 4)
			+ ((unsigned char)func << 8)
			+ (rr1 << 12)
			+ ((unsigned int)im << 16)
		);
	}

	static Instruction LI_JMP_CALL(Instruction::Opcode op, unsigned char rw, unsigned short im) {
		return Instruction(
			(int)op
			+ (rw << 4)
			+ ((im >> 8) << 8)
			+ ((im & 0xFF) << 16)
		);
	}
};
}
