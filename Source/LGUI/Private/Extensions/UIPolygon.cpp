// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/UIPolygon.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "LTweenManager.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

UUIPolygon::UUIPolygon(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIPolygon::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	Sides = FMath::Max(Sides, FullCycle ? 3 : 1);

	auto& triangles = InGeo.triangles;
	auto triangleCount = Sides * 3;
	UIGeometry::LGUIGeometrySetArrayNum(triangles, triangleCount);
	if (InTriangleChanged)
	{
		int index = 0;
		if (FullCycle)
		{
			for (int i = 0; i < Sides - 1; i++)
			{
				triangles[index++] = 0;
				triangles[index++] = i + 1;
				triangles[index++] = i + 2;
			}
			triangles[index++] = 0;
			triangles[index++] = Sides;
			triangles[index++] = 1;
		}
		else
		{
			for (int i = 0; i < Sides; i++)
			{
				triangles[index++] = 0;
				triangles[index++] = i + 1;
				triangles[index++] = i + 2;
			}
		}
	}

	auto& vertices = InGeo.vertices;
	auto& originVertices = InGeo.originVertices;
	int vertexCount = (FullCycle ? 1 : 2) + Sides;
	UIGeometry::LGUIGeometrySetArrayNum(vertices, vertexCount);
	UIGeometry::LGUIGeometrySetArrayNum(originVertices, vertexCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		//vert offset
		int VertexOffsetCount = FullCycle ? Sides : (Sides + 1);
		if (VertexOffsetArray.Num() != VertexOffsetCount)
		{
			if (VertexOffsetArray.Num() > VertexOffsetCount)
			{
				VertexOffsetArray.SetNumZeroed(VertexOffsetCount);
			}
			else
			{
				for (int i = VertexOffsetArray.Num(); i < VertexOffsetCount; i++)
				{
					VertexOffsetArray.Add(1.0f);
				}
			}
		}

		float calcStartAngle = StartAngle, calcEndAngle = EndAngle;
		if (InVertexPositionChanged)
		{
			auto width = this->GetWidth();
			auto height = this->GetHeight();
			auto pivot = FVector2f(this->GetPivot());
			//pivot offset
			float pivotOffsetX = 0, pivotOffsetY = 0;
			UIGeometry::CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY);
			float halfW = width * 0.5f;
			float halfH = height * 0.5f;

			if (FullCycle)calcEndAngle = calcStartAngle + 360.0f;
			float singleAngle = FMath::DegreesToRadians((calcEndAngle - calcStartAngle) / Sides);
			float angle = FMath::DegreesToRadians(calcStartAngle);

			float sin = FMath::Sin(angle);
			float cos = FMath::Cos(angle);

			float x = pivotOffsetX;
			float y = pivotOffsetY;
			originVertices[0].Position = FVector3f(0, x, y);

			for (int i = 0, count = Sides; i < count; i++)
			{
				sin = FMath::Sin(angle);
				cos = FMath::Cos(angle);
				x = cos * halfW * VertexOffsetArray[i] + pivotOffsetX;
				y = sin * halfH * VertexOffsetArray[i] + pivotOffsetY;
				originVertices[i + 1].Position = FVector3f(0, x, y);
				angle += singleAngle;
			}
			if (!FullCycle)
			{
				sin = FMath::Sin(angle);
				cos = FMath::Cos(angle);
				x = cos * halfW * VertexOffsetArray[Sides] + pivotOffsetX;
				y = sin * halfH * VertexOffsetArray[Sides] + pivotOffsetY;
				originVertices[Sides + 1].Position = FVector3f(0, x, y);
			}
		}

		if (InVertexUVChanged)
		{
			auto spriteInfo = this->GetSprite()->GetSpriteInfo();
			switch (UVType)
			{
			case UIPolygonUVType::SpriteRect:
			{
				if (FullCycle)calcEndAngle = calcStartAngle + 360.0f;
				float singleAngle = FMath::DegreesToRadians((calcEndAngle - calcStartAngle) / Sides);
				float angle = FMath::DegreesToRadians(calcStartAngle);

				float sin = FMath::Sin(angle);
				float cos = FMath::Cos(angle);

				float halfUVWidth = (spriteInfo.uv3X - spriteInfo.uv0X) * 0.5f;
				float halfUVHeight = (spriteInfo.uv3Y - spriteInfo.uv0Y) * 0.5f;
				float centerUVX = (spriteInfo.uv0X + spriteInfo.uv3X) * 0.5f;
				float centerUVY = (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f;

				float x = centerUVX;
				float y = centerUVY;
				vertices[0].TextureCoordinate[0] = FVector2f(x, y);

				int count = FullCycle ? Sides : (Sides + 1);
				for (int i = 0; i < count; i++)
				{
					sin = FMath::Sin(angle);
					cos = FMath::Cos(angle);
					x = cos * halfUVWidth + centerUVX;
					y = sin * halfUVHeight + centerUVY;
					vertices[i + 1].TextureCoordinate[0] = FVector2f(x, y);
					angle += singleAngle;
				}
			}
			break;
			case UIPolygonUVType::HeightCenter:
			{
				vertices[0].TextureCoordinate[0] = FVector2f(spriteInfo.uv0X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
				FVector2f otherUV(spriteInfo.uv3X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
				for (int i = 1; i < vertexCount; i++)
				{
					vertices[i].TextureCoordinate[0] = otherUV;
				}
			}
			break;
			case UIPolygonUVType::StretchSpriteHeight:
			{
				vertices[0].TextureCoordinate[0] = FVector2f(spriteInfo.uv0X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
				float uvX = spriteInfo.uv3X;
				float uvY = spriteInfo.uv0Y;
				float uvYInterval = (spriteInfo.uv3Y - spriteInfo.uv0Y) / (vertexCount - 2);
				for (int i = 1; i < vertexCount; i++)
				{
					auto& uv = vertices[i].TextureCoordinate[0];
					uv.X = uvX;
					uv.Y = uvY;
					uvY += uvYInterval;
				}
			}
			break;
			}
		}

		if (InVertexColorChanged)
		{
			UIGeometry::UpdateUIColor(&InGeo, GetFinalColor());
		}

		//additional data
		{
			//normal & tangent
			if (RenderCanvas->GetRequireNormal() || RenderCanvas->GetRequireTangent())
			{
				for (int i = 0; i < vertexCount; i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (RenderCanvas->GetRequireUV1())
			{
				for (int i = 0; i < vertexCount; i++)
				{
					vertices[i].TextureCoordinate[1] = FVector2f(0, 1);
				}
			}
		}
	}
}

void UUIPolygon::SetStartAngle(float value) {
	if (StartAngle != value)
	{
		StartAngle = value;
		MarkVerticesDirty(false, true, true, false);
	}
}
void UUIPolygon::SetEndAngle(float value) {
	if (EndAngle != value)
	{
		EndAngle = value;
		MarkVerticesDirty(false, true, true, false);
	}
}
void UUIPolygon::SetSides(int value) {
	if (Sides != value)
	{
		Sides = value;
		Sides = FMath::Max(Sides, FullCycle ? 3 : 1);
		MarkVerticesDirty(true, true, true, true);
	}
}
void UUIPolygon::SetUVType(UIPolygonUVType value)
{
	if (UVType != value)
	{
		UVType = value;
		MarkUVDirty();
	}
}
void UUIPolygon::SetVertexOffsetArray(const TArray<float>& value)
{
	if (VertexOffsetArray.Num() == value.Num())
	{
		VertexOffsetArray = value;
		MarkVertexPositionDirty();
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUIPolygon::SetVertexOffsetArray]Array count not equal! VertexOffsetArray:%d, value:%d"), VertexOffsetArray.Num(), value.Num());
	}
}
ULTweener* UUIPolygon::StartAngleTo(float endValue, float duration /* = 0.5f */, float delay /* = 0.0f */, ELTweenEase easeType /* = ELTweenEase::OutCubic */)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIPolygon::GetStartAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIPolygon::SetStartAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}
ULTweener* UUIPolygon::EndAngleTo(float endValue, float duration /* = 0.5f */, float delay /* = 0.0f */, ELTweenEase easeType /* = ELTweenEase::OutCubic */)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIPolygon::GetEndAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIPolygon::SetEndAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}


AUIPolygonActor::AUIPolygonActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIPolygon = CreateDefaultSubobject<UUIPolygon>(TEXT("UIPolygonComponent"));
	RootComponent = UIPolygon;
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif