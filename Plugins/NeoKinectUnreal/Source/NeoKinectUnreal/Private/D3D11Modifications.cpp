// Copyright 2016-2018 Rodrigo Villani Pereira

#include "D3D11Modifications.h"

// Namespace to avoid conflicts with Unreal global methods upon packaging.
namespace D3D11RHIModifications
{
	DEFINE_STAT(STAT_D3D11CreateTextureTime);
	DEFINE_STAT(STAT_D3D11TexturesAllocated);
	DEFINE_STAT(STAT_D3D11TexturesReleased);

	const TCHAR* GetD3D11TextureFormatString_mod(DXGI_FORMAT TextureFormat)
	{
		static const TCHAR* EmptyString = TEXT("");
		const TCHAR* TextureFormatText = EmptyString;
#define D3DFORMATCASE(x) case x: TextureFormatText = TEXT(#x); break;
		switch (TextureFormat)
		{
			D3DFORMATCASE(DXGI_FORMAT_R8G8B8A8_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_B8G8R8A8_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_B8G8R8X8_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_BC1_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_BC2_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_BC3_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_BC4_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_R16G16B16A16_FLOAT)
				D3DFORMATCASE(DXGI_FORMAT_R32G32B32A32_FLOAT)
				D3DFORMATCASE(DXGI_FORMAT_UNKNOWN)
				D3DFORMATCASE(DXGI_FORMAT_R8_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
				D3DFORMATCASE(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS)
				D3DFORMATCASE(DXGI_FORMAT_R32G8X24_TYPELESS)
				D3DFORMATCASE(DXGI_FORMAT_D24_UNORM_S8_UINT)
				D3DFORMATCASE(DXGI_FORMAT_R24_UNORM_X8_TYPELESS)
				D3DFORMATCASE(DXGI_FORMAT_R32_FLOAT)
				D3DFORMATCASE(DXGI_FORMAT_R16G16_UINT)
				D3DFORMATCASE(DXGI_FORMAT_R16G16_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_R16G16_SNORM)
				D3DFORMATCASE(DXGI_FORMAT_R16G16_FLOAT)
				D3DFORMATCASE(DXGI_FORMAT_R32G32_FLOAT)
				D3DFORMATCASE(DXGI_FORMAT_R10G10B10A2_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_R16G16B16A16_UINT)
				D3DFORMATCASE(DXGI_FORMAT_R8G8_SNORM)
				D3DFORMATCASE(DXGI_FORMAT_BC5_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_R1_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_R8G8B8A8_TYPELESS)
				D3DFORMATCASE(DXGI_FORMAT_B8G8R8A8_TYPELESS)
				D3DFORMATCASE(DXGI_FORMAT_BC7_UNORM)
				D3DFORMATCASE(DXGI_FORMAT_BC6H_UF16)
		default: TextureFormatText = EmptyString;
		}
#undef D3DFORMATCASE
		return TextureFormatText;
	}

	bool ShouldCountAsTextureMemory(uint32 BindFlags)
	{
		return (BindFlags & (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS)) == 0;
	}
	
	// @param b3D true:3D, false:2D or cube map
	static TStatId GetD3D11StatEnum(uint32 BindFlags, bool bCubeMap, bool b3D)
	{
#if STATS
		if (ShouldCountAsTextureMemory(BindFlags))
		{
			// normal texture
			if (bCubeMap)
			{
				return GET_STATID(STAT_TextureMemoryCube);
			}
			else if (b3D)
			{
				return GET_STATID(STAT_TextureMemory3D);
			}
			else
			{
				return GET_STATID(STAT_TextureMemory2D);
			}
		}
		else
		{
			// render target
			if (bCubeMap)
			{
				return GET_STATID(STAT_RenderTargetMemoryCube);
			}
			else if (b3D)
			{
				return GET_STATID(STAT_RenderTargetMemory3D);
			}
			else
			{
				return GET_STATID(STAT_RenderTargetMemory2D);
			}
		}
#endif
		return TStatId();
	}
	

	// Note: This function can be called from many different threads
	// @param TextureSize >0 to allocate, <0 to deallocate
	// @param b3D true:3D, false:2D or cube map
	void UpdateD3D11TextureStats_mod(uint32 BindFlags, uint32 MiscFlags, int64 TextureSize, bool b3D)
	{
		if (TextureSize == 0)
		{
			return;
		}

		int64 AlignedSize = (TextureSize > 0) ? Align(TextureSize, 1024) / 1024 : -(Align(-TextureSize, 1024) / 1024);
		if (ShouldCountAsTextureMemory(BindFlags))
		{
			FPlatformAtomics::InterlockedAdd(&GCurrentTextureMemorySize, AlignedSize);
		}
		else
		{
			FPlatformAtomics::InterlockedAdd(&GCurrentRendertargetMemorySize, AlignedSize);
		}

		bool bCubeMap = (MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0;

		INC_MEMORY_STAT_BY_FName(GetD3D11StatEnum(BindFlags, bCubeMap, b3D).GetName(), TextureSize);

		if (TextureSize > 0)
		{
			INC_DWORD_STAT(STAT_D3D11TexturesAllocated);
		}
		else
		{
			INC_DWORD_STAT(STAT_D3D11TexturesReleased);
		}
	}

	void D3D11TextureAllocated(TD3D11Texture2D<FD3D11BaseTexture2D>& Texture)
	{
		ID3D11Texture2D* D3D11Texture2D = Texture.GetResource();

		if (D3D11Texture2D)
		{
			if ((Texture.Flags & TexCreate_Virtual) == TexCreate_Virtual)
			{
				Texture.SetMemorySize(0);
			}
			else
			{
				D3D11_TEXTURE2D_DESC Desc;

				D3D11Texture2D->GetDesc(&Desc);
				check(Texture.IsCubemap() == ((Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0));

				int64 TextureSize = CalcTextureSize(Desc.Width, Desc.Height, Texture.GetFormat(), Desc.MipLevels) * Desc.ArraySize;

				Texture.SetMemorySize(TextureSize);
				UpdateD3D11TextureStats_mod(Desc.BindFlags, Desc.MiscFlags, TextureSize, false);
			}
		}
	};

	/** If true, guard texture creates with SEH to log more information about a driver crash we are seeing during texture streaming. */
#define GUARDED_TEXTURE_CREATES (PLATFORM_WINDOWS && !(UE_BUILD_SHIPPING || UE_BUILD_TEST))

	void SafeCreateTexture2D_mod(ID3D11Device* Direct3DDevice, int32 UEFormat, const D3D11_TEXTURE2D_DESC* TextureDesc, const D3D11_SUBRESOURCE_DATA* SubResourceData, ID3D11Texture2D** OutTexture2D)
	{
#if GUARDED_TEXTURE_CREATES
		bool bDriverCrash = true;
		__try
		{
#endif // #if GUARDED_TEXTURE_CREATES
			VERIFYD3D11CREATETEXTURERESULT(
				Direct3DDevice->CreateTexture2D(TextureDesc, SubResourceData, OutTexture2D),
				UEFormat,
				TextureDesc->Width,
				TextureDesc->Height,
				TextureDesc->ArraySize,
				TextureDesc->Format,
				TextureDesc->MipLevels,
				TextureDesc->BindFlags,
				TextureDesc->Usage,
				TextureDesc->CPUAccessFlags,
				TextureDesc->MiscFlags,
				TextureDesc->SampleDesc.Count,
				TextureDesc->SampleDesc.Quality,
				SubResourceData ? SubResourceData->pSysMem : nullptr,
				SubResourceData ? SubResourceData->SysMemPitch : 0,
				SubResourceData ? SubResourceData->SysMemSlicePitch : 0,
				Direct3DDevice
				);
#if GUARDED_TEXTURE_CREATES
			bDriverCrash = false;
		}
		__finally
		{
			if (bDriverCrash)
			{
				UE_LOG(LogD3D11RHI, Error,
					TEXT("Driver crashed while creating texture: %ux%ux%u %s(0x%08x) with %u mips, PF_ %d"),
					TextureDesc->Width,
					TextureDesc->Height,
					TextureDesc->ArraySize,
					GetD3D11TextureFormatString_mod(TextureDesc->Format),
					(uint32)TextureDesc->Format,
					TextureDesc->MipLevels,
					UEFormat
					);
			}
		}
#endif // #if GUARDED_TEXTURE_CREATES
	}
}