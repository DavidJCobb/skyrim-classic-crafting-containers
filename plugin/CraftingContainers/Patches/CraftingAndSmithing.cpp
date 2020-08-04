#include "CraftingAndSmithing.h"
#include "ReverseEngineered/Forms/TESObjectREFR.h"
#include "ReverseEngineered/Forms/FormComponents/TESContainer.h"
#include "ReverseEngineered/Player/PlayerCharacter.h"
#include "ReverseEngineered/Systems/Inventory.h"
#include "ReverseEngineered/UI/Crafting.h"
#include "ReverseEngineered/Types.h"
#include "Services/ContainerHelper.h"
#include "skse/SafeWrite.h"

namespace Patches {
   namespace CraftingAndSmithing {
      static std::vector<RE::TESObjectREFR*> s_containers; // cached list
      void UpdateContainers() { // called by assembly; be careful if changing the function signature
         s_containers.clear();
         ContainerHelper::get_nearby_containers(s_containers);
      }
      //
      // In order to change how SmithingMenu and ConstructibleObjectMenu check whether the player has the 
      // items required to use a COBJ recipe, we have to patch BGSConstructibleObject::PlayerHasNeededItems. 
      // That function has three callers. Two are called in loops: a call for each COBJ being processed. It 
      // would be spectacularly wasteful to scan the loaded area for nearby containers during each call, so 
      // instead, we're going to also patch upstream from the three call sites, in advance of any loops, so 
      // that we can just get the containers once and cache the list of them.
      //
      // As it happens, BGSConstructibleObject::PlayerHasNeededItems is used by both ConstructibleObjectMenu 
      // and SmithingMenu, so sadly, I can't really keep the patches for those two menus neatly separated.
      //
      // We need similar upstream patches for the call to actually remove a crafting recipe's items from the 
      // player's inventory.
      //
      namespace CheckRequiredMaterials {
         namespace Prep {
            namespace Tempering_RebuildItemList {
               __declspec(naked) void Outer() {
                  _asm {
                     mov  eax, 0x004D9F40; // reproduce patched-over call to TESObjectREFR::CountItemTypes
                     call eax;             //
                     push eax; // protect
                     call UpdateContainers;
                     pop  eax; // restore
                     mov  edx, 0x00857390;
                     jmp  edx;
                  }
               }
               void Apply() {
                  WriteRelJump(0x0085738B, (UInt32)&Outer); // CraftingSubMenus::SmithingMenu::RebuildItemList+0x3B
               }
            }
            namespace Crafting_RebuildItemList {
               __declspec(naked) void Outer() {
                  _asm {
                     push ebx;                         // reproduce patched-over instructions
                     mov  ebx, dword ptr [esp + 0x34]; //
                     call UpdateContainers;
                     mov  eax, 0x0084E26C;
                     jmp  eax;
                  }
               }
               void Apply() {
                  WriteRelJump(0x0084E267, (UInt32)&Outer); // CraftingSubMenus::ConstructibleObjectMenu::Subroutine0084E220+0x47
               }
            }
            namespace Crafting_OnBeforeCraft {
               //
               // This BGSConstructibleObject::PlayerHasNeededItems call site isn't actually part of a loop, so 
               // let's just wrap the call itself.
               //
               // This call occurs just before the game decides to show a confirmation prompt asking the player 
               // whether they're sure they want to craft their selected item. Presumably this is a failsafe used 
               // in case the player's inventory has changed between the time ConstructibleObjectMenu generated 
               // its list of COBJ recipes and the time the player actually chooses to try and craft one.
               //
               __declspec(naked) void _stdcall Outer() {
                  _asm {
                     push ecx; // protect
                     call UpdateContainers;
                     pop  ecx; // restore
                     mov  eax, 0x00491690; // jump to BGSConstructibleObject::PlayerHasNeededItems
                     jmp  eax;
                  }
               }
               void Apply() {
                  WriteRelCall(0x00858A90, (UInt32)&Outer); // CraftingSubMenus::ConstructibleObjectMenu::Subroutine00858A30+0x60
               }
            }
            //
            void Apply() {
               Tempering_RebuildItemList::Apply();
               Crafting_RebuildItemList::Apply();
               Crafting_OnBeforeCraft::Apply();
            }
         }
         //
         bool _stdcall Check(RE::TESContainer* requirements) {
            auto*    player = *RE::g_thePlayer;
            uint32_t size   = requirements->numEntries;
            for (uint32_t i = 0; i < size; ++i) {
               auto*    entry  = requirements->entries[i]; // Bethesda doesn't check for nullptr so neither will I
               uint32_t needed = entry->count;
               uint32_t remove = ContainerHelper::non_quest_item_count(player, entry->form);
               if (remove >= needed)
                  continue;
               needed -= remove;
               for (auto* container : s_containers) {
                  remove = ContainerHelper::non_quest_item_count(container, entry->form);
                  if (remove >= needed) {
                     needed = 0;
                     break;
                  }
                  needed -= remove;
               }
               //
               // Done checking all containers. If they didn't all, in total, have the amount of the item 
               // needed, then fail.
               //
               if (needed)
                  return false;
            }
            return true;
         }
         __declspec(naked) void Outer_OnBeforeCraft() {
            //
            // Both the crafting and tempering menus double-check whether the player is allowed to act 
            // on a recipe at the instant the player attempts to do so. However, the crafting menu's 
            // call to BGSConstructibleObject::PlayerHasNeededItems is inlined, so we have to patch it 
            // a little differently.
            //
            // This patch will replace a call to TESContainer::InventoryHasAllOfMyItems where the 
            // "container" in question is the COBJ's list of required materials.
            //
            _asm {
               push ecx;
               call Check; // stdcall
               retn 4;
            }
         }
         __declspec(naked) void Outer() {
            //
            // This is used to just straight-up replace BGSConstructibleObject::PlayerHasNeededItems 
            // as a function.
            //
            _asm {
               lea  ecx, [ecx + 0x14]; // ecx = &this->requirements;
               push ecx;
               call Check; // stdcall
               retn;
            }
         }
         void Apply() {
            Prep::Apply();
            //
            WriteRelJump(0x00491690, (UInt32)&Outer); // replace BGSConstructibleObject::PlayerHasNeededItems
            SafeWrite16 (0x00491690 + 5, 0x9090);
            SafeWrite8  (0x00491690 + 7, 0x90);
            //
            WriteRelCall(0x00491703, (UInt32)&Outer_OnBeforeCraft); // CraftingSubMenus::ConstructibleObjectMenu::EntryData::AttemptCraft+0x33
         }
      }
      namespace ConsumeRequiredMaterials {
         BOOL Functor(RE::TESForm* item, uint32_t count) {
            RE::ref_handle handle;
            //
            auto     player = (*RE::g_thePlayer);
            uint32_t amount = ContainerHelper::non_quest_item_count(player, item);
            uint32_t remove;
            if (amount > 0) {
               remove = count > amount ? amount : count;
               player->Unk_56(handle, item, remove, 0, nullptr, nullptr, false, false);
               if (count <= amount)
                  return true;
               count -= amount;
            }
            for (auto* container : s_containers) {
               amount = ContainerHelper::non_quest_item_count(container, item);
               remove = count > amount ? amount : count;
               container->Unk_56(handle, item, remove, 0, nullptr, nullptr, false, false);
               if (count <= amount)
                  return true;
               count -= amount;
            }
            if (count > 0)
               _MESSAGE("[WARNING] SmithingMenu: Tempering failed to consume %u of ingredient %08X. Was a container modified?", count, item->formID);
            return true;
         }
         void Apply() {
            // MOV DWORD PTR [ESP + 0x10], REPLACE_THIS_DWORD;
            SafeWrite32(0x0085757F + 4, (UInt32)&Functor); // CraftingSubMenus::SmithingMenu::TemperCurrentItem+0x5F+0x04
            SafeWrite32(0x00491715 + 4, (UInt32)&Functor); // CraftingSubMenus::ConstructibleObjectMenu::EntryData::AttemptCraft+0x45+0x04
         }
      }
      namespace DisplayRequiredMaterials {
         void _stdcall Inner(RE::TESContainer* cont, RE::TESContainer::get_craft_requirements_t& reqs) {
            auto     player = (*RE::g_thePlayer);
            uint32_t size   = cont->numEntries;
            for (uint32_t i = 0; i < size; ++i) {
               auto*    entry  = cont->entries[i];
               uint32_t amount = ContainerHelper::non_quest_item_count(player, entry->form);
               for (auto* container : s_containers)
                  amount += ContainerHelper::non_quest_item_count(container, entry->form);
               auto& element = reqs.results.append();
               element.name     = RE::GetFormName(entry->form);
               element.count    = amount;
               element.required = entry->count;
            }
         }
         __declspec(naked) void _stdcall Outer(RE::TESContainer::get_craft_requirements_t&) {
            //
            // We are replacing calls to TESContainer::GetCraftRequirementsForDisplay.
            //
            _asm {
               mov  eax, dword ptr [esp + 0x4];
               mov  eax, dword ptr [eax];
               push eax;
               push ecx;
               call Inner; // stdcall
               retn 4;
            }
         }
         void Apply() {
            WriteRelCall(0x00856FA5, (UInt32)&Outer); // CraftingSubMenus::SmithingMenu::UpdateDisplayedRequirements+0x95
            WriteRelCall(0x00850CB1, (UInt32)&Outer); // CraftingSubMenus::ConstructibleObjectMenu::UpdateDisplayedRequirements+0x91
         }
      }
      //
      void Apply() {
         CheckRequiredMaterials::Apply();
         ConsumeRequiredMaterials::Apply();
         DisplayRequiredMaterials::Apply();
      }
   }
}