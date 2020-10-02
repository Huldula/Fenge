#include "fgpch.h"

#include "Core/Log.h"
#include "Parser/Lexer.h"

using namespace fenge;

int main() {
	std::string input;
	while (true) {
		std::cout << "> " << std::flush;
		std::getline(std::cin, input);
		if (input == "q")
			break;
		Lexer lexer = Lexer(input);
		LexerResult* result = lexer.generateTokens();
		LOG(result->toString());
		delete result;
	}
	return 0;
}
