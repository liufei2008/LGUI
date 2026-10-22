// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LTweener.h"
#include "Core/Components/LexImage.h"
#include "Lex2DLineRendererBase.generated.h"


UENUM(BlueprintType, Category = "LexUI")
enum class ELex2DLineRenderer_EndType :uint8
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
class LGUI_API ULex2DLineRendererBase : public ULexImage
{
	GENERATED_BODY()

public:	
	ULex2DLineRendererBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay()override;

	UPROPERTY(EditAnywhere, Category = "LexUI")
		float LineWidth = 10.0f;
	//Draw extra quad at start and end.
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELex2DLineRenderer_EndType EndType = ELex2DLineRenderer_EndType::Cap;
	/** When EndType is Cap, if LineWidth bigger or smaller than Sprite's width, then cap size will scale with it. */
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (EditCondition="EndType==ELex2DLineRenderer_EndType::Cap"))
		bool bEndCapSizeAffectByLineWidth = false;
	//This will slide line's width from left to right.
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (ClampMin = "0", ClampMax = "1"))
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

	//Begin LexVisualBatchMesh interface
	virtual void OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	//End LexVisualBatchMesh interface

	FORCEINLINE bool AngleLargerThanPi(const FVector2D& A, const FVector2D& B)
	{
		float temp = A.X * B.Y - B.X * A.Y;
		return temp < 0;
	}
	void GenerateLinePoint(const FVector2D& InCurrentPoint, const FVector2D& InPrevPoint, const FVector2D& InNextPoint
		, float InLineLeftWidth, float InLineRightWidth
		, FVector2D& OutPosA, FVector2D& OutPosB
		, FVector2D& InOutPrevLineDir);
	FORCEINLINE bool CanConnectStartEndPoint(int InPointCount) { return EndType == ELex2DLineRenderer_EndType::ConnectStartAndEnd && InPointCount >= 3; }
	void Update2DLineRendererBaseTriangle(FLexUIGeometry& InGeo, const TArray<FVector2D>& InPointArray);
	void Update2DLineRendererBaseUV(FLexUIGeometry& InGeo, const TArray<FVector2D>& InPointArray);
	void Update2DLineRendererBaseVertex(FLexUIGeometry& InGeo, const TArray<FVector2D>& InPointArray);
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		float GetLineWidth()const { return LineWidth; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ELex2DLineRenderer_EndType GetEndType()const { return EndType; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		float GetLineWidthOffset()const { return LineWidthOffset; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetEndType(ELex2DLineRenderer_EndType newValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetLineWidth(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetLineWidthOffset(float newValue);

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ULTweener* LineWidthTo(float endValue, float duration, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
};
