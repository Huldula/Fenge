#pragma once

namespace fenge {

class MemoryManager {
public:
	static CADDR STACK_SIZE = 0x100;

	std::vector<Instruction*> push(CBYTE reg) const {
		return std::vector<Instruction*>({
			InstructionFactory::ST(Register::SP, reg, 0x0000),
			InstructionFactory::LI(Register::ZERO, 0x0001),
			InstructionFactory::REG(Instruction::Function::ADD, Register::SP, Register::SP, Register::IM)
			});
	}

	std::vector<Instruction*> pop(CBYTE reg) const {
		return std::vector<Instruction*>({
			InstructionFactory::LD(reg, Register::SP, 0x0000),
			InstructionFactory::LI(Register::ZERO, 0x0001),
			InstructionFactory::REG(Instruction::Function::SUB, Register::SP, Register::SP, Register::IM)
			});
	}

	std::vector<Instruction*> pushForContext(Context* context) const {
		auto out = std::vector<Instruction*>();
		BYTE count = 0;
		for (BYTE reg = Register::GP_MIN; reg <= Register::GP_MAX; reg++) {
			if (context->registers[reg].isGenerallyUsed) {
				count++;
				out.push_back(InstructionFactory::ST(Register::SP, reg, count));
			}
		}
		if (count + context->stackMemPointer > 0) {
			out.push_back(InstructionFactory::LI(Register::ZERO, count + context->stackMemPointer));
			out.push_back(InstructionFactory::REG(Instruction::Function::ADD, Register::SP, Register::SP, Register::IM));
		}
		return out;
	}

	std::vector<Instruction*> popForContext(Context* context) const {
		auto out = std::vector<Instruction*>();
		BYTE count = 0;
		for (BYTE reg = Register::GP_MAX; reg >= Register::GP_MIN; reg--) {
			if (context->registers[reg].isGenerallyUsed) {
				out.push_back(InstructionFactory::LD(reg, Register::SP, count + context->stackMemPointer));
				count++;
			}
		}
		if (count + context->stackMemPointer > 0) {
			auto adder = std::vector<Instruction*>({
				InstructionFactory::LI(Register::ZERO, count + context->stackMemPointer),
				InstructionFactory::REG(Instruction::Function::SUB, Register::SP, Register::SP, Register::IM)
				});
			INSERT_TO_BEGIN(out, adder);
		}
		return out;
	}

	ADDR addrPointer = STACK_SIZE;
	CADDR malloc() {
		return ++addrPointer;
	}
};

}
