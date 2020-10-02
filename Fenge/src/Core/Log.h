#pragma once

#include "fgpch.h"

#ifdef FG_DEBUG
	#define LOG(x) std::cerr << x << "\n";
#else
	#define LOG(x)
#endif
