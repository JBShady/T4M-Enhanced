#include "t4_headers.h"
#include "StdInc.h"
#include "T4.h"

#include "safetyhook.hpp"
#include <unordered_set>
#include <d3dx9shader.h>
#pragma comment(lib, "d3dx9.lib")

#include "MemoryMgr.h"

SafetyHookInline Material_Register_FastFileD;
std::unordered_set<std::string> materials_found;




static unsigned int* g_compiledPostfxBytecode = nullptr;
static DWORD g_compiledPostfxSize = 0;

// Hack shader replacement for postfx_color
void CompileAndInjectHLSLShader(Material* material, const char* shaderName)
{
	IDirect3DDevice9* device = *(IDirect3DDevice9**)0x463E490;


	if (g_compiledPostfxBytecode == nullptr)
	{
		// Xbox 360 gamma curve approximation function thanks to Parallellines0451 as it was provided by them
		// error rate for approximation is extremely low, implementing 100% correct gamma curve is slower and not worth it!
		const char* hlslCode = R"(
sampler2D colorMapSampler : register(s0);
float4 colorTintBase : register(c5);
float4 colorTintDelta : register(c6);
float4 colorBias : register(c7);
float4 gammaToggle : register(c187);  // x = X360 toggle (100.0 = on), y = r_gamma value (0.0 = fullscreen/skip)

struct PS_INPUT
{
    float2 texCoord : TEXCOORD0;
};

float X360GammaApprox(float x)
{
    float A = 0.541901;
    float B = 1.13465;
    float C = 13.53054;
    float D = 6.56649;
    float E = 0.311465;
    
    float f1 = A * x;
    float f2 = pow(x, B) * (1.0 - exp2(-C * x));
    float f3 = saturate(x * D + E);
    
    return lerp(f1, f2, f3);
}

float3 X360GammaApprox(float3 color)
{
    return float3(X360GammaApprox(color.x), X360GammaApprox(color.y), X360GammaApprox(color.z));
}

float4 main(PS_INPUT input) : COLOR0
{

    float4 r0 = tex2D(colorMapSampler, input.texCoord);
    

    const float3 lumaWeights = float3(0.298999995, 0.587000012, 0.114);
    float luma = dot(r0.rgb, lumaWeights);
    

    r0.rgb = r0.rgb * colorBias.w + luma;
    

    float3 tint = colorTintDelta.rgb * luma + colorTintBase.rgb;
    

    float3 finalColor = r0.rgb * tint + colorBias.rgb;
    
    if (gammaToggle.x == 100.0)
    {
        finalColor = X360GammaApprox(finalColor);
    }
    

    if (gammaToggle.y != 0.0)
    {
        float exponent = 1.0 / gammaToggle.y;
        finalColor = pow(saturate(finalColor), exponent);
    }
    
    return float4(finalColor, 1.0);
}
    )";

		ID3DXBuffer* shader = NULL;
		ID3DXBuffer* errors = NULL;

		HRESULT hr = D3DXCompileShader(
			hlslCode,
			strlen(hlslCode),
			NULL,
			NULL,
			"main",
			"ps_3_0",
			0,
			&shader,
			&errors,
			NULL
		);

		if (FAILED(hr) || !shader)
		{
			if (errors)
			{
				Com_Printf(0, "HLSL compilation failed:\n%s\n", (char*)errors->GetBufferPointer());
				errors->Release();
			}
			return;
		}

		Com_Printf(0, "HLSL shader compiled successfully! Size: %d bytes\n", shader->GetBufferSize());

		g_compiledPostfxSize = shader->GetBufferSize();
		g_compiledPostfxBytecode = (unsigned int*)malloc(g_compiledPostfxSize);
		if(g_compiledPostfxSize)
		memcpy(g_compiledPostfxBytecode, shader->GetBufferPointer(), g_compiledPostfxSize);

		shader->Release();
	}
	else
	{
		Com_Printf(0, "Reusing already compiled shader\n");
	}

	MaterialTechniqueSet* techSet = material->techniqueSet;

	for (int i = 0; i < 59; i++)
	{
		MaterialTechnique* technique = techSet->techniques[i];
		if (!technique) continue;

		for (int j = 0; j < technique->passCount; j++)
		{
			MaterialPass* pass = &technique->passArray[j];

			if (pass->pixelShader)
			{

				if (pass->pixelShader->prog.ps)
				{
					IDirect3DPixelShader9* oldShader = (IDirect3DPixelShader9*)pass->pixelShader->prog.ps;
					oldShader->Release();
				}


				IDirect3DPixelShader9* newPixelShader = NULL;
				HRESULT hr = device->CreatePixelShader((DWORD*)g_compiledPostfxBytecode, &newPixelShader);

				if (SUCCEEDED(hr) && newPixelShader)
				{
					pass->pixelShader->prog.ps = newPixelShader;
					pass->pixelShader->prog.loadDef.program = g_compiledPostfxBytecode;
					pass->pixelShader->prog.loadDef.programSize = (unsigned short)g_compiledPostfxSize;

					Com_Printf(0, "Injected cached shader into pass %d\n", j);
				}
			}
		}
	}
}

