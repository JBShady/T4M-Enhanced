// ==========================================================
// T4M project
//
// Component: clientdll
// Purpose: Increasing memory pool sizes
//
// Initial author: TheApadayo
//
// Started: 2015-07-18
// ==========================================================

#include "StdInc.h"

void PatchT4_MemoryLimits()
{
	// increase pool sizes to similar (or greater) t5 sizes.
	DB_ReallocXAssetPool(ASSET_TYPE_FX, 600);
	DB_ReallocXAssetPool(ASSET_TYPE_IMAGE, 4096);
	DB_ReallocXAssetPool(ASSET_TYPE_LOADED_SOUND, 2400);
	DB_ReallocXAssetPool(ASSET_TYPE_MATERIAL, 4096);
	DB_ReallocXAssetPool(ASSET_TYPE_WEAPON, 320);
	DB_ReallocXAssetPool(ASSET_TYPE_XMODEL, 1500);

	// change the size of g_mem from 0x12C00000 to 0x19600000, UGX-Mod v1.1 is pretty fucking huge
	// had to increase due to it crashing in Com_BeginParseSession
	*(DWORD*)0x5F5492 = 0x19600000; //0x14800000
	*(DWORD*)0x5F54D1 = 0x19600000; //0x14800000
	*(DWORD*)0x5F54DB = 0x19600000; //0x14800000

	// change the num of entities available to be spawned in G_Spawn from 1022 to 1500
	// still a W.I.P. is missing array and hash table(?) changes
	//PatchMemory(0x0054EAC3, (PBYTE)"\xDC\x05", 2);
}
