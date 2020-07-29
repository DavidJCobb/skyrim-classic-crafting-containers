#pragma once
#include "helpers/ini.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace CraftingContainers {
   namespace INI {
      extern cobb::ini::file& get();
      //
      // INI SETTING DEFINITIONS.
      //
      // The macro used here intentionally differs from the one used in the CPP file. Keep the namespaces 
      // synchronized between the two files.
      //
      #define COBB_MAKE_INI_SETTING(name, category, value) extern cobb::ini::setting name;
      namespace General {
         COBB_MAKE_INI_SETTING(fMaxContainerDistance, "General", 768.0F);
      };
      #undef COBB_MAKE_INI_SETTING
   };
};