#include "Inventory.h"
#include "ReverseEngineered/Types.h"
#include "skse/GameAPI.h"

namespace RE {
   UInt8 InventoryEntryData::GetSoulSize() const {
      auto form = this->type;
      if (!type || type->formType != kFormType_SoulGem)
         return 0;
      UInt8 size = ((TESSoulGem*)type)->soulSize;
      if (size)
         return size;
      if (!this->extendDataList)
         return 0;
      auto extra = &((RE::tList<RE::BaseExtraList>*)this->extendDataList)->items;
      for (; extra; extra = extra->next) {
         auto list = extra->data;
         if (!list)
            continue;
         size = CALL_MEMBER_FN(list, GetExtraSoulSize)();
         if (size)
            return size;
      }
      return 0;
   };

   namespace InventoryUtils {
   }
}