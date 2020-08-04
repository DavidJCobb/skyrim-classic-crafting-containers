#include "AlchemyMenu.h"
#include "ReverseEngineered/Forms/TESObjectREFR.h"
#include "ReverseEngineered/Player/PlayerCharacter.h"
#include "ReverseEngineered/UI/Crafting.h"
#include "ReverseEngineered/Types.h"
#include "Services/ContainerHelper.h"
#include "skse/SafeWrite.h"

namespace Patches {
   namespace AlchemyMenu {
      namespace PopulateItemList {
         void _stdcall Inner(RE::CraftingSubMenus::AlchemyMenu* menu) {
            menu->ImportIngredientsFrom(*RE::g_thePlayer);
            //
            std::vector<RE::TESObjectREFR*> containers;
            ContainerHelper::get_nearby_containers(containers);
            for (auto* container : containers) {
               //
               // The container getter ignores any containers that are owned by people or groups other than 
               // the player. You might think that we need to strip out owned items within unowned containers, 
               // but we don't. Bethesda usually doesn't and AFAIK technically can't set ownership on individual 
               // items inside of an unowned container; the only possible exception I can think of is something 
               // spawned via a quest alias, but the AlchemyMenu rejects quest items anyway.
               //
               menu->ImportIngredientsFrom(container, true);
            }
         }
         __declspec(naked) void _stdcall Outer() {
            _asm {
               add  esp, 0x8; // discard arguments
               push ebp;
               call Inner; // stdcall
               mov  eax, 0x00850587;
               jmp  eax;
            }
         }
         void Apply() {
            WriteRelJump(0x008504D5, (UInt32)&Outer);
         }
      }
      namespace ConsumeItem {
         RE::ref_handle& _stdcall Inner(RE::ref_handle& out, RE::TESForm* item, int32_t count, uint32_t arg4, RE::BaseExtraList* extra, RE::TESObjectREFR* transferTo, bool arg7, bool arg8) {
            auto     player = (*RE::g_thePlayer);
            //uint32_t amount = player->GetItemCountFast(item);
            uint32_t amount = ContainerHelper::non_quest_item_count(player, item);
            uint32_t remove;
            if (amount > 0) {
               remove = count > amount ? amount : count;
               player->Unk_56(out, item, remove, arg4, extra, transferTo, arg7, arg8);
               if (count <= amount)
                  return out;
               count -= amount;
            }
            std::vector<RE::TESObjectREFR*> containers;
            ContainerHelper::get_nearby_containers(containers);
            for (auto* container : containers) {
               //amount = container->GetItemCountFast(item);
               amount = ContainerHelper::non_quest_item_count(container, item);
               remove = count > amount ? amount : count;
               container->Unk_56(out, item, remove, arg4, extra, transferTo, arg7, arg8);
               if (count <= amount)
                  return out;
               count -= amount;
            }
            if (count > 0)
               _MESSAGE("[WARNING] AlchemyMenu: Crafting failed to consume %u of ingredient %08X. Was a container modified?", count, item->formID);
            return out;
         }
         __declspec(naked) void _stdcall Outer() {
            _asm {
               lea edx, [esp + 0x44]; // reproduce patched-over instructions
               push edx;              //
               call Inner;
               mov  edx, 0x0085562F;
               jmp  edx;
            }
         }
         void Apply() {
            WriteRelJump(0x00855628, (UInt32)&Outer);
         }
      }
      //
      void Apply() {
         PopulateItemList::Apply();
         ConsumeItem::Apply();
      }
   }
}