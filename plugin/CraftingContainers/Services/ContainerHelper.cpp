#include "ContainerHelper.h"
#include "ReverseEngineered/Forms/TESObjectCELL.h"
#include "ReverseEngineered/Forms/TESObjectREFR.h"
#include "ReverseEngineered/Player/PlayerCharacter.h"
#include "ReverseEngineered/Systems/GameData.h"
#include "ReverseEngineered/Systems/Inventory.h"
#include "ReverseEngineered/ExtraData.h"
#include <algorithm>

namespace {
   void _searchCell(RE::TESObjectCELL* cell, std::vector<RE::TESObjectREFR*>& out) {
      auto player = *RE::g_thePlayer;
      //
      CALL_MEMBER_FN(cell, CellRefLockEnter)();
      UInt32 count = cell->objectList.count;
      for (UInt32 i = 0; i < count; i++) {
         RE::TESObjectREFR* refr = nullptr;
         cell->objectList.GetNthItem(i, refr);
         if (!refr)
            continue;
         auto base = refr->baseForm;
         if (!base || base->formType != RE::form_type::container)
            continue;
         if (CALL_MEMBER_FN(refr, GetDistanceSquared)(player, true, false) > 589824.0F) // max distance of 768 world units (TODO: make configurable)
            continue;
         auto owner = (RE::TESForm*) CALL_MEMBER_FN((RE::BaseExtraList*)&refr->extraData, GetExtraOwnership)();
         if (owner && owner != player) {
            //
            // TODO: if (owner) is a faction, then check whether the player is a member.
            //
            continue;
         }
         out.push_back(refr);
      }
      CALL_MEMBER_FN(cell, CellRefLockExit)();
   }
}
namespace ContainerHelper {
   void get_nearby_containers(std::vector<RE::TESObjectREFR*>& out) {
      auto player = (*RE::g_thePlayer);
      auto cell   = player->parentCell;
      _searchCell(cell, out);
      if (!(cell->unk2C & RE::TESObjectCELL::kCellFlag_IsInterior)) {
         UInt32 count = 0;
         RE::TESObjectCELL** cells = (RE::TESObjectCELL**) ((RE::TES*)*g_TES)->CopyGridCells(&count);
         if (!cells)
            return;
         for (UInt32 i = 0; i < count; i++) {
            auto current = cells[i];
            if (current && current != cell)
               _searchCell(current, out);
         }
         free(cells);
      }
      std::sort(out.begin(), out.end(), [player](RE::TESObjectREFR* a, RE::TESObjectREFR* b) {
         auto dist_a = CALL_MEMBER_FN(a, GetDistanceSquared)(player, true, false);
         auto dist_b = CALL_MEMBER_FN(b, GetDistanceSquared)(player, true, false);
         return dist_a < dist_b;
      });
   }
   int32_t non_quest_item_count(RE::TESObjectREFR* refr, RE::TESForm* item) {
      auto* inventory = RE::GetInventory(refr);
      if (inventory) {
         auto filter = RE::InventoryUtils::QuestItemFilter();
         return CALL_MEMBER_FN(inventory, GetMatchingItemCount)(item, filter);
      }
      return 0;
   }
}