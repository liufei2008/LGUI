// Copyright 2019-Present LexLiu. All Rights Reserved.


#include "Extensions/LexPostProcessRenderElement_Text.h"

#include "Core/LexUIGeometry.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexVisualPostProcess.h"
#include "Core/Components/LexWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Extensions/LexPostProcessRenderElement.h"

void ULexPostProcessRenderElement_Text::BeginPlay()
{
	Super::BeginPlay();
	RegisterPostProcessChangedEvent();
}

void ULexPostProcessRenderElement_Text::EndPlay()
{
	Super::EndPlay();
	UnregisterPostProcessChangedEvent();
}

#if WITH_EDITOR
void ULexPostProcessRenderElement_Text::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	auto PropName = PropertyAboutToChange->GetFName();
	if (PropName == GET_MEMBER_NAME_CHECKED(ULexPostProcessRenderElement_Text, PostProcess))
	{
		UnregisterPostProcessChangedEvent();
	}
}
void ULexPostProcessRenderElement_Text::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropName = Property->GetFName();
		if (PropName == GET_MEMBER_NAME_CHECKED(ULexPostProcessRenderElement_Text, PostProcess))
		{
			RegisterPostProcessChangedEvent();
			MarkTextureDirty();
		}
		else if (PropName == GET_MEMBER_NAME_CHECKED(ULexPostProcessRenderElement_Text, OverrideMaterial))
		{
			MaterialInstanceDynamic = nullptr;
			MarkMaterialDirty();
		}
	}
}
#endif

void ULexPostProcessRenderElement_Text::RegisterPostProcessChangedEvent()
{
	if (bHasRegisterPostProcessChangedEvent)return;
	if (PostProcess.IsValid())
	{
		bHasRegisterPostProcessChangedEvent = true;
		PostProcess->GetWidget()->GetDimensionChangedEvent().AddWeakLambda(this, [=, this](bool, bool, bool)
		{
			MarkCanvasUpdate();
		});
		PostProcess->GetWidget()->GetTransformChangedEvent().AddWeakLambda(this, [=, this]()
		{
			MarkCanvasUpdate();
		});
		PostProcess->GetRenderTargetChangedEvent().AddWeakLambda(this, [=, this](UTextureRenderTarget2D*)
		{
			MarkTextureDirty();
			MarkMaterialDirty();
		});
	}
}

void ULexPostProcessRenderElement_Text::UnregisterPostProcessChangedEvent()
{
	if (!bHasRegisterPostProcessChangedEvent)return;
	bHasRegisterPostProcessChangedEvent = false;
	if (PostProcess.IsValid() && PostProcess->GetWidget())
	{
		PostProcess->GetWidget()->GetDimensionChangedEvent().RemoveAll(this);
		PostProcess->GetWidget()->GetTransformChangedEvent().RemoveAll(this);
		PostProcess->GetRenderTargetChangedEvent().RemoveAll(this);
	}
}

void ULexPostProcessRenderElement_Text::SetMaterialParameter()
{
	if (PostProcess.IsValid())
	{
		CheckMaterialInstanceDynamic();
		if (IsValid(MaterialInstanceDynamic))
		{
			if (PostProcess.IsValid())
			{
				if (PostProcess->GetRenderType() == ELexVisualPostProcessRenderType::RenderTarget)
				{
					MaterialInstanceDynamic->SetTextureParameterValue(LexUI_PostProcessTexture, PostProcess->GetOutputRenderTarget());
					ULexPostProcessRenderElement::SetMaterialMatrixProperty(PostProcess.Get(), MaterialInstanceDynamic);
				}
			}
		}
	}
}

void ULexPostProcessRenderElement_Text::CheckMaterialInstanceDynamic()
{
	if (!IsValid(MaterialInstanceDynamic))
	{
		if (IsValid(OverrideMaterial))
		{
			MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(OverrideMaterial, this);
			MaterialInstanceDynamic->SetFlags(RF_Transient);
		}
	}
}

void ULexPostProcessRenderElement_Text::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
	Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
}

void ULexPostProcessRenderElement_Text::OnTransformChanged(bool InPositionChanged, bool InScaleChanged)
{
	Super::OnTransformChanged(InPositionChanged, InScaleChanged);
}

UTexture* ULexPostProcessRenderElement_Text::GetTextureToCreateGeometry()
{
	return Super::GetTextureToCreateGeometry();
}

FName ULexPostProcessRenderElement_Text::LexUI_PostProcessTexture = FName(TEXT("LexUI_PostProcessTexture"));
UMaterialInterface* ULexPostProcessRenderElement_Text::GetMaterialToCreateGeometry()
{
	CheckMaterialInstanceDynamic();
	return MaterialInstanceDynamic;
}

void ULexPostProcessRenderElement_Text::OnBeforeCreateOrUpdateGeometry()
{
	Super::OnBeforeCreateOrUpdateGeometry();
	RegisterPostProcessChangedEvent();
	SetMaterialParameter();//this will set parameter no mater geometry changes
}

void ULexPostProcessRenderElement_Text::OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged,
                                                    bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	Super::OnUpdateGeometry(InGeo, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
}
