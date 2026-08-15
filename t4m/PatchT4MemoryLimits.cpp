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

// rgp.sortedMaterials replacement. Sized above ASSET_TYPE_MATERIAL's pool size
// because DB_EnumXAssets_FastFile (sub_48DF60) has no bound and also walks
// duplicate entries of the same asset across loaded zones.
#define NEW_SORTED_MATERIALS_SIZE 8192

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

	// =====================================================================
	// For the record what is this things :
	// Fix rgp.sortedMaterials overflow
	// dword_3BF1880 is not a standalone array, it's the base of rgp
	// (r_global_permanent_t, 0x2280 bytes) whose first member is
	// Material* sortedMaterials[2048]. R_LoadWorld and sub_719F40 both call
	// sub_48DF60(6 /*ASSET_TYPE_MATERIAL*/, dword_3BF1880, 0x800) then sort the
	// result. sub_48DF60 never reads that third arg — it writes ALL matching
	// assets, so with the material pool at 4096 it overflows and corrupts:
	//   - dword_3BF3884 (rgp.materialCount, at rgp + 0x2004)
	//   - dword_3BF392C (rgp.world, at rgp + 0x20AC)
	// This causes both observed crashes via xdbg:
	//   0x719A2E: sort comparator gets garbage → access violation
	//   0x491500: corrupted rgp.world → bitfield access violation
	// =====================================================================

	static DWORD* newSortedMaterials = (DWORD*)VirtualAlloc(
		NULL,
		NEW_SORTED_MATERIALS_SIZE * sizeof(DWORD),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);

	if (!newSortedMaterials) {
		Com_Printf(0, "^1ERROR: Failed to allocate expanded sortedMaterials array\n");
		return;
	}

	DWORD newMaterialsAddr = (DWORD)newSortedMaterials;

	DWORD oldProtect;
	VirtualProtect((LPVOID)0x6DC960, 0x742012 - 0x6DC960, PAGE_EXECUTE_READWRITE, &oldProtect);

	// --- Patch the 11 references to rgp.sortedMaterials ---

	*(DWORD*)0x6DC964 = newMaterialsAddr;
	*(DWORD*)0x6DCA8C = newMaterialsAddr;
	*(DWORD*)0x6E993D = newMaterialsAddr;
	*(DWORD*)0x705784 = newMaterialsAddr;
	*(DWORD*)0x70579F = newMaterialsAddr;
	*(DWORD*)0x719F52 = newMaterialsAddr;
	*(DWORD*)0x719F6D = newMaterialsAddr;
	*(DWORD*)0x741C11 = newMaterialsAddr;
	*(DWORD*)0x741C98 = newMaterialsAddr;
	*(DWORD*)0x741EB7 = newMaterialsAddr;
	*(DWORD*)0x74200E = newMaterialsAddr;
}
