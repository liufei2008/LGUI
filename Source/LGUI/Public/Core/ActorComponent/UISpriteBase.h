// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUISpriteData.h"
#include "UIRenderable.h"
#include "UISpriteBase.generated.h"

//This is base class for create custom mesh based on UISprite. Just override OnCreateGeometry() and OnUpdateGeometry(...) to create or update your own geometry
UCLASS(ClassGroup = (LGUI), Abstract, NotBlueprintable)
class LGUI_API UUISpriteBase : public UUIRenderable
{
	GENERATED_BODY()

public:	
	UUISpriteBase();
	~UUISpriteBase();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void EditorForceUpdateImmediately() override;
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

	UPROPERTY(EditAnywhere, Category = "LGUI")
		ULGUISpriteData* sprite;

	virtual void OnCreateGeometry();
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);

	virtual bool HaveDataToCreateGeometry() { return true; }
private:
	void CreateGeometry();
	virtual void UpdateGeometry(const bool& parentTransformChanged)override;
public:
	void ApplyAtlasTextureScaleUp();

	UFUNCTION(BlueprintCallable, Category = "LGUI") ULGUISpriteData* GetSprite()const { return sprite; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSprite(ULGUISpriteData* newSprite, bool setSize = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromSpriteData();
};
