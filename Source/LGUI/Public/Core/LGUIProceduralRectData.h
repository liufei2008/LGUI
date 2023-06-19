// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUIProceduralRectData.generated.h"

class UTexture2D;

UCLASS(ClassGroup = (LGUI), BlueprintType)
class LGUI_API ULGUIProceduralRectData :public UDataAsset
{
	GENERATED_BODY()
public:
	void PreEditChange(FProperty* PropertyAboutToChange);
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
private:

	UPROPERTY(EditAnywhere, Category = "LGUI")
		UMaterialInterface* DefaultMaterials[(int)ELGUICanvasClipType::Custom];
	/** Texture to fill buffer data, and decode to buffer in shader. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		UTexture2D* Texture = nullptr;

	//how many bytes in single block
	int BlockSizeInByte = 4;
	//how many pixels in single block
	int BlockPixelCount = 1;

	int TextureSize = 1;
	//Pixel position
	FIntVector2 CurrentPosition = FIntVector2(0, 0);
	bool bIsInitialized = false;
	TArray<FIntVector2> NotUsingPositionArray;

	void CreateTexture();
	bool ExpandTexture();

	virtual void PostInitProperties()override;
	void CheckMaterials();
public:
	/**
	 * Initialize this buffer.
	 * @param InBlockSize byte count
	 */
	void Init(int InBlockSizeInByte);
	int GetBlockSizeInByte()const { return BlockSizeInByte; }
	/**
	 * Request a new block area with initialize block size.
	 * @return Start position in texture's data
	 */
	FIntVector2 RegisterBuffer();
	void UnregisterBuffer(const FIntVector2& InPosition);
	void UpdateBlock(const FIntVector2& InPosition, void* InData);

	UTexture2D* GetDataTexture()const { return Texture; }
	UMaterialInterface* GetMaterial(ELGUICanvasClipType clipType);

	DECLARE_EVENT_OneParam(ULGUIProceduralRectData, FOnDataTextureChange, UTexture2D*);
	FOnDataTextureChange OnDataTextureChange;
};