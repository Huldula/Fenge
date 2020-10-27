#pragma once

namespace fenge {

class Register {
public:
	bool isOccupied = false;
	bool isGenerallyUsed = false;
	Variable* containedVar = nullptr;

	static CBYTE ZERO = 0x0;
	static CBYTE BP = 0x1;
	static CBYTE IM = 0x2;
	static CBYTE RET = 0x3;
	static CBYTE ARG_MIN = 0x3;
	static CBYTE ARG_MAX = 0x7;
	static CBYTE GP_MIN = 0x8;
	static CBYTE GP_MAX = 0xE;

	static CBYTE SP = 0xF; // temporary
};

}
