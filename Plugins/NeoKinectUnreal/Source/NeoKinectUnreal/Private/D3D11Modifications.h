// Copyright 2016-2018 Rodrigo Villani Pereira

#pragma once

#include "CoreMinimal.h"

// D3D11
#include "Windows/AllowWindowsPlatformTypes.h"
#include <d3d11.h>
#include "Windows/HideWindowsPlatformTypes.h"

DECLARE_LOG_CATEGORY_CLASS(LogD3D11RHI, Log, All);

#include "ShaderCore.h"
// D3D11RHI module
#include "D3D11State.h"
#include "D3D11Resources.h"
#include "D3D11Util.h"
#include "RHI.h"
#include "Containers/ResourceArray.h"

#include "RenderUtils.h"


// Namespace to avoid conflicts with Unreal global methods upon packaging.
namespace D3D11RHIModifications
{
	constexpr uint32 DX_MAX_MSAA_COUNT = 8;

	DECLARE_CYCLE_STAT_EXTERN(TEXT("CreateTexture time"), STAT_D3D11CreateTextureTime, STATGROUP_D3D11RHI, );
	DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Textures Allocated"), STAT_D3D11TexturesAllocated, STATGROUP_D3D11RHI, );
	DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Textures Released"), STAT_D3D11TexturesReleased, STATGROUP_D3D11RHI, );
	
	/** Find an appropriate DXGI format for the input format and SRGB setting. */
	inline DXGI_FORMAT FindShaderResourceDXGIFormat(DXGI_FORMAT InFormat, bool bSRGB)
	{
		if (bSRGB)
		{
			switch (InFormat)
			{
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case DXGI_FORMAT_BC1_TYPELESS:         return DXGI_FORMAT_BC1_UNORM_SRGB;
			case DXGI_FORMAT_BC2_TYPELESS:         return DXGI_FORMAT_BC2_UNORM_SRGB;
			case DXGI_FORMAT_BC3_TYPELESS:         return DXGI_FORMAT_BC3_UNORM_SRGB;
			case DXGI_FORMAT_BC7_TYPELESS:         return DXGI_FORMAT_BC7_UNORM_SRGB;
			};
		}
		else
		{
			switch (InFormat)
			{
			case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case DXGI_FORMAT_BC1_TYPELESS:      return DXGI_FORMAT_BC1_UNORM;
			case DXGI_FORMAT_BC2_TYPELESS:      return DXGI_FORMAT_BC2_UNORM;
			case DXGI_FORMAT_BC3_TYPELESS:      return DXGI_FORMAT_BC3_UNORM;
			case DXGI_FORMAT_BC7_TYPELESS:      return DXGI_FORMAT_BC7_UNORM;
			};
		}
		switch (InFormat)
		{
			case DXGI_FORMAT_R24G8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_R32_FLOAT;
			case DXGI_FORMAT_R16_TYPELESS: return DXGI_FORMAT_R16_UNORM;
			// Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
			case DXGI_FORMAT_R32G8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		}
		return InFormat;
	}

	const TCHAR* GetD3D11TextureFormatString_mod(DXGI_FORMAT TextureFormat);

	/** Find the appropriate depth-stencil targetable DXGI format for the given format. */
	inline DXGI_FORMAT FindDepthStencilDXGIFormat(DXGI_FORMAT InFormat)
	{
		switch (InFormat)
		{
			case DXGI_FORMAT_R24G8_TYPELESS:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			// Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
			case DXGI_FORMAT_R32G8X24_TYPELESS:
				return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			case DXGI_FORMAT_R32_TYPELESS:
				return DXGI_FORMAT_D32_FLOAT;
			case DXGI_FORMAT_R16_TYPELESS:
				return DXGI_FORMAT_D16_UNORM;
		};
		return InFormat;
	}

	/**
	* Returns whether the given format contains stencil information.
	* Must be passed a format returned by FindDepthStencilDXGIFormat, so that typeless versions are converted to their corresponding depth stencil view format.
	*/
	inline bool HasStencilBits(DXGI_FORMAT InFormat)
	{
		switch (InFormat)
		{
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return true;
			// Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			return true;
		};
		return false;
	}

	bool ShouldCountAsTextureMemory(uint32 BindFlags);
	
	// @param b3D true:3D, false:2D or cube map
	static TStatId GetD3D11StatEnum(uint32 BindFlags, bool bCubeMap, bool b3D);
	

	// Note: This function can be called from many different threads
	// @param TextureSize >0 to allocate, <0 to deallocate
	// @param b3D true:3D, false:2D or cube map
	void UpdateD3D11TextureStats_mod(uint32 BindFlags, uint32 MiscFlags, int64 TextureSize, bool b3D);

