#include "t4_headers.h"
#include "StdInc.h"
#include "MemoryMgr.h"
#include <safetyhook.hpp>

typedef void*(__cdecl* R_CreateDynamicBuffersT)();
R_CreateDynamicBuffersT R_CreateDynamicBuffers = nullptr;

constexpr uint32_t MB_SIZE = 104857;

using namespace Memory::VP;

dvar_t* r_increase_render_buffers;

dvar_t* r_buf_skinnedCacheVb = nullptr;

dvar_t* r_buf_tempSkin = nullptr;
dvar_t* r_buf_dynamicVertexBuffer = nullptr;
dvar_t* r_buf_dynamicIndexBuffer = nullptr;
dvar_t* r_buf_preTessIndexBuffer = nullptr;

dvar_t* cg_fov_tweaks;
dvar_t* cg_fov_gun;
dvar_t* cg_fovScale_gun;
// Raises limits of several buffers, reference taken from iw3xo-dev by xoxor4d https://github.com/xoxor4d/iw3xo-dev
void* __cdecl R_CreateDynamicBuffers_hook() {
	if (r_increase_render_buffers && r_increase_render_buffers->current.boolean) {
		if (r_buf_dynamicVertexBuffer) {
			// Default 1 MB
			Patch<uint32_t>((0x0070EC1A + 3), r_buf_dynamicVertexBuffer->current.integer * MB_SIZE);
			Patch<uint32_t>((0x0070EC42), r_buf_dynamicVertexBuffer->current.integer * MB_SIZE);
			Patch<uint32_t>((0x70ED2C + 1), r_buf_dynamicVertexBuffer->current.integer * MB_SIZE);
		}

		if (r_buf_skinnedCacheVb) {
			// default 60 MB?
			Patch<uint32_t>((0x0070EC77 + 1), r_buf_skinnedCacheVb->current.integer * MB_SIZE);
			Patch<uint32_t>((0x0070EC42), r_buf_skinnedCacheVb->current.integer * MB_SIZE);
			Patch<uint32_t>((0x70ED2C + 1), r_buf_skinnedCacheVb->current.integer * MB_SIZE);

			Patch<uint32_t>((0x71DD7C + 2), ((uint32_t)((float)(r_buf_skinnedCacheVb->current.integer * MB_SIZE) / 1.44444444f))); // ok this idk f why
			Patch<uint32_t>((0x71DD95 + 2), r_buf_skinnedCacheVb->current.integer * MB_SIZE);
		}

		if (r_buf_tempSkin) {
			// default 60 MB?
			Patch<uint32_t>((0x6F52A7 + 1), r_buf_tempSkin->current.integer * MB_SIZE);

		}

		if (r_buf_dynamicIndexBuffer) {
			// default 2 MB
			Patch<uint32_t>((0x70ECF8 + 3), (r_buf_dynamicIndexBuffer->current.integer * MB_SIZE) / 2); // idk why
			Patch<uint32_t>((0x70EDA6 + 1), r_buf_dynamicIndexBuffer->current.integer * MB_SIZE);
			Patch<uint32_t>((0x70EE1E + 1), r_buf_dynamicIndexBuffer->current.integer * MB_SIZE);
		}

		if (r_buf_preTessIndexBuffer) {
			// default 2 MB
			Patch<uint32_t>((0x0070EDEA + 3), (r_buf_preTessIndexBuffer->current.integer * MB_SIZE) / 2); // idk why
			Patch<uint32_t>((0x70EE7D + 1), r_buf_preTessIndexBuffer->current.integer * MB_SIZE);
		}
	}
	return R_CreateDynamicBuffers();
}

dvar_t* cg_fov_default;

dvar_t* cg_gun_fovcomp_x;

dvar_t* cg_gun_fovcomp_y;
#include <unordered_set>
#include <string_view>

struct GfxMatrix
{
	float m[4][4];
};


