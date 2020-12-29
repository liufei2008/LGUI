﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UISpriteBase.h"
#include "LTweenHeaders.h"
#include "UI2DLineRendererBase.generated.h"


UENUM(BlueprintType)
enum class EUI2DLineRenderer_EndType :uint8
{
	None,
	//Draw a cap at start and end.
	Cap,
	//Connect start and end with a line
	ConnectStartAndEnd,
};
/**
 * Render line use given points.
 */
UCLASS(ClassGroup = (LGUI), Abstract, NotBlueprintable)
class LGUI_API UUI2DLineRendererBase : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UUI2DLineRendererBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay()override;

	UPROPERTY(EditAnywhere, Category = LGUI)
		float LineWidth = 10.0f;
	//Draw extra quad at start and end.
	UPROPERTY(EditAnywhere, Category = LGUI)
		EUI2DLineRenderer_EndType EndType = EUI2DLineRenderer_EndType::Cap;
	//This will slide line's width from left to right.
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = "0", ClampMax = "1"))
		float LineWidthOffset = 0.5f;

	static TArray<FVector2D> EmptyArray;
	virtual const TArray<FVector2D>& GetCalcaultedPointArray()PURE_VIRTUAL(UUI2DLineRendererBase::GetCalcaultedPointArray, return EmptyArray;)
	virtual void CalculatePoints()PURE_VIRTUAL(UUI2DLineRendererBase::CalculatePoints, );
	//override start point tangent direction when EndType == Cap
	virtual bool OverrideStartPointTangentDirection() { return false; }
	//override end point tangent direction when EndType == Cap
	virtual bool OverrideEndPointTangentDirection() { return false; }
	//if OverrideStartPointTangentDirection return true, then this function must be implemented
	virtual FVector2D GetStartPointTangentDirection();
	//if OverrideEndPointTangentDirection return true, then this function must be implemented
	virtual FVector2D GetEndPointTangentDirection();

	//Begin UIRenderable interface
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual bool HaveDataToCreateGeometry()override;
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	//End UIRenderable interface

	FORCEINLINE bool AngleLargerThanPi(const FVector2D& A, const FVector2D& B)
	{
		float temp = A.X * B.Y - B.X * A.Y;
		return temp < 0;
	}
	void GenerateLinePoint(const FVector2D& InCurrentPoint, const FVector2D& InPrevPoint, const FVector2D& InNextPoint
		, float InLineLeftWidth, float InLineRightWidth
		, FVector2D& OutPosA, FVector2D& OutPosB
		, FVector2D& InOutPrevLineDir);
	void Generate2DLineGeometry(const TArray<FVector2D>& InPointArray);
	FORCEINLINE bool CanConnectStartEndPoint(int InPointCount) { return EndType == EUI2DLineRenderer_EndType::ConnectStartAndEnd && InPointCount >= 3; }
	void Update2DLineRendererBaseUV(const TArray<FVector2D>& InPointArray);
	void Update2DLineRendererBaseVertex(const TArray<FVector2D>& InPointArray);

	//for start point when grow value == 0, that means not possible to calculate direction because two points are too close.
	FVector2D CacheStartPointDirection = FVector2D(1, 0);
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetLineWidth() { return LineWidth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		EUI2DLineRenderer_EndType GetEndType() { return EndType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetLineWidthOffset() { return LineWidthOffset; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetEndType(EUI2DLineRenderer_EndType newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetLineWidth(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetLineWidthOffset(float newValue);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		class ULTweener* LineWidthTo(float endValue, float duration, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
};
