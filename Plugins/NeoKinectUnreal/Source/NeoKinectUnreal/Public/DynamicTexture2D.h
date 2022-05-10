// Copyright 2016-2018 Rodrigo Villani Pereira

#pragma once

#include "Engine/Texture.h"
#include "DynamicTexture2DResource.h"

#include "DynamicTexture2D.generated.h"

UCLASS(HideCategories=(Object), BlueprintType, Category="NeoKinect|Textures")
class UDynamicTexture2D : public UTexture
{
	GENERATED_BODY()

public:
	UDynamicTexture2D();

	/** The width of the texture. */
	int32 SizeX;

	/** The height of the texture. */
	int32 SizeY;

	/** The format of the texture. */
	UPROPERTY(transient)
	TEnumAsByte<enum EPixelFormat> Format;

	/** The number of mip-maps in the texture. */
	int32 NumMips;

	/** Whether the texture can be used as a resolve target. */
	uint32 bIsResolveTarget : 1;

	// Begin UTexture Interface.
	virtual FTextureResource* CreateResource() override;
	virtual EMaterialValueType GetMaterialType() const override { return MCT_Texture2D; }
	virtual float GetSurfaceWidth() const override;
	virtual float GetSurfaceHeight() const override;
	// End UTexture Interface.

	FORCEINLINE int32 GetSizeX() const
	{
		return SizeX;
	}
	FORCEINLINE int32 GetSizeY() const
	{
		return SizeY;
	}

	/**
	 * Gets the X size of the texture, in pixels
	 */
	UFUNCTION(BlueprintCallable, meta=(DisplayName = "GetSizeX"), Category="NeoKinect|Textures")
	int32 Blueprint_GetSizeX() const;

	/**
	 * Gets the Y size of the texture, in pixels
	 */
	UFUNCTION(BlueprintCallable, meta=(DisplayName = "GetSizeY"), Category="NeoKinect|Textures")
	int32 Blueprint_GetSizeY() const;

	/**
	* Updates the texture memory on the GPU. Must be called from the render thread!
	*
	* @param Buffer		- Pointer to the Buffer texels to copy to the GPU texture memory.
	* @param BufferSize	- Size of the buffer in bytes.
	*/
	FORCEINLINE void UpdateContents(const uint8* Buffer, const SIZE_T BufferSize)
	{
		FDynamicTexture2DResource* DynamicResource = static_cast<FDynamicTexture2DResource*>(Resource);
		if (!DynamicResource || !DynamicResource->IsInitialized()) return;
		DynamicResource->UpdateTextureBuffer(Buffer, BufferSize);
	}

	/**
	 * Initializes the texture with 1 mip-level and creates the render resource.
	 *
	 * @param InSizeX			- Width of the texture, in texels
	 * @param InSizeY			- Height of the texture, in texels
	 * @param InFormat			- Format of the texture, defaults to PF_B8G8R8A8
	 * @param InIsResolveTarget	- Whether the texture can be used as a resolve target
	 */
	void Init(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat = PF_B8G8R8A8, bool InIsResolveTarget = false);

	/** Creates and initializes a new Texture2DDynamic with the requested settings */
	static UDynamicTexture2D* Create(int32 InSizeX, int32 InSizeY, EPixelFormat InFormat = PF_B8G8R8A8, bool InIsResolveTarget = false);
};