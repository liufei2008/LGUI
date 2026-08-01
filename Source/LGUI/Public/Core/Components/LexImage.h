// Copyright 2025-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexVisualBatchMesh.h"
#include "Core/ILexUISpriteRenderInterface.h"
#include "Core/LexUIImageBrush.h"
#include "Slate/SlateTextureAtlasInterface.h"
#include "LexImage.generated.h"

/**
 * LexImage is render entry for Sprite & Texture & Material
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, Blueprintable)
class LGUI_API ULexImage : public ULexVisualBatchMesh, public ILexUISpriteRenderInterface
{
	GENERATED_BODY()
public:
	ULexImage(const FObjectInitializer& ObjectInitializer);

	static FName GetPropertyName_Brush()
	{
		return GET_MEMBER_NAME_CHECKED(ULexImage, Brush);
	}
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image", Getter, Setter, meta = (AllowPrivateAccess = true))
	FLexUIImageBrush Brush;

	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry()override;
	virtual void OnUpdateGeometry(FLexUIGeometry& InMesh, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	bool bHasAddToSprite = false;
	void UnregisterFromSprite();
public:
#pragma region ILexUISpriteRenderInterface
	virtual ULexUISpriteData_BaseObject* SpriteRenderGetSprite_Implementation()const override;
	virtual void ApplyAtlasTextureChange_Implementation()override;
#pragma endregion
	UFUNCTION()
	const FLexUIImageBrush& GetBrush()const { return Brush; }

	UFUNCTION()
	void SetBrush(const FLexUIImageBrush& Value);
	//If you keep using LexUISpriteData brush in this LexImage, then this function is better performance than SetBrush
	UFUNCTION(BlueprintCallable, Category = "Image")
	void SetBrush_LexUISprite(ULexUISpriteData_BaseObject* Value);
	//If you keep using SlateTextureAtlas brush in this LexImage, then this function is better performance than SetBrush
	UFUNCTION(BlueprintCallable, Category = "Image")
	void SetBrush_SlateSprite(TScriptInterface<ISlateTextureAtlasInterface> Value);
	UFUNCTION(BlueprintCallable, Category = "Image")
	void SetBrush_Texture(UTexture* Value);
	UFUNCTION(BlueprintCallable, Category = "Image")
	void SetBrush_Material(UTexture* Value);
	
	UFUNCTION(BlueprintCallable, Category = "Image")
	void SetBrushTintColor(FColor Value);

	virtual float GetPreferredWidth() const override;
	virtual float GetPreferredHeight() const override;
};
