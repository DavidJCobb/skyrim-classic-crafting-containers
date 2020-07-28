#pragma once
#include <vector>

namespace RE {
   class TESForm;
   class TESObjectREFR;
   namespace InventoryUtils {
      class QuestItemFilter;
   }
}

namespace ContainerHelper {
   void get_nearby_containers(std::vector<RE::TESObjectREFR*>& out); // sorted by distance, too; nearest containers to the player are first
   int32_t non_quest_item_count(RE::TESObjectREFR* refr, RE::TESForm* item);
}