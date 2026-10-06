// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LTweener.h"
#include "Core/Components/LexImage.h"
#include "LexPolygon.generated.h"


UENUM(BlueprintType, Category = LGUI)
enum class ELexPolygonUVType :uint8
{
	//Use full rect uv
	SpriteRect,
	//Use left center as polygon's center, and right center as polygon's ring uv
	HeightCenter,
	//Use left center as polygon's center, right bottom as polygon ring's start, and right top as polygon ring's end
	StretchSpriteHeight,
};
/**
 * render a solid polygon shape
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULexPolygon : public ULexImage
{
	GENERATED_BODY()

public:	
	ULexPolygon(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool FullCycle = true;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float StartAngle = 0.0f;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (EditCondition = "!FullCycle"))
		float EndAngle = 90.0f;
	//Sides of polygon
	UPROPERTY(EditAnywhere, Category = "LexUI")
		int Sides = 3;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexPolygonUVType UVType = ELexPolygonUVType::SpriteRect;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta=(UIMin="0.0", UIMax="1.0"))
		TArray<float> VertexOffsetArray;
	
	virtual void OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI") bool GetFullCycle()const { return FullCycle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") float GetStartAngle()const { return StartAngle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") float GetEndAngle()const { return EndAngle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") int GetSides()const { return Sides; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") ELexPolygonUVType GetUVType()const { return UVType; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") const TArray<float>& GetVertexOffsetArray()const { return VertexOffsetArray; }
	//Return direct mutable array for edit and change. Call MarkVertexPositionDirty() function after change.
	TArray<float>& GetVertexOffsetArray_Direct() { return VertexOffsetArray; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetFullCycle(bool value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetStartAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetEndAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetSides(int value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetUVType(ELexPolygonUVType value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetVertexOffsetArray(const TArray<float>& value);

	UFUNCTION(BlueprintCallable, Category = "LTweenLexUI")
		ULTweener* StartAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = "LTweenLexUI")
		ULTweener* EndAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
};

