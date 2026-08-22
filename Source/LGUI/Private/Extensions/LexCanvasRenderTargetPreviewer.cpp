// Copyright 2019-Present LexLiu. All Rights Reserved.


#include "Extensions/LexCanvasRenderTargetPreviewer.h"
#include "Core/LexUIGeometry.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexVisualPostProcess.h"
#include "Core/Components/LexWidget.h"
#include "Engine/TextureRenderTarget2D.h"

void ULexCanvasRenderTargetPreviewer::BeginPlay()
{
	Super::BeginPlay();
	if (!bHasRegisterRenderTargetChangedEvent)
	{
		RegisterRenderTargetChangedEvent();
	}
}

void ULexCanvasRenderTargetPreviewer::EndPlay()
{
	Super::EndPlay();
	if (bHasRegisterRenderTargetChangedEvent)
	{
		UnregisterRenderTargetChangedEvent();
	}
}

void ULexCanvasRenderTargetPreviewer::OnRegister()
{
	Super::OnRegister();
	if (!bHasRegisterRenderTargetChangedEvent)
	{
		RegisterRenderTargetChangedEvent();
	}
}

void ULexCanvasRenderTargetPreviewer::OnUnregister()
{
	Super::OnUnregister();
	if (bHasRegisterRenderTargetChangedEvent)
	{
		UnregisterRenderTargetChangedEvent();
	}
}

#if WITH_EDITOR
void ULexCanvasRenderTargetPreviewer::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	auto PropName = PropertyAboutToChange->GetFName();
	if (PropName == GET_MEMBER_NAME_CHECKED(ULexCanvasRenderTargetPreviewer, Canvas))
	{
		UnregisterRenderTargetChangedEvent();
	}
}
void ULexCanvasRenderTargetPreviewer::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropName = Property->GetFName();
		if (PropName == GET_MEMBER_NAME_CHECKED(ULexCanvasRenderTargetPreviewer, Canvas))
		{
			RegisterRenderTargetChangedEvent();
			MarkTextureDirty();
			UpdateSpriteData();
		}
	}
}
#endif


void ULexCanvasRenderTargetPreviewer::RegisterRenderTargetChangedEvent()
{
	if (bHasRegisterRenderTargetChangedEvent)return;
	if (Canvas.IsValid())
	{
		bHasRegisterRenderTargetChangedEvent = true;
		Canvas->GetRenderTargetChangedEvent().AddLambda([=, WeakThis = MakeWeakObjectPtr(this)](UTextureRenderTarget2D*)
		{
			if (!WeakThis.IsValid())return;
			WeakThis->MarkTextureDirty();
			WeakThis->MarkVerticesDirty(true, true, true, true);
			WeakThis->UpdateSpriteData();
		});
		MarkTextureDirty();
		MarkVerticesDirty(true, true, true, true);
		UpdateSpriteData();
	}
}

void ULexCanvasRenderTargetPreviewer::UnregisterRenderTargetChangedEvent()
{
	if (!bHasRegisterRenderTargetChangedEvent)return;
	bHasRegisterRenderTargetChangedEvent = false;
	if (Canvas.IsValid())
	{
		Canvas->GetRenderTargetChangedEvent().RemoveAll(this);
		MarkTextureDirty();
	}
}

void ULexCanvasRenderTargetPreviewer::UpdateSpriteData()
{
	if (Canvas.IsValid())
	{
		if (auto RenderTarget = Canvas->GetRenderTarget())
		{
			SpriteInfo.Width = RenderTarget->SizeX;
			SpriteInfo.Height = RenderTarget->SizeY;
			
			SpriteInfo.MinUV.X = 0;
			SpriteInfo.MinUV.Y = 0;
			SpriteInfo.MaxUV.Y = 1;
			SpriteInfo.MaxUV.X = 1;

			MarkVertexUVDirty();
		}
	}
}

void ULexCanvasRenderTargetPreviewer::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
	Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
	UpdateSpriteData();
}

void ULexCanvasRenderTargetPreviewer::OnTransformChanged(bool InPositionChanged, bool InScaleChanged)
{
	Super::OnTransformChanged(InPositionChanged, InScaleChanged);
	UpdateSpriteData();
}

UTexture* ULexCanvasRenderTargetPreviewer::GetTextureToCreateGeometry()
{
	if (Canvas.IsValid())
	{
		if (Canvas->GetActualRenderMode() == ELexRenderMode::RenderTarget)
		{
			return Canvas->GetRenderTarget();
		}
	}
	return nullptr;
}

UMaterialInterface* ULexCanvasRenderTargetPreviewer::GetMaterialToCreateGeometry()
{
	return Material;
}

void ULexCanvasRenderTargetPreviewer::OnBeforeCreateOrUpdateGeometry()
{
	if (!bHasRegisterRenderTargetChangedEvent)
	{
		RegisterRenderTargetChangedEvent();
	}
}

void ULexCanvasRenderTargetPreviewer::OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged,
                                                    bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InGeo.Texture != nullptr)
	{
		auto Widget = GetWidget();
		auto RenderCanvas = Widget->GetRenderCanvas();
		FLexUIGeometry::UpdateUIRectSimpleVertex(&InGeo,
				Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), SpriteInfo, RenderCanvas, this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
	}
	else
	{
		InGeo.Clear();
	}
}
