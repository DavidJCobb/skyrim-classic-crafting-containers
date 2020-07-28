#pragma once
#include <cassert>
#include <cstdint>
#include <vector>
#include "helpers/rtti.h"

namespace Patches {
   namespace Exploratory {
      namespace DumpRTTIsByVTBLs {
         uint32_t vtbls[] = {
            0x010A6234, // AttackAnimationArrayMap // trying to nail down info on the BSTHashMap
            0x010841B4, // TESObjectACTI // just out of curiosity
         };
         //
         void Apply() {
            _MESSAGE("DUMPING RTTI INFORMATION FOR VTBLs...");
            _MESSAGE("Base class offsets may be slightly incorrect since we don't have \"live\" objects to follow.");
            for (size_t i = 0; i < std::extent<decltype(vtbls)>::value; ++i) {
               auto& col = cobb::rtti::complete_object_locator::get_from_vtbl(vtbls[i]);
               if (!col.typeinfo) {
                  _MESSAGE(" - Unknown");
                  continue;
               }
               _MESSAGE(" - %s", col.typeinfo->name());
               //
               auto* hierarchy = col.hierarchy;
               if (!hierarchy)
                  continue;
               std::string indent;
               std::vector<uint8_t> nesting;
               for (uint32_t j = 0; j < hierarchy->base_class_count; ++j) {
                  auto* base = hierarchy->base_classes[j];
                  auto* ti   = base ? base->typeinfo : nullptr;
                  if (ti)
                     _MESSAGE("%s   +%08X: %s", indent.c_str(), base->pmd.member, ti->name());
                  else
                     _MESSAGE("%s   +????????: Unknown", indent.c_str());
                  //
                  if (nesting.size()) {
                     for (auto& v : nesting)
                        --v;
                     while (nesting.size() && !nesting.back()) {
                        nesting.pop_back();
                        indent.resize(indent.size() - 3);
                     }
                  }
                  if (base->contained_base_count) {
                     nesting.push_back(base->contained_base_count);
                     indent += "   ";
                  }
               }
            }
            _MESSAGE("DONE.");
         }
      }
   }
}