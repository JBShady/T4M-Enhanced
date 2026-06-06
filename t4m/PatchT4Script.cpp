// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Functionality to interact with the GameScript 
//          runtime.
//
// Initial author: NTAuthority
// Adapted: 2015-07-21
// Started: 2011-12-19
// ==========================================================

#include "t4_headers.h"
#include "StdInc.h"
#include "T4.h"

#include "safetyhook.hpp"

#include "MemoryMgr.h"

WeaponDef** bg_weaponDefs = (WeaponDef**)0x8F6770;
const WeaponDef* __cdecl BG_GetWeaponDef(unsigned int weaponIndex) {
	return bg_weaponDefs[weaponIndex];
}

bool __cdecl BG_IsOverheatingWeapon(unsigned int weapIndex)
{
	return BG_GetWeaponDef(weapIndex)->overheatWeapon != 0;
}

int BG_GetWeaponIndexForName(const char* name) {
	if (*(bool*)0x018F6DB8);
	return ((int(__cdecl*)(const char*, void*))0x41D4C0)(name, (void*)0x4FE980);

	return ((int(__cdecl*)(const char*))0x41D470)(name);
}

int __cdecl Scr_GetNumParam(scriptInstance_t inst);

dvar_t** developer = (dvar_t**)0x01F55288;
dvar_t** developer_script = (dvar_t**)0x01F9646C;
dvar_t** logfile = (dvar_t**)0x01F552BC;

dvar_t* developer_funcdump;

// COD5R HUD stuff
dvar_t* cg_drawHealthCount;
dvar_t* cg_drawHealthCountCoop;
dvar_t* cg_drawGamepadHUD;
dvar_t* cg_drawDpadLogos;
dvar_t* cg_SoloScoreColorWhite;
dvar_t* cg_drawTimers;
dvar_t* cg_drawTrapTimers;

dvar_t* cg_lowerGun; // lower gun 1st person
dvar_t* zombiemode_dev; // experimental COD5R features

// these below are all fixable within gsc, but i'll add them in for whoever wants to use them, same names as Pluto but im not sure if this implementation is 1:1 with pluto
dvar_t* g_disable_zombie_grab;

dvar_t* g_fix_tesla_bug;

dvar_t* g_fix_health_sets_max;



// custom functions
typedef struct
{
	const char* functionName;
	scr_function_t functionCall;
	int developerOnly;
} scr_funcdef_t;

typedef struct
{
	const char* functionName;
	scr_function_t functionCall;
	int developerOnly;
} scr_methdef_t;


#pragma region setupFunctions
static std::map<std::string, scr_funcdef_t> scriptFunctions;
static std::map<std::string, scr_methdef_t> scriptMethods;

scr_function_t Scr_GetCustomFunction(const char** name, int* isDeveloper)
{
	scr_funcdef_t func = scriptFunctions[*name];

	if (func.functionName)
	{
		*name = func.functionName;
		*isDeveloper = func.developerOnly;

		return func.functionCall;
	}
	else
		return NULL;
}


void Scr_DeclareFunction(const char* name, scr_function_t func, bool developerOnly = false)
{
	scr_funcdef_t funcDef;
	funcDef.functionName = name;
	funcDef.functionCall = func;
	funcDef.developerOnly = (developerOnly) ? 1 : 0;

	scriptFunctions[name] = funcDef;
}

scr_function_t Scr_GetCustomMethod(const char** name, int* isDeveloper)
{
	scr_methdef_t method = scriptMethods[*name];

	if (method.functionName)
	{
		*name = method.functionName;
		*isDeveloper = method.developerOnly;

		return method.functionCall;
	}

	return NULL;
}

void Scr_DeclareMethod(const char* name, scr_function_t func, bool developerOnly = false)
{
	scr_methdef_t methodDef;
	methodDef.functionName = name;
	methodDef.functionCall = func;
	methodDef.developerOnly = developerOnly ? 1 : 0;

	scriptMethods[name] = methodDef;
}
#pragma endregion setupFunctions

