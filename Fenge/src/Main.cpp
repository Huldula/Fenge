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
	if (parserResult.error.isError())
		LOG(parserResult.toString());

	Compiler compiler = Compiler();
	CompilerResult compilerResult = compiler.compile(parserResult.node);
	LOG(compilerResult.toOutputString());
	LOG(compilerResult.toReadableString());
	//LOG(compilerResult.toString());

	delete parserResult.node;
	lexerResult.deleteTokens();
	return "";
}

#define ASSERT(instruction, val) \
if (instruction.value() != val) { \
	LOG(std::hex << instruction.value()) \
} else { \
	LOG("subba") \
} \
//LOG(std::hex << (int)instruction.getFunctionSafe()) 

void consoleInput() {
	std::string input;
	while (true) {
		std::cout << "> " << std::flush;
		std::getline(std::cin, input);
		if (input == "q")
			break;
		std::cout << getComilerOutput(input) << std::endl;
	}
}


void fileInput(const std::string& fileName) {
	std::ifstream file;
	std::stringstream stream;
	std::string str;
	file.open(fileName);
	while (getline(file, str)) {
		LOG(str);
		stream << str;
	}
	file.close();
	std::cout << getComilerOutput(stream.str()) << std::endl;
}

int main() {
	//consoleInput();
	fileInput("../examples/bouncing_ball.fluff");
	//fileInput("../examples/test.fluff");
	return 0;
}