	void D3D11TextureAllocated(TD3D11Texture2D<FD3D11BaseTexture2D>& Texture);

	void SafeCreateTexture2D_mod(ID3D11Device* Direct3DDevice, int32 UEFormat, const D3D11_TEXTURE2D_DESC* TextureDesc, const D3D11_SUBRESOURCE_DATA* SubResourceData, ID3D11Texture2D** OutTexture2D);

	FORCEINLINE FTexture2DRHIRef RHICreateDynamicTexture2D(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumSamples, ETextureCreateFlags Flags, FRHIResourceCreateInfo& CreateInfo)
	{
		const uint32 NumMips = 1;

		/* Original parameters on Epic's code (D3D11Texture.cpp)
		return CreateD3D11Texture2D<FD3D11BaseTexture2D>(
			SizeX, SizeY, 1,
			false, false, Format,
			NumMips, NumSamples, Flags, CreateInfo);

		Original function declaration
		template<typename BaseResourceType> TD3D11Texture2D<BaseResourceType>* FD3D11DynamicRHI::CreateD3D11Texture2D(
		uint32 SizeX, uint32 SizeY, uint32 SizeZ,
		bool bTextureArray, bool bCubeTexture, uint8 Format,
		uint32 NumMips, uint32 NumSamples, uint32 Flags, FRHIResourceCreateInfo& CreateInfo)
		*/

		check(SizeX > 0 && SizeY > 0);
		checkf(SizeX <= GetMax2DTextureDimension(), TEXT("Requested texture2d x size too large: %i, %i"), SizeX, GetMax2DTextureDimension());
		checkf(SizeY <= GetMax2DTextureDimension(), TEXT("Requested texture2d y size too large: %i, %i"), SizeY, GetMax2DTextureDimension());
		check(!(Flags & TexCreate_UAV));

		SCOPE_CYCLE_COUNTER(STAT_D3D11CreateTextureTime);

		bool bPooledTexture = true;

		const bool bSRGB = (Flags & TexCreate_SRGB) != 0;

		const DXGI_FORMAT PlatformResourceFormat = Flags & TexCreate_Shared ? DXGI_FORMAT_B8G8R8A8_UNORM : (DXGI_FORMAT)GPixelFormats[Format].PlatformFormat;
		const DXGI_FORMAT PlatformShaderResourceFormat = FindShaderResourceDXGIFormat(PlatformResourceFormat, bSRGB);
		const DXGI_FORMAT PlatformRenderTargetFormat = FindShaderResourceDXGIFormat(PlatformResourceFormat, bSRGB);

		// Determine the MSAA settings to use for the texture.
		D3D11_DSV_DIMENSION DepthStencilViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		D3D11_RTV_DIMENSION RenderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		D3D11_SRV_DIMENSION ShaderResourceViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		uint32 CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // this is the crucial change
		D3D11_USAGE TextureUsage = D3D11_USAGE_DYNAMIC; // this too
		bool bCreateShaderResource = true;

		uint32 ActualMSAACount = NumSamples;

		uint32 ActualMSAAQuality = (ActualMSAACount <= DX_MAX_MSAA_COUNT) ? 0 : 0xffffffff;

		// 0xffffffff means not supported
		if (ActualMSAAQuality == 0xffffffff || (Flags & TexCreate_Shared) != 0)
		{
			// no MSAA
			ActualMSAACount = 1;
			ActualMSAAQuality = 0;
		}

		if (ActualMSAACount > 1)
		{
			DepthStencilViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			RenderTargetViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			ShaderResourceViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			bPooledTexture = false;
		}

		if (NumMips < 1 || SizeX != SizeY || (1 << (NumMips - 1)) != SizeX || (Flags & TexCreate_Shared) != 0)
		{
			bPooledTexture = false;
		}

		if (Flags & TexCreate_CPUReadback)
		{
			check(!(Flags & TexCreate_RenderTargetable));
			check(!(Flags & TexCreate_DepthStencilTargetable));
			check(!(Flags & TexCreate_ShaderResource));

			CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			TextureUsage = D3D11_USAGE_STAGING;
			bCreateShaderResource = false;
		}

		// Describe the texture.
		D3D11_TEXTURE2D_DESC TextureDesc;
		ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		TextureDesc.Width = SizeX;
		TextureDesc.Height = SizeY;
		TextureDesc.MipLevels = NumMips;
		TextureDesc.ArraySize = 1;
		TextureDesc.Format = PlatformResourceFormat;
		TextureDesc.SampleDesc.Count = ActualMSAACount;
		TextureDesc.SampleDesc.Quality = ActualMSAAQuality;
		TextureDesc.Usage = TextureUsage;
		TextureDesc.BindFlags = bCreateShaderResource? D3D11_BIND_SHADER_RESOURCE : 0;
		TextureDesc.CPUAccessFlags = CPUAccessFlags;
		TextureDesc.MiscFlags = 0;

		if (Flags & TexCreate_Shared)
		{
			TextureDesc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
		}

		// Set up the texture bind flags.
		bool bCreateRTV = false;
		bool bCreateDSV = false;
		bool bCreatedRTVPerSlice = false;

		if (Flags & TexCreate_RenderTargetable)
		{
			check(!(Flags & TexCreate_DepthStencilTargetable));
			check(!(Flags & TexCreate_ResolveTargetable));
			TextureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			bCreateRTV = true;
		}
		else if (Flags & TexCreate_DepthStencilTargetable)
		{
			check(!(Flags & TexCreate_RenderTargetable));
			check(!(Flags & TexCreate_ResolveTargetable));
			TextureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			bCreateDSV = true;
		}
		else if (Flags & TexCreate_ResolveTargetable)
		{
			check(!(Flags & TexCreate_RenderTargetable));
			check(!(Flags & TexCreate_DepthStencilTargetable));
			if (Format == PF_DepthStencil || Format == PF_ShadowDepth || Format == PF_D24)
			{
				TextureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
				bCreateDSV = true;
			}
			else
			{
				TextureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				bCreateRTV = true;
			}
		}

		if (bCreateDSV && !(Flags & TexCreate_ShaderResource))
		{
			TextureDesc.BindFlags &= ~D3D11_BIND_SHADER_RESOURCE;
			bCreateShaderResource = false;
		}

		if (bCreateDSV || bCreateRTV)
		{
			bPooledTexture = false;
		}

		FVRamAllocation VRamAllocation;

		TRefCountPtr<ID3D11Texture2D> TextureResource;
		TRefCountPtr<ID3D11ShaderResourceView> ShaderResourceView;
		TArray<TRefCountPtr<ID3D11RenderTargetView> > RenderTargetViews;
		TRefCountPtr<ID3D11DepthStencilView> DepthStencilViews[FExclusiveDepthStencil::MaxIndex];

		Flags &= ~TexCreate_Virtual;

		ID3D11Device* Direct3DDevice = reinterpret_cast<ID3D11Device*>(RHIGetNativeDevice());

		if (!IsValidRef(TextureResource))
		{
			TArray<D3D11_SUBRESOURCE_DATA> SubResourceData;

			if (CreateInfo.BulkData)
			{
				uint8* Data = (uint8*)CreateInfo.BulkData->GetResourceBulkData();

				// each mip of each array slice counts as a subresource
				SubResourceData.AddZeroed(NumMips);
				
				uint32 SliceOffset = 0;
				for (uint32 ArraySliceIndex = 0; ArraySliceIndex < 1; ++ArraySliceIndex)
				{
					uint32 MipOffset = 0;
					for (uint32 MipIndex = 0; MipIndex < NumMips; ++MipIndex)
					{
						uint32 DataOffset = SliceOffset + MipOffset;
						uint32 SubResourceIndex = ArraySliceIndex * NumMips + MipIndex;

						uint32 NumBlocksX = FMath::Max<uint32>(1, (SizeX >> MipIndex) / GPixelFormats[Format].BlockSizeX);
						uint32 NumBlocksY = FMath::Max<uint32>(1, (SizeY >> MipIndex) / GPixelFormats[Format].BlockSizeY);

						SubResourceData[SubResourceIndex].pSysMem = &Data[DataOffset];
						SubResourceData[SubResourceIndex].SysMemPitch = NumBlocksX * GPixelFormats[Format].BlockBytes;
						SubResourceData[SubResourceIndex].SysMemSlicePitch = NumBlocksX * NumBlocksY * SubResourceData[MipIndex].SysMemPitch;

						MipOffset += NumBlocksY * SubResourceData[MipIndex].SysMemPitch;
					}
					SliceOffset += MipOffset;
				}
			}

			{
				SafeCreateTexture2D_mod(Direct3DDevice, Format, &TextureDesc, CreateInfo.BulkData != NULL ? (const D3D11_SUBRESOURCE_DATA*)SubResourceData.GetData() : NULL, TextureResource.GetInitReference());
			}

			if (bCreateRTV)
			{
				// Create a render target view for the mip
				D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
				FMemory::Memzero(&RTVDesc, sizeof(RTVDesc));
				RTVDesc.Format = PlatformRenderTargetFormat;
				RTVDesc.ViewDimension = RenderTargetViewDimension;
				RTVDesc.Texture2D.MipSlice = 0;

				TRefCountPtr<ID3D11RenderTargetView> RenderTargetView;
				VERIFYD3D11RESULT_EX(Direct3DDevice->CreateRenderTargetView(TextureResource, &RTVDesc, RenderTargetView.GetInitReference()), Direct3DDevice);
				RenderTargetViews.Add(RenderTargetView);
			}

			if (bCreateDSV)
			{
				// Create a depth-stencil-view for the texture.
				D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
				FMemory::Memzero(&DSVDesc, sizeof(DSVDesc));
				DSVDesc.Format = FindDepthStencilDXGIFormat(PlatformResourceFormat);
				DSVDesc.ViewDimension = DepthStencilViewDimension;
				DSVDesc.Texture2D.MipSlice = 0;

				for (uint32 AccessType = 0; AccessType < FExclusiveDepthStencil::MaxIndex; ++AccessType)
				{
					// Create a read-only access views for the texture.
					// Read-only DSVs are not supported in Feature Level 10 so 
					// a dummy DSV is created in order reduce logic complexity at a higher-level.
					if (Direct3DDevice->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_0 || Direct3DDevice->GetFeatureLevel() == D3D_FEATURE_LEVEL_11_1)
					{
						DSVDesc.Flags = (AccessType & FExclusiveDepthStencil::DepthRead_StencilWrite) ? D3D11_DSV_READ_ONLY_DEPTH : 0;
						if (HasStencilBits(DSVDesc.Format))
						{
							DSVDesc.Flags |= (AccessType & FExclusiveDepthStencil::DepthWrite_StencilRead) ? D3D11_DSV_READ_ONLY_STENCIL : 0;
						}
					}
					VERIFYD3D11RESULT_EX(Direct3DDevice->CreateDepthStencilView(TextureResource, &DSVDesc, DepthStencilViews[AccessType].GetInitReference()), Direct3DDevice);
				}
			}
		}
		check(IsValidRef(TextureResource));

		// Create a shader resource view for the texture.
		if (bCreateShaderResource)
		{
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
				SRVDesc.Format = PlatformShaderResourceFormat;

				SRVDesc.ViewDimension = ShaderResourceViewDimension;
				SRVDesc.Texture2D.MostDetailedMip = 0;
				SRVDesc.Texture2D.MipLevels = NumMips;
				VERIFYD3D11RESULT_EX(Direct3DDevice->CreateShaderResourceView(TextureResource, &SRVDesc, ShaderResourceView.GetInitReference()), Direct3DDevice);
			}

			check(IsValidRef(ShaderResourceView));
		}

