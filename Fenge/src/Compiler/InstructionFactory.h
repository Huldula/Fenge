#pragma once

#include "Instruction.h"

namespace fenge {

class InstructionFactory {
public:

	static Instruction NOP() {
		return Instruction((int)Instruction::Opcode::NOP);
	}

	static Instruction REG(const Instruction::Function func, BYTE rw, BYTE rr1, BYTE rr2) {
		return Instruction(
			(int)Instruction::Opcode::REG
			+ (rw << 4)
			+ (((BYTE)func & 0xF) << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ (((BYTE)func >> 4) << 20)
		);
	}

	static Instruction LWI(const Instruction::Function func, BYTE rw, BYTE rr1, BYTE im) {
		return LWI_UPI(Instruction::Opcode::LWI, func, rw, rr1, im);
	}

	static Instruction UPI(const Instruction::Function func, BYTE rw, BYTE rr1, BYTE im) {
		return LWI_UPI(Instruction::Opcode::UPI, func, rw, rr1, im);
	}

	static Instruction LI(BYTE rw, SHORT im) {
		return LI_JMP_CALL(Instruction::Opcode::LI, rw, im);
	}

	static Instruction JMP(BYTE rw, SHORT im) {
		return LI_JMP_CALL(Instruction::Opcode::JMP, rw, im);
	}

	static Instruction JMPC(const Instruction::Function func, BYTE rr1, BYTE rr2, BYTE im) {
		return Instruction(
			(int)Instruction::Opcode::JMPC
			+ ((im & 0xF) << 4)
			+ ((BYTE)func << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ (((BYTE)im >> 4) << 20)
		);
	}

	static Instruction LD(BYTE rw, BYTE rr1, SHORT im) {
		return Instruction(
			(int)Instruction::Opcode::LD
			+ (rw << 4)
			+ ((im >> 8) << 8)
			+ (rr1 << 12)
			+ ((im & 0xFF) << 16)
		);
	}

	static Instruction ST(BYTE rr1, BYTE rr2, SHORT im) {
		return Instruction(
			(int)Instruction::Opcode::ST
			+ ((im & 0xF) << 4)
			+ ((im >> 8) << 8)
			+ (rr1 << 12)
			+ (rr2 << 16)
			+ ((im & 0xF0) << 16) // only 16, because it has 4 0s at the end
		);
	}

	static Instruction CALL(BYTE rw, SHORT im) {
		return LI_JMP_CALL(Instruction::Opcode::CALL, rw, im);
	}

	static Instruction RET() {
		return Instruction((int)Instruction::Opcode::RET);
	}

private:
	static Instruction LWI_UPI(const Instruction::Opcode op, const Instruction::Function func, BYTE rw, BYTE rr1, BYTE im) {
		return Instruction(
			(int)op
			+ (rw << 4)
			+ ((BYTE)func << 8)
			+ (rr1 << 12)
			+ ((int)im << 16)
		);
	}

	static Instruction LI_JMP_CALL(const Instruction::Opcode op, BYTE rw, SHORT im) {
		return Instruction(
			(int)op
			+ (rw << 4)
			+ ((im >> 8) << 8)
			+ ((im & 0xFF) << 16)
		);
	}
};
}
