// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualBatchMesh.h"
#include "Core/ILexUISpriteRenderInterface.h"
#include "LexSpriteBase.generated.h"

class ULexUISpriteData_BaseObject;

/**
 * This is base class for create custom mesh based on UISprite.
 */
UCLASS(ClassGroup = (LGUI), Abstract, NotBlueprintable)
class LGUI_API ULexSpriteBase : public ULexVisualBatchMesh
	, public ILexUISpriteRenderInterface
{
	GENERATED_BODY()

public:	
	ULexSpriteBase(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
protected:
	virtual void OnPreChangeSpriteProperty();
	virtual void OnPostChangeSpriteProperty();
#endif
public:
	void CheckSpriteData();
	static FName GetPropertyName_Sprite()
	{
		return GET_MEMBER_NAME_CHECKED(ULexSpriteBase, Sprite);
	}
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay()override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
	virtual void BeginDestroy() override;
protected:
	friend class SLexUISpriteBorderEditor;
	friend class FLexSpriteBaseCustomization;
	
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (DisplayThumbnail = "true"))
	TObjectPtr<ULexUISpriteData_BaseObject> Sprite = nullptr;
	/** Use a custom material to render this sprite */
	UPROPERTY(EditAnywhere, Category = "LexUI")
	UMaterialInterface* OverrideMaterial = nullptr;

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry() override;

	virtual bool ReadPixelFromMainTexture(const FVector2D& InUV, FColor& OutPixel)const override;

	bool bHasAddToSprite = false;

public:

	UFUNCTION(BlueprintCallable, Category = "LGUI") ULexUISpriteData_BaseObject* GetSprite()const { return Sprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UMaterialInterface* GetOverrideMaterial()const{return OverrideMaterial;}
#pragma region LexUISpriteRenderInterface
	virtual ULexUISpriteData_BaseObject* SpriteRenderGetSprite_Implementation()const override{ return Sprite; }
	virtual void ApplyAtlasTextureChange_Implementation()override;
#pragma endregion

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSprite(ULexUISpriteData_BaseObject* Value, bool bSetSize = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromSpriteData();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetOverrideMaterial(UMaterialInterface* Value);
};