struct GfxViewParms
{
	GfxMatrix viewMatrix;
	GfxMatrix projectionMatrix;
	GfxMatrix viewProjectionMatrix;
	GfxMatrix inverseViewProjectionMatrix;
	float origin[4];
	float axis[3][3];
	float depthHackNearClip;
	float zNear;
	int pad;
};

void InfinitePerspectiveMatrix(const float tan_half_fov_x, const float tan_half_fov_y, const float z_near, float(*mtx)[4])
{
	(*mtx)[0] = 0.99951172f / tan_half_fov_x;
	(*mtx)[5] = 0.99951172f / tan_half_fov_y;
	(*mtx)[10] = 0.99951172f;
	(*mtx)[11] = 1.0f;
	(*mtx)[14] = 0.99951171875f * -z_near;
}

float calculate_gunfov_with_zoom(float fov_val)
{

	WeaponDef** BG_WeaponNames = (WeaponDef**)0x8F6770;
	float calc_fov = 80.0f;
	const auto& cg_fovMin = *(dvar_t**)0x339CBE0;
	const auto& cg_fov = *(dvar_t**)0x0368EB70;
	const auto& cg_fovScale = *(dvar_t**)0x03688A04;
	float fovScale = 1.f;

	cg_s* cgs = (cg_s*)0x034732B8;

	unsigned int offhand_index = cgs->predictedPlayerState.offHandIndex;

	if ((cgs->predictedPlayerState.weapFlags & 2) == 0)
	{
		offhand_index = cgs->predictedPlayerState.weapon;
	}

	const auto weapon = BG_WeaponNames[offhand_index];
	bool isGasWeapon = false;


	if (cg_fov_tweaks->current.integer >= 2 && weapon) {
		isGasWeapon = BG_WeaponNames[offhand_index]->weapType == WEAPTYPE_GAS;
	}


	fovScale = isGasWeapon ? cg_fovScale->current.value : cg_fovScale_gun->current.value;



	// #
	auto check_flags_and_fovmin = [&]() -> float
		{
			if ((cgs->predictedPlayerState.eFlags & 0x300) != 0)
			{
				calc_fov = 55.0f;
			}

			if (cg_fovMin->current.value - calc_fov >= 0.0f)
			{
				calc_fov = cg_fovMin->current.value;
			}

			return calc_fov * fovScale;
		};



	if (cgs->predictedPlayerState.pm_type == 5)
	{
		return check_flags_and_fovmin();
	}


	calc_fov = isGasWeapon ? cg_fov->current.value : fov_val;
	if (weapon->aimDownSight)
	{
		if (cgs->predictedPlayerState.fWeaponPosFrac == 1.0f)
		{
			calc_fov = weapon->fAdsZoomFov;
			return check_flags_and_fovmin();
		}

		if (cgs->predictedPlayerState.fWeaponPosFrac != 0.0f)
		{
			float ads_factor = 0.0f;

			if (cgs->playerEntity.bPositionToADS)
			{
				const float w_pos_frac = cgs->predictedPlayerState.fWeaponPosFrac - (1.0f - weapon->fAdsZoomInFrac);
				if (w_pos_frac <= 0.0f)
				{
					return check_flags_and_fovmin();
				}

				ads_factor = w_pos_frac / weapon->fAdsZoomInFrac;
			}
			else
			{
				const float w_pos_frac = cgs->predictedPlayerState.fWeaponPosFrac - (1.0f - weapon->fAdsZoomOutFrac);
				if (w_pos_frac <= 0.0f)
				{
					return check_flags_and_fovmin();
				}

				ads_factor = w_pos_frac / weapon->fAdsZoomOutFrac;
			}

			if (ads_factor > 0.0f)
			{
				calc_fov = calc_fov - ads_factor * (calc_fov - weapon->fAdsZoomFov);
			}
		}
	}

	return check_flags_and_fovmin();
}

