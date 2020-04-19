﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture.h"
#include "LGUISettings.generated.h"

//Atlas texture size must be power of 2
UENUM(BlueprintType)
enum class ELGUIAtlasTextureSizeType :uint8
{
	SIZE_256x256 = 0		UMETA(DisplayName = "256x256"),
	SIZE_512x512			UMETA(DisplayName = "512x512"),
	SIZE_1024x1024			UMETA(DisplayName = "1024x1024"),
	SIZE_2048x2048			UMETA(DisplayName = "2048x2048"),
	SIZE_4096x4096			UMETA(DisplayName = "4096x4096"),
	SIZE_8192x8192			UMETA(DisplayName = "8192x8192"),
};
UENUM(BlueprintType)
enum class ELGUIAtlasPackingType :uint8
{
	//dynamic pack, without mipmap
	Dynamic,
	//pack atlas when first time the game start
	Static,
};

USTRUCT(BlueprintType)
struct LGUI_API FLGUIAtlasSettings
{
	GENERATED_BODY()
public:
	/*
	when packing sprites into one single texture, we will use this size to create a blank texture, then insert sprites. if texture is full(cannot insert anymore sprite), a new larger texture will be created. 
	if initialSize is too small, some lag or freeze may happen when creating new texture.
	if initialSize is too large, it is not efficient to sample large texture on GPU.
	*/
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		ELGUIAtlasTextureSizeType atlasTextureInitialSize = ELGUIAtlasTextureSizeType::SIZE_1024x1024;
	//whether or not use srgb for generate atlas texture
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		bool atlasTextureUseSRGB = true;
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		TEnumAsByte<TextureFilter> atlasTextureFilter = TextureFilter::TF_Trilinear;
	//space between two sprites when package into atlas
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		int32 spaceBetweenSprites = 2;
	//UPROPERTY(EditAnywhere, config, Category = Sprite)
	//	ELGUIAtlasPackingType packingType = ELGUIAtlasPackingType::Dynamic;
};
//for LGUI config
UCLASS(config=EditorPerProjectUserSettings)
class LGUI_API ULGUISettings :public UObject
{
	GENERATED_BODY()
public:
	//default atlas setting
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		FLGUIAtlasSettings defaultAtlasSetting;
	//override atlasSettings for your packingTag, otherwise use defaultAtlasSettings
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		TMap<FName, FLGUIAtlasSettings> atlasSettingForSpecificPackingTag;
	//new created uiitem will use this trace channel;
	UPROPERTY(EditAnywhere, config, Category = UIItem)
		TEnumAsByte<ETraceTypeQuery> defaultTraceChannel = TraceTypeQuery3;
	//if LGUICanvas update times is greater than this in single frame, than a warning will show in Ouput Log, that means something not good.
	//do not change this unless you know what you doing.
	UPROPERTY(EditAnywhere, config, Category = LGUICanvas)
		int32 maxCanvasUpdateTimeInOneFrame = 10;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
public:
	static int32 GetAtlasTextureInitialSize(const FName& InPackingTag);
	static bool GetAtlasTextureSRGB(const FName& InPackingTag);
	static int32 GetAtlasTexturePadding(const FName& InPackingTag);
	static TextureFilter GetAtlasTextureFilter(const FName& InPackingTag);
	//static ELGUIAtlasPackingType GetAtlasPackingType(const FName& InPackingTag);
	static const TMap<FName, FLGUIAtlasSettings>& GetAllAtlasSettings();
private:
	FORCEINLINE static int32 ConvertAtlasTextureSizeTypeToSize(const ELGUIAtlasTextureSizeType& InType)
	{
		return FMath::Pow(2, (int32)InType) * 256;
	}
	FORCEINLINE static const FLGUIAtlasSettings& GetAtlasSettings(const FName& InPackingTag);
};