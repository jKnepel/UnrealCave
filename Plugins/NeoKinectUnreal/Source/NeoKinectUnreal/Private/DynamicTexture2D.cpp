// Copyright 2016-2018 Rodrigo Villani Pereira

#include "DynamicTexture2D.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "DeviceProfiles/DeviceProfileManager.h"

using namespace D3D11RHIModifications;

/*=============================================================================
	DynamicTexture2D.cpp: Implementation of UDynamicTexture2D.
=============================================================================*/

/*-----------------------------------------------------------------------------
	FDynamicTexture2DResource
-----------------------------------------------------------------------------*/

/** Initialization constructor. */
FDynamicTexture2DResource::FDynamicTexture2DResource(UDynamicTexture2D* InOwner)
	: Owner(InOwner)
{
}

/** Returns the width of the texture in pixels. */
uint32 FDynamicTexture2DResource::GetSizeX() const
{
	return Owner->SizeX;
}

/** Returns the height of the texture in pixels. */
uint32 FDynamicTexture2DResource::GetSizeY() const
{
	return Owner->SizeY;
}

/** Called when the resource is initialized. This is only called by the rendering thread. */
void FDynamicTexture2DResource::InitRHI()
{
	// Create the sampler state RHI resource.
	FSamplerStateInitializerRHI SamplerStateInitializer
		(
			(ESamplerFilter)UDeviceProfileManager::Get().GetActiveProfile()->GetTextureLODSettings()->GetSamplerFilter(Owner),
			AM_Wrap,
			AM_Wrap,
			AM_Wrap
			);
	SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);

	ETextureCreateFlags Flags = ETextureCreateFlags::TexCreate_None;
	if (Owner->bIsResolveTarget)
	{
		Flags |= ETextureCreateFlags::TexCreate_ResolveTargetable;
		bIgnoreGammaConversions = true;		// Note, we're ignoring Owner->SRGB (it should be false).
	}
	else if (Owner->SRGB)
	{
		Flags |= ETextureCreateFlags::TexCreate_SRGB;
	}
	if (Owner->bNoTiling)
	{
		Flags |= ETextureCreateFlags::TexCreate_NoTiling;
	}
	FRHIResourceCreateInfo CreateInfo;
	Texture2DRHI = RHICreateDynamicTexture2D(GetSizeX(), GetSizeY(), Owner->Format, 1, Flags, CreateInfo);
	TextureRHI = Texture2DRHI;
	RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, TextureRHI);
}

/** Called when the resource is released. This is only called by the rendering thread. */
void FDynamicTexture2DResource::ReleaseRHI()
{
	RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, nullptr);
	FTextureResource::ReleaseRHI();
	Texture2DRHI.SafeRelease();
}

/** Returns the Texture2DRHI, which can be used for locking/unlocking the mips. */
FTexture2DRHIRef FDynamicTexture2DResource::GetTexture2DRHI()
{
	return Texture2DRHI;
}

/*-----------------------------------------------------------------------------
	UDynamicTexture2D
-----------------------------------------------------------------------------*/

UDynamicTexture2D::UDynamicTexture2D() {}

FTextureResource* UDynamicTexture2D::CreateResource()
{
	return new FDynamicTexture2DResource(this);
}

float UDynamicTexture2D::GetSurfaceWidth() const
{
	return SizeX;
}

float UDynamicTexture2D::GetSurfaceHeight() const
{
	return SizeY;
}

int32 UDynamicTexture2D::Blueprint_GetSizeX() const
{
	return GetSizeX();
}

int32 UDynamicTexture2D::Blueprint_GetSizeY() const
{
	return GetSizeY();
}

void UDynamicTexture2D::Init(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat/*=2*/, bool InIsResolveTarget/*=false*/)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	Format = (EPixelFormat)InFormat;
	NumMips = 1;
	bIsResolveTarget = InIsResolveTarget;

	// Initialize the resource.
	UpdateResource();
}

UDynamicTexture2D* UDynamicTexture2D::Create(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat, bool InIsResolveTarget)
{
	EPixelFormat DesiredFormat = EPixelFormat(InFormat);
	if (InSizeX > 0 && InSizeY > 0)
	{

		UDynamicTexture2D* NewTexture = NewObject<UDynamicTexture2D>();
		if (NewTexture != NULL)
		{
			// Disable compression
			NewTexture->CompressionSettings = TC_Default;
#if WITH_EDITORONLY_DATA
			NewTexture->CompressionNone = true;
			NewTexture->MipGenSettings = TMGS_NoMipmaps;
			NewTexture->CompressionNoAlpha = true;
			NewTexture->DeferCompression = false;
#endif // #if WITH_EDITORONLY_DATA
			if (InIsResolveTarget)
			{
				//				NewTexture->SRGB				= false;
				NewTexture->bNoTiling = false;
			}
			else
			{
				// Untiled format
				NewTexture->bNoTiling = true;
			}

			NewTexture->Init(InSizeX, InSizeY, DesiredFormat, InIsResolveTarget);
		}
		return NewTexture;
	}
	else
	{
		UE_LOG(LogTexture, Warning, TEXT("Invalid parameters specified for UDynamicTexture2D::Create()"));
		return NULL;
	}
}