		TD3D11Texture2D<FD3D11BaseTexture2D>* Texture2D = new TD3D11Texture2D<FD3D11BaseTexture2D>(
			reinterpret_cast<FD3D11DynamicRHI*>(GDynamicRHI),
			TextureResource,
			ShaderResourceView,
			bCreatedRTVPerSlice,
			TextureDesc.ArraySize,
			RenderTargetViews,
			DepthStencilViews,
			SizeX,
			SizeY,
			1,
			NumMips,
			ActualMSAACount,
			(EPixelFormat)Format,
			false,
			Flags,
			bPooledTexture,
			CreateInfo.ClearValueBinding
#if PLATFORM_SUPPORTS_VIRTUAL_TEXTURES
			, RawTextureMemory
#endif
			);

		Texture2D->ResourceInfo.VRamAllocation = VRamAllocation;

		D3D11TextureAllocated(*Texture2D);

		if (CreateInfo.BulkData)
		{
			CreateInfo.BulkData->Discard();
		}

		return Texture2D;
	}

	FORCEINLINE void RHIUpdateDynamicTexture2D(const FTexture2DRHIRef TextureRHIRef, const uint8* Buffer, const SIZE_T BufferSize)
	{
		if (!TextureRHIRef || !TextureRHIRef.IsValid()) return;

		FRHICommandListExecutor::GetImmediateCommandList().ImmediateFlush(EImmediateFlushType::FlushRHIThread);

		ID3D11Device* NativeDevice = reinterpret_cast<ID3D11Device*>(RHIGetNativeDevice());
		ID3D11Texture2D* NativeTexture = reinterpret_cast<ID3D11Texture2D*>(TextureRHIRef->GetNativeResource());
		D3D11_TEXTURE2D_DESC Desc;
		NativeTexture->GetDesc(&Desc);
		ID3D11DeviceContext* DeviceContext(nullptr);
		NativeDevice->GetImmediateContext(&DeviceContext);
		if (DeviceContext)
		{
			D3D11_MAPPED_SUBRESOURCE MappedResource;
			ZeroMemory(&MappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
			DeviceContext->Map(NativeTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
			FMemory::Memcpy(MappedResource.pData, Buffer, BufferSize);
			DeviceContext->Unmap(NativeTexture, 0);
		}
	}
}
