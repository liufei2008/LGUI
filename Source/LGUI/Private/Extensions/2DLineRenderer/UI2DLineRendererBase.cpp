// Copyright 2019 LexLiu. All Rights Reserved.

#include "Extensions/2DLineRenderer/UI2DLineRendererBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

DECLARE_CYCLE_STAT(TEXT("UI2DLine Update"), STAT_2DLineUpdate, STATGROUP_LGUI);

UUI2DLineRendererBase::UUI2DLineRendererBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUI2DLineRendererBase::BeginPlay()
{
	Super::BeginPlay();
}

const TArray<FVector2D>& UUI2DLineRendererBase::GetCalcaultedPointArray()
{
	checkf(0, TEXT("[UUI2DLineRendererBase::GetCalcaultedPointArray]This function must be override!"));
	return PointArrayWithGrowValue;//must return anything
}

void UUI2DLineRendererBase::Update2DLineRendererBaseUV(const TArray<FVector2D>& InPointArray, int InEndPointRepeatCount)
{
	auto& uvs = geometry->uvs;
	int pointCount = InPointArray.Num();
	int vertexCount = geometry->originVerticesCount;
	if (uvs.Num() == 0)
	{
		uvs.AddDefaulted(vertexCount);
	}
	auto spriteInfo = sprite->GetSpriteInfo();
	float uvXInterval = (spriteInfo->uv3X - spriteInfo->uv0X) / (pointCount - 1);
	float uvX = spriteInfo->uv0X;
	for (int i = 0; i < vertexCount; i += 2)
	{
		auto& uvi = uvs[i];
		auto& uvi1 = uvs[i + 1];
		uvi.X = uvX;
		uvi.Y = spriteInfo->uv0Y;
		uvi1.X = uvX;
		uvi1.Y = spriteInfo->uv3Y;

		uvX += uvXInterval;
	}
}
void UUI2DLineRendererBase::Update2DLineRendererBaseVertex(const TArray<FVector2D>& InPointArray, int InEndPointRepeatCount)
{
	int pointCount = InPointArray.Num();
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	UIGeometry::CalculatePivotOffset(widget.width, widget.height, widget.pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	auto& vertices = geometry->vertices;
	if (vertices.Num() == 0)
	{
		int vertexCount = (pointCount + InEndPointRepeatCount) * 2;
		geometry->originVerticesCount = vertexCount;
		vertices.AddUninitialized(vertexCount);
	}

	int vertIndex = 0;
	FVector2D pos0, pos1;
	float halfLineWidth = LineWidth * 0.5f;
	if (CanConnectStartEndPoint(pointCount))
	{
		GenerateLinePoint(InPointArray[0], InPointArray[pointCount - 1], InPointArray[1], halfLineWidth, pos0, pos1);
	}
	else
	{
		FVector2D v0 = InPointArray[0];
		FVector2D v0to1 = InPointArray[1] - v0;
		FVector2D dir; float magnitude;
		v0to1.ToDirectionAndLength(dir, magnitude);
		float v0to1LengthReciprocal = magnitude == 0 ? 0 : 1.0f / magnitude;
		auto normal0 = FVector2D(v0to1.Y, -v0to1.X);//equal to rotate 90 degree
		normal0 *= v0to1LengthReciprocal;

		switch (LineWidthSide)
		{
		case LineWidthSideType::Left:
		{
			pos0 = v0;
			pos1 = v0 - LineWidth * normal0;
		}
		break;
		case LineWidthSideType::Middle:
		{
			pos0 = v0 + halfLineWidth * normal0;
			pos1 = v0 - halfLineWidth * normal0;
		}
		break;
		case LineWidthSideType::Right:
		{
			pos0 = v0 + LineWidth * normal0;
			pos1 = v0;
		}
		break;
		}
	}

	vertices[0] = FVector(pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY, 0);
	vertices[1] = FVector(pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY, 0);

	int i = 1;
	if (pointCount >= 3)
	{
		for (; i < pointCount - 1; i++)
		{
			FVector2D posA, posB;
			GenerateLinePoint(InPointArray[i], InPointArray[i - 1], InPointArray[i + 1], halfLineWidth, posA, posB);
			vertices[i + i] = FVector(posA.X + pivotOffsetX, posA.Y + pivotOffsetY, 0);
			vertices[i + i + 1] = FVector(posB.X + pivotOffsetX, posB.Y + pivotOffsetY, 0);
		}
	}

	auto i2 = i + i;
	if (CanConnectStartEndPoint(pointCount))
	{
		FVector2D posA, posB;
		GenerateLinePoint(InPointArray[pointCount - 1], InPointArray[pointCount - 2], InPointArray[0], halfLineWidth, posA, posB);
		vertices[i2] = FVector(posA.X + pivotOffsetX, posA.Y + pivotOffsetY, 0);
		vertices[i2 + 1] = FVector(posB.X + pivotOffsetX, posB.Y + pivotOffsetY, 0);
	}
	else
	{
		FVector2D vEnd2 = InPointArray[pointCount - 2];
		FVector2D vEnd1 = InPointArray[pointCount - 1];
		if (vEnd2 == vEnd1)
		{
			vertices[i2] = vertices[i2 - 2];
			vertices[i2 + 1] = vertices[i2 - 1];
		}
		else
		{
			FVector2D v1to2 = vEnd1 - vEnd2;
			FVector2D dir; float magnitude;
			v1to2.ToDirectionAndLength(dir, magnitude);
			auto normal0 = FVector2D(dir.Y, -dir.X);//equal to rotate 90 degree

			switch (LineWidthSide)
			{
			case LineWidthSideType::Left:
			{
				pos0 = vEnd1;
				pos1 = vEnd1 - LineWidth * normal0;
			}
			break;
			case LineWidthSideType::Middle:
			{
				pos0 = vEnd1 + halfLineWidth * normal0;
				pos1 = vEnd1 - halfLineWidth * normal0;
			}
			break;
			case LineWidthSideType::Right:
			{
				pos0 = vEnd1 + LineWidth * normal0;
				pos1 = vEnd1;
			}
			break;
			}

			vertices[i2] = FVector(pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY, 0);
			vertices[i2 + 1] = FVector(pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY, 0);
		}
	}

	//fill end point
	auto endPoint1 = vertices[i2];
	auto endPoint2 = vertices[i2 + 1];
	for (int j = pointCount * 2, count = (pointCount + InEndPointRepeatCount) * 2; j < count; j += 2)
	{
		vertices[j] = endPoint1;
		vertices[j + 1] = endPoint2;
	}
}
void UUI2DLineRendererBase::Generate2DLineGeometry(const TArray<FVector2D>& InPointArray)
{
	int pointCount = InPointArray.Num();
	if (pointCount < 2)
	{
		geometry->Clear();
		return;
	}
	if (pointCount == 2)
	{
		if (InPointArray[0] == InPointArray[1])
		{
			geometry->Clear();
			return;
		}
	}
	
	//triangles
	auto& triangles = geometry->triangles;
	if (triangles.Num() == 0)
	{
		int triangleIndicesCount = (pointCount - 1) * 2 * 3;
		if (CanConnectStartEndPoint(pointCount))
		{
			triangleIndicesCount += 6;
		}
		geometry->originTriangleCount = triangleIndicesCount;
		triangles.AddDefaulted(triangleIndicesCount);
		int i = 0;
		for (int count = pointCount - 1; i < count; i++)
		{
			int j = i * 2;
			int k = i * 6;
			triangles[k] = j;
			triangles[k + 1] = j + 2;
			triangles[k + 2] = j + 3;

			triangles[k + 3] = j;
			triangles[k + 4] = j + 3;
			triangles[k + 5] = j + 1;
		}
		if (CanConnectStartEndPoint(pointCount))
		{
			int j = i * 2;
			int k = i * 6;
			triangles[k] = j;
			triangles[k + 1] = 0;
			triangles[k + 2] = 1;

			triangles[k + 3] = j;
			triangles[k + 4] = 1;
			triangles[k + 5] = j + 1;
		}
	}

	if (UseGrowValue)
	{
		//calculate grow value
		PointDistanceArray.Reset(pointCount);
		TotalLength = 0.0f;
		PointDistanceArray.Add(0);
		if (ReverseGrow)
		{
			FVector2D prevPoint = InPointArray[pointCount - 1];
			for (int i = pointCount - 2; i >= 0; i--)
			{
				auto distance = FVector2D::Distance(prevPoint, InPointArray[i]);
				TotalLength += distance;
				PointDistanceArray.Add(TotalLength);
				prevPoint = InPointArray[i];
			}
		}
		else
		{
			FVector2D prevPoint = InPointArray[0];
			for (int i = 1; i < pointCount; i++)
			{
				auto distance = FVector2D::Distance(prevPoint, InPointArray[i]);
				TotalLength += distance;
				PointDistanceArray.Add(TotalLength);
				prevPoint = InPointArray[i];
			}
		}

		auto distance = TotalLength * GrowValue;
		auto interplateValue = 0.0f;
		auto pointIndex = FindGrowPointIndex(distance, PointDistanceArray, interplateValue);
		PointArrayWithGrowValue.Reset(InPointArray.Num());
		FVector2D endPoint;
		if (ReverseGrow)
		{
			for (int i = 0; i <= pointIndex; i++)
			{
				PointArrayWithGrowValue.Add(InPointArray[pointCount - 1 - i]);
			}
			endPoint = FMath::Lerp(InPointArray[pointCount - 1 - pointIndex], InPointArray[pointCount - 1 - (pointIndex + 1)], interplateValue);
		}
		else
		{
			for (int i = 0; i <= pointIndex; i++)
			{
				PointArrayWithGrowValue.Add(InPointArray[i]);
			}
			endPoint = FMath::Lerp(InPointArray[pointIndex], InPointArray[pointIndex + 1], interplateValue);
		}
		PointArrayWithGrowValue.Add(endPoint);
		//vertices
		Update2DLineRendererBaseVertex(PointArrayWithGrowValue, pointCount - pointIndex - 1);
		//uvs
		Update2DLineRendererBaseUV(PointArrayWithGrowValue, pointCount - pointIndex - 1);
	}
	else
	{
		//vertices
		Update2DLineRendererBaseVertex(PointArrayWithGrowValue, 0);
		//uvs
		Update2DLineRendererBaseUV(PointArrayWithGrowValue, 0);
	}
	//colors
	UIGeometry::UpdateUIColor(geometry, GetFinalColor());

	//not set anything for uv1
	int vertexCount = geometry->vertices.Num();
	//normals
	if (RenderCanvas->GetRequireNormal())
	{
		auto& normals = geometry->normals;
		if (normals.Num() == 0)
		{
			normals.AddDefaulted(vertexCount);
		}
	}
	//tangents
	if (RenderCanvas->GetRequireTangent())
	{
		auto& tangents = geometry->tangents;
		if (tangents.Num() == 0)
		{
			tangents.AddDefaulted(vertexCount);
		}
	}
	//uvs1
	if (RenderCanvas->GetRequireUV1())
	{
		auto& uvs1 = geometry->uvs1;
		if (uvs1.Num() == 0)
		{
			uvs1.AddDefaulted(vertexCount);
		}
	}
	//uvs2
	if (RenderCanvas->GetRequireUV2())
	{
		auto& uvs2 = geometry->uvs2;
		if (uvs2.Num() == 0)
		{
			uvs2.AddDefaulted(vertexCount);
		}
	}
	//uvs3
	if (RenderCanvas->GetRequireUV3())
	{
		auto& uvs3 = geometry->uvs3;
		if (uvs3.Num() == 0)
		{
			uvs3.AddDefaulted(vertexCount);
		}
	}
}

bool AngleLargeThanPi(const FVector2D& A, const FVector2D& B)
{
	float temp = A.X * B.Y - B.X * A.Y;
	return temp < 0;
}
void UUI2DLineRendererBase::GenerateLinePoint(const FVector2D& InCurrentPoint, const FVector2D& InPrevPoint, const FVector2D& InNextPoint, float InHalfWidth, FVector2D& OutPosA, FVector2D& OutPosB)
{
	if (InCurrentPoint == InPrevPoint || InCurrentPoint == InNextPoint)
	{
		OutPosA = InCurrentPoint;
		OutPosB = InCurrentPoint;
		return;
	}
	FVector2D normalizedV1 = (InPrevPoint - InCurrentPoint).GetSafeNormal();
	FVector2D normalizedV2 = (InNextPoint - InCurrentPoint).GetSafeNormal();
	if (normalizedV1 == -normalizedV2)
	{
		auto itemNormal = FVector2D(normalizedV2.Y, -normalizedV2.X);
		switch (LineWidthSide)
		{
		case LineWidthSideType::Left:
		{
			OutPosA = InCurrentPoint;
			OutPosB = InCurrentPoint - LineWidth * itemNormal;
		}
		break;
		case LineWidthSideType::Middle:
		{
			OutPosA = InCurrentPoint + InHalfWidth * itemNormal;
			OutPosB = InCurrentPoint - InHalfWidth * itemNormal;
		}
		break;
		case LineWidthSideType::Right:
		{
			OutPosA = InCurrentPoint + LineWidth * itemNormal;
			OutPosB = InCurrentPoint;
		}
		break;
		}
		
	}
	else
	{
		auto itemNormal = normalizedV1 + normalizedV2;
		itemNormal.Normalize();
		if (itemNormal.X == 0 && itemNormal.Y == 0)//wrong normal
		{
			itemNormal = FVector2D(normalizedV2.Y, -normalizedV2.X);
		}
		float prevDotN = FVector2D::DotProduct(normalizedV1, itemNormal);
		float angle = FMath::Acos(prevDotN);
		float sin = FMath::Sin(angle);
		itemNormal = AngleLargeThanPi(normalizedV1, normalizedV2) ? -itemNormal : itemNormal;
		switch (LineWidthSide)
		{
		case LineWidthSideType::Left:
		{
			OutPosA = InCurrentPoint;
			OutPosB = InCurrentPoint - LineWidth / sin * itemNormal;
		}
		break;
		case LineWidthSideType::Middle:
		{
			OutPosA = InCurrentPoint + InHalfWidth / sin * itemNormal;
			OutPosB = InCurrentPoint - InHalfWidth / sin * itemNormal;
		}
		break;
		case LineWidthSideType::Right:
		{
			OutPosA = InCurrentPoint + LineWidth / sin * itemNormal;
			OutPosB = InCurrentPoint;
		}
		break;
		}
	}
}
int UUI2DLineRendererBase::FindGrowPointIndex(float distance, const TArray<float>& distanceArray, float& interplateValue)
{
	int result = 0;
	for (int i = 0, count = distanceArray.Num(); i < count; i++)
	{
		auto listItem = distanceArray[i];
		if (distance > listItem)
		{
			result = i;
		}
		else
		{
			break;
		}
	}
	interplateValue = (distance - distanceArray[result]) / (distanceArray[result + 1] - distanceArray[result]);
	return result;
}





void UUI2DLineRendererBase::OnCreateGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_2DLineUpdate);
	Generate2DLineGeometry(GetCalcaultedPointArray());
}
void UUI2DLineRendererBase::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_2DLineUpdate);
	auto& CurrentPointArray = GetCalcaultedPointArray();
	int pointCount = CurrentPointArray.Num();
	if (UseGrowValue)
	{
		//calculate grow value
		auto distance = TotalLength * GrowValue;
		auto interplateValue = 0.0f;
		auto pointIndex = FindGrowPointIndex(distance, PointDistanceArray, interplateValue);
		PointArrayWithGrowValue.Reset(CurrentPointArray.Num());
		FVector2D endPoint;
		if (ReverseGrow)
		{
			for (int i = 0; i <= pointIndex; i++)
			{
				PointArrayWithGrowValue.Add(CurrentPointArray[pointCount - 1 - i]);
			}
			endPoint = FMath::Lerp(CurrentPointArray[pointCount - 1 - pointIndex], CurrentPointArray[pointCount - 1 - (pointIndex + 1)], interplateValue);
		}
		else
		{
			for (int i = 0; i <= pointIndex; i++)
			{
				PointArrayWithGrowValue.Add(CurrentPointArray[i]);
			}
			endPoint = FMath::Lerp(CurrentPointArray[pointIndex], CurrentPointArray[pointIndex + 1], interplateValue);
		}
		PointArrayWithGrowValue.Add(endPoint);

		if (InVertexPositionChanged)
		{
			Update2DLineRendererBaseVertex(PointArrayWithGrowValue, pointCount - pointIndex - 1);
		}
		if (InVertexColorChanged)
		{
			UIGeometry::UpdateUIColor(geometry, GetFinalColor());
		}
		if (InVertexUVChanged)
		{
			Update2DLineRendererBaseUV(PointArrayWithGrowValue, pointCount - pointIndex - 1);
		}
	}
	else
	{
		if (InVertexPositionChanged)
		{
			Update2DLineRendererBaseVertex(CurrentPointArray, 0);
		}
		if (InVertexColorChanged)
		{
			UIGeometry::UpdateUIColor(geometry, GetFinalColor());
		}
		if (InVertexUVChanged)
		{
			Update2DLineRendererBaseUV(CurrentPointArray, 0);
		}
	}
}

