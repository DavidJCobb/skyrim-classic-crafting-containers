#pragma once
#include <vector>

namespace RE {
   class TESObjectREFR;
}

namespace ContainerHelper {
   void get_nearby_containers(std::vector<RE::TESObjectREFR*>& out); // sorted by distance, too; nearest containers to the player are first
}