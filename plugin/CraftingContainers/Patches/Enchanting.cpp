#include "Enchanting.h"
#include "ReverseEngineered/Forms/TESObjectREFR.h"
#include "ReverseEngineered/Player/PlayerCharacter.h"
#include "ReverseEngineered/Systems/Inventory.h"
#include "ReverseEngineered/UI/Crafting.h"
#include "Services/ContainerHelper.h"
#include "skse/SafeWrite.h"

namespace Patches {
   namespace Enchanting {
      namespace PopulateItemList {
         void _stdcall Inner(RE::CraftingSubMenus::EnchantConstructMenu* menu) {
            std::vector<RE::TESObjectREFR*> containers;
            ContainerHelper::get_nearby_containers(containers);
            for (auto* container : containers)
               menu->ImportSoulGemsFrom(container, true);
         }
         __declspec(naked) void Outer() {
            _asm {
               mov  eax, 0x00A49B40; // reproduce patched-over call to BSTArrayCount::BSTArrayCount
               call eax;             // 
               push ebx;
               call Inner; // stdcall
               mov  eax, 0x00850364;
               jmp  eax;
            }
         }
         void Apply() {
            WriteRelJump(0x0085035F, (UInt32)&Outer);
         }
      }
      namespace ConsumeItem {
         RE::BSUntypedPointerHandle& _stdcall Inner(RE::BSUntypedPointerHandle& out, RE::TESForm* item, int32_t count, uint32_t arg4, RE::BaseExtraList* extra, RE::TESObjectREFR* transferTo, bool arg7, bool arg8) {
            _MESSAGE("Consuming %u used soul gems of type %08X...", count, item->formID);
            auto     player = (*RE::g_thePlayer);
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
               amount = ContainerHelper::non_quest_item_count(container, item);
               remove = count > amount ? amount : count;
               container->Unk_56(out, item, remove, arg4, extra, transferTo, arg7, arg8);
               if (count <= amount)
                  return out;
               count -= amount;
            }
            if (count > 0)
               _MESSAGE("[WARNING] EnchantConstructMenu: Crafting failed to consume %u of soul gem %08X. Was a container modified?", count, item->formID);
            return out;
         }
         __declspec(naked) void _stdcall Outer() {
            _asm {
               lea  eax, [esp + 0x34]; // reproduce patched-over instructions
               push eax;              //
               call Inner;
               mov  edx, 0x00852F4D;
               jmp  edx;
            }
         }
         void Apply() {
            WriteRelJump(0x00852F46, (UInt32)&Outer);
         }
      }
      //
      void Apply() {
         PopulateItemList::Apply();
         ConsumeItem::Apply();
      }
   }
}