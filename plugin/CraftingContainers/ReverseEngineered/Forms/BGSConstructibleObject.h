#pragma once
#include <cstdint>
#include "TESForm.h"

namespace RE {// 30
   class TESFurniture;
   //
   class BGSConstructibleObject : public TESForm {
      //
      // This form is used to represent both crafting recipes (wherein materials are expended to 
      // create a new item) and tempering recipes (wherein materials are expended to improve an 
      // existing item). How does the game tell the two apart? Each recipe can specify the type 
      // of workbench that is allowed to use it (by way of a BGSKeyword*); in turn, different 
      // workbenches activate different menus, which in turn treat the recipes differently.
      //
      // For a tempering recipe, the (createdObject) must be the item to be improved.
      //
      public:
         static constexpr form_type_t form_type = form_type::constructible_object;
         //
         // Members:
         TESContainer container;     // 14 // not inherited
         void*        conditions;    // 20 // linked list
         TESForm*     createdObject; // 24
         BGSKeyword*  workbenchKeyword; // 28
         uint16_t     quantity;      // 2C
         uint16_t     pad2E;         // 2E
         //
         MEMBER_FN_PREFIX(BGSConstructibleObject);
         DEFINE_MEMBER_FN(CanCraftAtWorkbench, bool, 0x00491050, TESFurniture*, bool checkConditions);
   };
}