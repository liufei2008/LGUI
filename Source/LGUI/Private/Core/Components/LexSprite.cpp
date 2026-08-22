// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexSprite.h"
#include "LGUI.h"
#include "Core/LexUIGeometry.h"
#include "Core/Components/LexCanvas.h"
#include "Core/LexUISpriteData_BaseObject.h"
#include "Core/Components/LexWidget.h"


ULexSprite::ULexSprite(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
}

void ULexSprite::BeginPlay()
{
	Super::BeginPlay();
}

void ULexSprite::OnRegister()
{
	Super::OnRegister();
	if (DrawType == ELexUISpriteDrawType::Tiled)
	{
		CalculateTiledParams();
	}
}
#if WITH_EDITOR
void ULexSprite::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propName = Property->GetFName();
		if (propName == GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial90))
		{
			FillOrigin = (uint8)FillOriginType_Radial90;
			FillOriginType_Radial180 = (ELexUISpriteFillOriginType_Radial180)FillOrigin;
			FillOriginType_Radial360 = (ELexUISpriteFillOriginType_Radial360)FillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial180))
		{
			FillOrigin = (uint8)FillOriginType_Radial180;
			FillOriginType_Radial90 = (ELexUISpriteFillOriginType_Radial90)FillOrigin;
			FillOriginType_Radial360 = (ELexUISpriteFillOriginType_Radial360)FillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(ULexSprite, FillOriginType_Radial360))
		{
			FillOrigin = (uint8)FillOriginType_Radial360;
			FillOriginType_Radial180 = (ELexUISpriteFillOriginType_Radial180)FillOrigin;
			FillOriginType_Radial90 = (ELexUISpriteFillOriginType_Radial90)FillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(ULexSprite, Sprite))
		{
			if (IsValid(Sprite))
			{
				if (Sprite->GetSpriteInfo().HasBorder())
				{
					if (this->DrawType == ELexUISpriteDrawType::Normal)
					{
						this->SetDrawType(ELexUISpriteDrawType::Sliced);
					}
				}
			}
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(ULexSprite, FlipMode))
		{
			MarkVerticesDirty(false, true, true, false);
		}
		if (IsValid(Sprite) && DrawType == ELexUISpriteDrawType::Tiled)
		{
			CalculateTiledParams();
		}
	}
}
#endif

