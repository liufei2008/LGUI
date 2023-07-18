// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/2DLineRenderer/UI2DLineRendererBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "LTweenManager.h"

DECLARE_CYCLE_STAT(TEXT("UI2DLine Update"), STAT_2DLineUpdate, STATGROUP_LGUI);

TArray<FVector2D> UUI2DLineRendererBase::EmptyArray;
UUI2DLineRendererBase::UUI2DLineRendererBase(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUI2DLineRendererBase::BeginPlay()
{
	Super::BeginPlay();
}

void UUI2DLineRendererBase::Update2DLineRendererBaseUV(UIGeometry& InGeo, const TArray<FVector2D>& InPointArray)
{
	auto& vertices = InGeo.vertices;
	int pointCount = InPointArray.Num();

	const auto& spriteInfo = sprite->GetSpriteInfo();
	float uvY = (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f;
	int i = 0; 
	for (; i < pointCount; i++)
	{
		auto& uvi0 = vertices[i + i].TextureCoordinate[0];
		auto& uvi1 = vertices[i + i + 1].TextureCoordinate[0];
		uvi0.X = spriteInfo.uv0X;
		uvi1.X = spriteInfo.uv3X;
		uvi0.Y = uvY;
		uvi1.Y = uvY;
	}
	if (EndType == EUI2DLineRenderer_EndType::Cap)
	{
		//start point cap
		{
			auto& uvi0 = vertices[i + i + 2].TextureCoordinate[0];
			auto& uvi1 = vertices[i + i + 3].TextureCoordinate[0];
			uvi0.X = spriteInfo.uv0X;
			uvi1.X = spriteInfo.uv3X;
			uvi0.Y = spriteInfo.uv0Y;
			uvi1.Y = spriteInfo.uv0Y;
		}
		//end point cap
		{
			auto& uvi0 = vertices[i + i].TextureCoordinate[0];
			auto& uvi1 = vertices[i + i + 1].TextureCoordinate[0];
			uvi0.X = spriteInfo.uv0X;
			uvi1.X = spriteInfo.uv3X;
			uvi0.Y = spriteInfo.uv3Y;
			uvi1.Y = spriteInfo.uv3Y;
		}
	}
}

void UUI2DLineRendererBase::Update2DLineRendererBaseTriangle(UIGeometry& InGeo, const TArray<FVector2D>& InPointArray)
{
	int pointCount = InPointArray.Num();
	auto& triangles = InGeo.triangles;

	int pointIndex = 0;
	int vertIndex = 0, triangleIndex = 0;
	for (int count = pointCount - 1; pointIndex < count; pointIndex++)
	{
		vertIndex = pointIndex * 2;
		triangleIndex = pointIndex * 6;
		triangles[triangleIndex] = vertIndex;
		triangles[triangleIndex + 1] = vertIndex + 2;
		triangles[triangleIndex + 2] = vertIndex + 3;

		triangles[triangleIndex + 3] = vertIndex;
		triangles[triangleIndex + 4] = vertIndex + 3;
		triangles[triangleIndex + 5] = vertIndex + 1;
	}
	if (CanConnectStartEndPoint(pointCount))
	{
		int j = pointIndex * 2;
		int k = pointIndex * 6;
		triangles[k] = j;
		triangles[k + 1] = 0;
		triangles[k + 2] = 1;

		triangles[k + 3] = j;
		triangles[k + 4] = 1;
		triangles[k + 5] = j + 1;
	}
	else if (EndType == EUI2DLineRenderer_EndType::Cap)
	{
		vertIndex = pointIndex * 2;
		triangleIndex = pointIndex * 6;
		//start point cap
		{
			triangles[triangleIndex + 0] = vertIndex;
			triangles[triangleIndex + 1] = vertIndex + 2;
			triangles[triangleIndex + 2] = vertIndex + 3;

			triangles[triangleIndex + 3] = vertIndex;
			triangles[triangleIndex + 4] = vertIndex + 3;
			triangles[triangleIndex + 5] = vertIndex + 1;
		}
		//end point cap
		{
			triangles[triangleIndex + 6] = vertIndex + 4;
			triangles[triangleIndex + 7] = 0;
			triangles[triangleIndex + 8] = 1;

			triangles[triangleIndex + 9] = vertIndex + 4;
			triangles[triangleIndex + 10] = 1;
			triangles[triangleIndex + 11] = vertIndex + 5;
		}
	}
}

void UUI2DLineRendererBase::Update2DLineRendererBaseVertex(UIGeometry& InGeo, const TArray<FVector2D>& InPointArray)
{
	int pointCount = InPointArray.Num();
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0;
	UIGeometry::CalculatePivotOffset(this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), pivotOffsetX, pivotOffsetY);
	float halfW = this->GetWidth() * 0.5f;
	float halfH = this->GetHeight() * 0.5f;
	//positions
	auto& originVertices = InGeo.originVertices;

	FVector2D pos0, pos1;
	float lineLeftWidth = LineWidth * LineWidthOffset;
	float lineRightWidth = LineWidth * (1.0f - LineWidthOffset);
	FVector2D prevLineDir = FVector2D(1, 0);
	
	if (CanConnectStartEndPoint(pointCount))
	{
		GenerateLinePoint(InPointArray[0], InPointArray[pointCount - 1], InPointArray[1], lineLeftWidth, lineRightWidth, pos0, pos1, prevLineDir);
		originVertices[0].Position = FVector3f(0, pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY);
		originVertices[1].Position = FVector3f(0, pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY);
	}
	else
	{
		//start point
		FVector2D v0 = InPointArray[0];
		FVector2D v0to1 = InPointArray[1] - v0;
		FVector2D dir; 
		FVector2D widthDir;
		if (OverrideStartPointTangentDirection())
		{
			dir = GetStartPointTangentDirection();
		}
		else
		{
			float magnitude;
			v0to1.ToDirectionAndLength(dir, magnitude);
			if (magnitude < KINDA_SMALL_NUMBER)//the two points are too close
			{
				dir = FVector2D(1, 0);
			}
		}
		prevLineDir = dir;
		widthDir = FVector2D(dir.Y, -dir.X);//rotate 90 degree

		pos0 = v0 + lineLeftWidth * widthDir;
		pos1 = v0 - lineRightWidth * widthDir;

		originVertices[0].Position = FVector3f(0, pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY);
		originVertices[1].Position = FVector3f(0, pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY);

		if (EndType == EUI2DLineRenderer_EndType::Cap)
		{	
			//start point cap
			auto spriteInfo = sprite->GetSpriteInfo();
			auto capPoint = v0 - dir * (spriteInfo.HasBorder() ? spriteInfo.borderBottom : spriteInfo.height * 0.5f);

			pos0 = capPoint + lineLeftWidth * widthDir;
			pos1 = capPoint - lineRightWidth * widthDir;

			auto vertIndex = (pointCount + 1) * 2;
			originVertices[vertIndex].Position = FVector3f(0, pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY);
			originVertices[vertIndex + 1].Position = FVector3f(0, pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY);
		}
	}

	int i = 1;
	if (pointCount >= 3)
	{
		for (; i < pointCount - 1; i++)
		{
			FVector2D posA, posB;
			GenerateLinePoint(InPointArray[i], InPointArray[i - 1], InPointArray[i + 1], lineLeftWidth, lineRightWidth, posA, posB, prevLineDir);
			originVertices[i + i].Position = FVector3f(0, posA.X + pivotOffsetX, posA.Y + pivotOffsetY);
			originVertices[i + i + 1].Position = FVector3f(0, posB.X + pivotOffsetX, posB.Y + pivotOffsetY);
		}
	}

	auto i2 = i + i;
	if (CanConnectStartEndPoint(pointCount))
	{
		FVector2D posA, posB;
		GenerateLinePoint(InPointArray[pointCount - 1], InPointArray[pointCount - 2], InPointArray[0], lineLeftWidth, lineRightWidth, posA, posB, prevLineDir);
		originVertices[i2].Position = FVector3f(0, posA.X + pivotOffsetX, posA.Y + pivotOffsetY);
		originVertices[i2 + 1].Position = FVector3f(0, posB.X + pivotOffsetX, posB.Y + pivotOffsetY);
	}
	else
	{
		//end point
		FVector2D vEnd2 = InPointArray[pointCount - 2];
		FVector2D vEnd1 = InPointArray[pointCount - 1];
		//if (vEnd2 == vEnd1)
		//{
		//	originVertices[i2].Position = originVertices[i2 - 2].Position;
		//	originVertices[i2 + 1].Position = originVertices[i2 - 1].Position;
		//}
		//else
		{
			FVector2D v1to2 = vEnd1 - vEnd2;
			FVector2D dir; 
			FVector2D widthDir;
			if (OverrideEndPointTangentDirection())
			{
				dir = GetEndPointTangentDirection();
			}
			else
			{
				float magnitude;
				v1to2.ToDirectionAndLength(dir, magnitude);
				if (magnitude < KINDA_SMALL_NUMBER)//the two points are too close
				{
					dir = prevLineDir;
					widthDir = FVector2D(dir.Y, -dir.X);
				}
			}
			widthDir = FVector2D(dir.Y, -dir.X);//rotate 90 degree

			pos0 = vEnd1 + lineLeftWidth * widthDir;
			pos1 = vEnd1 - lineRightWidth * widthDir;

			originVertices[i2].Position = FVector3f(0, pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY);
			originVertices[i2 + 1].Position = FVector3f(0, pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY);

			if (EndType == EUI2DLineRenderer_EndType::Cap)
			{
				//end point cap
				auto spriteInfo = sprite->GetSpriteInfo();
				auto capPoint = vEnd1 + dir * (spriteInfo.HasBorder() ? spriteInfo.borderTop : spriteInfo.height * 0.5f);

				pos0 = capPoint + lineLeftWidth * widthDir;
				pos1 = capPoint - lineRightWidth * widthDir;

				auto vertIndex = pointCount * 2;
				originVertices[vertIndex].Position = FVector3f(0, pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY);
				originVertices[vertIndex + 1].Position = FVector3f(0, pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY);
			}
		}
	}
}

void UUI2DLineRendererBase::GenerateLinePoint(const FVector2D& InCurrentPoint, const FVector2D& InPrevPoint, const FVector2D& InNextPoint
	, float InLineLeftWidth, float InLineRightWidth
	, FVector2D& OutPosA, FVector2D& OutPosB
	, FVector2D& InOutPrevLineDir)
{
	if (InCurrentPoint == InPrevPoint || InCurrentPoint == InNextPoint)
	{
		auto itemNormal = FVector2D(InOutPrevLineDir.Y, -InOutPrevLineDir.X);
		OutPosA = InCurrentPoint + InLineLeftWidth * itemNormal;
		OutPosB = InCurrentPoint - InLineRightWidth * itemNormal;
		return;
	}
	FVector2D normalizedV1 = (InPrevPoint - InCurrentPoint).GetSafeNormal();
	FVector2D normalizedV2 = (InNextPoint - InCurrentPoint).GetSafeNormal();
	if (normalizedV1 == -normalizedV2)
	{
		InOutPrevLineDir = normalizedV2;
		auto itemNormal = FVector2D(normalizedV2.Y, -normalizedV2.X);
		OutPosA = InCurrentPoint + InLineLeftWidth * itemNormal;
		OutPosB = InCurrentPoint - InLineRightWidth * itemNormal;
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
		itemNormal = AngleLargerThanPi(normalizedV1, normalizedV2) ? -itemNormal : itemNormal;
		OutPosA = InCurrentPoint + InLineLeftWidth / sin * itemNormal;
		OutPosB = InCurrentPoint - InLineRightWidth / sin * itemNormal;
		InOutPrevLineDir = normalizedV2;
	}
}


void UUI2DLineRendererBase::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_2DLineUpdate);
	auto& CurrentPointArray = GetCalcaultedPointArray();
	int pointCount = CurrentPointArray.Num();
	if (pointCount < 2)
	{
		geometry->Clear();
		return;
	}
	
	auto& triangles = InGeo.triangles;
	int triangleIndicesCount = (pointCount - 1) * 2 * 3;
	if (CanConnectStartEndPoint(pointCount))
	{
		triangleIndicesCount += 6;
	}
	else if (EndType == EUI2DLineRenderer_EndType::Cap)
	{
		triangleIndicesCount += 12;
	}
	UIGeometry::LGUIGeometrySetArrayNum(triangles, triangleIndicesCount);
	if (InTriangleChanged)
	{
		Update2DLineRendererBaseTriangle(InGeo, CurrentPointArray);
	}

	auto& vertices = InGeo.vertices;
	auto& originVertices = InGeo.originVertices;
	int vertexCount = pointCount * 2;
	if (EndType == EUI2DLineRenderer_EndType::Cap)
	{
		vertexCount += 4;
	}
	UIGeometry::LGUIGeometrySetArrayNum(vertices, vertexCount);
	UIGeometry::LGUIGeometrySetArrayNum(originVertices, vertexCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		if (InVertexPositionChanged)
		{
			Update2DLineRendererBaseVertex(InGeo, CurrentPointArray);
		}
		if (InVertexUVChanged)
		{
			Update2DLineRendererBaseUV(InGeo, CurrentPointArray);
		}
		if (InVertexColorChanged)
		{
			UIGeometry::UpdateUIColor(&InGeo, GetFinalColor());
		}

		//normal & tangent
		if (RenderCanvas->GetRequireNormal() || RenderCanvas->GetRequireTangent())
		{
			for (int i = 0; i < originVertices.Num(); i++)
			{
				originVertices[i].Normal = FVector3f(-1, 0, 0);
				originVertices[i].Tangent = FVector3f(0, 1, 0);
			}
		}
	}
}

