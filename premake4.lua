-- create Makefile with "./premake4 gmake"

solution "homegear-addon"
   configurations { "Release", "Debug", "Profiling" }

   configuration { "native", "linux", "gmake" }
      defines
      {
         "FORTIFY_SOURCE=2",
      }
      linkoptions { "-Wl,-rpath=/lib/homegear", "-Wl,-rpath=/usr/lib/homegear", "-Wl,-soname,libhomegear-addon.so.0" }
   
   project "homegear-addon"
      kind "SharedLib"
      language "C++"
      files { "*.h", "*.cpp" }
      files { "./*.h", "./*.cpp", "./HelperFunctions/*.h", "./HelperFunctions/*.cpp", "./Encoding/*.h", "./Encoding/*.cpp" }
      linkoptions { "-l pthread" }
      buildoptions { "-Wall", "-std=c++11", "-fPIC" }
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
		 libdirs { "./lib/Debug" }
         targetdir "bin/Debug"
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }
		 libdirs { "./lib/Release" }
         targetdir "bin/Release"

      configuration "Profiling"
         defines { "NDEBUG" }
         flags { "Optimize", "Symbols" }
         libdirs { "./lib/Profiling" }
         targetdir "bin/Profiling"
         buildoptions { "-pg" }
         linkoptions { "-pg" }
