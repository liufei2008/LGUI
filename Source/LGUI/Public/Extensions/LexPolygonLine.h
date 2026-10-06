// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Extensions/2DLineRenderer/Lex2DLineRendererBase.h"
#include "LTweener.h"
#include "LexPolygonLine.generated.h"


/**
 * render a polygon line shape
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULexPolygonLine : public ULex2DLineRendererBase
{
	GENERATED_BODY()

public:
	ULexPolygonLine(const FObjectInitializer& ObjectInitializer);

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
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (UIMin = "0.0", UIMax = "1.0"))
		TArray<float> VertexOffsetArray;

	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)TArray<FVector2D> CurrentPointArray;

	//Begin UI2DLineRendererBase interface
	virtual const TArray<FVector2D>& GetCalcaultedPointArray()override
	{
		return CurrentPointArray;
	}
	virtual void CalculatePoints()override;
	virtual bool OverrideStartPointTangentDirection()override { return true; }
	virtual bool OverrideEndPointTangentDirection()override { return true; }
	virtual FVector2D GetStartPointTangentDirection()override;
	virtual FVector2D GetEndPointTangentDirection()override;
	//End UI2DLineRendererBase interface
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI") bool GetFullCycle()const { return FullCycle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") float GetStartAngle()const { return StartAngle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") float GetEndAngle()const { return EndAngle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") int GetSides()const { return Sides; }
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
		void SetVertexOffsetArray(const TArray<float>& value);

	UFUNCTION(BlueprintCallable, Category = "LTweenLexUI")
		ULTweener* StartAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = "LTweenLexUI")
		ULTweener* EndAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
};

