// Copyright 2016-2018 Rodrigo Villani Pereira

#pragma once

#include "D3D11Modifications.h"
#include "TextureResource.h"

/** A dynamic texture 2D resource. */
class  FDynamicTexture2DResource : public FTextureResource
{
public:
	/** Initialization constructor */
	FDynamicTexture2DResource(class UDynamicTexture2D* InOwner);

	/** Returns the width of the texture in pixels. */
	virtual uint32 GetSizeX() const override;

	/** Returns the height of the texture in pixels */
	virtual uint32 GetSizeY() const override;

	/** Called when the resource is initialized. This is only called by the rendering thread. */
	virtual void InitRHI() override;

	/** Called when the resource is released. This is only called by the rendering thread. */
	virtual void ReleaseRHI() override;

	/** Returns the Texture2DRHI, which can be used for locking/unlocking the mips. */
	FTexture2DRHIRef GetTexture2DRHI();

	/**
	 * Updates the texture memory on the GPU. Must be called from the render thread!
	 *
	 * @param Buffer		- Pointer to the Buffer texels to copy to the GPU texture memory.
	 * @param BufferSize	- Size of the buffer in bytes.
	 */
	FORCEINLINE void UpdateTextureBuffer(const uint8* Buffer, const size_t BufferSize)
	{
		D3D11RHIModifications::RHIUpdateDynamicTexture2D(GetTexture2DRHI(), Buffer, BufferSize);
	}

private:
	/** The owner of this resource. */
	class UDynamicTexture2D* Owner;
	/** Texture2D reference, used for locking/unlocking the mips. */
	FTexture2DRHIRef Texture2DRHI;
};