void ULexSprite::OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	auto Widget = GetWidget();
	auto RenderCanvas = Widget->GetRenderCanvas();
	bool bFlipH = false, bFlipV = false;
	switch (FlipMode)
	{
	case ELexUISpriteFlipMode::Horizontal:
		bFlipH = true;
		break;
	case ELexUISpriteFlipMode::Vertical:
		bFlipV = true;
		break;
	case ELexUISpriteFlipMode::Both:
		bFlipH = bFlipV = true;
		break;
	case ELexUISpriteFlipMode::Nothing:
	default:
		break;
	}
	auto SpriteInfo = Sprite->GetSpriteInfo();
	FLexUISpriteInfo::ApplyFlip(SpriteInfo, bFlipH, bFlipV);
	switch (DrawType)
	{
	case ELexUISpriteDrawType::Normal:
		FLexUIGeometry::UpdateUIRectSimpleVertex(&InGeo, 
			Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, RenderCanvas, this, GetFinalColor(), 
			InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
		break;
	case ELexUISpriteDrawType::Sliced:
		if (SpriteInfo.HasBorder())
		{
			FLexUIGeometry::UpdateUIRectBorderVertex(&InGeo, bFillCenter, Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, RenderCanvas, this, GetFinalColor(),
				1.0f / PixelsPerUnitMultiplier, 
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		else
		{
			FLexUIGeometry::UpdateUIRectSimpleVertex(&InGeo,
				Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, RenderCanvas, this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
	break;
	case ELexUISpriteDrawType::Tiled:
		if (SpriteInfo.HasBorder())
		{
			FLexUIGeometry::UpdateUIRectTiledBorderVertex(&InGeo, bFillCenter, SpriteInfo, RenderCanvas, this, Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize, GetFinalColor(), 
				1.0f / PixelsPerUnitMultiplier,
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		else
		{
			FLexUIGeometry::UpdateUIRectTiledVertex(&InGeo, SpriteInfo, RenderCanvas, this, Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize, GetFinalColor(), 
				1.0f / PixelsPerUnitMultiplier,
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		break;
	case ELexUISpriteDrawType::Filled:
	{
		switch (FillMethod)
		{
		case ELexUISpriteFillMethod::Horizontal:
		case ELexUISpriteFillMethod::Vertical:
			FLexUIGeometry::UpdateUIRectFillHorizontalVerticalVertex(&InGeo, Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, FillDirectionFlip, FillAmount, FillMethod == ELexUISpriteFillMethod::Horizontal, RenderCanvas, this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case ELexUISpriteFillMethod::Radial90:
			FLexUIGeometry::UpdateUIRectFillRadial90Vertex(&InGeo, Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, FillDirectionFlip, FillAmount, (ELexUISpriteFillOriginType_Radial90)FillOrigin, RenderCanvas, this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case ELexUISpriteFillMethod::Radial180:
			FLexUIGeometry::UpdateUIRectFillRadial180Vertex(&InGeo, Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, FillDirectionFlip, FillAmount, (ELexUISpriteFillOriginType_Radial180)FillOrigin, RenderCanvas, this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case ELexUISpriteFillMethod::Radial360:
			FLexUIGeometry::UpdateUIRectFillRadial360Vertex(&InGeo, Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, FillDirectionFlip, FillAmount, (ELexUISpriteFillOriginType_Radial360)FillOrigin, RenderCanvas, this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		}
	}
	break;
	}
}

void ULexSprite::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
    Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
	if (!IsValid(Sprite))return;
	if (DrawType == ELexUISpriteDrawType::Tiled)
	{
        if (InWidthChange || InHeightChange)
        {
			CalculateTiledParams();
        }
	}
    else
    {
        if (InPivotChange || InWidthChange || InHeightChange)
        {
			MarkVertexPositionDirty();
		}
    }
}

void ULexSprite::CalculateTiledParams()
{
	if (!Sprite->IsIndividual())
	{
		auto Widget = GetWidget();
		if (Widget->GetWidth() <= 0 || Widget->GetHeight() <= 0)
		{
			if (Tiled_WidthRectCount != 0)
			{
				Tiled_WidthRectCount = 0;
				Tiled_WidthRemainedRectSize = 0;
				MarkVerticesDirty(true, true, true, false);
			}
			if (Tiled_HeightRectCount != 0)
			{
				Tiled_HeightRectCount = 0;
				Tiled_HeightRemainedRectSize = 0;
				MarkVerticesDirty(true, true, true, false);
			}
			return;
		}
		auto& SpriteInfo = Sprite->GetSpriteInfo();
		auto BorderSize = SpriteInfo.Border.GetDesiredSize2f();
		auto MultipliedBorderSize = BorderSize / PixelsPerUnitMultiplier;
		auto SpriteSize = FVector2f(SpriteInfo.Width, SpriteInfo.Height);
		auto MultipliedSpriteSize = SpriteSize / PixelsPerUnitMultiplier;
		bool bTriangleDirty = false;
		
		if (Widget->GetWidth() > MultipliedBorderSize.X)
		{
			float WidthCountFloat = (Widget->GetWidth() - MultipliedBorderSize.X) / (MultipliedSpriteSize.X - MultipliedBorderSize.X);
			int WidthCount = (int)WidthCountFloat + 1;//rect count of width-direction, +1 means not-full-size rect
			if (WidthCount != Tiled_WidthRectCount)
			{
				Tiled_WidthRectCount = WidthCount;
				bTriangleDirty = true;
			}
			float RemainedWidth = (WidthCountFloat - (WidthCount - 1)) * (MultipliedSpriteSize.X - MultipliedBorderSize.X);//not-full-size rect's width
			if (RemainedWidth != Tiled_WidthRemainedRectSize)
			{
				Tiled_WidthRemainedRectSize = RemainedWidth;
			}
		}
		else
		{
			Tiled_WidthRectCount = 0;
			Tiled_WidthRemainedRectSize = 0;
		}

		if (Widget->GetHeight() > MultipliedBorderSize.Y)
		{
			float HeightCountFloat = (Widget->GetHeight() - MultipliedBorderSize.Y) / (MultipliedSpriteSize.Y - MultipliedBorderSize.Y);
			int HeightCount = (int)HeightCountFloat + 1;//rect count of height-direction, +1 means not-full-size rect
			if (HeightCount != Tiled_HeightRectCount)
			{
				Tiled_HeightRectCount = HeightCount;
				bTriangleDirty = true;
			}
			float RemainedHeight = (HeightCountFloat - (HeightCount - 1)) * (MultipliedSpriteSize.Y - MultipliedBorderSize.Y);//not-full-size rect's height
			if (RemainedHeight != Tiled_HeightRemainedRectSize)
			{
				Tiled_HeightRemainedRectSize = RemainedHeight;
			}
		}
		else
		{
			Tiled_HeightRectCount = 0;
			Tiled_HeightRemainedRectSize = 0;
		}
		
		MarkVerticesDirty(bTriangleDirty, true, true, true);
	}
	else
	{
		MarkVerticesDirty(false, true, true, false);
	}
}

void ULexSprite::SetDrawType(ELexUISpriteDrawType Value) {
	if (DrawType != Value)
	{
		DrawType = Value;
		MarkVerticesDirty(true, true, true, true);
		if (DrawType == ELexUISpriteDrawType::Tiled)
		{
			CalculateTiledParams();
		}
	}
}

void ULexSprite::SetFlipMode(ELexUISpriteFlipMode Value)
{
	if (FlipMode != Value)
	{
		FlipMode = Value;
		MarkVerticesDirty(false, true, true, false);
	}
}

void ULexSprite::SetPixelsPerUnitMultiplier(float Value)
{
	if (PixelsPerUnitMultiplier != Value)
	{
		PixelsPerUnitMultiplier = Value;
		PixelsPerUnitMultiplier = FMath::Max(0.01f, PixelsPerUnitMultiplier);
		if (DrawType == ELexUISpriteDrawType::Sliced || DrawType == ELexUISpriteDrawType::Tiled)
		{
			MarkVertexPositionDirty();
		}
	}
}

void ULexSprite::SetFillCenter(bool Value)
{
	if (bFillCenter != Value)
	{
		bFillCenter = Value;
		if (DrawType == ELexUISpriteDrawType::Sliced || DrawType == ELexUISpriteDrawType::Tiled)
		{
			MarkVertexPositionDirty();
		}
	}
}

void ULexSprite::SetFillMethod(ELexUISpriteFillMethod Value)
{
	if (FillMethod != Value)
	{
		FillMethod = Value;
		if (DrawType == ELexUISpriteDrawType::Filled)
		{
			MarkVerticesDirty(true, true, true, true);
		}
	}
}
void ULexSprite::SetFillOrigin(uint8 Value)
{
	if (FillOrigin != Value)
	{
		FillOrigin = Value;
		if (DrawType == ELexUISpriteDrawType::Filled)
		{
			if (FillMethod == ELexUISpriteFillMethod::Radial90)
			{
				MarkVerticesDirty(false, true, true, false);
			}
			else if (FillMethod == ELexUISpriteFillMethod::Radial180 || FillMethod == ELexUISpriteFillMethod::Radial360)
			{
				MarkVerticesDirty(true, true, true, true);
			}
		}
	}
}
void ULexSprite::SetFillDirectionFlip(bool Value)
{
	if (FillDirectionFlip != Value)
	{
		FillDirectionFlip = Value;
		if (DrawType == ELexUISpriteDrawType::Filled)
		{
			MarkVerticesDirty(false, true, true, false);
		}
	}
}
void ULexSprite::SetFillAmount(float Value)
{
	if (FillAmount != Value)
	{
		FillAmount = Value;
		if (DrawType == ELexUISpriteDrawType::Filled)
		{
			MarkVerticesDirty(false, true, true, false);
		}
	}
}
