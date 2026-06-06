#pragma once

typedef enum
{
	DVAR_FLAG_ARCHIVE = 1 << 0,				// 0x0001
	DVAR_FLAG_USERINFO = 1 << 1,			// 0x0002
	DVAR_FLAG_SERVERINFO = 1 << 2,			// 0x0004
	DVAR_FLAG_SYSTEMINFO = 1 << 3,			// 0x0008
	DVAR_FLAG_INIT = 1 << 4,				// 0x0010
	DVAR_FLAG_LATCH = 1 << 5,				// 0x0020
	DVAR_FLAG_ROM = 1 << 6,					// 0x0040
	DVAR_FLAG_CHEAT = 1 << 7,				// 0x0080
	DVAR_FLAG_DEVELOPER = 1 << 8,			// 0x0100
	DVAR_FLAG_SAVED = 1 << 9,				// 0x0200
	DVAR_FLAG_NORESTART = 1 << 10,			// 0x0400
	DVAR_FLAG_CHANGEABLE_RESET = 1 << 12,	// 0x1000
	DVAR_FLAG_EXTERNAL = 1 << 14,			// 0x4000
	DVAR_FLAG_AUTOEXEC = 1 << 15,			// 0x8000
} dvar_flag;

enum dvarType_t : __int8
{
	DVAR_TYPE_BOOL = 0x0,
	DVAR_TYPE_FLOAT = 0x1,
	DVAR_TYPE_FLOAT_2 = 0x2,
	DVAR_TYPE_FLOAT_3 = 0x3,
	DVAR_TYPE_FLOAT_4 = 0x4,
	DVAR_TYPE_INT = 0x5,
	DVAR_TYPE_ENUM = 0x6,
	DVAR_TYPE_STRING = 0x7,
	DVAR_TYPE_COLOR = 0x8,
	DVAR_TYPE_INT64 = 0x9,
	DVAR_TYPE_LINEAR_COLOR_RGB = 0xA,
	DVAR_TYPE_COLOR_XYZ = 0xB,
	DVAR_TYPE_COUNT = 0xC,
};


union dvar_value_t {
	char*	string;
	int		integer;
	float	value;
	bool	boolean;
	bool    enabled;
	float	vec2[2];
	float	vec3[3];
	float	vec4[4];
	BYTE	color[4]; //to get float: multiply by 0.003921568859368563 - BaberZz
	__int64 integer64; // only in Tx
};

union dvar_maxmin_t {
	int i;
	float f;
};






typedef struct __declspec(align(4)) dvar_t
{
	//startbyte:endbyte
	const char*		name; //0:3
	const char*		description; //4:7
	unsigned __int16	flags; //8:11
	dvarType_t			type; //12:12
	char			modified;
	char			pad2[4]; //13:15
	dvar_value_t	current; //16:31
	dvar_value_t	latched; //32:47
	dvar_value_t	defaulta; //48:64
	dvar_maxmin_t min; //65:67
	dvar_maxmin_t max; //68:72 woooo

	inline bool isEnabled() {
		return (this && this->current.boolean);
	}

} dvar_t;

enum scriptInstance_t
{
	SCRIPTINSTANCE_SERVER = 0x0,
	SCRIPTINSTANCE_CLIENT = 0x1,
	SCRIPT_INSTANCE_MAX = 0x2,
};

typedef void(__cdecl * xcommand_t)(void);

#define cmd_functions_ADDR 0x01F416F4

typedef struct cmd_function_s
{
	struct cmd_function_s *next;
	char *name;
	char *autocomplete1;
	char *autocomplete2;
	xcommand_t function;
};

struct CmdArgs
{
	int nesting;
	int localClientNum[8];
	int controllerIndex[8];
	//itemDef_s *itemDef[8];
	int argshift[8];
	int argc[8];
	const char **argv[8];
	char textPool[8192];
	const char *argvPool[512];
	int usedTextPool[8];
	int totalUsedArgvPool;
	int totalUsedTextPool;
};

struct XZoneInfo {
	const char* name;
	int allocFlags;
	int freeFlags;
};

struct vec2_t { float x, y; };
struct vec3_t { float x, y, z; };
struct vec4_t { float x, y, z, w; };

struct ConDrawInputGlob
{
	char autoCompleteChoice[64];
	int matchIndex;
	int matchCount;
	const char *inputText;
	int inputTextLen;
	bool hasExactMatch;
	bool mayAutoComplete;
	float x;
	float y;
	float leftX;
	float fontHeight;
};

