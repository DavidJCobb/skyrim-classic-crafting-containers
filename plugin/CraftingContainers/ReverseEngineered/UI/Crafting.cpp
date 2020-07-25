#include "Crafting.h"
#include "ReverseEngineered/Forms/TESObjectREFR.h"
#include "ReverseEngineered/ExtraData.h"

namespace RE {
   namespace CraftingSubMenus {
      void AlchemyMenu::ImportIngredientsFrom(TESObjectREFR* container) {
         auto count = CALL_MEMBER_FN(container, CountItemTypes)(false, true);
         for (uint32_t index = 0; index < count; ++index) {
            auto entry = CALL_MEMBER_FN(container, GetInventoryEntryAt)(index, false);
            if (!entry)
               continue;
            if (entry && entry->type->formType == 0x1E && !CALL_MEMBER_FN(entry, IsQuestObject)()) {
               RE::CraftingSubMenus::AlchemyMenu::EntryData data;
               data.ingredient = entry;
               //
               bool esp13;
               bool esp20 = false;
               DEFINE_SUBROUTINE(int32_t, _find, 0x0084C710, decltype(availableIngredients)&, RE::CraftingSubMenus::AlchemyMenu::EntryData*, uint32_t compareFunctor, bool& out);
               auto eax = _find(this->availableIngredients, &data, 0x0084D770, esp13);
               if (eax != -1)
                  CALL_MEMBER_FN(&this->availableIngredients, Subroutine0084EE40)(eax, &data);
               if (!data.ingredient)
                  continue;
               entry = data.ingredient;
            }
            entry->Delete();
            entry = nullptr;
         }
      }
   }
}