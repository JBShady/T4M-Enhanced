// ==========================================================
// T4M project
// 
// Component: clientdll
// Purpose: World at War patches
//
// Initial author: UNKNOWN
// Started: 2015-07-08
// ==========================================================

#include "StdInc.h"
#include "MemoryMgr.h"
#include "IniReader.h"
#include <safetyhook.hpp>
void loadGameOverlay();
void LAACheck();
void PatchT4();
void PatchT4_MemoryLimits();
void PatchT4_Branding();
void PatchT4_Console();
void PatchT4_Dvars();
void PatchT4_Menus();
void PatchT4_NoBorder();
void PatchT4_PreLoad();
void PatchT4_Script();
void PatchT4_SteamDRM();
void PatchT4_FileDebug();
void PatchT4_Load();
void PatchT4E_Window();
void PatchT4E_Shaders();
void PatchT4E_Render();

void PatchT4E_Weapons();
void PatchT4E_Pathing();

void PatchT4E_Input();
void PatchT4E_Rumble();

void PatchT4E_UI();

void Sys_RunInit()
{
	LAACheck();
	PatchT4();
}

void PatchT4()
{
	//*(const char**)0x00840FF0 = "raw";
	PatchT4_SteamDRM();
	PatchT4_PreLoad();
	PatchT4_MemoryLimits();
	PatchT4_Branding();
	PatchT4_Console();
	PatchT4_Dvars();
	PatchT4_Menus();
	PatchT4_NoBorder();
	PatchT4_Script();
	PatchT4_Load();
	PatchT4E_Window();
	PatchT4E_Shaders();
	PatchT4E_Render();

	PatchT4E_UI();

	PatchT4E_Weapons();
	PatchT4E_Pathing();

	PatchT4E_Input();
	PatchT4E_Rumble();

	// check if game got started using steam
	if (!GetModuleHandle("gameoverlayrenderer.dll"))
		loadGameOverlay();
}

void *MemCpyFix(void *a1, void **a2, int len)
{
	return memcpy(a1, a2, len);
}

void PatchT4_PreLoad()
{
	Detours::X86::DetourFunction((PBYTE)0x007AFFC0, (PBYTE)&MemCpyFix);
	nop(0x0059D6F4, 5); // disable Com_DvarDump from Com_Init_Try_Block_Function
	nop(0x005FF743, 5); // disable Sys_CreateSplash
	//nop(0x005FF698, 5); // disable Sys_CheckCrashOrRerun
	//nop(0x005FE685, 5); // disable Sys_HasConfigureChecksumChanged
	CIniReader ini;

	// as done in Juiced Patch https://github.com/kobraworksmodding/Saints-Row-2-Juiced-Patch/blob/main/Monkey%20Patch/Audio/XACT.cpp , although SR2 uses XACT and XACT's version is supposed to be inline with XAudio,
	// in SR2 this might cause a rare crash to occur that I've fixed there, hopefully this doesn't raise much issues - Clippy95

	if (ini.ReadInteger("Fixes", "UseFixedXAudio", 1) != 0){
		GUID xaudio = { 0x4c5e637a, 0x16c7, 0x4de3, 0x9c, 0x46, 0x5e, 0xd2, 0x21, 0x81, 0x96, 0x2d }; // XAudio 2.3
		Memory::VP::Patch(0x0089DA98, xaudio);
	}
	// Increase hunk total
	Memory::VP::Patch<uint32_t>((0x005E3CD1 + 6), 15728640);

	// Remove duplicate calls in serverthread
	Memory::VP::Nop(0x00636686, 0x2D);

}

void PatchT4_SteamDRM()
{
	// check if steam exe before continuing, fixes LAN issues, code from ineedbots
	if (*(DWORD*)0x401000 != 0x9EF490B8)
		return;

	// Replace encrypted .text segment
	DWORD size = 0x3EA000;
	std::string data = GetBinaryResource(IDB_TEXT);
	uncompress((unsigned char*)0x401000, &size, (unsigned char*)data.data(), data.size());

	// Apply new entry point
	HMODULE hModule = GetModuleHandle(NULL);
	PIMAGE_DOS_HEADER header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule + header->e_lfanew);
	ntHeader->OptionalHeader.AddressOfEntryPoint = 0x3AF316;
}

void PatchT4_Menus()
{
	static auto CG_CheckHudObjectiveDisplay_hook = safetyhook::create_mid(0x004379D0, [](SafetyHookContext& ctx) {
		if (isZombieMode())
			ctx.eip = retptr;
		});
	//nop(0x437ACC, 5); // disable CG_CheckHudObjectiveDisplay call
	//nop(0x6680D2, 2); // disable jmp for onlinegame dvar check
	static auto onlinegame_dvar_check = safetyhook::create_mid(0x006680CE, [](SafetyHookContext& ctx) {
		if (isZombieMode())
			ctx.eip = 0x006680D4;
		});



	//static auto MapRestart1 = safetyhook::create_mid(0x0062B7C0, [](SafetyHookContext& ctx) {
	//	cdecl_call<int>(0x435D80);
	//	});

	Memory::VP::Nop(0x00438875, 10);

	static auto draw_scoreboard_new1 = safetyhook::create_mid(0x006680D4, [](SafetyHookContext& ctx) {
		game::cg_s* cgArray = (game::cg_s*)0x034732B8;

		if (cgArray->nextSnap && cgArray->nextSnap->ps.pm_type == game::pmtype_t::PM_INTERMISSION) {
			ctx.eip = 0x006680DD;
			return;
		}

		});

}

// code from https://github.com/momo5502/cod-mod/
void loadGameOverlay()
{
	try
	{
		std::string m_steamDir;
		HKEY hRegKey;

		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &hRegKey) == ERROR_SUCCESS)
		{
			char pchSteamDir[MAX_PATH];
			DWORD dwLength = sizeof(pchSteamDir);
			RegQueryValueExA(hRegKey, "InstallPath", NULL, NULL, (BYTE*)pchSteamDir, &dwLength);
			RegCloseKey(hRegKey);

			m_steamDir = pchSteamDir;
		}
		// causes a stack overflow if left in
		//Com_Printf(0, "Loading %s\\gameoverlayrenderer.dll...\n", m_steamDir.c_str());
		HMODULE overlay = LoadLibrary(va("%s\\gameoverlayrenderer.dll", m_steamDir.c_str()));

		if (overlay)
		{
			FARPROC _SetNotificationPosition = GetProcAddress(overlay, "SetNotificationPosition");

			if (_SetNotificationPosition)
			{
				((void(*)(uint32_t))_SetNotificationPosition)(1);
			}
		}
	}
	catch (int e)
	{
		//Com_Printf(0, "Failed to inject Steam's gameoverlay: %d", e);
	}
}