bool UUI2DLineRendererBase::HaveDataToCreateGeometry()
{
	return GetCalcaultedPointArray().Num() > 0;
}

void UUI2DLineRendererBase::SetConnectStartEndPoint(bool newValue)
{
	if (ConnectStartEndPoint != newValue)
	{
		ConnectStartEndPoint = newValue;
		MarkTriangleDirty();
	}
}
void UUI2DLineRendererBase::SetLineWidth(float newValue)
{
	if (LineWidth != newValue)
	{
		LineWidth = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRendererBase::SetLineWidthSizeType(LineWidthSideType newValue)
{
	if (LineWidthSide != newValue)
	{
		LineWidthSide = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRendererBase::SetGrowValue(float newValue)
{
	if (GrowValue != newValue)
	{
		GrowValue = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRendererBase::SetFlipGrow(bool newValue)
{
	if (ReverseGrow != newValue)
	{
		ReverseGrow = newValue;
		MarkVertexPositionDirty();
	}
}

ULTweener* UUI2DLineRendererBase::LineWidthTo(float endValue, float duration, float delay, LTweenEase easeType)
{
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRendererBase::GetLineWidth), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRendererBase::SetLineWidth), endValue, duration)
		->SetDelay(delay)->SetEase(easeType);
}
ULTweener* UUI2DLineRendererBase::GrowValueTo(float endValue, float duration, float delay, LTweenEase easeType)
{
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRendererBase::GetGrowValue), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRendererBase::SetGrowValue), endValue, duration)
		->SetDelay(delay)->SetEase(easeType);
}