Material* __cdecl Material_Register_FastFile(const char* name)
{
	Material* loaded_m = Material_Register_FastFileD.unsafe_ccall<Material*>(name);

	if (strcmp(name, "postfx_color") == 0)
	{
		CompileAndInjectHLSLShader(loaded_m, name);
	}

	return loaded_m;
}

void SetPSConstF(UINT reg, const float* data, UINT count = 1)
{
	IDirect3DDevice9* pDevice = *reinterpret_cast<IDirect3DDevice9**>(reinterpret_cast<void*>(0x463E490));
	if (pDevice)
		pDevice->SetPixelShaderConstantF(reg, data, count);
}

dvar_t* r_gamma_x360;

dvar_t* r_gamma_windowed;

SafetyHookInline EndFrame_hook{};

void UpdateSafeAreaLive();

void __cdecl EndFrame() {

	float whatever[4]{ 0.f,0.f,0.f,0.f };
	if (r_gamma_x360) {
		whatever[0] = r_gamma_x360->current.boolean ? 100.f : 0.f;
	}

	dvar_t* r_fullscreen = *(dvar_t**)0x042B6F88;
	dvar_t* r_gamma = *(dvar_t**)0x042B6FB4;
	if (r_gamma && r_fullscreen && r_gamma_windowed && ((!r_fullscreen->current.boolean && r_gamma_windowed->current.integer) || r_gamma_windowed->current.integer >= 2)) {
		whatever[1] = r_gamma->current.value;
	}

	if (force_gamma_update ||
		(r_gamma_windowed && r_gamma_windowed->modified) ||
		(r_gamma && r_gamma->modified) ||
		(r_gamma_x360 && r_gamma_x360->modified)) {
		SetPSConstF(187, whatever);
		if (r_gamma_x360) {
			r_gamma_x360->modified = false;
		}
	}

	if (r_gamma_windowed && r_gamma) {
		if (r_gamma_windowed->modified) {
			r_gamma_windowed->modified = false;
			// EndFrame will check it and set it to false
			r_gamma->modified = true;

		}
	}

	EndFrame_hook.unsafe_ccall();
}

uintptr_t SCR_UpdateFrame_addr;
void PatchT4E_Rumble_Frame();

void __cdecl SCR_UpdateFrame() {

	UpdateSafeAreaLive();
	cdecl_call<void>(SCR_UpdateFrame_addr);
	PatchT4E_Rumble_Frame();
}

void PatchT4E_Shaders() {
	EndFrame_hook = safetyhook::create_inline(0x6FBF30, EndFrame);
	
	Memory::VP::InterceptCall(0x4793E0, SCR_UpdateFrame_addr,SCR_UpdateFrame);

	static auto CalcGammaRamp_ignore = safetyhook::create_mid(0x6D550B, [](SafetyHookContext& ctx) {

		if (r_gamma_windowed && r_gamma_windowed->current.integer >= 2) {
			ctx.xmm1.f32[0] = 1.f;
		}

		});

	r_gamma_x360 = Dvar_RegisterBool(false, "r_gamma_x360", DVAR_FLAG_ARCHIVE, "Xbox 360 Gamma Correction");
	r_gamma_windowed = Dvar_RegisterInt(0, "r_gamma_alt",0,2, DVAR_FLAG_ARCHIVE,"Applies r_gamma in post-fx, 1 is for Windowed mode only, 2 is for both Windowed and Fullscreen and ignores old DX9 Gamma");

	Material_Register_FastFileD = safetyhook::create_inline(0x6E9C00, &Material_Register_FastFile);


}
