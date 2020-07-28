#pragma once
#include "skse/GameFormComponents.h"
#include "ReverseEngineered/Shared.h""

namespace RE {
   class TESForm;

   class TESContainer : public BaseFormComponent { // sizeof == 0xC
      public:
         struct Entry {
            struct Data {
               UInt32 unk00;
               UInt32 unk04;
               float  unk08;
            };
            //
            UInt32   count;
            TESForm* form;
            Data*    data; // extra data?
         };

         Entry** entries;    // 04
         UInt32  numEntries; // 08

         MEMBER_FN_PREFIX(TESContainer);
         DEFINE_MEMBER_FN(Contains, bool, 0x0044F220, TESForm* item);

         using for_each_functor_t = BOOL(*)(TESForm*, uint32_t count); // return any value other than 1 to stop iterating
         DEFINE_MEMBER_FN(ForEach, void, 0x00491200, for_each_functor_t);
   };
   namespace TESContainerForEachFunctors {
      static DEFINE_SUBROUTINE(BOOL, RemoveAllOfMyItemsFromPlayer, 0x00491100, TESForm*, uint32_t);
   }
}
