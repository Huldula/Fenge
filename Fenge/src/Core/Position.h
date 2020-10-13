#pragma once

#include "fgpch.h"

#include "Macros.h"

namespace fenge {

class Position {
private:
	int index_;
	int line_;
	int col_;
public:
	Position() : index_(-1), line_(0), col_(-1) { };
	Position(int index) : index_(index), line_(0), col_(-1) { };
	Position(const int index, const int line, const int col) : index_(index), line_(line), col_(col) { };

	void advance() {
		index_++;
	}

	void advance(const char currentChar) {
		index_++;
		if (IS_NEW_LINE_CHAR(currentChar)) {
			col_ = 0;
			line_++;
		} else {
			col_++;
		}
	}

	int index() const {
		return index_;
	}
};

} // namespace fenge
