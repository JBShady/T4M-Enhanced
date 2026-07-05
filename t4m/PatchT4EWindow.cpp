// ==========================================================
// T4M-Enhanced project
// 
//
// Initial Purpose: Re-implement safe area from console version
//
// Initial author: Clippy95 / JerryALT (copied from 
//                 Window.cpp for safearea) (iw3sp_mod project)
// Adapted: 2025-01-13
// Started: 2025-01-13
// ==========================================================

#include <safetyhook.hpp>
#include "StdInc.h"
dvar_t* safeArea_horizontal;
dvar_t* safeArea_vertical;

//dvar_t* safeArea_horizontal_player;
//dvar_t* safeArea_vertical_player;

float safeArea_horizontal_set = 1.f;
float safeArea_vertical_set = 1.f;

//#define safeArea_horz safeArea_horizontal_player->current.value

//#define safeArea_vert safeArea_vertical_player->current.value

#define safeArea_horz safeArea_horizontal_set

#define safeArea_vert safeArea_vertical_set

struct ScreenPlacement
{
	float scaleVirtualToReal[2];
	float scaleVirtualToFull[2];
	float scaleRealToVirtual[2];
	float virtualViewableMin[2];
	float virtualViewableMax[2];
	float realViewportSize[2];
	float realViewableMin[2];
	float realViewableMax[2];
	float subScreen[2];
};


float* ScrPlace_CalcSafeAreaOffsets(float* realViewableMin, float* realViewableMax, float viewportX, float viewportY, float viewportWidth, float viewportHeight, float aspect, float safeAreaRatioHorz, float safeAreaRatioVert, float* virtualViewableMin, float* virtualViewableMax, int width, int height)
{
	double v15;
	double v16;
	double v17;
	double v18;
	double v19;
	double v20;
	float v21;
	float v22;
	float v23;
	float v24;
	float v25;
	float v26;
	float v27;
	float v28;
	float a10a;
	float a11c;
	float a11a;
	float a11b;
	float a12b;
	float a12c;
	float a12d;
	int a12a;

	a11c = (float)width;
	v21 = (1.0 - safeAreaRatioHorz) * 0.5 * a11c;
	a10a = (float)height;
	v23 = (1.0 - safeAreaRatioVert) * 0.5 * a10a;
	a12b = v21 + 0.5;
	a12c = floor(a12b);
	v22 = a12c;
	a12d = v23 + 0.5;
	*(float*)&a12a = floor(a12d);
	v27 = a11c - v22;
	v28 = a10a - *(float*)&a12a;
	if (viewportX >= (double)v22)
		v22 = viewportX;
	if (viewportY < (double)*(float*)&a12a)
	{
		v15 = viewportY;
		v16 = viewportX;
		v24 = *(float*)&a12a;
	}
	else
	{
		v15 = viewportY;
		v16 = viewportX;
		v24 = viewportY;
	}
	v17 = viewportWidth;
	a11a = v16 + viewportWidth;
	v18 = v27;
	if (a11a <= (double)v27)
		v18 = a11a;
	v25 = v18;
	v19 = viewportHeight;
	a11b = v15 + viewportHeight;
	v20 = v28;
	if (a11b <= (double)v28)
		v20 = a11b;
	v26 = v20;
	*realViewableMin = v22 - v16;
	realViewableMin[1] = v24 - v15;
	*realViewableMax = v25 - v16;
	realViewableMax[1] = v26 - v15;
	*virtualViewableMin = *realViewableMin * aspect * (640.0 / v17);
	virtualViewableMin[1] = realViewableMin[1] * (480.0 / v19);
	*virtualViewableMax = 640.0 / v17 * (aspect * *realViewableMax);
	virtualViewableMax[1] = 480.0 / v19 * realViewableMax[1];

	return virtualViewableMin;
}

