// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture.h"
#include "LGUISettings.generated.h"

/** Atlas texture size must be power of 2 */
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIAtlasTextureSizeType :uint8
{
	SIZE_256x256 = 0		UMETA(DisplayName = "256x256"),
	SIZE_512x512			UMETA(DisplayName = "512x512"),
	SIZE_1024x1024			UMETA(DisplayName = "1024x1024"),
	SIZE_2048x2048			UMETA(DisplayName = "2048x2048"),
	SIZE_4096x4096			UMETA(DisplayName = "4096x4096"),
	SIZE_8192x8192			UMETA(DisplayName = "8192x8192"),
};
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIAtlasPackingType :uint8
{
	/** dynamic pack, without mipmap */
	Dynamic,
	/** pack atlas when first time the game start */
	Static,
};
/**
 * Aniti Aliasing(MSAA) for LGUI screen space UI renderring
 */
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIScreenSpaceUIAntiAliasing :uint8
{
	Hidden=0				UMETA(Hidden),
	Disabled=1,
	SampleCount_2x=2		UMETA(DisplayName = "2x"),
	SampleCount_4x=4		UMETA(DisplayName = "4x"),
	SampleCount_8x=8		UMETA(DisplayName = "8x"),
};

USTRUCT(BlueprintType)
struct LGUI_API FLGUIAtlasSettings
{
	GENERATED_BODY()
public:
	/**
	 * when packing sprites into one single texture, we will use this size to create a blank texture, then insert sprites. if texture is full(cannot insert anymore sprite), a new larger texture will be created. 
	 * if initialSize is too small, some lag or freeze may happen when creating new texture.
	 * if initialSize is too large, it is not efficient to sample large texture on GPU.
	*/
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		ELGUIAtlasTextureSizeType atlasTextureInitialSize = ELGUIAtlasTextureSizeType::SIZE_1024x1024;
	/** whether or not use srgb for generate atlas texture */
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		bool atlasTextureUseSRGB = true;
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		TEnumAsByte<TextureFilter> atlasTextureFilter = TextureFilter::TF_Trilinear;
	/** space between two sprites when package into atlas */
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		int32 spaceBetweenSprites = 2;
	//UPROPERTY(EditAnywhere, config, Category = Sprite)
	//	ELGUIAtlasPackingType packingType = ELGUIAtlasPackingType::Dynamic;
};

class ULGUIBehaviour;
/** for LGUI config */
UCLASS(config=Engine, defaultconfig)
class LGUI_API ULGUISettings :public UObject
{
	GENERATED_BODY()
public:
	/** default atlas setting */
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		FLGUIAtlasSettings defaultAtlasSetting;
	/** override atlasSettings for your packingTag, otherwise use defaultAtlasSettings */
	UPROPERTY(EditAnywhere, config, Category = Sprite)
		TMap<FName, FLGUIAtlasSettings> atlasSettingForSpecificPackingTag;
	/** new created uiitem will use this trace channel; */
	UPROPERTY(EditAnywhere, config, Category = "LGUI")
		TEnumAsByte<ETraceTypeQuery> defaultTraceChannel = TraceTypeQuery3;

	UPROPERTY(EditAnywhere, config, Category = "Rendering")
		ELGUIScreenSpaceUIAntiAliasing antiAliasing = ELGUIScreenSpaceUIAntiAliasing::Disabled;

	/** 
	 * When use auto manage depth, only 2D elements can be batched.
	 * Ruls for telling if a UI element is 2D (convert the UI element in Canvas's relative space):
	 *		Relative location.Z less than threshold.
	 *		Relative rotation.X/Y less than threshold.
	 * This is the threshold for determine if the UI element is 2D.
	 */
	//UPROPERTY(EditAnywhere, config, Category = "LGUI", meta = (ClampMin = "0.00001", ClampMax = "100"))
	//	float autoManageDepthThreshold = KINDA_SMALL_NUMBER;
#ifdef LGUI_DRAWCALLMODE_AUTO
#if WITH_EDITORONLY_DATA
	static float cacheAutoManageDepthThreshold;
#endif
#endif
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
	static ELGUIScreenSpaceUIAntiAliasing GetAntiAliasingSampleCount();
#ifdef LGUI_DRAWCALLMODE_AUTO
	static float GetAutoManageDepthThreshold();
#endif
	FORCEINLINE static int32 ConvertAtlasTextureSizeTypeToSize(const ELGUIAtlasTextureSizeType& InType)
	{
		return FMath::Pow(2.0, (double)InType) * 256;
	}
private:
	FORCEINLINE static const FLGUIAtlasSettings& GetAtlasSettings(const FName& InPackingTag);
};

//@todo:save config in editor
UCLASS(config=Editor, defaultconfig)
class LGUI_API ULGUIEditorSettings : public UObject
{
	GENERATED_BODY()
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	static int32 GetLGUIPreview_EditorViewIndex();
	static void SetLGUIPreview_EditorViewIndex(int32 value);
	static bool GetPreserveHierarchyState();
	static float GetDelayRestoreHierarchyTime();
#endif
	virtual bool IsEditorOnly()const override { return true; }
#if WITH_EDITORONLY_DATA
	//show screen space LGUI on target editor view. 
	UPROPERTY(config)
		int32 LGUIPreview_EditorViewIndex = 6;
	static FSimpleMulticastDelegate LGUIPreviewSetting_EditorPreviewViewportIndexChange;
	/**
	 * Preserve hierarchy state", "Preserve \"World Outliner\"'s actor state. When reload a level, all actor will expand and temporarily hidden actor become visible, so use this option can keep these actor and folder's state.\n\
	 * Note: If actors in folder and the folder is not expanded, then these actors's state will not affected, because I can't get these tree items.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LGUI Editor")
		bool PreserveHierarchyState = true;
	/**
	 * Sometimes when there are too many actors in level, restore hierarchy will not work. Then increase this value may solve the issue.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LGUI Editor")
		float DelayRestoreHierarchyTime = 0.2f;
#endif
};
