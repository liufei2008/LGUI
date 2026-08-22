// Copyright 2025-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexImage.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"
#include "Core/LexUIImageBrush.h"
#include "Core/LexUIGeometry.h"
#include "Core/LexUISpriteData.h"
#include "Slate/SlateTextureAtlasInterface.h"
#include "Utils/LexUIUtils.h"

#if WITH_EDITOR
void ULexImage::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	const FName PropertyName = PropertyAboutToChange->GetFName();
	if (PropertyName == FLexUIImageBrush::GetPropertyName_ResourceObject())
	{
		UnregisterFromSprite();
	}
}
void ULexImage::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName MemberName = PropertyChangedEvent.GetMemberPropertyName();
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	static const FName BrushName = GET_MEMBER_NAME_CHECKED(ULexImage, Brush);

	if (MemberName == BrushName || PropertyName == FLexUIImageBrush::GetPropertyName_ResourceObject())
	{
		if (auto Widget = GetWidget())
		{
			ULexWidget::MarkLayoutForRebuild(Widget);
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(FLexUIImageBrush, FlipMode))
	{
		MarkVerticesDirty(false, true, true, false);
	}
}
#endif

void ULexImage::UnregisterFromSprite()
{
	if (bHasAddToSprite)
	{
		if (auto LexSprite = Cast<ULexUISpriteData_BaseObject>(Brush.GetResourceObject()))
		{
			LexSprite->RemoveUISprite(this);
			bHasAddToSprite = false;
		}
	}
}

ULexUISpriteData_BaseObject* ULexImage::SpriteRenderGetSprite_Implementation() const
{
	if (auto LexSprite = Cast<ULexUISpriteData_BaseObject>(Brush.GetResourceObject()))
	{
		return LexSprite;
	}
	return nullptr;
}

void ULexImage::ApplyAtlasTextureChange_Implementation()
{
	check(bHasAddToSprite);
	auto LexSprite = (ULexUISpriteData_BaseObject*)Brush.GetResourceObject();
	UIGeometry->Texture = LexSprite->GetAtlasTexture();
	GetWidget()->MarkCanvasUpdate(true);
}

ULexImage::ULexImage(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	Brush.SetResourceObject(ULexUISpriteData::GetDefaultWhiteSolid());
	Brush.DrawAs = ELexUIImageBrushDrawType::Image;
}

UTexture* ULexImage::GetTextureToCreateGeometry()
{
	if (auto Texture = Cast<UTexture>(Brush.GetResourceObject()))
	{
		return Texture;
	}
	if (auto SlateTextureAtlas = Cast<ISlateTextureAtlasInterface>(Brush.GetResourceObject()))
	{
		return SlateTextureAtlas->GetSlateAtlasData().AtlasTexture;
	}
	if (auto LexSprite = Cast<ULexUISpriteData_BaseObject>(Brush.GetResourceObject()))
	{
		if (!bHasAddToSprite)
		{
			LexSprite->AddUISprite(this);
			bHasAddToSprite = true;
		}
		return LexSprite->GetAtlasTexture();
	}
	return FLexUIUtils::GetDefaultWhiteTexture();
}
UMaterialInterface* ULexImage::GetMaterialToCreateGeometry()
{
	if (auto Material = Cast<UMaterialInterface>(Brush.GetResourceObject()))
	{
		return Material;
	}
	return nullptr;
}

