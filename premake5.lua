include "./vendor/premake/premake_customization/solution_items.lua"

workspace "Fenge"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

	solution_items
	{
		".editorconfig"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Fenge"
	location "Fenge"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	pchheader "fgpch.h"
	pchsource "Fenge/src/fgpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src"
	}

	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"FG_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "FG_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "FG_RELEASE"
		optimize "On"
