// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBatchGeometryRenderable.h"
#include "Core/IUISpriteRenderableInterface.h"
#include "UISpriteBase.generated.h"

class ULGUISpriteData_BaseObject;

/**
 * This is base class for create custom mesh based on UISprite.
 */
UCLASS(ClassGroup = (LGUI), Abstract, NotBlueprintable)
class LGUI_API UUISpriteBase : public UUIBatchGeometryRenderable
	, public IUISpriteRenderableInterface
{
	GENERATED_BODY()

public:	
	UUISpriteBase(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
protected:
	virtual void OnPreChangeSpriteProperty();
	virtual void OnPostChangeSpriteProperty();
#endif
public:
	void CheckSpriteData();
	static const FName GetSpritePropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(UUISpriteBase, sprite);
	}
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
protected:
	friend class SLGUISpriteBorderEditor;
	friend class SLGUISpriteSelector;
	friend class FUISpriteBaseCustomization;

	/** sprite may override by UISelectable(UIButton, UIToggle, UISlider ...) */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		TObjectPtr<ULGUISpriteData_BaseObject> sprite = nullptr;

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;

	virtual bool ReadPixelFromMainTexture(const FVector2D& InUV, FColor& OutPixel)const override;

	bool bHasAddToSprite = false;

public:

	UFUNCTION(BlueprintCallable, Category = "LGUI") ULGUISpriteData_BaseObject* GetSprite()const { return sprite; }
#pragma region UISpriteRenderableInterface
	virtual ULGUISpriteData_BaseObject* SpriteRenderableGetSprite_Implementation()const override{ return sprite; }
	virtual void ApplyAtlasTextureScaleUp_Implementation()override;
	virtual void ApplyAtlasTextureChange_Implementation()override;
#pragma endregion

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSprite(ULGUISpriteData_BaseObject* newSprite, bool setSize = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromSpriteData();
};
