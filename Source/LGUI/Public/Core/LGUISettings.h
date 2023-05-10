// Copyright 2019-Present LexLiu. All Rights Reserved.

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
};

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

	/**
	 * LGUI renderer use ISceneViewExtension to render, so this value can sort with other view extensions, higher comes first.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Rendering")
		int32 PriorityInSceneViewExtension = 0;

	/** 
	 * 3D UI elements is almost not possible to check overlap, so a 3D UI element only allowed to batch to last drawcall from drawcall list, as long as common check is passed (material/ texture).
	 * Only 2D elements are easier to check overlap and batch together.
	 *		Rules for telling if a UI element is 2D (convert the UI element in Canvas's relative space):
	 *			Relative location.Z less than threshold.
	 *			Relative rotation.X/Y less than threshold.
	 * This is the threshold for determine if the UI element is 2D.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LGUI", meta = (ClampMin = "0.00001", ClampMax = "100"))
		float AutoBatchThreshold = 0.01f;
#if WITH_EDITORONLY_DATA
	static float CacheAutoBatchThreshold;
#endif
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
public:
	static int32 GetAtlasTextureInitialSize(const FName& InPackingTag);
	static bool GetAtlasTextureSRGB(const FName& InPackingTag);
	static int32 GetAtlasTexturePadding(const FName& InPackingTag);
	static TextureFilter GetAtlasTextureFilter(const FName& InPackingTag);
	static const TMap<FName, FLGUIAtlasSettings>& GetAllAtlasSettings();
	static float GetAutoBatchThreshold();
	static int32 ConvertAtlasTextureSizeTypeToSize(const ELGUIAtlasTextureSizeType& InType);
	static int32 GetPriorityInSceneViewExtension();
private:
	static const FLGUIAtlasSettings& GetAtlasSettings(const FName& InPackingTag);
};

//@todo:save config in editor
UCLASS(config=Editor, defaultconfig)
class LGUI_API ULGUIEditorSettings : public UObject
{
	GENERATED_BODY()
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)override;
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
	static FSimpleMulticastDelegate LGUIEditorSetting_PreserveHierarchyStateChange;
	/**
	 * Sometimes when there are too many actors in level, restore hierarchy will not work. Then increase this value may solve the issue.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LGUI Editor")
		float DelayRestoreHierarchyTime = 0.2f;
	/**
	 * Prefabs in these folders will appear in "LGUI Tools" menu, so we can easily create our own UI control.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LGUI Editor", meta = (LongPackageName))
		TArray<FDirectoryPath> ExtraPrefabFolders;
	/**
	 * Draw helper box on selected UI element.
	 */
	UPROPERTY(config)
		bool bDrawHelperFrame = true;
	/** Show anchor tool on selected UI element. */
	UPROPERTY(config)
		bool bShowAnchorTool = true;
	/**
	 * Draw navigation visulaizer
	 */
	UPROPERTY(Transient)
		bool bDrawSelectableNavigationVisualizer = false;

	UPROPERTY(config)
		bool ShowLGUIColumnInSceneOutliner = true;
#endif
};
