// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/Actor/UIBaseActor.h"
#include "Extensions/2DLineRenderer/UI2DLineRendererBase.h"
#include "LTweener.h"
#include "UIRing.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIRing : public UUI2DLineRendererBase
{
	GENERATED_BODY()

public:	
	UUIRing(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay()override;

	UPROPERTY(EditAnywhere, Category = LGUI)
		float StartAngle = 0.0f;
	UPROPERTY(EditAnywhere, Category = LGUI)
		float EndAngle = 90.0f;
	//line segment
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = "0", ClampMax = "200"))
		int Segment = 12;

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
	UFUNCTION(BlueprintCallable, Category = LGUI)float GetStartAngle()const { return StartAngle; }
	UFUNCTION(BlueprintCallable, Category = LGUI)float GetEndAngle()const { return EndAngle; }
	UFUNCTION(BlueprintCallable, Category = LGUI)int GetSegment()const { return Segment; }

	UFUNCTION(BlueprintCallable, Category = LGUI)void SetStartAngle(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)void SetEndAngle(float newValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)void SetSegment(int newValue);

	UFUNCTION(BlueprintCallable, Category = "LTweenLGUI")
		ULTweener* StartAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = "LTweenLGUI")
		ULTweener* EndAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
};


UCLASS(ClassGroup = LGUI)
class LGUI_API AUIRingActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIRingActor();

	virtual UUIItem* GetUIItem()const override { return UIElement; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIElement; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIRing* Get2DLineRing()const { return UIElement; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UUIRing> UIElement;

};