// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LGUIDynamicSpriteAtlasData.generated.h"


class ULGUISpriteData;
class IUISpriteRenderableInterface;

/** Data container for dyanmically generated sprite atlas */
USTRUCT()
struct LGUI_API FLGUIDynamicSpriteAtlasData
{
	GENERATED_BODY()
	/** atlasTexture is the real texture for render */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
	UTexture2D* atlasTexture = nullptr;
	/** information needed when insert a sprite */
	rbp::MaxRectsBinPack atlasBinPack;
	/** sprites belong to this atlas */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<ULGUISpriteData*> spriteDataArray;
	/** collection of all UISprite whitch use this atlas to render */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", AdvancedDisplay)
	TArray<TScriptInterface<IUISpriteRenderableInterface>> renderSpriteArray;

	void EnsureAtlasTexture(const FName& packingTag);
	void CreateAtlasTexture(const FName& packingTag, int oldTextureSize, int newTextureSize);
	/** create a new texture with size * 2 */
	int32 ExpendTextureSize(const FName& packingTag);
	int32 GetWillExpendTextureSize()const;
	void CheckSprite(const FName& packingTag);

	class FLGUIAtlasTextureExpandEvent : public TMulticastDelegate<void(UTexture2D*, int32)>//why not use DECLARE_EVENT here? because DECLARE_EVENT use "friend class XXX", but I need "friend struct"
	{
		friend struct FLGUIDynamicSpriteAtlasData;
	};
	/** atlas texture size may change when dynamic packing, this event will be called when that happen. */
	FLGUIAtlasTextureExpandEvent OnTextureSizeExpanded;
};

UCLASS(NotBlueprintable, NotBlueprintType)
class LGUI_API ULGUIDynamicSpriteAtlasManager :public UObject
{
	GENERATED_BODY()
public:
	static ULGUIDynamicSpriteAtlasManager* Instance;
private:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		TMap<FName, FLGUIDynamicSpriteAtlasData> atlasMap;
protected:
	virtual void BeginDestroy()override;
public:
	static bool InitCheck();
	const TMap<FName, FLGUIDynamicSpriteAtlasData>& GetAtlasMap() { return atlasMap; }
	static FLGUIDynamicSpriteAtlasData* FindOrAdd(const FName& packingTag);
	static FLGUIDynamicSpriteAtlasData* Find(const FName& packingTag);
	static void ResetAtlasMap();

	/**
	 * Dispose and release atlas by packingTag.
	 * This will not dispose the LGUISpriteData.
	 * Default "Main" tag is not allowed to be disposed.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (WorldContext = "WorldContextObject"))
		static void DisposeAtlasByPackingTag(FName inPackingTag);

	DECLARE_EVENT(ULGUIDynamicSpriteAtlasManager, FLGUIAtlasMapChangeEvent);

	FLGUIAtlasMapChangeEvent OnAtlasMapChanged;
};