void UUI2DLineRendererBase::OnBeforeCreateOrUpdateGeometry()
{
	CalculatePoints();
}

FVector2D UUI2DLineRendererBase::GetStartPointTangentDirection()
{
	UE_LOG(LGUI, Error, TEXT("This function (UUI2DLineRendererBase::GetStartPointTangentDirection) must be implemented if (UUI2DLineRendererBase::OverrideStartPointTangentDirection) return true!"));
	return FVector2D::ZeroVector;
}
FVector2D UUI2DLineRendererBase::GetEndPointTangentDirection()
{
	UE_LOG(LGUI, Error, TEXT("This function (UUI2DLineRendererBase::GetEndPointTangentDirection) must be implemented if (UUI2DLineRendererBase::OverrideEndPointTangentDirection) return true"));
	return FVector2D::ZeroVector;
}

void UUI2DLineRendererBase::SetEndType(EUI2DLineRenderer_EndType newValue)
{
	if (EndType != newValue)
	{
		EndType = newValue;
		MarkVerticesDirty(true, true, true, true);
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
void UUI2DLineRendererBase::SetLineWidthOffset(float newValue)
{
	if (LineWidthOffset != newValue)
	{
		LineWidthOffset = newValue;
		MarkVertexPositionDirty();
	}
}

ULTweener* UUI2DLineRendererBase::LineWidthTo(float endValue, float duration, float delay, ELTweenEase easeType)
{
	return ULTweenManager::To(this->GetWorld(), FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRendererBase::GetLineWidth), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRendererBase::SetLineWidth), endValue, duration)
		->SetDelay(delay)->SetEase(easeType);
}