void set_gunfov(GfxViewParms* view_parms)
{
	if (cg_fov_tweaks && cg_fov_tweaks->current.integer)
	{
		auto viewportWidth = (float)*(int*)0x03BED830;
		auto viewportHeight = (float)*(int*)0x03BED834;
		// calc gun fov (includes weapon zoom)
		const float gun_fov = calculate_gunfov_with_zoom(cg_fov_gun->current.value);
		const float w_fov = 0.75f * tanf(gun_fov * 0.01745329238474369f * 0.5f);

		const float tan_half_x = (static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight)) * w_fov;
		const float tan_half_y = w_fov;

		// calc projection matrix
		float proj_mtx[4][4] = {};
		InfinitePerspectiveMatrix(tan_half_x, tan_half_y, view_parms->zNear, proj_mtx);

		// only overwrite the projection matrix ;)
		memcpy(view_parms->projectionMatrix.m, proj_mtx, sizeof(GfxMatrix));
	}
}

struct SVHash {
	using is_transparent = void; // enables heterogeneous lookup
	size_t operator()(std::string_view sv) const noexcept {
		return std::hash<std::string_view>{}(sv);
	}
};

dvar_t* cg_gun_fovcomp_z;
struct SVEq {
	using is_transparent = void;
	bool operator()(std::string_view a, std::string_view b) const noexcept {
		return a == b;
	}
};

static std::unordered_set<std::string> g_viewmodelEffectNames;

// need to figure out when its safe to call this
void ClearViewModelEffects() {

	if (!g_viewmodelEffectNames.empty()) {
		g_viewmodelEffectNames.clear();
	}

}

dvar_t* cg_fovCompMax;
inline bool IsViewmodelByName(const char* name) {
	return name && g_viewmodelEffectNames.contains(std::string{ name });
}

dvar_t* cg_fovComp_enable;

dvar_t* cg_fovComp_fovscale;

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

	v6 = (float)((cg_fov->current.value * fovscale) - cg_fov_default->current.value)
		* (float)(1.0 / (float)(cg_fovCompMax->current.value - cg_fov_default->current.value));
	if ((float)(v6 - 1.0) < 0.0)
		v7 = (float)((cg_fov->current.value * fovscale) - cg_fov_default->current.value)
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



struct GfxDrawSurfListInfo // sizeof=0x28
{                                       // ...
	const GfxDrawSurf* drawSurfs;
	unsigned int drawSurfCount;
	uint32_t baseTechType; // ...
	const struct GfxViewInfo* viewInfo;
	float viewOrigin[4];
	const GfxLight* light;
	int cameraView;
};

void R_SplitEmissives(
	const GfxDrawSurfListInfo* inInfo,
	GfxDrawSurfListInfo* outWorld,
	GfxDrawSurfListInfo* outViewmodel)
{
	static GfxDrawSurf worldSurfs[8192];
	static GfxDrawSurf vmSurfs[8192];
	unsigned int worldCount = 0;
	unsigned int vmCount = 0;

	memset(worldSurfs, 0, sizeof(worldSurfs));
	memset(vmSurfs, 0, sizeof(vmSurfs));

	// Copy base info
	*outWorld = *inInfo;
	*outViewmodel = *inInfo;

	// Split into two lists
	for (unsigned int i = 0; i < inInfo->drawSurfCount; i++)
	{
		const GfxDrawSurf* surf = &inInfo->drawSurfs[i];
		bool isViewmodel = (surf->fields.unused & 0x1) != 0;

		if (isViewmodel)
			vmSurfs[vmCount++] = *surf;
		else
			worldSurfs[worldCount++] = *surf;
	}

	// Assign outputs
	outWorld->drawSurfs = worldSurfs;
	outWorld->drawSurfCount = worldCount;

	outViewmodel->drawSurfs = vmSurfs;
	outViewmodel->drawSurfCount = vmCount;
}


