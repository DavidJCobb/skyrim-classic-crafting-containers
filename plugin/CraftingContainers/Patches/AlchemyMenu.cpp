#include "AlchemyMenu.h"
#include "ReverseEngineered/Forms/TESObjectREFR.h"
#include "ReverseEngineered/Player/PlayerCharacter.h"
#include "ReverseEngineered/UI/Crafting.h"
#include "Services/ContainerHelper.h"
#include "skse/SafeWrite.h"

namespace Patches {
   namespace AlchemyMenu {
      namespace PopulateItemList {
         void _stdcall Inner(RE::CraftingSubMenus::AlchemyMenu* menu) {
            _MESSAGE("AlchemyMenu: PopulateItemList: Bringing in the player's ingredients...");
            menu->ImportIngredientsFrom(*RE::g_thePlayer);
            _MESSAGE("AlchemyMenu: PopulateItemList: Got %u ingredients.", menu->availableIngredients.count);
            //
            std::vector<RE::TESObjectREFR*> containers;
            ContainerHelper::get_nearby_containers(containers);
            _MESSAGE("AlchemyMenu: ConsumeItem: There are %u containers available to try.", containers.size());
            for (auto* container : containers) {
               _MESSAGE("AlchemyMenu: PopulateItemList: Bringing in ingredients from container %08X...", container->formID);
               //
               // TODO: This will import owned items in unowned containers, which is... extremely not ideal. 
               // We want to do that if the container is the player, since they should be able to use anything 
               // they've stolen, but we extremely don't want to do it otherwise.
               //
               menu->ImportIngredientsFrom(container);
               _MESSAGE("AlchemyMenu: PopulateItemList: We have %u ingredients...", menu->availableIngredients.count);
               //
               // TODO: There is another problem with just running this in a loop, on multiple containers: if 
               // more than one container (the player included) contains the same type of item, then instead 
               // of those listings being merged, they'll actually be separate. For example, if there are 13 
               // Elves Ear in a container and 2 in the player's inventory, they won't see a single listing 
               // of 15 Elves Ear in the Alchemy menu, but rather two separate listings of 13 and 2 respect-
               // ively.
               //
               // At first, this seems like a minor cosmetic issue. However, when you brew a potion, if any 
               // of the ingredients that you used had multiple listings in the menu, then the menu will be 
               // unable to preserve your selection (i.e. all ingredients will be deselected, which is not 
               // great).
               //
            }
            _MESSAGE("AlchemyMenu: PopulateItemList: Done.");
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
         RE::BSUntypedPointerHandle& _stdcall Inner(RE::BSUntypedPointerHandle& out, RE::TESForm* item, int32_t count, uint32_t arg4, RE::BaseExtraList* extra, RE::TESObjectREFR* transferTo, bool arg7, bool arg8) {
            _MESSAGE("AlchemyMenu: ConsumeItem: We want to remove %u of %08X; let's do the player first.", count, item->formID);
            auto     player = (*RE::g_thePlayer);
            uint32_t amount = player->GetItemCountFast(item);
            uint32_t remove;
            if (amount > 0) {
               remove = count > amount ? amount : count;
               player->Unk_56(out, item, remove, arg4, extra, transferTo, arg7, arg8);
               if (count <= amount) {
                  _MESSAGE("AlchemyMenu: ConsumeItem: Done.");
                  return out;
               }
               count -= amount;
            }
            _MESSAGE("AlchemyMenu: ConsumeItem: There are %u items left to remove.", count);
            std::vector<RE::TESObjectREFR*> containers;
            ContainerHelper::get_nearby_containers(containers);
            _MESSAGE("AlchemyMenu: ConsumeItem: There are %u containers available to try.", containers.size());
            for (auto* container : containers) {
               amount = container->GetItemCountFast(item);
               remove = count > amount ? amount : count;
               container->Unk_56(out, item, remove, arg4, extra, transferTo, arg7, arg8);
               if (count <= amount) {
                  _MESSAGE("AlchemyMenu: ConsumeItem: Done.");
                  return out;
               }
               count -= amount;
               _MESSAGE("AlchemyMenu: ConsumeItem: There are %u items left to remove.", count);
            }
            _MESSAGE("AlchemyMenu: ConsumeItem: Done.");
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