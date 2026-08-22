// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/2DLineRenderer/Lex2DLineRendererBase.h"
#include "LGUI.h"
#include "Core/LexUIGeometry.h"
#include "Core/Components/LexCanvas.h"
#include "Core/LexUISpriteData_BaseObject.h"
#include "LTweenManager.h"
#include "Core/Components/LexWidget.h"

DECLARE_CYCLE_STAT(TEXT("UI2DLine Update"), STAT_2DLineUpdate, STATGROUP_LGUI);

TArray<FVector2D> ULex2DLineRendererBase::EmptyArray;
ULex2DLineRendererBase::ULex2DLineRendererBase(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
}

void ULex2DLineRendererBase::BeginPlay()
{
	Super::BeginPlay();
}

void ULex2DLineRendererBase::Update2DLineRendererBaseUV(FLexUIGeometry& InGeo, const TArray<FVector2D>& InPointArray)
{
	auto& vertices = InGeo.Vertices;
	int pointCount = InPointArray.Num();

	FVector2f MinUV;
	FVector2f MaxUV;
	if (bHasAddToSprite)
	{
		auto LexSprite = (ULexUISpriteData_BaseObject*)Brush.GetResourceObject();
		auto& SpriteInfo = LexSprite->GetSpriteInfo();
		MinUV = SpriteInfo.MinUV;
		MaxUV = SpriteInfo.MaxUV;
	}
	else
	{
		MinUV = FVector2f(Brush.UVRegion.X, Brush.UVRegion.Y);
		MaxUV = FVector2f(Brush.UVRegion.Z, Brush.UVRegion.W);
	}
	
	float uvY = (MaxUV.Y + MinUV.Y) * 0.5f;
	int i = 0;
	for (; i < pointCount; i++)
	{
		auto& uvi0 = vertices[i + i].TextureCoordinate[0];
		auto& uvi1 = vertices[i + i + 1].TextureCoordinate[0];
		uvi0.X = MinUV.X;
		uvi1.X = MaxUV.X;
		uvi0.Y = uvY;
		uvi1.Y = uvY;
	}
	
	if (EndType == ELex2DLineRenderer_EndType::Cap)
	{
		//start point cap
		{
			auto& uvi0 = vertices[i + i + 2].TextureCoordinate[0];
			auto& uvi1 = vertices[i + i + 3].TextureCoordinate[0];
			uvi0.X = MinUV.X;
			uvi0.Y = MinUV.Y;
			uvi1.X = MaxUV.X;
			uvi1.Y = MinUV.Y;
		}
		//end point cap
		{
			auto& uvi0 = vertices[i + i].TextureCoordinate[0];
			auto& uvi1 = vertices[i + i + 1].TextureCoordinate[0];
			uvi0.X = MinUV.X;
			uvi0.Y = MaxUV.Y;
			uvi1.X = MaxUV.X;
			uvi1.Y = MaxUV.Y;
		}
	}
}

