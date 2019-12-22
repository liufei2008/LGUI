// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIRing.generated.h"

UENUM(BlueprintType)
enum class UIRingUVType:uint8
{
	SpriteRect				UMETA(DisplayName = "SpriteRect"),
	WidthCenter				UMETA(DisplayName = "WidthCenter"),
	HeightCenter			UMETA(DisplayName = "HeightCenter"),
	StretchSpriteHeight		UMETA(DisplayName = "StretchSpriteHeight"),
	StretchSpriteWidth		UMETA(DisplayName = "StretchSpriteWidth"),
};

//render a ring shape
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIRing : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UUIRing(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float startAngle = 0.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float endAngle = 90.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 segment = 16;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UIRingUVType uvType = UIRingUVType::SpriteRect;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float ringWidth = 1.0f;

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetStartAngle()const { return startAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	float GetEndAngle()const { return endAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	UIRingUVType GetUVType()const { return uvType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	uint8 GetSegment()const { return segment; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	float GetRingWidth()const { return ringWidth; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStartAngle(float newStartAngle);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEndAngle(float newEndAngle);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUVType(UIRingUVType newUVType);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSegment(uint8 newSegment);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRingWidth(float newRingWidth);
};

UCLASS()
class LGUI_API AUIRingActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUIRingActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIRing; }
	FORCEINLINE UUIRing* GetUIRing()const { return UIRing; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUIRing* UIRing;

};
