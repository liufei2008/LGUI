// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LexUIDataAsTexture.generated.h"

UENUM(BlueprintType)
enum class ELexUIDataAsTexturePixelFormat:uint8
{
	R8,
	R16,
	R32,
	R8G8B8A8,
	R16G16B16A16,
	R32G32B32A32,
};
UCLASS(ClassGroup = (LexUI), BlueprintType)
class LGUI_API ULexUIDataAsTexture :public UDataAsset
{
	GENERATED_BODY()
public:
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange)override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
	virtual void BeginDestroy() override;
private:
	/** Texture to fill buffer data, and decode to buffer in shader. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LexUI")
	TObjectPtr<UTexture> Texture = nullptr;

	ELexUIDataAsTexturePixelFormat PixelFormat = ELexUIDataAsTexturePixelFormat::R8;
	int BytesPerPixel = 4;
	//how many bytes in single block
	int BlockSizeInByte = 4;
	//how many pixels in single block
	int BlockPixelCount = 1;

	int TextureWidth = 1;
	int TextureHeight = 1;
	//Pixel position
	FIntVector2 CurrentPosition = FIntVector2(0, 0);
	bool bIsInitialized = false;
	int32 TextureMaxSize = 4096;
	TArray<FIntVector2> NotUsingPositionArray;
	struct FPendingUpdateData
	{
		int PosX = 0, PosY = 0;
		TArray<uint8> Data;
		int DataPixelCount = 1;
	};
	TArray<FPendingUpdateData> PendingUpdateDataArray;
	bool bBatchUpdateMode = false;

	void CreateTexture();
	bool ExpandTextureWidth();
	bool ExpandTextureHeight();
protected:
	virtual void PostInitProperties()override;
public:
	/**
	 * Initialize this buffer.
	 * @param InBlockSizeInByte byte count
	 * @param InInitialTextureHeight texture size when first create it
	 */
	void Init(int InBlockSizeInByte, ELexUIDataAsTexturePixelFormat InPixelFormat, int InInitialTextureHeight = 32, int InMaxTextureSize = 4096);
	int GetBlockSizeInByte()const { return BlockSizeInByte; }
	/**
	 * Request a new block area with initialize block size.
	 * @return Start position in texture's data
	 */
	int RegisterBuffer();
	void UnregisterBuffer(int InPosition);
	void UpdateBlock(int InBufferPosition, TArray<uint8> InData);
	void UpdateBlock(int InBufferPositionXOffset, int InBufferPosition, TArray<uint8> InData, int InDataPixelCount);

	bool GetIsBatchUpdateMode()const { return bBatchUpdateMode; }
	void PrepareForBatchUpdate();
	void Flush();

	UTexture* GetDataTexture()const { return Texture; }

	DECLARE_EVENT_OneParam(ULexUIDataAsTexture, FOnDataTextureChange, UTexture*);
	FOnDataTextureChange OnDataTextureChange;
};