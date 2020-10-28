#pragma once

namespace fenge {

class Helper {
public:
	static std::string toHexString(const int i) {
		std::stringstream stream;
		stream << std::hex << i;
		return std::string(stream.str());
	}

	static std::string toHexString(const int i, const int length) {
		std::stringstream stream;
		stream << std::hex << i;
		std::string out = std::string(stream.str());
		while (out.length() < length) {
			out = std::string("0") + out;
		}
		return out;
	}
};


}
