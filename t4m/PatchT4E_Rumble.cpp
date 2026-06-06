#include "t4_headers.h"
#include "StdInc.h"
#include "T4.h"

#include "include\\safetyhook.hpp"
#include "include\\cod\\clientscript\\cscr_vm.hpp"

#include <climits>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstring>

namespace
{
	struct XInputVibrationState
	{
		WORD leftMotor;
		WORD rightMotor;
	};

	struct XInputGamepadState
	{
		WORD buttons;
		BYTE leftTrigger;
		BYTE rightTrigger;
		SHORT thumbLX;
		SHORT thumbLY;
		SHORT thumbRX;
		SHORT thumbRY;
	};

	struct XInputState
	{
		DWORD packetNumber;
		XInputGamepadState gamepad;
	};

	typedef DWORD(WINAPI* XInputGetState_t)(DWORD dwUserIndex, XInputState* pState);
	typedef DWORD(WINAPI* XInputSetState_t)(DWORD dwUserIndex, XInputVibrationState* pVibration);

	enum RumbleSourceType
	{
		RUMBLE_SOURCE_INVALID = -1,
		RUMBLE_SOURCE_ENTITY = 0,
		RUMBLE_SOURCE_POS = 1,
	};

	static const int RUMBLE_ENTITY_NONE = -1;

	struct RumbleGraph
	{
		char graphName[64];
		float knots[16][2];
		int knotCount;
	};

	struct RumbleInfo
	{
		char name[64];
		int durationMs;
		float range;
		RumbleGraph* highGraph;
		RumbleGraph* lowGraph;
		bool fadeWithDistance;
		bool broadcast;
		char durationDvar[64];
		char loopDvar[64];
	};

	struct ActiveRumble
	{
		RumbleInfo* rumbleInfo;
		int startTime;
		bool loop;
		RumbleSourceType sourceType;
		int entityNum;
		float pos[3];
	};

	struct RumbleGlobals
	{
		RumbleGraph graphs[64];
		int graphCount;
		RumbleInfo infos[32];
		int infoCount;
		ActiveRumble active[32];
	};

	static RumbleGlobals g_rumble{};
	static XInputGetState_t g_xinputGetState = nullptr;
	static XInputSetState_t g_xinputSetState = nullptr;
	static dvar_t* gpad_rumble = nullptr;
	static dvar_t* gpad_rumbleScale = nullptr;
	static dvar_t* cg_drawrumbledebug = nullptr;
	static dvar_t* gpad_rumble_debug_spam = nullptr;
	static DWORD g_manualRumbleEndTime = 0;
	static float g_manualRumbleLow = 0.0f;
	static float g_manualRumbleHigh = 0.0f;

	static gentity_s* g_entities = reinterpret_cast<gentity_s*>(0x0176C6F0);
	static game::GamePad* s_gamePads = reinterpret_cast<game::GamePad*>(0x022991A8);

	static SafetyHookInline Weapon_Melee_hook{};

	const char* Scr_GetStringServer(const unsigned int index)
	{
		return Scr_GetString(index, SCRIPTINSTANCE_SERVER);
	}

	unsigned int Scr_GetNumParamServer()
	{
		return ::Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
	}

	void Scr_GetVectorServer(float* value, const unsigned int index)
	{
		cdecl_call<void>(0x0069A220, game::SCRIPTINSTANCE_SERVER, value, index);
	}

	void Scr_ErrorServer(const char* message)
	{
		cdecl_call<void>(0x0069AB70, message, game::SCRIPTINSTANCE_SERVER, 0);
	}

	void Scr_ObjectErrorServer(const char* message)
	{
		cdecl_call<void>(0x0069AC30, game::SCRIPTINSTANCE_SERVER, message);
	}

	inline game::cg_s* GetCg()
	{
		return reinterpret_cast<game::cg_s*>(0x034732B8);
	}

	inline game::centity_s* CG_GetEntity(const int localClientNum, const unsigned int entNum)
	{
		return cdecl_call<game::centity_s*>(0x004010D0, localClientNum, entNum);
	}

	float Clamp01(const float value)
	{
		if (value < 0.0f)
		{
			return 0.0f;
		}

		if (value > 1.0f)
		{
			return 1.0f;
		}

		return value;
	}

	void CopyVec3(float* out, const float* in)
	{
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
	}

