// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBatchGeometryRenderable.h"
#include "UISpriteBase.generated.h"

class ULGUISpriteData_BaseObject;

/**
 * This is base class for create custom mesh based on UISprite.
 */
UCLASS(ClassGroup = (LGUI), Abstract, NotBlueprintable)
class LGUI_API UUISpriteBase : public UUIBatchGeometryRenderable
{
	GENERATED_BODY()

public:	
	UUISpriteBase(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void CheckSpriteData();
protected:
	virtual void OnPreChangeSpriteProperty();
	virtual void OnPostChangeSpriteProperty();
#endif
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
		ULGUISpriteData_BaseObject* sprite;

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;

	bool bHasAddToSprite = false;
public:
	void ApplyAtlasTextureScaleUp();

	UFUNCTION(BlueprintCallable, Category = "LGUI") ULGUISpriteData_BaseObject* GetSprite()const { return sprite; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSprite(ULGUISpriteData_BaseObject* newSprite, bool setSize = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromSpriteData();
};