void ULexImage::OnUpdateGeometry(FLexUIGeometry& InMesh, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	auto Widget = this->GetWidget();
	auto RenderSize = Widget->GetSize();
	auto Pivot = Widget->GetPivot();
	auto RenderCanvas = Widget->GetRenderCanvas();
	auto FinalColor = FLexUIUtils::MultiplyColor(Brush.TintColor, this->GetFinalColor());

	bool bFlipH = false, bFlipV = false;
	switch (Brush.FlipMode)
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

	switch (Brush.DrawAs)
	{
	case ELexUIImageBrushDrawType::None:
		return;
	case ELexUIImageBrushDrawType::Image:
		{
			DRAW_AS_IMAGE:
			FLexUISpriteInfo SpriteInfo;
			if (bHasAddToSprite)
			{
				auto LexSprite = (ULexUISpriteData_BaseObject*)Brush.GetResourceObject();
				SpriteInfo = LexSprite->GetSpriteInfo();
			}
			else
			{
				SpriteInfo.Width = Brush.ImageSize.X;
				SpriteInfo.Height = Brush.ImageSize.Y;
				SpriteInfo.ApplyUV(0, 0, SpriteInfo.Width, SpriteInfo.Height, 1.0f / SpriteInfo.Width, 1.0f / SpriteInfo.Height, Brush.UVRegion);
			}
			FLexUISpriteInfo::ApplyFlip(SpriteInfo, bFlipH, bFlipV);
			FLexUIGeometry::UpdateUIRectSimpleVertex(&InMesh, RenderSize.X, RenderSize.Y, FVector2f(Pivot)
			, SpriteInfo, RenderCanvas, this, FinalColor
			, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
		}
		break;
	case ELexUIImageBrushDrawType::Border:
	case ELexUIImageBrushDrawType::Box:
		{
			if (bHasAddToSprite)
			{
				auto LexSprite = (ULexUISpriteData_BaseObject*)Brush.GetResourceObject();
				if (!LexSprite->GetSpriteInfo().HasBorder())
					goto DRAW_AS_IMAGE;
			}
			else
			{
				if (Brush.Margin.Left == 0 && Brush.Margin.Right == 0 && Brush.Margin.Top == 0 && Brush.Margin.Bottom == 0)
					goto DRAW_AS_IMAGE;
			}
			bool bFillCenter = Brush.DrawAs == ELexUIImageBrushDrawType::Box;
			FLexUISpriteInfo SpriteInfo;
			if (bHasAddToSprite)
			{
				auto LexSprite = (ULexUISpriteData_BaseObject*)Brush.GetResourceObject();
				SpriteInfo = LexSprite->GetSpriteInfo();
			}
			else
			{
				SpriteInfo.Width = Brush.ImageSize.X;
				SpriteInfo.Height = Brush.ImageSize.Y;
				SpriteInfo.Border.Left = Brush.Margin.Left * Brush.ImageSize.X;
				SpriteInfo.Border.Right = Brush.Margin.Right * Brush.ImageSize.X;
				SpriteInfo.Border.Top = Brush.Margin.Top * Brush.ImageSize.Y;
				SpriteInfo.Border.Bottom = Brush.Margin.Bottom * Brush.ImageSize.Y;
				SpriteInfo.ApplyUV(0, 0, SpriteInfo.Width, SpriteInfo.Height, 1.0f / SpriteInfo.Width, 1.0f / SpriteInfo.Height, Brush.UVRegion);
				SpriteInfo.ApplyBorderUV(1.0f / SpriteInfo.Width, 1.0f / SpriteInfo.Height);
			}
			FLexUISpriteInfo::ApplyFlip(SpriteInfo, bFlipH, bFlipV);
			FLexUIGeometry::UpdateUIRectBorderVertex(&InMesh, bFillCenter, RenderSize.X, RenderSize.Y, FVector2f(Pivot)
				, SpriteInfo, RenderCanvas, this, FinalColor
				, Brush.PixelsPerUnitMultiplier
				, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
		}
		break;
	}
}

void ULexImage::PostInitProperties()
{
	Super::PostInitProperties();
}

void ULexImage::BeginDestroy()
{
	Super::BeginDestroy();
	//unregister from sprite
	UnregisterFromSprite();
}

void ULexImage::OnRegister()
{
	Super::OnRegister();
	if (auto LexSprite = Cast<ULexUISpriteData_BaseObject>(Brush.GetResourceObject()))
	{
		if (!bHasAddToSprite)
		{
			LexSprite->AddUISprite(this);
			bHasAddToSprite = true;
		}
	}
}

void ULexImage::OnUnregister()
{
	Super::OnUnregister();
	UnregisterFromSprite();
}

void ULexImage::SetBrush(const FLexUIImageBrush& Value)
{
	UnregisterFromSprite();
	
	MarkVerticesDirty(true, true, true, Brush.TintColor != Value.TintColor);
	MarkTextureDirty();
	MarkMaterialDirty();
	Brush = Value;
	ULexWidget::MarkLayoutForRebuild(GetWidget());
}

void ULexImage::SetBrush_LexUISprite(ULexUISpriteData_BaseObject* Value)
{
	auto OldLexSprite = Cast<ULexUISpriteData_BaseObject>(Brush.GetResourceObject());
	auto NewLexSprite = Value;
	//handle Sprite
	if (OldLexSprite != nullptr && NewLexSprite != nullptr)
	{
		if(OldLexSprite->GetAtlasTexture() != NewLexSprite->GetAtlasTexture())
		{
			//remove from old
			if (bHasAddToSprite)
			{
				OldLexSprite->RemoveUISprite(this);
				bHasAddToSprite = false;
			}
			//add to new
			{
				NewLexSprite->AddUISprite(this);
				bHasAddToSprite = true;
			}
			MarkTextureDirty();
		}
		MarkVertexUVDirty();
		Brush.SetResourceObject(Value);
		ULexWidget::MarkLayoutForRebuild(GetWidget());
		return;
	}
	if (OldLexSprite != nullptr)
	{
		//remove from old
		if (bHasAddToSprite)
		{
			OldLexSprite->RemoveUISprite(this);
			bHasAddToSprite = false;
		}
	}
	if (NewLexSprite != nullptr)
	{
		NewLexSprite->AddUISprite(this);
		bHasAddToSprite = true;
	}
	MarkVerticesDirty(false, false, true, false);
	MarkTextureDirty();
	if (Cast<UMaterialInterface>(Brush.GetResourceObject()) != nullptr)//if old brush is material then mark material dirty
	{
		MarkMaterialDirty();
	}
	Brush.SetResourceObject(Value);
	ULexWidget::MarkLayoutForRebuild(GetWidget());
}

void ULexImage::SetBrush_SlateSprite(TScriptInterface<ISlateTextureAtlasInterface> Value)
{
	//remove from old sprite
	UnregisterFromSprite();
	
	auto OldSlateSprite = Cast<ISlateTextureAtlasInterface>(Brush.GetResourceObject());
	auto NewSlateSprite = Value;
	if (OldSlateSprite != nullptr && NewSlateSprite != nullptr)
	{
		if (OldSlateSprite->GetSlateAtlasData().AtlasTexture != NewSlateSprite->GetSlateAtlasData().AtlasTexture)
		{
			MarkTextureDirty();
		}
		MarkVertexUVDirty();
		Brush.SetResourceObject(Value.GetObject());
		ULexWidget::MarkLayoutForRebuild(GetWidget());
		return;
	}

	MarkVerticesDirty(true, true, true, false);
	MarkTextureDirty();
	if (Cast<UMaterialInterface>(Brush.GetResourceObject()) != nullptr)//if old brush is material then mark material dirty
	{
		MarkMaterialDirty();
	}
	Brush.SetResourceObject(Value.GetObject());
	ULexWidget::MarkLayoutForRebuild(GetWidget());
}

void ULexImage::SetBrush_Texture(UTexture* Value)
{
	//remove from old sprite
	UnregisterFromSprite();
	
	MarkVerticesDirty(true, true, true, false);
	MarkTextureDirty();
	if (Cast<UMaterialInterface>(Brush.GetResourceObject()) != nullptr)//if old brush is material then mark material dirty
	{
		MarkMaterialDirty();
	}
	Brush.SetResourceObject(Value);
	ULexWidget::MarkLayoutForRebuild(GetWidget());
}
void ULexImage::SetBrush_Material(UTexture* Value)
{
	//remove from old sprite
	UnregisterFromSprite();
	
	MarkVerticesDirty(true, true, true, false);
	MarkTextureDirty();
	MarkMaterialDirty();
	Brush.SetResourceObject(Value);
	ULexWidget::MarkLayoutForRebuild(GetWidget());
}

void ULexImage::SetBrushTintColor(FColor Value)
{
	if (Brush.TintColor != Value)
	{
		Brush.TintColor = Value;
		MarkColorDirty();
	}
}

float ULexImage::GetPreferredWidth() const
{
	return Brush.ImageSize.X;
}

float ULexImage::GetPreferredHeight() const
{
	return Brush.ImageSize.Y;
}
