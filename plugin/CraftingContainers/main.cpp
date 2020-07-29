#include "skse/PluginAPI.h"		// super
#include "skse/skse_version.h"	// What version of SKSE is running?
#include "skse/SafeWrite.h"
#include "skse/GameAPI.h"
#include <shlobj.h>				// CSIDL_MYCODUMENTS
#include <psapi.h>  // MODULEINFO, GetModuleInformation
#pragma comment( lib, "psapi.lib" ) // needed for PSAPI to link properly
#include <string>

#include "Services/INI.h"
#include "Patches/Exploratory.h"
#include "Patches/AlchemyMenu.h"
#include "Patches/CraftingAndSmithing.h"
#include "Patches/Enchanting.h"

#include "skse/GameRTTI.h"
#include "skse/GameObjects.h"

PluginHandle g_pluginHandle = kPluginHandle_Invalid;

static const char* g_pluginName    = "CraftingContainers";
const UInt32       g_pluginVersion = 0x01000000; // 0xAABBCCDD = AA.BB.CC.DD with values converted to decimal // major.minor.update.internal-build-or-zero

extern "C" {
   //
   // SKSEPlugin_Query: Called by SKSE to learn about this plug-in and check that it's safe to load.
   //
   bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info) {
      gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim\\SKSE\\CraftingContainers.log");
      gLog.SetPrintLevel(IDebugLog::kLevel_Error);
      gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);
      //
      _MESSAGE("Query");
      //
      // Populate info structure.
      //
      info->infoVersion = PluginInfo::kInfoVersion;
      info->name        = g_pluginName;
      info->version     = g_pluginVersion;
      {  // Log version number
         auto  v     = info->version;
         UInt8 main  = v >> 0x18;
         UInt8 major = v >> 0x10;
         UInt8 minor = v >> 0x08;
         _MESSAGE("Current version is %d.%d.%d.", main, major, minor);
      }
      {  // Get run-time information
         HMODULE    baseAddr = GetModuleHandle("CraftingContainers"); // DLL filename
         MODULEINFO info;
         if (baseAddr && GetModuleInformation(GetCurrentProcess(), baseAddr, &info, sizeof(info)))
            _MESSAGE("We're loaded to the span of memory at %08X - %08X.", info.lpBaseOfDll, (UInt32)info.lpBaseOfDll + info.SizeOfImage);
      }
      //
      // Store plugin handle so we can identify ourselves later.
      //
      g_pluginHandle = skse->GetPluginHandle();
      if (skse->isEditor) {
         _MESSAGE("We've been loaded in the Creation Kit. Marking as incompatible.");
         return false;
      } else if (skse->runtimeVersion != RUNTIME_VERSION_1_9_32_0) {
         _MESSAGE("Unsupported Skyrim version: %08X.", skse->runtimeVersion);
         return false;
      }
      //
      // This plug-in supports the current Skyrim and SKSE versions:
      //
      return true;
   }
   //
   // SKSEPlugin_Load: Called by SKSE to load this plug-in.
   //
   bool SKSEPlugin_Load(const SKSEInterface* skse) {
      _MESSAGE("Load.");
      CraftingContainers::INI::get().load();
      {  // Patches:
         Patches::Exploratory::Apply();
         Patches::AlchemyMenu::Apply();
         Patches::CraftingAndSmithing::Apply();
         Patches::Enchanting::Apply();
      }
      return true;
   }
};