// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Extensions/2DLineRenderer/Lex2DLineRendererBase.h"
#include "LTweener.h"
#include "LexRing.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable)
class LGUI_API ULexRing : public ULex2DLineRendererBase
{
	GENERATED_BODY()

public:	
	ULexRing(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay()override;

	UPROPERTY(EditAnywhere, Category = "LexUI")
		float StartAngle = 0.0f;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float EndAngle = 90.0f;
	//line segment
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (ClampMin = "0", ClampMax = "200"))
		int Segment = 12;

	UPROPERTY(VisibleAnywhere, Transient, Category = "LexUI")TArray<FVector2D> CurrentPointArray;

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
	UFUNCTION(BlueprintCallable, Category = "LexUI")float GetStartAngle()const { return StartAngle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")float GetEndAngle()const { return EndAngle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")int GetSegment()const { return Segment; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")void SetStartAngle(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI")void SetEndAngle(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI")void SetSegment(int newValue);

	UFUNCTION(BlueprintCallable, Category = "LTweenLexUI")
		ULTweener* StartAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = "LTweenLexUI")
		ULTweener* EndAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
};

