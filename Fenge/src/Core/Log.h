#pragma once

#include <iostream>

#ifdef FG_DEBUG
	#define LOG(x) std::cout << x << "\n";
#else
	#define LOG(x)
#endif