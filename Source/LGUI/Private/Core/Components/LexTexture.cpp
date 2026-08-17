// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexTexture.h"
#include "Core/LexUIGeometry.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"


ULexTexture::ULexTexture(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
}

void ULexTexture::BeginPlay()
{
	Super::BeginPlay();
}

void ULexTexture::OnRegister()
{
	Super::OnRegister();
	if (DrawType == ELexUISpriteDrawType::Tiled)
	{
		CalculateTiledParams();
	}
}
#if WITH_EDITOR
void ULexTexture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	CheckSpriteInfo();
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropName = Property->GetFName();
		if (PropName == GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial90))
		{
			FillOrigin = (uint8)FillOriginType_Radial90;
			FillOriginType_Radial180 = (ELexUISpriteFillOriginType_Radial180)FillOrigin;
			FillOriginType_Radial360 = (ELexUISpriteFillOriginType_Radial360)FillOrigin;
		}
		else if (PropName == GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial180))
		{
			FillOrigin = (uint8)FillOriginType_Radial180;
			FillOriginType_Radial90 = (ELexUISpriteFillOriginType_Radial90)FillOrigin;
			FillOriginType_Radial360 = (ELexUISpriteFillOriginType_Radial360)FillOrigin;
		}
		else if (PropName == GET_MEMBER_NAME_CHECKED(ULexTexture, FillOriginType_Radial360))
		{
			FillOrigin = (uint8)FillOriginType_Radial360;
			FillOriginType_Radial180 = (ELexUISpriteFillOriginType_Radial180)FillOrigin;
			FillOriginType_Radial90 = (ELexUISpriteFillOriginType_Radial90)FillOrigin;
		}
		if (IsValid(Texture) && DrawType == ELexUISpriteDrawType::Tiled)
		{
			CalculateTiledParams();
		}
	}
}
#endif

void ULexTexture::CheckSpriteInfo()
{
	if (IsValid(Texture))
	{
		SpriteInfo.Width = Texture->GetSurfaceWidth();
		SpriteInfo.Height = Texture->GetSurfaceHeight();
		float InvWidth = 1.0f / SpriteInfo.Width, InvHeight = 1.0f / SpriteInfo.Height;
		SpriteInfo.ApplyUV(0, 0, SpriteInfo.Width, SpriteInfo.Height, InvWidth, InvHeight, UVRect);
		if (SpriteInfo.HasBorder())
		{
			SpriteInfo.ApplyBorderUV(InvWidth, InvHeight);
		}
		else
		{
			SpriteInfo.BorderMinUV = SpriteInfo.MinUV;
			SpriteInfo.BorderMaxUV = SpriteInfo.MaxUV;
		}
	}
}

void ULexTexture::OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	auto Widget = GetWidget();
	auto RenderCanvas = Widget->GetRenderCanvas();
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

void ULexTexture::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
    Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
	if (!IsValid(Texture))return;
	if (DrawType == ELexUISpriteDrawType::Tiled)
	{
        if (InWidthChange || InHeightChange)
        {
        	CheckSpriteInfo();
            MarkVertexUVDirty();
        	CalculateTiledParams();
        }
	}
    if (InPivotChange || InWidthChange || InHeightChange)
    {
        MarkVertexPositionDirty();
    }
}

void ULexTexture::CalculateTiledParams()
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

void ULexTexture::SetDrawType(ELexUISpriteDrawType Value)
{
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
void ULexTexture::SetSpriteInfo(FLexUISpriteInfo Value) 
{
	if (SpriteInfo != Value)
	{
		SpriteInfo = Value;
		MarkVertexUVDirty();
		CheckSpriteInfo();
	}
}

void ULexTexture::SetUVRect(FVector4f Value)
{
	if (UVRect != Value)
	{
		UVRect = Value;
		MarkVertexUVDirty();
		CheckSpriteInfo();
	}
}

void ULexTexture::SetPixelsPerUnitMultiplier(float Value)
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

void ULexTexture::SetFillCenter(bool Value)
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

void ULexTexture::SetTexture(UTexture* Value)
{
	if (Texture != Value)
	{
		Super::SetTexture(Value);
		CheckSpriteInfo();
	}
}

void ULexTexture::SetFillMethod(ELexUISpriteFillMethod Value)
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
void ULexTexture::SetFillOrigin(uint8 Value)
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
void ULexTexture::SetFillDirectionFlip(bool Value)
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
void ULexTexture::SetFillAmount(float Value)
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