#pragma region engineFunctions
int __cdecl Scr_GetNumParam(scriptInstance_t inst)
{
	DWORD *value = (DWORD *)0x03BD471C; // getNumParamArray location
	return value[4298 * inst];
}

RefString *__cdecl GetRefString(scriptInstance_t inst, unsigned int stringValue)
{
	DWORD *gScrMemTreePub = (DWORD *)0x03702390;
	return (RefString *)&(&gScrMemTreePub)[3 * stringValue + 1];
}

char *__cdecl SL_ConvertToString(unsigned int stringValue, scriptInstance_t inst)
{
	char *v3;
	if (stringValue)
		v3 = GetRefString(inst, stringValue)->str;
	else
		v3 = 0;
	return v3;
}

void Scr_ClearOutParams(scriptInstance_t v1)
{
	static DWORD dwCall = 0x00693DA0;

	__asm
	{
		mov edi, v1
		call[dwCall]
	}
}

void __cdecl IncInParam(scriptInstance_t inst)
{
	Scr_ClearOutParams(inst);

	// TO-DO: define sys_error
	//if (dword_A05AC98[4298 * inst] == dword_A05AC8C[4298 * inst])
	//	Sys_Error("Internal script stack overflow");
	((DWORD *)0xA05AC98)[4296 * inst] += 8;
	++((DWORD *)0xA05ACA0)[4296 * inst];
}

void __cdecl Scr_AddInt(int value, scriptInstance_t inst)
{
	IncInParam(inst);
	*(DWORD *)((((DWORD *)0x03BD4710)[4296 * value]) + 4) = 6;
	*(DWORD *)(((DWORD *)0x03BD4710)[4296 * value]) = value;
}
#pragma endregion engineFunctions

#pragma region engineHKFunctions
void(__cdecl *__cdecl Scr_GetFunction_Hook(const char **pName, int *type))()
{
	// this is aids and I don't care
	// also if running debugger and a customf func is executed the debugger will instadie
	void(__cdecl *function)();
	// check if the function passed is part of our custom funcs
	if (!(scriptFunctions.find(std::string(*pName)) != scriptFunctions.end()))
		function = (void(__cdecl *)())Scr_GetFunction(pName, type);
	else
		function = (void(__cdecl *)())Scr_GetCustomFunction(pName, type);

	if (developer_funcdump->current.boolean && function != 0)
		Com_Printf(0, "[GSC] Function: %s\nType: %i\nAddress: 0x%X\n\n", *pName, *type, function);

	return function;
}

int Scr_GetMethod(int *type, const char **pName)
{
	// also aids and again do not care. #3
	int function;

	if (scriptMethods.find(std::string(*pName)) != scriptMethods.end())
	{
		function = (int)Scr_GetCustomMethod(pName, type);

		if (developer_funcdump->current.boolean && function != 0)
			Com_Printf(0, "[GSC] Method: %s\nType: %i\nAddress: 0x%X\n\n", *pName, *type, function);

		return function;
	}

	*type = 0;
	function = Player_GetMethod(pName);

	if (!function)
	{
		function = ScriptEnt_GetMethod(pName);
		if (!function)
		{
			function = ScriptVehicle_GetMethod(pName);
			if (!function)
			{
				function = HudElem_GetMethod(pName);
				if (!function)
				{
					function = Helicopter_GetMethod(pName);
					if (!function)
					{
						function = Actor_GetMethod(pName);
						if (!function)
							function = BuiltIn_GetMethod(pName, type);
					}
				}
			}
		}
	}

	if (developer_funcdump->current.boolean && function != 0)
		Com_Printf(0, "[GSC] Method: %s\nType: %i\nAddress: 0x%X\n\n", *pName, *type, function);

	return function;
}


void __declspec(naked) Scr_GetMethod_Hook(int *type, const char **pName)
{
	__asm
	{
		push esi // pName
		push edi // type
		call Scr_GetMethod
		add esp, 8
		retn
	}
}