void CG_PlayBoltedEffect_midhook_replace_weaponflash(SafetyHookContext& ctx) {
	if (!cg_fov_tweaks->current.integer)
		return;
	FxEffectDef* flash = *(FxEffectDef**)ctx.esp;

	bool isViewModel = *(bool*)(ctx.esp + 0x18);
	if (isViewModel) {
		g_viewmodelEffectNames.insert(std::string{ flash->name });
		//printf("flash %s\n", flash->name);
	}
}

void CG_PlayBoltedEffect_midhook_replace(SafetyHookContext& ctx){
	if (!cg_fov_tweaks->current.integer)
		return;
	FxEffectDef* flash = *(FxEffectDef**)ctx.esp;
	g_viewmodelEffectNames.insert(std::string{ flash->name });
	//printf("flash %s\n", flash->name);
}

void __stdcall R_AddCodeMeshDrawSurf_hook1_midasm_hook(GfxDrawSurf* ctx, uintptr_t esp) {
	const char* fx_name = *(const char**)(esp + 0x18);
	//printf("FX name %s\n", fx_name);
}

void __declspec(naked) R_AddCodeMeshDrawSurf_hook1_midstub()
{
	__asm
	{



		mov[eax + 4], edx

		pushad

		push esp
		push eax
		call R_AddCodeMeshDrawSurf_hook1_midasm_hook

		pop eax
		pop esp

		popad

		pop  edi
		retn
	}
}

void R_DrawEmissive(uintptr_t something)
{
	static DWORD R_DrawEmissive_realcall = 0x6E7DC0;

	__asm
	{
		mov esi, something
		call[R_DrawEmissive_realcall]
	}
}

bool isThisWorldPass;
bool viewmodel_pass;
using mat3x3 = float[3][3];
using mat4x3 = float[4][3];
using mat4x4 = float[4][4];

