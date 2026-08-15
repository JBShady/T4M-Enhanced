#include "t4_headers.h"
#include "StdInc.h"
#include "MemoryMgr.h"
#include <safetyhook.hpp>

typedef void*(__cdecl* R_CreateDynamicBuffersT)();
R_CreateDynamicBuffersT R_CreateDynamicBuffers = nullptr;

constexpr uint32_t MB_SIZE = 1048576; // 1 MB

using namespace Memory::VP;

dvar_t* r_increase_render_buffers;

dvar_t* r_buf_skinnedCacheVb = nullptr;

dvar_t* r_buf_tempSkin = nullptr;
dvar_t* r_buf_dynamicVertexBuffer = nullptr;
dvar_t* r_buf_dynamicIndexBuffer = nullptr;
dvar_t* r_buf_preTessIndexBuffer = nullptr;

// Raises limits of several buffers, reference taken from iw3xo-dev by xoxor4d https://github.com/xoxor4d/iw3xo-dev
void* __cdecl R_CreateDynamicBuffers_hook() {
	if (r_increase_render_buffers && r_increase_render_buffers->current.boolean) {
		if (r_buf_dynamicVertexBuffer) {
			// Original: 1 MB. Patches capacity field + D3D size + error message.
			const uint32_t vbSize = r_buf_dynamicVertexBuffer->current.integer * MB_SIZE;

			Patch<uint32_t>((0x0070EC1A + 3), vbSize); 
			Patch<uint32_t>((0x0070EC42),      vbSize); 
			Patch<uint32_t>((0x70ED2C  + 1),   vbSize); 
		}

		if (r_buf_skinnedCacheVb) {
			// Original: 6 MB. Two D3D vertex buffers + allocator soft/hard limits.
			// Soft limit = size * (9/13) ≈ 69.2% — matches the original nullsub_2 address ratio.
			// FPU soft limit ratio: 5347738.0 / 6291456.0 ≈ 85% of hard limit.
			const uint32_t skinnedSize = r_buf_skinnedCacheVb->current.integer * MB_SIZE;

			Patch<uint32_t>((0x0070EC77 + 1), skinnedSize); 
			Patch<uint32_t>((0x71DD7C  + 2),  (uint32_t)((float)skinnedSize / 1.44444444f)); 
			Patch<uint32_t>((0x71DD95  + 2),  skinnedSize); 
			Patch<float>   (0x8AF24C,          (float)skinnedSize * (5347738.0f / 6291456.0f)); 
		}

		if (r_buf_tempSkin) {
			// Original: 6 MB. VirtualAlloc'd CPU memory for skinned mesh temp storage.
			Patch<uint32_t>((0x6F52A7 + 1), r_buf_tempSkin->current.integer * MB_SIZE); 
		}

		if (r_buf_dynamicIndexBuffer) {
			// Original: 2 MB D3D buffer. Capacity field stores index count (size/2 for D3DFMT_INDEX16).
			const uint32_t ibSize = r_buf_dynamicIndexBuffer->current.integer * MB_SIZE;
			
			Patch<uint32_t>((0x70ECF8 + 3), ibSize / 2); 
			Patch<uint32_t>((0x70EDA6 + 1), ibSize);     
			Patch<uint32_t>((0x70EE1E + 1), ibSize);     
		}

		//if (r_buf_preTessIndexBuffer) {
		//	// Original: 2 MB D3D buffer. Same layout as dynamicIndexBuffer.
		//	const uint32_t ptibSize = r_buf_preTessIndexBuffer->current.integer * MB_SIZE;
		//	Patch<uint32_t>((0x0070EDEA + 3), ptibSize / 2); 
		//	Patch<uint32_t>((0x70EE7D  + 1),  ptibSize);     
		//}
	}
	return R_CreateDynamicBuffers();
}

dvar_t* cg_fov_default;

dvar_t* cg_gun_fovcomp_x;

dvar_t* cg_gun_fovcomp_y;

dvar_t* cg_gun_fovcomp_z;

dvar_t* cg_fovCompMax;

dvar_t* cg_fovComp_enable;

dvar_t* cg_fovComp_fovscale;


game::dvar_s* cg_fov_user;
game::dvar_s* cg_fovscale_user;


void Update_cg_fov_user(float* cg_fov_value) {
	game::dvar_s* cg_fov_min = *(game::dvar_s**)0x0339CBE0;

	game::dvar_s* cg_fov = *(game::dvar_s**)0x0368EB70;

	float mult = cg_fov_user->current.value / 65.f;
	*cg_fov_value = std::clamp(*cg_fov_value * mult, cg_fov_min->current.value, cg_fov_user->domain.value.max);

}

void Update_cg_fovscale_user(float* cg_fovscale_value) {
	game::dvar_s* cg_fovscale = *(game::dvar_s**)0x03688A04;
	float mult = cg_fovscale_user->current.value / 1.f;
	*cg_fovscale_value = std::clamp(*cg_fovscale_value * mult, cg_fovscale->domain.value.min, cg_fovscale->domain.value.max);

}

