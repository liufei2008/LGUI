// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIBatchMeshRenderable.h"
#include "Curves/CurveFloat.h"
#include "LGUICustomMesh.generated.h"

class UIGeometry;
class ULGUICanvas;
class UUIBatchMeshRenderable;

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUICustomMesh : public ULGUIGeometryHelper
{
	GENERATED_BODY()
public:
	/**
	 * Fill the mesh data.
	 * @param InRenderable The UI element which will use this mesh.
	 * @param InTriangleChanged Normally just ignore this.
	 * @param InVertexPositionChanged Normally just ignore this.
	 * @param InVertexUVChanged Normally just ignore this.
	 * @param InVertexColorChanged Normally just ignore this.
	 */
	virtual void OnFillMesh(UUIBatchMeshRenderable* InRenderable, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
	/**
	 * Get uv value on raycast hit point
	 * @param InRenderable The UI element which will use this mesh.
	 * @param InHitFaceIndex Hit point face id on the this mesh.
	 * @param InHitPoint Hit point.
	 * @param InLineStart Normally just use InHitFaceIndex and InHitPoint can get the right result, but you can use your own method to do line cast with InLineStart and InLineEnd parameters.
	 * @param InLineEnd See InLineStart.
	 * @param OutHitUV result uv
	 * @return true if hit suceess
	 */
	virtual bool GetHitUV(const UUIBatchMeshRenderable* InRenderable, const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)const;
protected:
	/**
	 * Fill the mesh data.
	 * @param InRenderable The UI element which will use this mesh.
	 * @param InTriangleChanged Normally just ignore this.
	 * @param InVertexPositionChanged Normally just ignore this.
	 * @param InVertexUVChanged Normally just ignore this.
	 * @param InVertexColorChanged Normally just ignore this.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnCreateMesh", AdvancedDisplay = 1))
	void ReceiveOnFillMesh(UUIBatchMeshRenderable* InRenderable, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
	/**
	 * Get uv value on raycast hit point
	 * @param InRenderable The UI element which will use this mesh.
	 * @param InHitFaceIndex Hit point face id on the this mesh.
	 * @param InHitPoint Hit point.
	 * @param InLineStart Normally just use InHitFaceIndex and InHitPoint can get the right result, but you can use your own method to do line cast with InLineStart and InLineEnd parameters.
	 * @param InLineEnd See InLineStart.
	 * @param OutHitUV result uv
	 * @return true if hit suceess
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "GetHitUV"))
	bool ReceiveGetHitUV(const UUIBatchMeshRenderable* InRenderable, const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)const;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	bool GetHitUVbyFaceIndex(const UUIBatchMeshRenderable* InRenderable, const int32& InHitFaceIndex, const FVector& InHitPoint, FVector2D& OutHitUV)const;
};

UCLASS(ClassGroup = LGUI, DisplayName="LGUICustomMesh Cylinder")
class LGUI_API ULGUICustomMesh_Cylinder : public ULGUICustomMesh
{
	GENERATED_BODY()
public:
	virtual void OnFillMesh(UUIBatchMeshRenderable* InRenderable, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual bool GetHitUV(const UUIBatchMeshRenderable* InRenderable, const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV) const override;
protected:
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = -180.0f, ClampMax = 180.0f))
	float CylinderArcAngle = 45;
};

UCLASS(ClassGroup = LGUI, DisplayName = "LGUICustomMesh CurvyPlane")
class LGUI_API ULGUICustomMesh_CurvyPlane : public ULGUICustomMesh
{
	GENERATED_BODY()
public:
	ULGUICustomMesh_CurvyPlane();
	virtual void OnFillMesh(UUIBatchMeshRenderable* InRenderable, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual bool GetHitUV(const UUIBatchMeshRenderable* InRenderable, const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV) const override;
protected:
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = 1, ClampMax = 200))
	int Segment = 10;
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (
		DisplayName = "Shape Curve",
		XAxisName = "Time 0-1",
		YAxisName = "Value 0-1"))
	FRuntimeFloatCurve ShapeCurve;
	UPROPERTY(EditAnywhere, Category = LGUI)
	float CurveScale = 100;
};
