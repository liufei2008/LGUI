// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUISpriteData.h"
#include "UITextureBase.h"
#include "UITexture.generated.h"

UENUM(BlueprintType)
enum class UITextureType :uint8
{
	Normal		 		UMETA(DisplayName = "Normal"),
	Sliced		 		UMETA(DisplayName = "Sliced"),
	SlicedFrame			UMETA(DisplayName = "SlicedFrame"),
	Tiled				UMETA(DisplayName = "Tiled"),
};
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUITexture : public UUITextureBase
{
	GENERATED_BODY()

public:	
	UUITexture();

#if WITH_EDITOR
	virtual void EditorForceUpdateImmediately() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void BeginPlay()override;
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UITextureType type = UITextureType::Normal;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUISpriteInfo spriteData;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector4 uvRect = FVector4(0, 0, 1, 1);

	void CheckSpriteData();

	virtual void WidthChanged()override;
	virtual void HeightChanged()override;

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") UITextureType GetTextureType()const { return type; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FLGUISpriteInfo GetSpriteData()const { return spriteData; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector4 GetUVRect()const { return uvRect; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetTextureType(UITextureType newType);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpriteData(FLGUISpriteInfo newSpriteData);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUVRect(FVector4 newUVRect);

	virtual void SetTexture(UTexture* newTexture)override;
};
