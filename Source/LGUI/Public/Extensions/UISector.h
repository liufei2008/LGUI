// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/Actor/UIBaseActor.h"
#include "UISector.generated.h"

UENUM(BlueprintType)
enum class UISectorUVType:uint8
{
	SpriteRect				UMETA(DisplayName = "SpriteRect"),
	WidthCenter				UMETA(DisplayName = "WidthCenter"),
	HeightCenter			UMETA(DisplayName = "HeightCenter"),
	StretchSpriteHeight		UMETA(DisplayName = "StretchSpriteHeight"),
	StretchSpriteWidth		UMETA(DisplayName = "StretchSpriteWidth"),
};

/**
 * render a sector shape
 * @deprecated This class is deprecated, use UIPolygon instead
 */
UCLASS(Deprecated, ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UDEPRECATED_UISector : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UDEPRECATED_UISector(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float startAngle = 0.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float endAngle = 90.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 segment = 16;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UISectorUVType uvType = UISectorUVType::SpriteRect;
	
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetStartAngle()const { return startAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetEndAngle()const { return endAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UISectorUVType GetUVType()const { return uvType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") uint8 GetSegment()const { return segment; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStartAngle(float newStartAngle);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEndAngle(float newEndAngle);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUVType(UISectorUVType newUVType);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSegment(uint8 newSegment);
};

/**
 * render a ring shape
 * @deprecated This class is deprecated, use UIPolygon instead
 */
UCLASS(Deprecated)
class LGUI_API ADEPRECATED_UISectorActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	ADEPRECATED_UISectorActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UISector_DEPRECATED; }
	FORCEINLINE UDEPRECATED_UISector* GetUISector()const { return UISector_DEPRECATED; }
private:
	UPROPERTY(Transient, meta = (DeprecatedProperty, DeprecationMessage = "This class is deprecated, use UIPolygon instead"))
		class UDEPRECATED_UISector* UISector_DEPRECATED;

};