void __cdecl CG_CalculateWeaponMovement_Debug(const cg_s* cgameGlob, float* origin)
{
	float v2;
	float v3;
	float v4;
	float v5;
	float v6;
	float v7;
	float fovcomp_y;
	float fovcomp_z;
	float fovCoeff;

	dvar_t* cg_fov = *(dvar_t**)0x0368EB70;

	dvar_t* cg_gun_x = *(dvar_t**)0x034660EC;

	dvar_t* cg_gun_y = *(dvar_t**)0x0339B758;

	dvar_t* cg_gun_z = *(dvar_t**)0x03466074;

	dvar_t* cg_fovscale = *(dvar_t**)0x03688A04;

	float fovscale = cg_fovComp_fovscale->isEnabled() ? cg_fovscale->current.value : 1.f;

	float cg_fov_current = cg_fov->current.value;

	Update_cg_fovscale_user(&fovscale);

	Update_cg_fov_user(&cg_fov_current);

	v6 = (float)((cg_fov_current * fovscale) - cg_fov_default->current.value)
		* (float)(1.0 / (float)(cg_fovCompMax->current.value - cg_fov_default->current.value));
	if ((float)(v6 - 1.0) < 0.0)
		v7 = (float)((cg_fov_current * fovscale) - cg_fov_default->current.value)
		* (float)(1.0 / (float)(cg_fovCompMax->current.value - cg_fov_default->current.value));
	else
		v7 = 1.f;
	if ((float)(0.0 - v6) < 0.0)
		v2 = v7;
	else
		v2 = 0.f;
	fovCoeff = (float)(1.0 - cgameGlob->predictedPlayerState.fWeaponPosFrac) * v2;
	fovcomp_y = cg_gun_fovcomp_y->current.value * fovCoeff;
	fovcomp_z = cg_gun_fovcomp_z->current.value * fovCoeff;
	v5 = cg_gun_x->current.value + (float)(cg_gun_fovcomp_x->current.value * fovCoeff);
	*origin = (float)(v5 * cgameGlob->viewModelAxis[0][0]) + *origin;
	origin[1] = (float)(v5 * cgameGlob->viewModelAxis[0][1]) + origin[1];
	origin[2] = (float)(v5 * cgameGlob->viewModelAxis[0][2]) + origin[2];
	v4 = cg_gun_y->current.value + fovcomp_y;
	*origin = (float)(v4 * cgameGlob->viewModelAxis[1][0]) + *origin;
	origin[1] = (float)(v4 * cgameGlob->viewModelAxis[1][1]) + origin[1];
	origin[2] = (float)(v4 * cgameGlob->viewModelAxis[1][2]) + origin[2];
	v3 = cg_gun_z->current.value + fovcomp_z;
	*origin = (float)(v3 * cgameGlob->viewModelAxis[2][0]) + *origin;
	origin[1] = (float)(v3 * cgameGlob->viewModelAxis[2][1]) + origin[1];
	origin[2] = (float)(v3 * cgameGlob->viewModelAxis[2][2]) + origin[2];
}


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

constexpr auto MEGABYTE = 1048576.0;

void CG_DrawMemoryfunc(float* y) {
	//char buffer[200]{};
	ScreenPlacement& scrPlaceView = *(ScreenPlacement*)(0x00957360);

	MEMORYSTATUSEX status{};
	dvar_t* cg_debugInfoCornerOffset = *(dvar_t**)0x03688B44;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	if (GlobalMemoryStatusEx(&status)) {
		double v_total = status.ullTotalVirtual / MEGABYTE;
		double v_free = status.ullAvailVirtual / MEGABYTE;
		double p_total = status.ullTotalPhys / MEGABYTE;
		double p_free = status.ullAvailPhys / MEGABYTE;

		char buffer[128]{};
		sprintf_s(buffer,"Physical Memory: %5.2f / %5.2f (%5.2f free)",
			p_total - p_free, p_total, p_free);



		float x = scrPlaceView.virtualViewableMax[0]
			- scrPlaceView.virtualViewableMin[0]
			+ cg_debugInfoCornerOffset->current.value;
		//			- 50.0;


		*y += CG_CornerDebugPrint(buffer, x, *y, 0.f, (char*)0x00840FF0, (float*)0x008397F0);

		sprintf_s(buffer,"Virtual Memory: %5.2f / %5.2f (%5.2f free)",
			v_total - v_free, v_total, v_free);
		*y += CG_CornerDebugPrint(buffer, x, *y, 0.f, (char*)0x00840FF0, (float*)0x008397F0);
	}

}


