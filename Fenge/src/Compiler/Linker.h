#pragma once

namespace fenge {

class Linker {
public:

	static void link(std::vector<Instruction*>& instructions, std::vector<Function>& functions) {
		for (Function& func : functions) {
			CADDR funcAddr = instructions.size();
			if (func.calledFrom.size() > 0)
				instructions.insert(instructions.end(), func.instructions.begin(), func.instructions.end());
			for (Instruction* instr : func.calledFrom) {
				instr->setCallAddr(funcAddr);
			}
		}
	}
};

}

