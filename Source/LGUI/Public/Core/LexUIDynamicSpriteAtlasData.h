// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LexUIDynamicSpriteAtlasData.generated.h"


class ULexUISpriteData;
class ILexUISpriteRenderInterface;

/** Data container for dynamically generated Sprite atlas */
USTRUCT()
struct LGUI_API FLexUIDynamicSpriteAtlasData
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, Category = "LexUI")
	FName PackingTag;
	/** AtlasTexture is the real texture for render */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LexUI")
	TArray<TObjectPtr<UTexture2D>> AtlasTextureArray;
	/** information needed when insert a Sprite */
	TArray<rbp::MaxRectsBinPack> AtlasBinPackArray;
	/** sprites belong to this atlas */
	UPROPERTY(VisibleAnywhere, Category = "LexUI")
	TArray<TObjectPtr<ULexUISpriteData>> SpriteDataArray;
	/** collection of all objects that use this atlas to render. Object must implement IUISpriteRenderableInterface. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LexUI", AdvancedDisplay)
	TArray<TWeakObjectPtr<UObject>> RenderSpriteArray;

	void EnsureAtlasTexture();
	void CreateAtlasTexture(int InTextureSize);
	/** expand texture array */
	void ExpandAtlasAreaArray();
	void CheckSprite();
	int32 GetAtlasTextureSize();

	bool PackSprite(ULexUISpriteData* Sprite);
	void CopySpriteTextureToAtlas(ULexUISpriteData* InSprite, UTexture2D* InAtlasTexture, rbp::Rect InPackedRect, int32 InAtlasTexturePadding);
};

UCLASS(NotBlueprintable, NotBlueprintType)
class LGUI_API ULexUIDynamicSpriteAtlasManager :public UObject
{
	GENERATED_BODY()
public:
	static ULexUIDynamicSpriteAtlasManager* Instance;
private:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LexUI")
		TMap<FName, FLexUIDynamicSpriteAtlasData> AtlasMap;
protected:
	virtual void BeginDestroy()override;
public:
	static bool InitCheck();
	const TMap<FName, FLexUIDynamicSpriteAtlasData>& GetAtlasMap() { return AtlasMap; }
	static FLexUIDynamicSpriteAtlasData* FindOrAdd(const FName& InPackingTag);
	static FLexUIDynamicSpriteAtlasData* Find(const FName& InPackingTag);
	static void ResetAtlasMap();

	/**
	 * Dispose and release atlas by PackingTag.
	 * This will not dispose the LexUISpriteData.
	 * Default "Main" tag is not allowed to be disposed.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		static void DisposeAtlasByPackingTag(FName InPackingTag);

	DECLARE_EVENT(ULexUIDynamicSpriteAtlasManager, FLexUIAtlasMapChangeEvent);

	FLexUIAtlasMapChangeEvent OnAtlasMapChanged;
};