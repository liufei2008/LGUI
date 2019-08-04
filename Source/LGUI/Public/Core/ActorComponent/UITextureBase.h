// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIRenderable.h"
#include "UITextureBase.generated.h"

//This is base class for create custom mesh based on UITexture. Just override OnCreateGeometry() and OnUpdateGeometry(...) to create or update your own geometry
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUITextureBase : public UUIRenderable
{
	GENERATED_BODY()

public:	
	UUITextureBase();

#if WITH_EDITOR
	virtual void EditorForceUpdateImmediately() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void CheckTexture();
#endif
protected:
	virtual void BeginPlay()override;
	friend class FUITextureBaseCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UTexture* texture = nullptr;

	virtual void OnCreateGeometry();
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
private:
	virtual void CreateGeometry();
	virtual void UpdateGeometry(const bool& parentTransformChanged) override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") UTexture* GetTexture()const { return texture; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual void SetTexture(UTexture* newTexture);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromTexture();
};
