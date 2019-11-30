// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UISpriteBase.h"
#include "LTweenHeaders.h"
#include "UI2DLineRendererBase.generated.h"


UENUM(BlueprintType)
enum class LineWidthSideType :uint8
{
	Left, Middle, Right
};
UCLASS(ClassGroup = (LGUI), Abstract, NotBlueprintable)
class LGUI_API UUI2DLineRendererBase : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UUI2DLineRendererBase();

protected:
	virtual void BeginPlay()override;

	UPROPERTY(EditAnywhere, Category = LGUI)float LineWidth = 10.0f;
	UPROPERTY(EditAnywhere, Category = LGUI)bool ConnectStartEndPoint = false;
	UPROPERTY(EditAnywhere, Category = LGUI)LineWidthSideType LineWidthSide = LineWidthSideType::Middle;
	//use grow value or not
	UPROPERTY(EditAnywhere, Category = LGUI)bool UseGrowValue = true;
	//grow the line from start to end
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "UseGrowValue"))float GrowValue = 1.0f;
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (EditCondition = "UseGrowValue"))bool ReverseGrow = false;
	//distance from current point to start point
	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)TArray<float> PointDistanceArray;
	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)float TotalLength;

	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)TArray<FVector2D> PointArrayWithGrowValue;
	virtual const TArray<FVector2D>& GetCalcaultedPointArray();

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual bool HaveDataToCreateGeometry()override;

	bool AngleLargerThanPi(const FVector2D& A, const FVector2D& B)
	{
		float temp = A.X * B.Y - B.X * A.Y;
		return temp < 0;
	}
	void GenerateLinePoint(const FVector2D& InCurrentPoint, const FVector2D& InPrevPoint, const FVector2D& InNextPoint, float InHalfWidth, FVector2D& OutPosA, FVector2D& OutPosB);
	void Generate2DLineGeometry(const TArray<FVector2D>& InPointArray);
	void Update2DLineRendererBaseUV(const TArray<FVector2D>& InPointArray, int InEndPointRepeatCount);
	void Update2DLineRendererBaseVertex(const TArray<FVector2D>& InPointArray, int InEndPointRepeatCount);
	int FindGrowPointIndex(float distance, const TArray<float>& distanceArray, float& interplateValue);
	FORCEINLINE bool CanConnectStartEndPoint(int InPointCount) { return ConnectStartEndPoint && InPointCount >= 3; }
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetLineWidth() { return LineWidth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetConnectStartEndPoint() { return ConnectStartEndPoint; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		LineWidthSideType GetLineWidthSideType() { return LineWidthSide; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetGrowValue() { return GrowValue; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetFlipGrow() { return ReverseGrow; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetTotalLength() { return TotalLength; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetConnectStartEndPoint(bool newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetLineWidth(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetLineWidthSizeType(LineWidthSideType newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetGrowValue(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetFlipGrow(bool newValue);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		class ULTweener* LineWidthTo(float endValue, float duration, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		class ULTweener* GrowValueTo(float endValue, float duration, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
};