static uintptr_t Scr_GetString_addr = 0x69A0D0;
uintptr_t __declspec(naked) Scr_GetString_asm(uint32_t index, scriptInstance_t instance)
{
	__asm 
	{
		push ebp
		mov ebp, esp
		sub esp, __LOCAL_SIZE


		push eax
		push esi

		mov eax, index
		mov esi, instance

		call Scr_GetString_addr

		pop esi
		pop eax

		mov esp, ebp
		pop ebp
		ret
	}
}

const char* Scr_GetString(uint32_t index, scriptInstance_t instance) {
	return (const char*)Scr_GetString_asm(index, instance);
}


void(__cdecl *__cdecl CScr_GetFunction_Hook(const char **pName, int *type))()
{
	// this is aids and I don't care #2
	// also if running debugger and a customf func is executed the debugger will instadie
	void(__cdecl *function)();

	function = (void(__cdecl *)())CScr_GetFunction(pName, type);

	if (developer_funcdump->current.boolean && function != 0)
		Com_Printf(0, "[CSC] Function: %s\nType: %i\nAddress: 0x%X\n\n", *pName, *type, function);

	return function;
}

void(__cdecl *__cdecl CScr_GetMethod_Hook(const char **pName, int *type))()
{
	// aids #3 woo!
	void(__cdecl *function)();

	function = (void(__cdecl *)())CScr_GetMethod(pName, type);

	if (developer_funcdump->current.boolean && function != 0)
		Com_Printf(0, "[CSC] Method: %s\nType: %i\nAddress: 0x%X\n\n", *pName, *type, function);

	return function;
}
#pragma endregion engineHKFunctions

void PlayerWeaponOverheatUpdate(gentity_s* ent, uint32_t weapon_index, float amount) {
	if (!BG_IsOverheatingWeapon(weapon_index))
		return;
	auto weapon = BG_GetWeaponDef(weapon_index);
	float &current_heat = ent->client->ps.heatpercent[weapon->iHeatIndex];
	bool &current_overheat  = ent->client->ps.overheating[weapon->iHeatIndex];
	current_heat = amount;
	if (amount == 0.f)
		current_overheat = false;
}
gentity_s* g_entities = (gentity_s*)0x0176C6F0;
#pragma region customFunctions
void GScr_PrintLnConsole(scr_entref_t entity)
{
	// gets amount of parameters
	if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 1)
		Com_Printf(0, "^3Have one!\n");
	else
		Com_Printf(0, "^1the cake is a lie\n\n");
	// iz ded af
	//Scr_AddInt(Scr_GetNumParam(SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
}
#pragma endregion customFunctions


int __cdecl Scr_GetInt(scriptInstance_t inst, unsigned int index)
{
	static DWORD func = 0x00699C50;
	int result;
	__asm
	{
		mov eax, inst
		mov ecx, index
		call func
		mov result, eax
	}
	return result;
}

// for zombies this is applied on each zombie spawn in _spawner
int __stdcall DisablePushPlayer() {

	if ((isZombieMode() && g_disable_zombie_grab->current.integer == 1) || g_disable_zombie_grab->current.integer >= 2) {
		return 0;
	}
	else
		return Scr_GetInt(SCRIPTINSTANCE_SERVER, 0);

}