void ULex2DLineRendererBase::Update2DLineRendererBaseTriangle(FLexUIGeometry& InGeo, const TArray<FVector2D>& InPointArray)
{
	int pointCount = InPointArray.Num();
	auto& triangles = InGeo.Triangles;

	int pointIndex = 0;
	for (int count = pointCount - 1; pointIndex < count; pointIndex++)
	{
		int vertIndex = pointIndex * 2;
		int triangleIndex = pointIndex * 6;
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
	else if (EndType == ELex2DLineRenderer_EndType::Cap)
	{
		int vertIndex = pointIndex * 2;
		int triangleIndex = pointIndex * 6;
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

void ULex2DLineRendererBase::Update2DLineRendererBaseVertex(FLexUIGeometry& InGeo, const TArray<FVector2D>& InPointArray)
{
	auto Widget = GetWidget();
	int pointCount = InPointArray.Num();
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0;
	FLexUIGeometry::CalculatePivotOffset(Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), pivotOffsetX, pivotOffsetY);
	float halfW = Widget->GetWidth() * 0.5f;
	float halfH = Widget->GetHeight() * 0.5f;
	//positions
	auto& originVertices = InGeo.OriginVertices;

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

		if (EndType == ELex2DLineRenderer_EndType::Cap)
		{	
			//start point cap
			float capSize = 0, spriteWidth = 0;
			if (bHasAddToSprite)
			{
				auto LexSprite = (ULexUISpriteData_BaseObject*)Brush.GetResourceObject();
				auto& SpriteInfo = LexSprite->GetSpriteInfo();
				capSize = SpriteInfo.HasBorder() ? SpriteInfo.Border.Bottom : SpriteInfo.Height * 0.5f;
				spriteWidth = SpriteInfo.Width;
			}
			else
			{
				capSize = Brush.ImageSize.Y * 0.5f;
				spriteWidth = Brush.ImageSize.X * 0.5f;
			}
			
			if (bEndCapSizeAffectByLineWidth)
			{
				capSize *= LineWidth / spriteWidth;
			}
			auto capPoint = v0 - dir * capSize;

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

			if (EndType == ELex2DLineRenderer_EndType::Cap)
			{
				//end point cap
				float capSize = 0, spriteWidth = 0;
				if (bHasAddToSprite)
				{
					auto LexSprite = (ULexUISpriteData_BaseObject*)Brush.GetResourceObject();
					auto& SpriteInfo = LexSprite->GetSpriteInfo();
					capSize = SpriteInfo.HasBorder() ? SpriteInfo.Border.Bottom : SpriteInfo.Height * 0.5f;
					spriteWidth = SpriteInfo.Width;
				}
				else
				{
					capSize = Brush.ImageSize.Y * 0.5f;
					spriteWidth = Brush.ImageSize.X * 0.5f;
				}
				if (bEndCapSizeAffectByLineWidth)
				{
					capSize *= LineWidth / spriteWidth;
				}
				auto capPoint = vEnd1 + dir * capSize;

				pos0 = capPoint + lineLeftWidth * widthDir;
				pos1 = capPoint - lineRightWidth * widthDir;

				auto vertIndex = pointCount * 2;
				originVertices[vertIndex].Position = FVector3f(0, pos0.X + pivotOffsetX, pos0.Y + pivotOffsetY);
				originVertices[vertIndex + 1].Position = FVector3f(0, pos1.X + pivotOffsetX, pos1.Y + pivotOffsetY);
			}
		}
	}
}

void ULex2DLineRendererBase::GenerateLinePoint(const FVector2D& InCurrentPoint, const FVector2D& InPrevPoint, const FVector2D& InNextPoint
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


void ULex2DLineRendererBase::OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_2DLineUpdate);
	auto& CurrentPointArray = GetCalcaultedPointArray();
	int pointCount = CurrentPointArray.Num();
	if (pointCount < 2)
	{
		UIGeometry->Clear();
		return;
	}
	
	auto& triangles = InGeo.Triangles;
	int triangleIndicesCount = (pointCount - 1) * 2 * 3;
	if (CanConnectStartEndPoint(pointCount))
	{
		triangleIndicesCount += 6;
	}
	else if (EndType == ELex2DLineRenderer_EndType::Cap)
	{
		triangleIndicesCount += 12;
	}
	FLexUIGeometry::LexUIGeometrySetArrayNum(triangles, triangleIndicesCount);
	if (InTriangleChanged)
	{
		Update2DLineRendererBaseTriangle(InGeo, CurrentPointArray);
	}

	auto& vertices = InGeo.Vertices;
	auto& originVertices = InGeo.OriginVertices;
	int vertexCount = pointCount * 2;
	if (EndType == ELex2DLineRenderer_EndType::Cap)
	{
		vertexCount += 4;
	}
	FLexUIGeometry::LexUIGeometrySetArrayNum(vertices, vertexCount);
	FLexUIGeometry::LexUIGeometrySetArrayNum(originVertices, vertexCount);
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
			FLexUIGeometry::UpdateUIColor(&InGeo, GetFinalColor());
		}

		//normal & tangent
		if (GetWidget()->GetRenderCanvas()->GetActualRequireNormalAndTangent())
		{
			for (int i = 0; i < originVertices.Num(); i++)
			{
				originVertices[i].Normal = FVector3f(-1, 0, 0);
				originVertices[i].Tangent = FVector3f(0, 1, 0);
			}
		}
	}
}

void ULex2DLineRendererBase::OnBeforeCreateOrUpdateGeometry()
{
	CalculatePoints();
}

FVector2D ULex2DLineRendererBase::GetStartPointTangentDirection()
{
	UE_LOG(LGUI, Error, TEXT("This function [%s].%d must be implemented if [OverrideStartPointTangentDirection] return true!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	return FVector2D::ZeroVector;
}
FVector2D ULex2DLineRendererBase::GetEndPointTangentDirection()
{
	UE_LOG(LGUI, Error, TEXT("This function [%s].%d must be implemented if [OverrideEndPointTangentDirection] return true!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	return FVector2D::ZeroVector;
}

void ULex2DLineRendererBase::SetEndType(ELex2DLineRenderer_EndType newValue)
{
	if (EndType != newValue)
	{
		EndType = newValue;
		MarkVerticesDirty(true, true, true, true);
	}
}
void ULex2DLineRendererBase::SetLineWidth(float newValue)
{
	if (LineWidth != newValue)
	{
		LineWidth = newValue;
		MarkVertexPositionDirty();
	}
}
void ULex2DLineRendererBase::SetLineWidthOffset(float newValue)
{
	if (LineWidthOffset != newValue)
	{
		LineWidthOffset = newValue;
		MarkVertexPositionDirty();
	}
}

ULTweener* ULex2DLineRendererBase::LineWidthTo(float endValue, float duration, float delay, ELTweenEase easeType)
{
	auto Tweener = ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &ULex2DLineRendererBase::GetLineWidth), FLTweenFloatSetterFunction::CreateUObject(this, &ULex2DLineRendererBase::SetLineWidth), endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(easeType)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
	}
	return Tweener;
}