float* ScrPlace_SetupViewport(ScreenPlacement *scrPlace, float viewportX, float viewportY, float viewportWidth, float viewportHeight)
{

	float vidConfig_aspectRatioScenePixel = *(float*)0x04DA90D4;

	viewportX = 0.0f;
	viewportY = 0.0f;

	viewportWidth = (float)*(int*)0x03BED830;
	viewportHeight = (float)*(int*)0x03BED834;

	vidConfig_aspectRatioScenePixel = 1.f;
	scrPlace->realViewportSize[0] = viewportWidth;
	scrPlace->realViewportSize[1] = viewportHeight;
	float adjustedRealWidth = 1.333333373069763 * viewportHeight / vidConfig_aspectRatioScenePixel;
	if (adjustedRealWidth > viewportWidth)
		adjustedRealWidth = viewportWidth;

	float* result = ScrPlace_CalcSafeAreaOffsets(
		scrPlace->realViewableMin,
		scrPlace->realViewableMax,
		viewportX,
		viewportY,
		viewportWidth,
		viewportHeight,
		viewportWidth / adjustedRealWidth,
		safeArea_horz,
		safeArea_vert,
		scrPlace->virtualViewableMin,
		scrPlace->virtualViewableMax,
		viewportWidth,
		viewportHeight
	);

	scrPlace->scaleVirtualToReal[0] = adjustedRealWidth / 640.0;
	scrPlace->scaleVirtualToReal[1] = viewportHeight / 480.0;
	scrPlace->scaleVirtualToFull[0] = viewportWidth / 640.0;
	scrPlace->scaleVirtualToFull[1] = viewportHeight / 480.0;
	scrPlace->scaleRealToVirtual[0] = 640.0 / adjustedRealWidth;
	scrPlace->scaleRealToVirtual[1] = 480.0 / viewportHeight;
	scrPlace->subScreen[0] = (viewportWidth - adjustedRealWidth) * 0.5;
	scrPlace->subScreen[1] = 0.f;
	return result;
}

__declspec(naked) void ScrPlace_SetupFloatViewportDetour() {
	__asm {
		pushad
		pushfd

		mov ecx, edi


		push[esp + 0x24 + 0x20]
		push[esp + 0x24 + 0x1C]
		push[esp + 0x24 + 0x18]
		push[esp + 0x24 + 0x14]
		push ecx

		call ScrPlace_SetupViewport

		add esp, 20

		popfd
		popad

		ret
	}
}

void CL_ResetViewport() {

	ScrPlace_SetupViewport((ScreenPlacement*)0x009573A8, 0, 0, 0, 0);
	ScrPlace_SetupViewport((ScreenPlacement*)0x00957360, 0, 0, 0, 0);
	ScrPlace_SetupViewport((ScreenPlacement*)0x00957318, 0, 0, 0, 0);
}
dvar_t* safeArea_updateLive;
void UpdateSafeAreaLive() {
	dvar_t* cl_paused = *(dvar_t**)0x01F552C4;
	dvar_t* cl_ingame = *(dvar_t**)0x03058418;

	if (safeArea_updateLive->current.integer == 1) {
		if ((cl_paused->modified || cl_ingame->modified || safeArea_updateLive->modified) || (safeArea_horizontal->modified || safeArea_vertical->modified)) {
			safeArea_horizontal->modified = false;
			safeArea_vertical->modified = false;

			if (cl_paused->current.integer == 0 && cl_ingame->current.enabled) {
				safeArea_horz = safeArea_horizontal->current.value;
				safeArea_vert = safeArea_vertical->current.value;
			}
			else if (cl_paused->current.integer || cl_ingame->current.enabled == 0) {
				safeArea_horz = 1.f;  // Both to 1.f for mode 1
				safeArea_vert = 1.f;
			}

			cl_paused->modified = false;
			cl_ingame->modified = false;
			safeArea_updateLive->modified = false;
			CL_ResetViewport();
		}
	}
	else if (safeArea_updateLive->current.integer == 2) {
		if ((cl_paused->modified ||  cl_ingame->modified) || safeArea_updateLive->modified || (safeArea_horizontal->modified || safeArea_vertical->modified)) {
			safeArea_horizontal->modified = false;
			safeArea_vertical->modified = false;

			if (cl_paused->current.integer == 0 && cl_ingame->current.enabled) {
				safeArea_horz = safeArea_horizontal->current.value;
				safeArea_vert = safeArea_vertical->current.value;
			}
			else if (cl_paused->current.integer || cl_ingame->current.enabled == 0) {
				safeArea_horz = safeArea_horizontal->current.value;  // Keep user's horz for mode 2
				safeArea_vert = 1.f;  // Only vert to 1.f
			}

			cl_paused->modified = false;
			cl_ingame->modified = false;
			safeArea_updateLive->modified = false;
			CL_ResetViewport();
		}
	}
	else if (safeArea_updateLive->current.integer >= 3 && (safeArea_horizontal->modified || safeArea_vertical->modified) || safeArea_updateLive->modified) {
		safeArea_horizontal->modified = false;
		safeArea_vertical->modified = false;
		safeArea_horz = safeArea_horizontal->current.value;
		safeArea_vert = safeArea_vertical->current.value;
		safeArea_updateLive->modified = false;
		CL_ResetViewport();
	}
}