void PatchT4_Script()
{
	void PatchT4E_Rumble_RegisterScriptBindings();
	developer_funcdump = Dvar_RegisterBool(0, "developer_funcdump", 0, "Dump script function information (engine)");

	cg_drawHealthCount = Dvar_RegisterBool(0, "cg_drawHealthCount", 0, "Draw developer health counter in solo (requires map restart)"); // requires NZ remastererd mod
	cg_drawHealthCountCoop = Dvar_RegisterBool(0, "cg_drawHealthCountCoop", 0, "Draw developer health counter in co-op (requires map restart)"); // requires NZ remastererd mod 
	cg_drawGamepadHUD = Dvar_RegisterBool(0, "cg_drawGamepadHUD", 0, "Draw gamepad style HUD optimized for controller use"); // requires NZ remastererd mod
	cg_drawDpadLogos = Dvar_RegisterBool(1, "cg_drawDpadLogos", 0, "Draw D-pad background textures"); // requires NZ remastererd mod
	cg_lowerGun = Dvar_RegisterBool(0, "cg_lowerGun", 0, "Enable weapon lowering while moving in solo (requires map restart)"); // requires NZ remastererd mod
	cg_SoloScoreColorWhite = Dvar_RegisterBool(0, "cg_SoloScoreColorWhite", 0, "Force white score color in solo (requires map restart)"); // requires NZ remastererd mod
	cg_drawTimers = Dvar_RegisterBool(0, "cg_drawTimers", 0, "Draw game and round timers (requires map restart)"); // requires NZ remastererd mod
	cg_drawTrapTimers = Dvar_RegisterBool(0, "cg_drawTrapTimers", 0, "Draw trap timers (requires map restart)"); // requires NZ remastererd mod
	zombiemode_dev = Dvar_RegisterBool(0, "zombiemode_dev", 0, "Enable experimental developer features for Nazi Zombies remastered mod (requires map restart)"); // requires NZ remastererd mod

	g_disable_zombie_grab = Dvar_RegisterInt(0, "g_disable_zombie_grab", 0, 2, DVAR_FLAG_CHEAT, "Disables pushPlayer() from executing\n1 = Disable when playing zombies\n2 = always disabled");

	g_fix_tesla_bug = Dvar_RegisterBool(0, "g_fix_tesla_bug",DVAR_FLAG_CHEAT, "Applies same wunderwaffe's 'fix' as seen in Black Ops 1");

	Memory::VP::InjectHook(0x4DDA72, DisablePushPlayer);

	static auto fix_tesla_bug = safetyhook::create_mid(0x4F5EE2, [](SafetyHookContext& ctx) {
		if (g_fix_tesla_bug->current.boolean)
			ctx.eip = 0x4F5F44;
		});



	g_fix_health_sets_max = Dvar_RegisterBool(0, "g_fix_health_sets_max", DVAR_FLAG_CHEAT, "Stops health also changing maxhealth");

	static auto fix_health_sets_max = safetyhook::create_mid(0x005309C0, [](SafetyHookContext& ctx) {
		if (g_fix_health_sets_max->current.boolean)
			ctx.eip = 0x5309C6;
		});

	static dvar_t* gsc_OverheatMaxAmmo = Dvar_RegisterBool(false, "gsc_OverheatMaxAmmo", 0, "Resets cooldown for 'overheat' weapon types when GiveMaxAmmo is called");

	// [GSC]
	Detours::X86::DetourFunction((PBYTE)0x00682DAF, (PBYTE)&Scr_GetFunction_Hook, Detours::X86Option::USE_CALL);
	Detours::X86::DetourFunction((PBYTE)0x00683043, (PBYTE)&Scr_GetMethod_Hook, Detours::X86Option::USE_CALL);
	Scr_DeclareFunction("printlnconsole", GScr_PrintLnConsole);
	PatchT4E_Rumble_RegisterScriptBindings();

	// i hate asm and safetyhook midhook ftw -clippy95
	static auto PlayerCmd_GiveMaxAmmo_midhook = safetyhook::create_mid(0x4EE157, [](SafetyHookContext& ctx) {
		if (gsc_OverheatMaxAmmo && gsc_OverheatMaxAmmo->current.boolean) {
			PlayerWeaponOverheatUpdate((gentity_s*)ctx.ebx, ctx.eax, 0.f);
		}
		});

	// [CSC]
	Detours::X86::DetourFunction((PBYTE)0x00682DC0, (PBYTE)&CScr_GetFunction_Hook, Detours::X86Option::USE_CALL);
	Detours::X86::DetourFunction((PBYTE)0x0068305C, (PBYTE)&CScr_GetMethod_Hook, Detours::X86Option::USE_CALL);

	nop(0x00465441, 2); // disable jnz on I_strnicmp for tesla notetrack

	// DON'T USE
	//nop(0x00668EDC, 5);
	//nop(0x00668F86, 5);
	//nop(0x00668E63, 5);
}
