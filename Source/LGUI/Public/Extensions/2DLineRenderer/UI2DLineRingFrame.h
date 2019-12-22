// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/Actor/UIBaseActor.h"
#include "UI2DLineRendererBase.h"
#include "LTweenHeaders.h"
#include "UI2DLineRingFrame.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUI2DLineRingFrame : public UUI2DLineRendererBase
{
	GENERATED_BODY()

public:	
	UUI2DLineRingFrame(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay()override;

	//use widget width or height as ring radius
	UPROPERTY(EditAnywhere, Category = LGUI)bool UseWidthOrHeightAsRadius = true;
	UPROPERTY(EditAnywhere, Category = LGUI)float AngleBegin = 0.0f;
	UPROPERTY(EditAnywhere, Category = LGUI)float AngleEnd = 90.0f;
	UPROPERTY(EditAnywhere, Category = LGUI)float FrameWidth = 50.0f;
	//line segment
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = "0", ClampMax = "200"))int Segment = 12;

	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)TArray<FVector2D> CurrentPointArray;

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	void CalculatePoints();

	virtual const TArray<FVector2D>& GetCalcaultedPointArray()override
	{
		return CurrentPointArray;
	}
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)float GetAngleBegin() { return AngleBegin; }
	UFUNCTION(BlueprintCallable, Category = LGUI)float GetAngleEnd() { return AngleEnd; }
	UFUNCTION(BlueprintCallable, Category = LGUI)float GetFrameWidth() { return FrameWidth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)int GetSegment() { return Segment; }

	UFUNCTION(BlueprintCallable, Category = LGUI)void SetAngleBegin(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)void SetAngleEnd(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)void SetFrameWidth(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)void SetSegment(float newValue);

	UFUNCTION(BlueprintCallable, Category = LGUI)ULTweener* AngleBeginTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LGUI)ULTweener* AngleEndTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LGUI)ULTweener* FrameWidthTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
};


UCLASS()
class LGUI_API AUI2DLineRingFrameActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUI2DLineRingFrameActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIElement; }
	FORCEINLINE UUI2DLineRingFrame* Get2DLineRingFrame()const { return UIElement; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUI2DLineRingFrame* UIElement;

};