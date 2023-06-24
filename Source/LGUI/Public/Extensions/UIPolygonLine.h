// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/Actor/UIBaseActor.h"
#include "Extensions/2DLineRenderer/UI2DLineRendererBase.h"
#include "LTweener.h"
#include "UIPolygonLine.generated.h"


/**
 * render a polygon line shape
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIPolygonLine : public UUI2DLineRendererBase
{
	GENERATED_BODY()

public:
	UUIPolygonLine(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool FullCycle = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float StartAngle = 0.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "!FullCycle"))
		float EndAngle = 90.0f;
	//Sides of polygon
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int Sides = 3;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0", UIMax = "1.0"))
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
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetStartAngle()const { return StartAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetEndAngle()const { return EndAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") int GetSides()const { return Sides; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") const TArray<float>& GetVertexOffsetArray()const { return VertexOffsetArray; }
	//Return direct mutable array for edit and change. Call MarkVertexPositionDirty() function after change.
	TArray<float>& GetVertexOffsetArray_Direct() { return VertexOffsetArray; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStartAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEndAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSides(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetVertexOffsetArray(const TArray<float>& value);

	UFUNCTION(BlueprintCallable, Category = "LTweenLGUI")
		ULTweener* StartAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = "LTweenLGUI")
		ULTweener* EndAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase easeType = LTweenEase::OutCubic);
};

/**
 * render a polygon line shape
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API AUIPolygonLineActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIPolygonLineActor();

	virtual UUIItem* GetUIItem()const override { return UIPolygonLine; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIPolygonLine; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIPolygonLine* GetUIPolygonLine()const { return UIPolygonLine; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UUIPolygonLine> UIPolygonLine;

};
