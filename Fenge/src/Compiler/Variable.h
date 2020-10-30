#pragma once

namespace fenge {

class Variable {
public:
	explicit Variable(std::string name, Token::Keyword datatype)
		: name(name), datatype(datatype) { }
	Variable(std::string name, Token::Keyword datatype, CBYTE reg)
		: reg(reg), name(name), datatype(datatype) { }
	Variable(std::string name, Token::Keyword datatype, CADDR addr)
		: name(name), datatype(datatype), addr(addr) { }
	Variable(std::string name, Token::Keyword datatype, CBYTE reg, CADDR addr)
		: reg(reg), name(name), datatype(datatype), addr(addr) { }

	static Variable& notFound() {
		return Variable("", Token::Keyword::NO_KEYWORD);
	}

	bool exists() const {
		return this != nullptr && datatype != Token::Keyword::NO_KEYWORD;
	}

	std::string name;
	Token::Keyword datatype;
	BYTE reg = 0x0; // 0x0 means not in reg
	CADDR addr = 0;
};

}
