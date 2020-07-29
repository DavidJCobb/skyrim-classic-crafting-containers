#include "INI.h"
#include "helpers/strings.h"
#include <algorithm>
#include <fstream>
#include "skse/Utilities.h"

//#include "Shlwapi.h" // for PathFileExists // doesn't work anyway; link error

namespace CraftingContainers {
   namespace INI {
      const std::string& _GetPath() {
         static std::string path;
         if (path.empty()) {
            std::string runtimePath = GetRuntimeDirectory();
            if (!runtimePath.empty()) {
               path = runtimePath;
               path += "Data\\SKSE\\Plugins\\CraftingContainers.ini";
            }
         }
         return path;
      };
      extern cobb::ini::file& get() {
         static cobb::ini::file instance = cobb::ini::file(_GetPath().c_str());
         return instance;
      }
      //
      // INI SETTING DEFINITIONS.
      //
      // The macro used here intentionally differs from the one used in the H file. Keep the namespaces 
      // synchronized between the two files.
      //
      #define COBB_MAKE_INI_SETTING(name, category, value) extern cobb::ini::setting name = cobb::ini::setting(get, #name, category, value);
      namespace General {
         COBB_MAKE_INI_SETTING(fMaxContainerDistance, "General", 768.0F);
      };
      #undef COBB_MAKE_INI_SETTING
   }
};