struct scr_localVar_t
{
	unsigned int name;
	unsigned int sourcePos;
};

struct scr_block_s
{
	int abortLevel;
	int localVarsCreateCount;
	int localVarsPublicCount;
	int localVarsCount;
	char localVarsInitBits[8];
	scr_localVar_t localVars[64];
};

union sval_u
{
	char type;
	unsigned int stringValue;
	unsigned int idValue;
	float floatValue;
	int intValue;
	sval_u *node;
	unsigned int sourcePosValue;
	const char *codePosValue;
	const char *debugString;
	scr_block_s *block;
};

enum bitsShit
{
	MEMORY_NODE_BITS = 0x10,
	MEMORY_NODE_COUNT = 0x10000,
	MT_SIZE = 0x100000,
	REFSTRING_STRING_OFFSET = 0x4,
};

struct __declspec(align(4)) RefString
{
	union bitsShit;
	char str[1];
};

struct __declspec(align(4)) VariableStackBuffer
{
	const char *pos;
	unsigned __int16 size;
	unsigned __int16 bufLen;
	unsigned int localId;
	char time;
	char buf[1];
};

union VariableUnion
{
	int intValue;
	float floatValue;
	unsigned int stringValue;
	const float *vectorValue;
	const char *codePosValue;
	unsigned int pointerValue;
	VariableStackBuffer *stackValue;
	unsigned int entityOffset;
};


struct XZoneName
{
	char name[64];
	int flags;
	int fileSize;
	BYTE dir; //enum FF_DIR
	bool loaded;
	BYTE pad[2];
};



#include "xasset.h"

// types
typedef void(__cdecl * CommandCB_t)(void);

/*
	Engine Functions
*/

extern "C"
{

	typedef int(__cdecl * AddFunction_t)(scriptInstance_t inst, int func);
	extern AddFunction_t AddFunction;

	extern void Cbuf_AddText(const char* text, int localClientNum);

	extern double CG_CornerDebugPrint(const char* text, float x, float y, float label_width, char* label, float* color);

	//typedef void(__cdecl * Cmd_AddCommand_t)(const char* name, CommandCB_t callback, cmd_function_s* data, char);
	//extern Cmd_AddCommand_t Cmd_AddCommand;

	typedef void(__cdecl * Com_Error_t)(int type, char* message, ...);
	extern Com_Error_t Com_Error;

	typedef void(*Com_Printf_t)(int channel, const char* format, ...);
	extern Com_Printf_t Com_Printf;

	typedef void(__cdecl * Com_PrintMessage_t)(int channel, const char *fmt, int error);
	extern Com_PrintMessage_t Com_PrintMessage;

	typedef const char *(__cdecl * DB_EnumXAssets_t)(XAssetType type, void(__cdecl *func)(XAssetHeader, void *), void *inData, bool includeOverride);
	extern DB_EnumXAssets_t DB_EnumXAssets;

	typedef void(__cdecl * DB_EnumXAssets_FastFile_t)(XAssetType type, void(__cdecl *func)(XAssetHeader, void *), void *inData);
	extern DB_EnumXAssets_FastFile_t DB_EnumXAssets_FastFile;

	typedef void* (__cdecl * DB_FindXAssetHeader_t)(int type, const char* filename);
	extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

	typedef void(*DB_LoadXAssets_t)(XZoneInfo* data, int count, int sync);
	extern DB_LoadXAssets_t DB_LoadXAssets;

	//typedef dvar_t* (__fastcall* DvarRegisterFloatFunc)(const char* dvarName, float defaultValue, float min, float max, int flags, const char* description);
	//extern DvarRegisterFloatFunc Dvar_RegisterFloat;

	extern dvar_t* Dvar_RegisterBool(bool value, const char *dvarName, int flags, const char *description = "");

	extern dvar_t* Dvar_RegisterFloat(const char* dvarName, float defaultValue, float min, float max, int flags, const char* description = "");

	extern dvar_t* Dvar_RegisterInt(int default_value, const char* name, int min, int max, int flags, const char* description = "");

	extern dvar_t* Dvar_RegisterEnum(const char** valueList, int defaultIndex, const char* dvarName, int flags, const char* description);

	typedef void(__cdecl * EmitMethod_t)(scriptInstance_t inst, sval_u expr, sval_u func_name, sval_u params, sval_u methodSourcePos, bool bStatement, scr_block_s *block);
	extern EmitMethod_t EmitMethod;

	typedef void* (*Scr_GetFunction_t)(const char **pName, int *type);
	extern Scr_GetFunction_t Scr_GetFunction;

	typedef void* (*CScr_GetFunction_t)(const char **pName, int *type);
	extern CScr_GetFunction_t CScr_GetFunction;

	typedef void* (*CScr_GetMethod_t)(const char **pName, int *type);
	extern CScr_GetMethod_t CScr_GetMethod;

	typedef dvar_t* (__cdecl*Dvar_FindMalleableVarT)(const char* name);
	extern Dvar_FindMalleableVarT Dvar_FindMalleableVar;

	//typedef void* (*Scr_GetMethod_t)(const char **pName, int *type);
	extern int Scr_GetMethod(int *type, const char **pName);

	typedef int(__cdecl * Player_GetMethod_t)(const char **pName);
	extern Player_GetMethod_t Player_GetMethod;

	typedef int(__cdecl * ScriptEnt_GetMethod_t)(const char **pName);
	extern ScriptEnt_GetMethod_t ScriptEnt_GetMethod;

	typedef int(__cdecl * ScriptVehicle_GetMethod_t)(const char **pName);
	extern ScriptVehicle_GetMethod_t ScriptVehicle_GetMethod;

	typedef int(__cdecl * HudElem_GetMethod_t)(const char **pName);
	extern HudElem_GetMethod_t HudElem_GetMethod;

	typedef int(__cdecl * Helicopter_GetMethod_t)(const char **pName);
	extern Helicopter_GetMethod_t Helicopter_GetMethod;

	typedef int(__cdecl * Actor_GetMethod_t)(const char **pName);
	extern Actor_GetMethod_t Actor_GetMethod;

	typedef int(__cdecl * BuiltIn_GetMethod_t)(const char **pName, int *type);
	extern BuiltIn_GetMethod_t BuiltIn_GetMethod;
	
	typedef void(*RemoveRefToValue_t)(scriptInstance_t inst, int type, VariableUnion u);
	extern RemoveRefToValue_t RemoveRefToValue;
}