void PatchT4E_Window() {

	static auto AspectRatioFix = safetyhook::create_mid(0x006D4FA0, [](SafetyHookContext& ctx) {
		// lazy af for proper hook so this will just work for now


		*(float*)0x008AFAC0 = (float)ctx.edx / (float)ctx.ecx;

		});

	safeArea_updateLive = Dvar_RegisterInt(2, "safeArea_updateLive", 0, 3, DVAR_FLAG_ARCHIVE,
		"Automatically updates the viewport when safearea is updated\n"
		"1 = applies safearea only when cl_paused == 0 && cl_ingame == 0, otherwise both = 1.f\n"
		"2 = same as 1 but only sets vertical = 1.f when paused/running\n"
		"3 = applies safearea always");
	safeArea_horizontal = Dvar_RegisterFloat("safeArea_horizontal", 1.0f, 0.15f, 1.0f, DVAR_FLAG_ARCHIVE, "Horizontal safe area as a fraction of the screen width");
	safeArea_vertical = Dvar_RegisterFloat("safeArea_vertical", 1.0f, 0.15f, 1.0f, DVAR_FLAG_ARCHIVE, "Vertical safe area as a fraction of the screen height");

	safeArea_vertical->modified = true;
	safeArea_horizontal->modified = true;

	//safeArea_horizontal_player = Dvar_RegisterFloat("safeArea_horizontal_applied", 1.0f, 0.15f, 1.0f, DVAR_FLAG_ARCHIVE | DVAR_FLAG_ROM, "Horizontal safe area as a fraction of the screen width");
	//safeArea_vertical_player = Dvar_RegisterFloat("safeArea_vertical_applied", 1.0f, 0.15f, 1.0f, DVAR_FLAG_ARCHIVE | DVAR_FLAG_ROM, "Vertical safe area as a fraction of the screen height");

	//static auto whatever = safetyhook::create_mid(0x005EF938, [](SafetyHookContext& ctx) {
	//	const char* name = (const char*)(ctx.ebx);

	//	if (strcmp(name, "cl_paused") == 0) {
	//		auto& value = ctx.eax;
	//		if (value == 0) {

	//			safeArea_horizontal->current.value = safeArea_horizontal_player->current.value;
	//			safeArea_vertical->current.value = safeArea_vertical_player->current.value;
	//			safeArea_horizontal->modified = true;
	//			safeArea_vertical->modified = true;

	//		}
	//		else if (value == 1) {

	//			safeArea_horizontal->current.value = 1.f;
	//			safeArea_vertical->current.value = 1.f;
	//			safeArea_horizontal->modified = true;
	//			safeArea_vertical->modified = true;

	//		}

	//	}
	//	});

	Detours::X86::DetourFunction((PBYTE)0x00644CCB, (PBYTE)&ScrPlace_SetupFloatViewportDetour, Detours::X86Option::USE_CALL);
	Detours::X86::DetourFunction((PBYTE)0x00644CEE, (PBYTE)&ScrPlace_SetupFloatViewportDetour, Detours::X86Option::USE_CALL);
	Detours::X86::DetourFunction((PBYTE)0x00644D11, (PBYTE)&ScrPlace_SetupFloatViewportDetour, Detours::X86Option::USE_CALL);

}

