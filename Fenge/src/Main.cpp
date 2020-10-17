#include "fgpch.h"

#include "Core/Log.h"
#include "Parser/Lexer.h"
#include "Parser/Parser.h"
#include "Compiler/Compiler.h"
#include "Compiler/InstructionFactory.h"

using namespace fenge;

std::string getComilerOutput(const std::string& input) {
	Lexer lexer = Lexer(input);
	LexerResult lexerResult = lexer.generateTokens();
	LOG(lexerResult.toString());
	if (lexerResult.error.isError()) {
		return lexerResult.toString();
	}
	Parser parser = Parser(lexerResult.tokens);
	ParserResult parserResult = parser.parse();
	LOG(parserResult.toString());
	delete parserResult.node;
	delete lexerResult.tokens.back();
	return "";
}

#define ASSERT(instruction, val) \
if (instruction.value() != val) { \
	LOG(std::hex << instruction.value()) \
} else { \
	LOG("subba") \
} \
LOG(std::hex << (int)instruction.getFunctionSafe()) 

void testInstructionFactory() {
	ASSERT(InstructionFactory::NOP(), 0x000000);
	ASSERT(InstructionFactory::REG(Instruction::Function::SHR, 0x3, 0x8, 0x9), 0x598031);
	ASSERT(InstructionFactory::LWI(Instruction::Function::EQ, 0x3, 0x8, 0xAB), 0xAB8132);
	ASSERT(InstructionFactory::UPI(Instruction::Function::EQ, 0x3, 0x8, 0xAB), 0xAB8133);
	ASSERT(InstructionFactory::LI(0x3, 0xABCD), 0xCDAB34);
	ASSERT(InstructionFactory::JMP(0x3, 0xABCD), 0xCDAB35);
	ASSERT(InstructionFactory::JMPC(Instruction::Function::EQ, 0x8, 0x9, 0xAB), 0xA981B6);
	ASSERT(InstructionFactory::LD(0x3, 0x8, 0xABC), 0xBC8A37);
	ASSERT(InstructionFactory::ST(0x8, 0x9, 0xABC), 0xB98AC8);
	ASSERT(InstructionFactory::CALL(0x3, 0xABCD), 0xCDAB39);
	ASSERT(InstructionFactory::RET(), 0x00000A);
}

int main() {
	testInstructionFactory();
	std::string input;
	while (true) {
		std::cout << "> " << std::flush;
		std::getline(std::cin, input);
		if (input == "q")
			break;
		std::cout << getComilerOutput(input) << std::endl;
	}
	return 0;
}