	float Vec3Distance(const float* a, const float* b)
	{
		const float dx = a[0] - b[0];
		const float dy = a[1] - b[1];
		const float dz = a[2] - b[2];
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	bool StringEqualsInsensitive(const char* a, const char* b)
	{
		if (!a || !b)
		{
			return false;
		}

		return _stricmp(a, b) == 0;
	}

	bool ParseBoolToken(const char* value)
	{
		if (!value)
		{
			return false;
		}

		return std::atoi(value) != 0
			|| StringEqualsInsensitive(value, "true")
			|| StringEqualsInsensitive(value, "yes")
			|| StringEqualsInsensitive(value, "on");
	}

	void RumbleStrcpy(char* dest, const char* src, const size_t size)
	{
		if (!dest || !size)
		{
			return;
		}

		if (!src)
		{
			dest[0] = '\0';
			return;
		}

		strncpy_s(dest, size, src, _TRUNCATE);
	}

	void RumbleDebugPrint(const char* format, ...)
	{
		if (!cg_drawrumbledebug || !cg_drawrumbledebug->current.enabled)
		{
			return;
		}

		char buffer[1024]{};
		va_list args;
		va_start(args, format);
		_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
		va_end(args);

		Com_Printf(0, "[rumble] %s\n", buffer);
	}

	void SkipWhitespaceAndComments(const char*& cursor)
	{
		for (;;)
		{
			while (*cursor && std::isspace(static_cast<unsigned char>(*cursor)))
			{
				++cursor;
			}

			if (cursor[0] == '/' && cursor[1] == '/')
			{
				cursor += 2;
				while (*cursor && *cursor != '\n')
				{
					++cursor;
				}
				continue;
			}

			if (cursor[0] == '/' && cursor[1] == '*')
			{
				cursor += 2;
				while (*cursor && !(cursor[0] == '*' && cursor[1] == '/'))
				{
					++cursor;
				}

				if (*cursor)
				{
					cursor += 2;
				}

				continue;
			}

			break;
		}
	}

	bool NextToken(const char*& cursor, std::string& token)
	{
		token.clear();
		SkipWhitespaceAndComments(cursor);

		if (!*cursor)
		{
			return false;
		}

		if (*cursor == '{' || *cursor == '}')
		{
			token.push_back(*cursor++);
			return true;
		}

		if (*cursor == '"')
		{
			++cursor;
			while (*cursor && *cursor != '"')
			{
				if (*cursor == '\\' && cursor[1])
				{
					++cursor;
				}

				token.push_back(*cursor++);
			}

			if (*cursor == '"')
			{
				++cursor;
			}

			return true;
		}

		while (*cursor
			&& !std::isspace(static_cast<unsigned char>(*cursor))
			&& *cursor != '{'
			&& *cursor != '}')
		{
			if (cursor[0] == '/' && (cursor[1] == '/' || cursor[1] == '*'))
			{
				break;
			}

			token.push_back(*cursor++);
		}

		return !token.empty();
	}

	std::string LoadRawTextAsset(const char* assetName)
	{
		game::RawFile* rawFile = cdecl_call<game::RawFile*>(0x0048DA30, ASSET_TYPE_RAWFILE, assetName, false, -1);
		if (!rawFile || !rawFile->buffer)
		{
			return {};
		}

		if (rawFile->len > 0)
		{
			return std::string(rawFile->buffer, rawFile->len);
		}

		return std::string(rawFile->buffer);
	}

	std::string MakeRumbleAssetPath(const char* name)
	{
		if (!name || !name[0])
		{
			return {};
		}

		if (std::strstr(name, "rumble/") == name || std::strstr(name, "rumble\\") == name)
		{
			return name;
		}

		return va("rumble/%s", name);
	}

	RumbleGraph* FindGraph(const char* name)
	{
		for (int i = 0; i < g_rumble.graphCount; ++i)
		{
			if (StringEqualsInsensitive(g_rumble.graphs[i].graphName, name))
			{
				return &g_rumble.graphs[i];
			}
		}

		return nullptr;
	}

	RumbleInfo* FindRumbleInfo(const char* name)
	{
		for (int i = 0; i < g_rumble.infoCount; ++i)
		{
			if (StringEqualsInsensitive(g_rumble.infos[i].name, name))
			{
				return &g_rumble.infos[i];
			}
		}

		return nullptr;
	}

	const char* SkipTaggedPayload(const char* text, const char* tag);

	bool ParseRumbleGraphText(const char* text, RumbleGraph& graph)
	{
		const char* cursor = SkipTaggedPayload(text, "RUMBLEGRAPHFILE");
		std::string token;

		if (!NextToken(cursor, token))
		{
			return false;
		}

		if (token == "{")
		{
			if (!NextToken(cursor, token))
			{
				return false;
			}
		}

		graph.knotCount = std::atoi(token.c_str());
		if (graph.knotCount < 0 || graph.knotCount > 16)
		{
			return false;
		}

		for (int i = 0; i < graph.knotCount; ++i)
		{
			if (!NextToken(cursor, token) || token == "}")
			{
				return false;
			}
			graph.knots[i][0] = static_cast<float>(std::atof(token.c_str()));

			if (!NextToken(cursor, token) || token == "}")
			{
				return false;
			}
			graph.knots[i][1] = static_cast<float>(std::atof(token.c_str()));
		}

		return true;
	}

	bool InfoStringValueForKey(const char* infoString, const char* key, std::string& outValue)
	{
		outValue.clear();
		if (!infoString || !key || !key[0])
		{
			return false;
		}

		const char* cursor = infoString;
		while (*cursor)
		{
			if (*cursor == '\\')
			{
				++cursor;
			}

			const char* keyStart = cursor;
			while (*cursor && *cursor != '\\')
			{
				++cursor;
			}
			std::string currentKey(keyStart, cursor - keyStart);
			if (!*cursor)
			{
				break;
			}

			++cursor;
			const char* valueStart = cursor;
			while (*cursor && *cursor != '\\')
			{
				++cursor;
			}
			std::string currentValue(valueStart, cursor - valueStart);

			if (StringEqualsInsensitive(currentKey.c_str(), key))
			{
				outValue = currentValue;
				return true;
			}
		}

		return false;
	}

	std::string MakeTextSnippet(const char* text, size_t maxLen = 120)
	{
		if (!text)
		{
			return {};
		}

		std::string snippet;
		snippet.reserve(maxLen);

		for (size_t i = 0; text[i] && i < maxLen; ++i)
		{
			unsigned char ch = static_cast<unsigned char>(text[i]);
			if (ch == '\r' || ch == '\n' || ch == '\t')
			{
				snippet.push_back(' ');
			}
			else if (std::isprint(ch))
			{
				snippet.push_back(static_cast<char>(ch));
			}
			else
			{
				snippet.push_back('?');
			}
		}

		return snippet;
	}

	const char* SkipTaggedPayload(const char* text, const char* tag)
	{
		if (!text || !tag)
		{
			return text;
		}

		const size_t tagLen = std::strlen(tag);
		if (_strnicmp(text, tag, tagLen) != 0)
		{
			return text;
		}

		const char* payload = text + tagLen;
		if (*payload == '\\' || *payload == '/' || std::isspace(static_cast<unsigned char>(*payload)))
		{
			while (std::isspace(static_cast<unsigned char>(*payload)))
			{
				++payload;
			}
			return payload;
		}

		return text;
	}

	RumbleGraph* LoadRumbleGraph(const char* name)
	{
		if (!name || !name[0])
		{
			return nullptr;
		}

		RumbleGraph* existing = FindGraph(name);
		if (existing)
		{
			RumbleDebugPrint("using cached graph '%s' with %d knots", name, existing->knotCount);
			return existing;
		}

		if (g_rumble.graphCount >= static_cast<int>(ARRAY_COUNT(g_rumble.graphs)))
		{
			Com_Printf(0, "LoadRumbleGraph: graph limit reached while loading '%s'\n", name);
			return nullptr;
		}

		const std::string assetPath = MakeRumbleAssetPath(name);
		const std::string text = LoadRawTextAsset(assetPath.c_str());
		if (text.empty())
		{
			Com_Printf(0, "LoadRumbleGraph: missing graph '%s'\n", assetPath.c_str());
			return nullptr;
		}

		RumbleGraph& graph = g_rumble.graphs[g_rumble.graphCount];
		std::memset(&graph, 0, sizeof(graph));
		RumbleStrcpy(graph.graphName, name, sizeof(graph.graphName));

		if (!ParseRumbleGraphText(text.c_str(), graph))
		{
			Com_Printf(0, "LoadRumbleGraph: failed to parse '%s' snippet=\"%s\"\n", assetPath.c_str(), MakeTextSnippet(text.c_str()).c_str());
			return nullptr;
		}

		RumbleDebugPrint("loaded graph '%s' with %d knots", name, graph.knotCount);
		++g_rumble.graphCount;
		return &graph;
	}

	bool ParseRumbleInfoText(const char* text, RumbleInfo& info, char* highGraphName, char* lowGraphName)
	{
		const char* infoText = SkipTaggedPayload(text, "RUMBLE");

		if (infoText && infoText[0] == '\\')
		{
			std::string value;

			if (InfoStringValueForKey(infoText, "highRumbleFile", value))
			{
				RumbleStrcpy(highGraphName, value.c_str(), 64);
			}

			if (InfoStringValueForKey(infoText, "lowRumbleFile", value))
			{
				RumbleStrcpy(lowGraphName, value.c_str(), 64);
			}

			if (InfoStringValueForKey(infoText, "duration", value))
			{
				info.durationMs = static_cast<int>(std::atof(value.c_str()) * 1000.0f);
			}

			if (InfoStringValueForKey(infoText, "range", value))
			{
				info.range = static_cast<float>(std::atof(value.c_str()));
			}

			if (InfoStringValueForKey(infoText, "fadeWithDistance", value))
			{
				info.fadeWithDistance = ParseBoolToken(value.c_str());
			}

			if (InfoStringValueForKey(infoText, "broadcast", value))
			{
				info.broadcast = ParseBoolToken(value.c_str());
			}

			if (InfoStringValueForKey(infoText, "durationDvar", value))
			{
				RumbleStrcpy(info.durationDvar, value.c_str(), sizeof(info.durationDvar));
			}

			if (InfoStringValueForKey(infoText, "loopDvar", value))
			{
				RumbleStrcpy(info.loopDvar, value.c_str(), sizeof(info.loopDvar));
			}

			return highGraphName[0] != '\0' && lowGraphName[0] != '\0';
		}

		const char* cursor = infoText;
		std::string key;
		std::string value;

		if (!NextToken(cursor, key))
		{
			return false;
		}

		if (key != "{")
		{
			if (!NextToken(cursor, key) || key != "{")
			{
				return false;
			}
		}

		while (NextToken(cursor, key))
		{
			if (key == "}")
			{
				return true;
			}

			if (!NextToken(cursor, value) || value == "}")
			{
				return false;
			}

			if (StringEqualsInsensitive(key.c_str(), "highRumbleFile"))
			{
				RumbleStrcpy(highGraphName, value.c_str(), 64);
			}
			else if (StringEqualsInsensitive(key.c_str(), "lowRumbleFile"))
			{
				RumbleStrcpy(lowGraphName, value.c_str(), 64);
			}
			else if (StringEqualsInsensitive(key.c_str(), "duration"))
			{
				info.durationMs = static_cast<int>(std::atof(value.c_str()) * 1000.0f);
			}
			else if (StringEqualsInsensitive(key.c_str(), "range"))
			{
				info.range = static_cast<float>(std::atof(value.c_str()));
			}
			else if (StringEqualsInsensitive(key.c_str(), "fadeWithDistance"))
			{
				info.fadeWithDistance = ParseBoolToken(value.c_str());
			}
			else if (StringEqualsInsensitive(key.c_str(), "broadcast"))
			{
				info.broadcast = ParseBoolToken(value.c_str());
			}
			else if (StringEqualsInsensitive(key.c_str(), "durationDvar"))
			{
				RumbleStrcpy(info.durationDvar, value.c_str(), sizeof(info.durationDvar));
			}
			else if (StringEqualsInsensitive(key.c_str(), "loopDvar"))
			{
				RumbleStrcpy(info.loopDvar, value.c_str(), sizeof(info.loopDvar));
			}
		}

		return true;
	}

	RumbleInfo* LoadRumbleInfo(const char* name)
	{
		if (!name || !name[0])
		{
			return nullptr;
		}

		RumbleInfo* existing = FindRumbleInfo(name);
		if (existing)
		{
			RumbleDebugPrint("using cached rumble '%s'", name);
			return existing;
		}

		if (g_rumble.infoCount >= static_cast<int>(ARRAY_COUNT(g_rumble.infos)))
		{
			Com_Printf(0, "LoadRumbleInfo: rumble limit reached while loading '%s'\n", name);
			return nullptr;
		}

		const std::string assetPath = MakeRumbleAssetPath(name);
		const std::string text = LoadRawTextAsset(assetPath.c_str());
		if (text.empty())
		{
			Com_Printf(0, "LoadRumbleInfo: missing rumble '%s'\n", assetPath.c_str());
			return nullptr;
		}

		RumbleInfo& info = g_rumble.infos[g_rumble.infoCount];
		char highGraphName[64]{};
		char lowGraphName[64]{};
		std::memset(&info, 0, sizeof(info));
		RumbleStrcpy(info.name, name, sizeof(info.name));

		if (!ParseRumbleInfoText(text.c_str(), info, highGraphName, lowGraphName))
		{
			Com_Printf(0, "LoadRumbleInfo: failed to parse '%s' snippet=\"%s\"\n", assetPath.c_str(), MakeTextSnippet(text.c_str()).c_str());
			return nullptr;
		}

		if (info.broadcast && info.range == 0.0f)
		{
			Com_Printf(0, "LoadRumbleInfo: broadcast rumble '%s' needs a non-zero range\n", name);
			return nullptr;
		}

		info.highGraph = LoadRumbleGraph(highGraphName);
		info.lowGraph = LoadRumbleGraph(lowGraphName);
		if (!info.highGraph || !info.lowGraph)
		{
			Com_Printf(0, "LoadRumbleInfo: graph load failed for '%s'\n", name);
			return nullptr;
		}

		RumbleDebugPrint(
			"loaded rumble '%s' duration=%dms range=%.2f fade=%d broadcast=%d high='%s' low='%s'",
			name,
			info.durationMs,
			info.range,
			info.fadeWithDistance ? 1 : 0,
			info.broadcast ? 1 : 0,
			highGraphName,
			lowGraphName);
		++g_rumble.infoCount;
		return &info;
	}

	float GraphGetValueFromFraction(const RumbleGraph& graph, const float fraction)
	{
		if (graph.knotCount <= 0)
		{
			return 0.0f;
		}

		if (fraction <= graph.knots[0][0])
		{
			return graph.knots[0][1];
		}

		for (int i = 1; i < graph.knotCount; ++i)
		{
			if (fraction <= graph.knots[i][0])
			{
				const float startX = graph.knots[i - 1][0];
				const float endX = graph.knots[i][0];
				const float startY = graph.knots[i - 1][1];
				const float endY = graph.knots[i][1];

				if (endX <= startX)
				{
					return endY;
				}

				const float lerp = (fraction - startX) / (endX - startX);
				return startY + (endY - startY) * lerp;
			}
		}

		return graph.knots[graph.knotCount - 1][1];
	}

	int GetRumbleDurationMs(const RumbleInfo& info)
	{
		if (info.durationDvar[0])
		{
			dvar_t* durationDvar = Dvars::Functions::Dvar_FindVar(info.durationDvar);
			if (durationDvar)
			{
				return static_cast<int>(durationDvar->current.value * 1000.0f);
			}
		}

		return info.durationMs;
	}

	bool ShouldLoopFromDvar(const RumbleInfo& info)
	{
		if (!info.loopDvar[0])
		{
			return false;
		}

		dvar_t* loopDvar = Dvars::Functions::Dvar_FindVar(info.loopDvar);
		return loopDvar && loopDvar->current.enabled;
	}

	void InvalidateActiveRumble(ActiveRumble& rumble)
	{
		rumble.rumbleInfo = nullptr;
		rumble.startTime = -1;
		rumble.loop = false;
		rumble.sourceType = RUMBLE_SOURCE_INVALID;
		rumble.entityNum = RUMBLE_ENTITY_NONE;
		rumble.pos[0] = 0.0f;
		rumble.pos[1] = 0.0f;
		rumble.pos[2] = 0.0f;
	}

	bool IsLocalPlayerEntity(const int entNum)
	{
		game::cg_s* cg = GetCg();
		return cg && cg->nextSnap && entNum == cg->nextSnap->ps.clientNum;
	}

	bool GetEntitySourcePosition(const int entNum, float* outPos)
	{
		if (entNum < 0 || entNum >= 1024)
		{
			return false;
		}

		if (!g_entities[entNum].r.inuse)
		{
			return false;
		}

		CopyVec3(outPos, g_entities[entNum].r.currentOrigin);
		return true;
	}

	int FindClosestToDyingActiveRumble(const int currentTime)
	{
		int bestIndex = 0;
		int smallestRemaining = INT_MAX;

		for (int i = 0; i < static_cast<int>(ARRAY_COUNT(g_rumble.active)); ++i)
		{
			ActiveRumble& active = g_rumble.active[i];
			if (!active.rumbleInfo)
			{
				return i;
			}

			int durationMs = GetRumbleDurationMs(*active.rumbleInfo);
			if (durationMs <= 0)
			{
				return i;
			}

			const int remaining = durationMs - (currentTime - active.startTime);
			if (remaining < smallestRemaining)
			{
				smallestRemaining = remaining;
				bestIndex = i;
			}
		}

		return bestIndex;
	}

	int NextAvailableRumbleSlot(const int currentTime)
	{
		for (int i = 0; i < static_cast<int>(ARRAY_COUNT(g_rumble.active)); ++i)
		{
			if (!g_rumble.active[i].rumbleInfo)
			{
				return i;
			}
		}

		return FindClosestToDyingActiveRumble(currentTime);
	}

	void EnsureXInput()
	{
		if (g_xinputGetState && g_xinputSetState)
		{
			return;
		}

		g_xinputGetState = reinterpret_cast<XInputGetState_t>(0x007C445C);
		g_xinputSetState = reinterpret_cast<XInputSetState_t>(0x007C4462);

		if (!g_xinputGetState || !g_xinputSetState)
		{
			RumbleDebugPrint("failed to assign engine XInput thunks");
		}
		else
		{
			RumbleDebugPrint("using engine XInput thunks get=0x7C445C set=0x7C4462");
		}
	}

	void ApplyRumbleState(float low, float high)
	{
		low = Clamp01(low);
		high = Clamp01(high);

		s_gamePads[0].lowRumble = low;
		s_gamePads[0].highRumble = high;

		if (!gpad_rumble || !gpad_rumble->current.enabled)
		{
			low = 0.0f;
			high = 0.0f;
		}
		else if (gpad_rumbleScale)
		{
			const float scale = Clamp01(gpad_rumbleScale->current.value);
			low *= scale;
			high *= scale;
		}

		EnsureXInput();
		if (!g_xinputSetState)
		{
			return;
		}

		XInputVibrationState vibration{};
		vibration.leftMotor = static_cast<WORD>(Clamp01(low) * 65535.0f);
		vibration.rightMotor = static_cast<WORD>(Clamp01(high) * 65535.0f);

		DWORD stateResults[4]{};
		DWORD setResults[4]{};
		bool anyConnected = false;
		bool anySuccess = false;
		for (DWORD userIndex = 0; userIndex < 4; ++userIndex)
		{
			bool connected = true;
			if (g_xinputGetState)
			{
				XInputState state{};
				stateResults[userIndex] = g_xinputGetState(userIndex, &state);
				connected = stateResults[userIndex] == ERROR_SUCCESS;
			}
			else
			{
				stateResults[userIndex] = ERROR_PROC_NOT_FOUND;
			}

			if (!connected)
			{
				setResults[userIndex] = ERROR_DEVICE_NOT_CONNECTED;
				continue;
			}

			anyConnected = true;
			setResults[userIndex] = g_xinputSetState(userIndex, &vibration);
			if (setResults[userIndex] == ERROR_SUCCESS)
			{
				anySuccess = true;
			}
		}

		if ((gpad_rumble_debug_spam && gpad_rumble_debug_spam->current.enabled) || ((!anySuccess || !anyConnected) && (low > 0.0f || high > 0.0f)))
		{
			Com_Printf(
				0,
				"rumble setstate low=%.2f high=%.2f get=[%lu,%lu,%lu,%lu] set=[%lu,%lu,%lu,%lu]\n",
				low,
				high,
				static_cast<unsigned long>(stateResults[0]),
				static_cast<unsigned long>(stateResults[1]),
				static_cast<unsigned long>(stateResults[2]),
				static_cast<unsigned long>(stateResults[3]),
				static_cast<unsigned long>(setResults[0]),
				static_cast<unsigned long>(setResults[1]),
				static_cast<unsigned long>(setResults[2]),
				static_cast<unsigned long>(setResults[3]));
		}
	}

	void StopAllRumblesInternal()
	{
		for (int i = 0; i < static_cast<int>(ARRAY_COUNT(g_rumble.active)); ++i)
		{
			InvalidateActiveRumble(g_rumble.active[i]);
		}

		g_manualRumbleEndTime = 0;
		g_manualRumbleLow = 0.0f;
		g_manualRumbleHigh = 0.0f;
		ApplyRumbleState(0.0f, 0.0f);
	}

	void StopRumbleInternal(const char* name, const int entityNum)
	{
		for (int i = 0; i < static_cast<int>(ARRAY_COUNT(g_rumble.active)); ++i)
		{
			ActiveRumble& active = g_rumble.active[i];
			if (!active.rumbleInfo)
			{
				continue;
			}

			if (entityNum != RUMBLE_ENTITY_NONE && (active.sourceType != RUMBLE_SOURCE_ENTITY || active.entityNum != entityNum))
			{
				continue;
			}

			if (StringEqualsInsensitive(active.rumbleInfo->name, name))
			{
				RumbleDebugPrint("stopped rumble '%s' on entity %d", name, entityNum);
				InvalidateActiveRumble(active);
			}
		}
	}

	bool PlayRumbleInternal(const char* name, const bool loop, const RumbleSourceType sourceType, const int entityNum, const float* pos)
	{
		game::cg_s* cg = GetCg();
		if (!cg || !cg->nextSnap)
		{
			return false;
		}

		RumbleInfo* info = LoadRumbleInfo(name);
		if (!info)
		{
			return false;
		}

		const int slot = NextAvailableRumbleSlot(cg->time);
		ActiveRumble& active = g_rumble.active[slot];
		InvalidateActiveRumble(active);

		active.rumbleInfo = info;
		active.startTime = cg->time;
		active.loop = loop || ShouldLoopFromDvar(*info);
		active.sourceType = sourceType;
		active.entityNum = entityNum;

		if (pos)
		{
			CopyVec3(active.pos, pos);
		}

		RumbleDebugPrint(
			"started rumble '%s' loop=%d source=%d ent=%d pos=(%.2f, %.2f, %.2f)",
			name,
			active.loop ? 1 : 0,
			static_cast<int>(sourceType),
			entityNum,
			active.pos[0],
			active.pos[1],
			active.pos[2]);

		return true;
	}

	void UpdateRumbles()
	{
		game::cg_s* cg = GetCg();
		float maxLow = 0.0f;
		float maxHigh = 0.0f;
		const DWORD currentTick = GetTickCount();

		if (currentTick < g_manualRumbleEndTime)
		{
			maxLow = g_manualRumbleLow;
			maxHigh = g_manualRumbleHigh;
		}
		else if (g_manualRumbleEndTime)
		{
			g_manualRumbleEndTime = 0;
			g_manualRumbleLow = 0.0f;
			g_manualRumbleHigh = 0.0f;
		}

		if (!cg || !cg->nextSnap)
		{
			ApplyRumbleState(maxLow, maxHigh);
			return;
		}

		float sourcePos[3]{};
		const float* receiverPos = cg->refdef.vieworg;

		for (int i = 0; i < static_cast<int>(ARRAY_COUNT(g_rumble.active)); ++i)
		{
			ActiveRumble& active = g_rumble.active[i];
			if (!active.rumbleInfo)
			{
				continue;
			}

			RumbleInfo& info = *active.rumbleInfo;
			const int durationMs = GetRumbleDurationMs(info);
			if (durationMs <= 0)
			{
				InvalidateActiveRumble(active);
				continue;
			}

			int elapsed = cg->time - active.startTime;
			if (elapsed >= durationMs)
			{
				if (active.loop || ShouldLoopFromDvar(info))
				{
					active.startTime = cg->time;
					elapsed = 0;
				}
				else
				{
					InvalidateActiveRumble(active);
					continue;
				}
			}

			float distanceScale = 1.0f;
			if (active.sourceType == RUMBLE_SOURCE_ENTITY)
			{
				if (!info.broadcast)
				{
					if (!cg->nextSnap || active.entityNum != cg->nextSnap->ps.clientNum)
					{
						continue;
					}
				}
				else if (!GetEntitySourcePosition(active.entityNum, sourcePos))
				{
					InvalidateActiveRumble(active);
					continue;
				}
			}
			else if (active.sourceType == RUMBLE_SOURCE_POS)
			{
				CopyVec3(sourcePos, active.pos);
			}

			if (active.sourceType != RUMBLE_SOURCE_INVALID && info.range > 0.0f)
			{
				if (active.sourceType == RUMBLE_SOURCE_ENTITY && !info.broadcast)
				{
					distanceScale = 1.0f;
				}
				else
				{
					const float distance = Vec3Distance(receiverPos, sourcePos);
					if (distance > info.range)
					{
						continue;
					}

					if (info.fadeWithDistance)
					{
						distanceScale *= 1.0f - (distance / info.range);
					}
				}
			}

			const float fraction = Clamp01(static_cast<float>(elapsed) / static_cast<float>(durationMs));
			const float low = Clamp01(GraphGetValueFromFraction(*info.lowGraph, fraction) * distanceScale);
			const float high = Clamp01(GraphGetValueFromFraction(*info.highGraph, fraction) * distanceScale);

			if (low > maxLow)
			{
				maxLow = low;
			}

			if (high > maxHigh)
			{
				maxHigh = high;
			}
		}

		ApplyRumbleState(maxLow, maxHigh);
	}

	void PlayWeaponFireRumble(gentity_s* attacker, const game::WeaponDef* weaponDef)
	{
		if (!attacker || !attacker->client || !weaponDef || !weaponDef->fireRumble || !weaponDef->fireRumble[0])
		{
			return;
		}

		if (!IsLocalPlayerEntity(attacker->s.number))
		{
			return;
		}

		PlayRumbleInternal(weaponDef->fireRumble, false, RUMBLE_SOURCE_ENTITY, attacker->s.number, nullptr);
	}

	void PlayWeaponMeleeImpactRumble(gentity_s* attacker, const game::weaponParms* weaponParams, gentity_s* hitEnt)
	{
		if (!hitEnt || !attacker || !attacker->client || !weaponParams || !weaponParams->weapDef)
		{
			return;
		}

		if (!IsLocalPlayerEntity(attacker->s.number))
		{
			return;
		}

		const game::WeaponDef* weaponDef = weaponParams->weapDef;
		if (!weaponDef->meleeImpactRumble || !weaponDef->meleeImpactRumble[0])
		{
			return;
		}

		PlayRumbleInternal(weaponDef->meleeImpactRumble, false, RUMBLE_SOURCE_ENTITY, attacker->s.number, nullptr);
	}

	void CG_PlayRumble_f()
	{
		if (Cmd_Argc() < 2)
		{
			Com_Printf(0, "usage: playrumble <rumblename>\n");
			return;
		}

		game::cg_s* cg = GetCg();
		if (!cg || !cg->nextSnap)
		{
			Com_Printf(0, "playrumble: no active cg snapshot\n");
			return;
		}

		if (!PlayRumbleInternal(Cmd_Argv(1), false, RUMBLE_SOURCE_ENTITY, cg->nextSnap->ps.clientNum, nullptr))
		{
			Com_Printf(0, "playrumble: failed to start '%s'\n", Cmd_Argv(1));
		}
		else
		{
			Com_Printf(0, "playrumble: started '%s'\n", Cmd_Argv(1));
		}
	}

	void CG_TestRumbleMotors_f()
	{
		if (Cmd_Argc() < 3)
		{
			Com_Printf(0, "usage: testrumblemotors <low 0-1> <high 0-1> [durationMs]\n");
			return;
		}

		const float low = Clamp01(static_cast<float>(std::atof(Cmd_Argv(1))));
		const float high = Clamp01(static_cast<float>(std::atof(Cmd_Argv(2))));
		int durationMs = Cmd_Argc() >= 4 ? std::atoi(Cmd_Argv(3)) : 500;
		if (durationMs < 1)
		{
			durationMs = 1;
		}
		game::cg_s* cg = GetCg();

		if (cg && cg->nextSnap)
		{
			RumbleDebugPrint("manual rumble armed with active snapshot time=%d", cg->time);
		}
		g_manualRumbleLow = low;
		g_manualRumbleHigh = high;
		g_manualRumbleEndTime = GetTickCount() + static_cast<DWORD>(durationMs);

		ApplyRumbleState(low, high);
		Com_Printf(0, "testrumblemotors: low=%.2f high=%.2f duration=%dms\n", low, high, durationMs);
		RumbleDebugPrint("manual motor test low=%.2f high=%.2f duration=%dms", low, high, durationMs);
	}

	void GScr_RumbleMethodError(const char* message)
	{
		Scr_ObjectErrorServer(message);
	}

	void GScr_RumbleFunctionError(const char* message)
	{
		Scr_ErrorServer(message);
	}
}

gentity_s* __cdecl Weapon_Melee_Hook(gentity_s* attacker, game::weaponParms* weaponParams, float range, float width, float height)
{
	gentity_s* hitEnt = Weapon_Melee_hook.unsafe_ccall<gentity_s*>(attacker, weaponParams, range, width, height);
	PlayWeaponMeleeImpactRumble(attacker, weaponParams, hitEnt);
	return hitEnt;
}

void GScr_PreCacheRumble(scr_entref_t)
{
	if (Scr_GetNumParamServer() != 1)
	{
		GScr_RumbleFunctionError("PreCacheRumble requires 1 parameter");
		return;
	}

	LoadRumbleInfo(Scr_GetStringServer(0));
}

void GScr_StopAllRumbles(scr_entref_t)
{
	StopAllRumblesInternal();
}

void GScr_PlayRumbleOnPos(scr_entref_t)
{
	if (Scr_GetNumParamServer() != 2)
	{
		GScr_RumbleFunctionError("PlayRumbleOnPos requires 2 parameters");
		return;
	}

	float pos[3]{};
	Scr_GetVectorServer(pos, 1);
	PlayRumbleInternal(Scr_GetStringServer(0), false, RUMBLE_SOURCE_POS, RUMBLE_ENTITY_NONE, pos);
}

void GScr_PlayLoopRumbleOnPos(scr_entref_t)
{
	if (Scr_GetNumParamServer() != 2)
	{
		GScr_RumbleFunctionError("PlayLoopRumbleOnPos requires 2 parameters");
		return;
	}

	float pos[3]{};
	Scr_GetVectorServer(pos, 1);
	PlayRumbleInternal(Scr_GetStringServer(0), true, RUMBLE_SOURCE_POS, RUMBLE_ENTITY_NONE, pos);
}

void GScr_PlayRumble(scr_entref_t entref)
{
	if (entref.classnum != game::CLASS_NUM_ENTITY)
	{
		GScr_RumbleMethodError("PlayRumble can only be called on entities");
		return;
	}

	if (Scr_GetNumParamServer() != 1)
	{
		GScr_RumbleMethodError("PlayRumble requires 1 parameter");
		return;
	}

	PlayRumbleInternal(Scr_GetStringServer(0), false, RUMBLE_SOURCE_ENTITY, entref.entnum, nullptr);
}

void GScr_PlayLoopRumble(scr_entref_t entref)
{
	if (entref.classnum != game::CLASS_NUM_ENTITY)
	{
		GScr_RumbleMethodError("PlayLoopRumble can only be called on entities");
		return;
	}

	if (Scr_GetNumParamServer() != 1)
	{
		GScr_RumbleMethodError("PlayLoopRumble requires 1 parameter");
		return;
	}

	PlayRumbleInternal(Scr_GetStringServer(0), true, RUMBLE_SOURCE_ENTITY, entref.entnum, nullptr);
}

void GScr_StopRumble(scr_entref_t entref)
{
	if (entref.classnum != game::CLASS_NUM_ENTITY)
	{
		GScr_RumbleMethodError("StopRumble can only be called on entities");
		return;
	}

	if (Scr_GetNumParamServer() != 1)
	{
		GScr_RumbleMethodError("StopRumble requires 1 parameter");
		return;
	}

	StopRumbleInternal(Scr_GetStringServer(0), entref.entnum);
}

void PatchT4E_Rumble_RegisterScriptBindings()
{
	Scr_DeclareFunction("PreCacheRumble", GScr_PreCacheRumble, false);
	Scr_DeclareFunction("PlayRumbleOnPos", GScr_PlayRumbleOnPos, false);
	Scr_DeclareFunction("PlayLoopRumbleOnPos", GScr_PlayLoopRumbleOnPos, false);
	Scr_DeclareFunction("StopAllRumbles", GScr_StopAllRumbles, false);

	Scr_DeclareMethod("PlayRumble", GScr_PlayRumble, false);
	Scr_DeclareMethod("PlayLoopRumble", GScr_PlayLoopRumble, false);
	Scr_DeclareMethod("StopRumble", GScr_StopRumble, false);
}

void PatchT4E_Rumble_Frame()
{
	UpdateRumbles();
}

void PatchT4E_Rumble()
{
	for (int i = 0; i < static_cast<int>(ARRAY_COUNT(g_rumble.active)); ++i)
	{
		InvalidateActiveRumble(g_rumble.active[i]);
	}

	gpad_rumble = Dvar_RegisterBool(true, "gpad_rumble", DVAR_FLAG_ARCHIVE | DVAR_FLAG_SAVED, "Enable controller rumble");
	gpad_rumbleScale = Dvar_RegisterFloat("gpad_rumbleScale", 1.0f, 0.0f, 1.0f, DVAR_FLAG_ARCHIVE | DVAR_FLAG_SAVED, "Global controller rumble scale");
	cg_drawrumbledebug = Dvar_RegisterBool(false, "cg_drawrumbledebug", 0, "Print rumble debug information");
	gpad_rumble_debug_spam = Dvar_RegisterBool(false, "gpad_rumble_debug_spam", 0, "Print XInput rumble SetState results");

	Cmd_AddCommand("playrumble", CG_PlayRumble_f);
	Cmd_AddCommand("testrumblemotors", CG_TestRumbleMotors_f);

	static auto bullet_fire_rumble = safetyhook::create_mid(0x004E6868, [](SafetyHookContext& ctx)
	{
		PlayWeaponFireRumble(reinterpret_cast<gentity_s*>(ctx.edi), reinterpret_cast<game::weaponParms*>(ctx.esi)->weapDef);
	});

	Weapon_Melee_hook = safetyhook::create_inline(0x005501C0, Weapon_Melee_Hook);
}