void __cdecl MatrixInverse44(const mat4x4& mat, mat4x4& dst)
{
	float src[16]{}; // [esp+0h] [ebp-78h]
	float tmp[12]{}; // [esp+44h] [ebp-34h]

	float det; // [esp+40h] [ebp-38h]
	int i; // [esp+74h] [ebp-4h]



	for (i = 0; i < 4; ++i)
	{
		src[i] = (mat)[i][0];
		src[i + 4] = (mat)[i][1];
		src[i + 8] = (mat)[i][2];
		src[i + 12] = (mat)[i][3];
	}
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	(dst)[0][0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
	(dst)[0][0] = (dst)[0][0] - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
	(dst)[0][1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
	(dst)[0][1] = (dst)[0][1] - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
	(dst)[0][2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
	(dst)[0][2] = (dst)[0][2] - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
	(dst)[0][3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
	(dst)[0][3] = (dst)[0][3] - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
	(dst)[1][0] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
	(dst)[1][0] = (dst)[1][0] - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
	(dst)[1][1] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
	(dst)[1][1] = (dst)[1][1] - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
	(dst)[1][2] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
	(dst)[1][2] = (dst)[1][2] - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
	(dst)[1][3] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
	(dst)[1][3] = (dst)[1][3] - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);
	tmp[0] = src[2] * src[7];
	tmp[1] = src[3] * src[6];
	tmp[2] = src[1] * src[7];
	tmp[3] = src[3] * src[5];
	tmp[4] = src[1] * src[6];
	tmp[5] = src[2] * src[5];
	tmp[6] = src[0] * src[7];
	tmp[7] = src[3] * src[4];
	tmp[8] = src[0] * src[6];
	tmp[9] = src[2] * src[4];
	tmp[10] = src[0] * src[5];
	tmp[11] = src[1] * src[4];
	(dst)[2][0] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
	(dst)[2][0] = (dst)[2][0] - (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
	(dst)[2][1] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
	(dst)[2][1] = (dst)[2][1] - (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
	(dst)[2][2] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
	(dst)[2][2] = (dst)[2][2] - (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
	(dst)[2][3] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
	(dst)[2][3] = (dst)[2][3] - (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
	(dst)[3][0] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
	(dst)[3][0] = (dst)[3][0] - (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
	(dst)[3][1] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
	(dst)[3][1] = (dst)[3][1] - (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
	(dst)[3][2] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
	(dst)[3][2] = (dst)[3][2] - (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
	(dst)[3][3] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
	(dst)[3][3] = (dst)[3][3] - (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);
	det = src[0] * (dst)[0][0] + src[1] * (dst)[0][1] + src[2] * (dst)[0][2] + src[3] * (dst)[0][3];



	det = 1.0 / det;
	for (i = 0; i < 16; ++i)
		(dst)[0][i] = (dst)[0][i] * det;
}

void __cdecl MatrixMultiply44(const mat4x4& in1, const mat4x4& in2, mat4x4& out)
{


	(out)[0][0] = (in1)[0][0] * (in2)[0][0]
		+ (in1)[0][1] * (in2)[1][0]
		+ (in1)[0][2] * (in2)[2][0]
		+ (in1)[0][3] * (in2)[3][0];
	(out)[0][1] = (in1)[0][0] * (in2)[0][1]
		+ (in1)[0][1] * (in2)[1][1]
		+ (in1)[0][2] * (in2)[2][1]
		+ (in1)[0][3] * (in2)[3][1];
	(out)[0][2] = (in1)[0][0] * (in2)[0][2]
		+ (in1)[0][1] * (in2)[1][2]
		+ (in1)[0][2] * (in2)[2][2]
		+ (in1)[0][3] * (in2)[3][2];
	(out)[0][3] = (in1)[0][0] * (in2)[0][3]
		+ (in1)[0][1] * (in2)[1][3]
		+ (in1)[0][2] * (in2)[2][3]
		+ (in1)[0][3] * (in2)[3][3];
	(out)[1][0] = (in1)[1][0] * (in2)[0][0]
		+ (in1)[1][1] * (in2)[1][0]
		+ (in1)[1][2] * (in2)[2][0]
		+ (in1)[1][3] * (in2)[3][0];
	(out)[1][1] = (in1)[1][0] * (in2)[0][1]
		+ (in1)[1][1] * (in2)[1][1]
		+ (in1)[1][2] * (in2)[2][1]
		+ (in1)[1][3] * (in2)[3][1];
	(out)[1][2] = (in1)[1][0] * (in2)[0][2]
		+ (in1)[1][1] * (in2)[1][2]
		+ (in1)[1][2] * (in2)[2][2]
		+ (in1)[1][3] * (in2)[3][2];
	(out)[1][3] = (in1)[1][0] * (in2)[0][3]
		+ (in1)[1][1] * (in2)[1][3]
		+ (in1)[1][2] * (in2)[2][3]
		+ (in1)[1][3] * (in2)[3][3];
	(out)[2][0] = (in1)[2][0] * (in2)[0][0]
		+ (in1)[2][1] * (in2)[1][0]
		+ (in1)[2][2] * (in2)[2][0]
		+ (in1)[2][3] * (in2)[3][0];
	(out)[2][1] = (in1)[2][0] * (in2)[0][1]
		+ (in1)[2][1] * (in2)[1][1]
		+ (in1)[2][2] * (in2)[2][1]
		+ (in1)[2][3] * (in2)[3][1];
	(out)[2][2] = (in1)[2][0] * (in2)[0][2]
		+ (in1)[2][1] * (in2)[1][2]
		+ (in1)[2][2] * (in2)[2][2]
		+ (in1)[2][3] * (in2)[3][2];
	(out)[2][3] = (in1)[2][0] * (in2)[0][3]
		+ (in1)[2][1] * (in2)[1][3]
		+ (in1)[2][2] * (in2)[2][3]
		+ (in1)[2][3] * (in2)[3][3];
	(out)[3][0] = (in1)[3][0] * (in2)[0][0]
		+ (in1)[3][1] * (in2)[1][0]
		+ (in1)[3][2] * (in2)[2][0]
		+ (in1)[3][3] * (in2)[3][0];
	(out)[3][1] = (in1)[3][0] * (in2)[0][1]
		+ (in1)[3][1] * (in2)[1][1]
		+ (in1)[3][2] * (in2)[2][1]
		+ (in1)[3][3] * (in2)[3][1];
	(out)[3][2] = (in1)[3][0] * (in2)[0][2]
		+ (in1)[3][1] * (in2)[1][2]
		+ (in1)[3][2] * (in2)[2][2]
		+ (in1)[3][3] * (in2)[3][2];
	(out)[3][3] = (in1)[3][0] * (in2)[0][3]
		+ (in1)[3][1] * (in2)[1][3]
		+ (in1)[3][2] * (in2)[2][3]
		+ (in1)[3][3] * (in2)[3][3];
}

void R_ChangeDepthHackNearClip(uintptr_t a1, unsigned int depthHackFlags) {

	if (a1)
	{
		float v5 = -0.0 - *(float*)(a1 + 0xC8C);
		++*(WORD*)(a1 + 0x12EE);
		++*(WORD*)(a1 + 0x12F2);
		++*(WORD*)(a1 + 0x12F4);
		*(float*)(a1 + 0xC8C) = v5;
		++*(WORD*)(a1 + 0x1240);
		*(DWORD*)(a1 + 0x1318) = 0;
	}

}

void PatchT4E_Render() {
	Memory::VP::Nop(0x0071A55C, 3);

	cg_fov_gun = Dvar_RegisterFloat("cg_fov_gun", 65.f, 65.f, 120.f, DVAR_FLAG_ARCHIVE,"Adjust gun fov separately (wont effect world fov)");

	cg_fovScale_gun = Dvar_RegisterFloat("cg_fovScale_gun", 1.f, 0.2f, 2.0f, DVAR_FLAG_ARCHIVE,"Adjust gun fovScale separately (wont effect world fov)");

	cg_fov_tweaks = Dvar_RegisterInt(0, "cg_fov_tweaks", 0, 2, DVAR_FLAG_ARCHIVE,"Enable gun fov tweaks(experimental does not currently effect Weapons eject brass)\n1 = enables for all weapons\n2 = enables for all weapons expect gas type weapons such as flamethrowers");

	static auto cg_fov_gun_hack = safetyhook::create_mid(0x006DE3F7, [](SafetyHookContext& ctx) {

		set_gunfov((GfxViewParms*)ctx.esi);

		});
	Memory::VP::Nop(0x006E8993, 5);
	static auto DrawCallTwice = safetyhook::create_mid(0x006E8993, [](SafetyHookContext& ctx) {
		isThisWorldPass = true;
		viewmodel_pass = false;
		R_DrawEmissive(ctx.edi);

		GfxViewParms* can_mod = (GfxViewParms*)ctx.edi;
		if (cg_fov_tweaks->current.integer) {
			MatrixMultiply44(
				can_mod->viewMatrix.m,
				can_mod->projectionMatrix.m,
				can_mod->viewProjectionMatrix.m
			);
			MatrixInverse44(
				can_mod->viewProjectionMatrix.m,
				can_mod->inverseViewProjectionMatrix.m
			);
		}
		isThisWorldPass = false;
		viewmodel_pass = true;
		R_DrawEmissive(ctx.edi);

		viewmodel_pass = false;

		});

	static auto test_hack = safetyhook::create_mid(0x6F8820, [](SafetyHookContext& ctx) {
		if (viewmodel_pass) {
			R_ChangeDepthHackNearClip(ctx.edi, 1);
		}
		});

	static auto R_AddCodeMeshDrawSurf_hook1 = safetyhook::create_mid(0x0071A55C, [](SafetyHookContext& ctx) {

		*(uint32_t*)(ctx.eax + 0x4) = ctx.edx;

		const char* fx_name = *(const char**)(ctx.esp + 0x18);
		//printf("fx name %s\n", fx_name);
		//if(strstr(fx_name, "muzzleflashes/") == fx_name)
		//printf("fx name %s\n", fx_name);

		if (IsViewmodelByName(fx_name) || strcmp("weapon/muzzleflashes/fx_raygun_view",fx_name) == 0) {
			GfxDrawSurf* drawSurf = (GfxDrawSurf*)ctx.eax;
			drawSurf->fields.unused |= 0x1;
			//printf("detected name %s %llx\n", fx_name, drawSurf->fields.unused);
		}
		});

	//Memory::VP::InjectHook(0x0071A55C, R_AddCodeMeshDrawSurf_hook1_midstub, HookType::Jump);

	static auto CG_PlayBoltedEffect_call1 = safetyhook::create_mid(0x469A92, &CG_PlayBoltedEffect_midhook_replace_weaponflash);
	static auto CG_PlayBoltedEffect_call2 = safetyhook::create_mid(0x66BB44, &CG_PlayBoltedEffect_midhook_replace);

	//// for testing
	//static auto CG_PlayBoltedEffect_main = safetyhook::create_mid(0x00448330, [](SafetyHookContext& ctx) {
	//	FxEffectDef* flash = *(FxEffectDef**)(ctx.esp + 0x4);
	//	if (flash && flash->name) {
	//		g_viewmodelEffectNames.insert(std::string_view{ flash->name });
	//		printf("flash %s\n", flash->name);
	//	}
	//	});


	static auto R_DrawEmissiveCallBack1 = safetyhook::create_mid(0x006E7D9B, [](SafetyHookContext& ctx) {
		static GfxDrawSurfListInfo World{};
		static GfxDrawSurfListInfo ViewModel{};
		R_SplitEmissives((GfxDrawSurfListInfo*)ctx.eax, &World, &ViewModel);
		ctx.eax = (uintptr_t)(isThisWorldPass ? &World : &ViewModel);

		});

	//AllocConsole();
	//FILE* fDummy;
	//freopen_s(&fDummy, "CONIN$", "r", stdin);
	//freopen_s(&fDummy, "CONOUT$", "w", stderr);
	//freopen_s(&fDummy, "CONOUT$", "w", stdout);

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
		FLT_MIN,
		FLT_MAX,
		0,
		"x position FOV offset compensation of the viewmodel");
	cg_gun_fovcomp_y = Dvar_RegisterFloat(
		"cg_gun_fovcomp_y",
		0.0,
		FLT_MIN,
		FLT_MAX,
		0,
		"y position FOV offset compensation of the viewmodel");
	cg_gun_fovcomp_z = Dvar_RegisterFloat(
		"cg_gun_fovcomp_z",
		0.0,
		FLT_MIN,
		FLT_MAX,
		0,
		"z position FOV offset compensation of the viewmodel");

	r_buf_skinnedCacheVb = Dvar_RegisterInt(120, "r_buf_skinnedCacheVb", 60, 512, DVAR_FLAG_ARCHIVE);
	r_buf_tempSkin = Dvar_RegisterInt(120, "r_buf_tempSkin", 60, 512, DVAR_FLAG_ARCHIVE);
	r_buf_dynamicVertexBuffer = Dvar_RegisterInt(3, "r_buf_dynamicVertexBuffer", 1, 16, DVAR_FLAG_ARCHIVE);
	r_buf_dynamicIndexBuffer = Dvar_RegisterInt(4, "r_buf_dynamicIndexBuffer", 2, 16, DVAR_FLAG_ARCHIVE);
	r_buf_preTessIndexBuffer = Dvar_RegisterInt(4, "r_buf_preTessIndexBuffer", 2, 16, DVAR_FLAG_ARCHIVE);

	r_increase_render_buffers = Dvar_RegisterBool(true, "r_increase_render_buffers", DVAR_FLAG_ARCHIVE, "increasing rendering buffers");

	Memory::VP::InterceptCall(0x6D594D, R_CreateDynamicBuffers, R_CreateDynamicBuffers_hook);


}