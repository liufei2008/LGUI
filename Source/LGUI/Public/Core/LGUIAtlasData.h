// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LGUIAtlasData.generated.h"


class ULGUISpriteData;
class UUISpriteBase;

/** atlas data container */
USTRUCT()
struct LGUI_API FLGUIAtlasData
{
	GENERATED_BODY()
	/** collection of all UISprite whitch use this atlas to render */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TWeakObjectPtr<UUISpriteBase>> renderSpriteArray;
	/** atlasTexture is the real texture for render */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
	UTexture2D* atlasTexture = nullptr;
	/** information needed when insert a sprite */
	rbp::MaxRectsBinPack atlasBinPack;
	/** sprites belong to this atlas */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<ULGUISpriteData*> spriteDataArray;

	void EnsureAtlasTexture(const FName& packingTag);
	void CreateAtlasTexture(const FName& packingTag, int oldTextureSize, int newTextureSize);
	/** create a new texture with size * 2 */
	int32 ExpendTextureSize(const FName& packingTag);
	int32 GetWillExpendTextureSize()const;

	class FLGUIAtlasTextureExpendEvent : public TMulticastDelegate<void(UTexture2D*, int32)>//why not use DECLARE_EVENT here? because DECLARE_EVENT use "friend class XXX", but I need "friend struct"
	{
		friend struct FLGUIAtlasData;
	};
	/** atlas texture size may change when dynamic packing, this event will be called when that happen. */
	FLGUIAtlasTextureExpendEvent OnTextureSizeExpanded;

	bool StaticPacking(const FName& packingTag);
private:
	bool PackAtlasTest(uint32 size, TArray<rbp::Rect>& result, int32 spaceBetweenSprites);
};

UCLASS(NotBlueprintable, NotBlueprintType)
class LGUI_API ULGUIAtlasManager :public UObject
{
	GENERATED_BODY()
public:
	static ULGUIAtlasManager* Instance;
private:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		TMap<FName, FLGUIAtlasData> atlasMap;
protected:
	virtual void BeginDestroy()override;
	bool isStaticAtlasPacked = false;
public:
	static bool InitCheck();
	const TMap<FName, FLGUIAtlasData>& GetAtlasMap() { return atlasMap; }
	static FLGUIAtlasData* FindOrAdd(const FName& packingTag);
	static FLGUIAtlasData* Find(const FName& packingTag);
	static void ResetAtlasMap();

	static void PackStaticAtlas();

	/**
	 * Dispose and release atlas by packingTag.
	 * This will not dispose the LGUISpriteData.
	 * Default "Main" tag is not allowed to be disposed.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (WorldContext = "WorldContextObject"))
		static void DisposeAtlasByPackingTag(FName inPackingTag);

	DECLARE_EVENT(ULGUIAtlasManager, FLGUIAtlasMapChangeEvent);

	FLGUIAtlasMapChangeEvent OnAtlasMapChanged;
};