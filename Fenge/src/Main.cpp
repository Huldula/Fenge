#include "fgpch.h"

#include "Core/Log.h"
#include "Parser/Lexer.h"
#include "Parser/Parser.h"

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

int main() {
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