struct scr_entref_t
{
	unsigned __int16 entnum;
	unsigned __int16 classnum;
	unsigned __int16 client;
};

typedef void(__cdecl * scr_function_t)(scr_entref_t);
void Scr_DeclareFunction(const char* name, scr_function_t func, bool developerOnly);
void Scr_DeclareMethod(const char* name, scr_function_t func, bool developerOnly);

// inline cmd functions
extern DWORD* cmd_id;
extern DWORD* cmd_argc;
extern DWORD** cmd_argv;

inline int Cmd_Argc(void)
{
	return cmd_argc[*cmd_id];
}

inline char *Cmd_Argv(int arg)
{
	if ((unsigned)arg >= cmd_argc[*cmd_id]) {
		return (char*)"";
	}
	return (char*)(cmd_argv[*cmd_id][arg]);
}

int __cdecl Scr_GetNumParam(scriptInstance_t inst);
const char* Scr_GetString(uint32_t index, scriptInstance_t instance);


/*
	Source Functions
*/
void* DB_ReallocXAssetPool(XAssetType type, unsigned int newSize);
char *__cdecl SL_ConvertToString(unsigned int stringValue, scriptInstance_t inst);
void Cmd_AddCommand(const char *cmd_name, xcommand_t function);

char *__cdecl DB_GetXAssetTypeName(int type);

extern XAssetEntryPoolEntry* g_assetEntryPool;
extern XZoneName* g_zoneNames;
extern unsigned __int16 * db_hashTable;

typedef const char *(__cdecl *DB_XAssetGetNameHandler)(XAssetHeader *);
extern DB_XAssetGetNameHandler *DB_XAssetGetNameHandlers;
void __cdecl DB_ListAssetPool(XAssetType type, bool count_only);
const char *__cdecl DB_GetXAssetHeaderName(int type, XAssetHeader *header);
const char *__cdecl DB_GetXAssetName(XAsset *asset);

int __cdecl DB_GetXAssetTypeSize(int type);

bool isZombieMode();

bool Com_SessionMode_IsZombiesGame();

bool IsReflectionMode();

namespace Dvars {
	namespace Functions {
		dvar_t* Dvar_FindVar(const char* name);
	}
}

void DoReturn();

#define retptr (uintptr_t)&DoReturn
