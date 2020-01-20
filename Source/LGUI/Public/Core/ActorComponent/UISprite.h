// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UISpriteBase.h"
#include "UISprite.generated.h"

UENUM(BlueprintType)
enum class UISpriteType :uint8
{
	Normal		 		UMETA(DisplayName = "Normal"),
	Sliced		 		UMETA(DisplayName = "Sliced"),
	SlicedFrame			UMETA(DisplayName = "SlicedFrame"),
	Tiled				UMETA(DisplayName = "Tiled"),
};
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUISprite : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UUISprite(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void EditorForceUpdateImmediately() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	friend class FUISpriteCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UISpriteType type = UISpriteType::Normal;

	virtual void WidthChanged()override;
	virtual void HeightChanged()override;

	//width direction rectangel count, in tiled mode
	int32 Tiled_WidthRectCount = 0;
	//height direction rectangel count, in tiled mode
	int32 Tiled_HeightRectCount = 0;
	//width direction half rectangel size, in tiled mode
	float Tiled_WidthRemainedRectSize = 0;
	//height direction half rectangel size, in tiled mode
	float Tiled_HeightRemainedRectSize = 0;

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") UISpriteType GetSpriteType()const { return type; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpriteType(UISpriteType newType);

};