inline void Add_User_Fov() {

	cg_fov_user = (game::dvar_s*)Dvar_RegisterFloat("cg_fov_user", 65.f, 1.f, 160.f, DVAR_FLAG_ARCHIVE, (const char*)0x00890514);
	cg_fovscale_user = (game::dvar_s*)Dvar_RegisterFloat("cg_fovscale_user", 1.f, 0.2f, 2.f, DVAR_FLAG_ARCHIVE, (const char*)0x00890540);

	static auto cg_fovscale_hook = safetyhook::create_mid(0x0042DF29, [](SafetyHookContext& ctx) {
		Update_cg_fovscale_user(&ctx.xmm0.f32[0]);
		});

	static auto cg_fov_hook1 = safetyhook::create_mid(0x00402345, [](SafetyHookContext& ctx) {
		Update_cg_fov_user(&ctx.xmm0.f32[0]);
		});

	static auto cg_fov_hook2 = safetyhook::create_mid(0x42DD6E, [](SafetyHookContext& ctx) {
		Update_cg_fov_user(&ctx.xmm3.f32[0]);
		});

	static auto cg_fov_hook3 = safetyhook::create_mid(0x42E013, [](SafetyHookContext& ctx) {
		Update_cg_fov_user(&ctx.xmm0.f32[0]);
		});


}

void PatchT4E_Render() {
	Add_User_Fov();
	static auto fovcomp_backport = safetyhook::create_mid(0x00469CD6, [](SafetyHookContext& ctx) {
		if (cg_fovComp_enable && cg_fovComp_enable->isEnabled()) {
			float* origin = (float*)(ctx.esp + 0x28);

			CG_CalculateWeaponMovement_Debug((cg_s*)(0x034732B8), origin);
			ctx.eip = 0x00469CE8;
		}
		});

	cg_fovComp_fovscale = Dvar_RegisterBool(false, "cg_fovComp_fovscale", DVAR_FLAG_ARCHIVE, "Takes into account fovscale for cg_fovComp");

	cg_fovComp_enable = Dvar_RegisterBool(false, "cg_fovComp_enable", DVAR_FLAG_ARCHIVE, "Enables backported fovComp behaviour from Black Ops 1");

	cg_fov_default = Dvar_RegisterFloat("cg_fov_default", 65.f, 10.f, 160.f, DVAR_FLAG_ARCHIVE, "User default field of view angle in degrees");

	cg_fovCompMax = Dvar_RegisterFloat(
		"cg_fovCompMax",
		85.0,
		1.0,
		160.0,
		0,
		"The maximum field of view to compensate for gun placement");

	cg_gun_fovcomp_x = Dvar_RegisterFloat(
		"cg_gun_fovcomp_x",
		-2.0,
		-FLT_MAX,
		FLT_MAX,
		0,
		"x position FOV offset compensation of the viewmodel");
	cg_gun_fovcomp_y = Dvar_RegisterFloat(
		"cg_gun_fovcomp_y",
		0.0,
		-FLT_MAX,
		FLT_MAX,
		0,
		"y position FOV offset compensation of the viewmodel");
	cg_gun_fovcomp_z = Dvar_RegisterFloat(
		"cg_gun_fovcomp_z",
		0.0,
		-FLT_MAX,
		FLT_MAX,
		0,
		"z position FOV offset compensation of the viewmodel");

	// Units are MB. Minimums correspond to original game buffer sizes.
	// Defaults are 2x original to provide a meaningful increase.
	r_buf_skinnedCacheVb        = Dvar_RegisterInt(12, "r_buf_skinnedCacheVb",        6,  64, DVAR_FLAG_ARCHIVE | DVAR_FLAG_LATCH); // orig: 6 MB
	r_buf_tempSkin              = Dvar_RegisterInt(12, "r_buf_tempSkin",              6,  64, DVAR_FLAG_ARCHIVE | DVAR_FLAG_LATCH); // orig: 6 MB
	r_buf_dynamicVertexBuffer   = Dvar_RegisterInt(2,  "r_buf_dynamicVertexBuffer",   1,  16, DVAR_FLAG_ARCHIVE | DVAR_FLAG_LATCH); // orig: 1 MB
	r_buf_dynamicIndexBuffer    = Dvar_RegisterInt(4,  "r_buf_dynamicIndexBuffer",    2,  16, DVAR_FLAG_ARCHIVE | DVAR_FLAG_LATCH); // orig: 2 MB
	r_buf_preTessIndexBuffer    = Dvar_RegisterInt(4,  "r_buf_preTessIndexBuffer",    2,  16, DVAR_FLAG_ARCHIVE | DVAR_FLAG_LATCH); // orig: 2 MB

	r_increase_render_buffers = Dvar_RegisterBool(true, "r_increase_render_buffers", DVAR_FLAG_ARCHIVE | DVAR_FLAG_LATCH, "increasing rendering buffers");

	InterceptCall(0x6D594D, R_CreateDynamicBuffers, R_CreateDynamicBuffers_hook);

	static dvar_t* CG_DrawMemory = Dvar_RegisterBool(false, "cg_drawMemory", DVAR_FLAG_ARCHIVE);


	Dvar_RegisterInt(0, "debug_show_viewpos", 0, 1, DVAR_FLAG_ARCHIVE);

	static auto debug_print = safetyhook::create_mid(0x00439613, [](SafetyHookContext& ctx) {
		if(CG_DrawMemory->isEnabled())
		CG_DrawMemoryfunc((float*)(ctx.esp + 0x4));
		});
}