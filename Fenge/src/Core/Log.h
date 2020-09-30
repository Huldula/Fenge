#pragma once

#include <iostream>

#ifdef FG_DEBUG
	#define LOG(x) std::cerr << x << "\n";
#else
	#define LOG(x)
#endif
