// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUICanvasCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/Actor/LGUIManagerActor.h"

#define LOCTEXT_NAMESPACE "LGUICanvasCustomization"
FLGUICanvasCustomization::FLGUICanvasCustomization()
{
}

FLGUICanvasCustomization::~FLGUICanvasCustomization()
{
}

TSharedRef<IDetailCustomization> FLGUICanvasCustomization::MakeInstance()
{
	return MakeShareable(new FLGUICanvasCustomization);
}
void FLGUICanvasCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<ULGUICanvas>(item.Get()))
		{
			TargetScriptArray.Add(validItem);
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[FLGUICanvasCustomization]Get TargetScript is null"));
		return;
	}

	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptArray[0].Get());

	if (TargetScriptArray[0]->GetWorld())
	{
		if (!TargetScriptArray[0]->GetWorld()->IsGameWorld())
		{
			TargetScriptArray[0]->MarkCanvasUpdate();
			TargetScriptArray[0]->MarkRebuildAllDrawcall();
		}
	}
	TargetScriptArray[0]->TopMostCanvas = nullptr;
	TargetScriptArray[0]->CheckTopMostCanvas();
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	TArray<FName> needToHidePropertyNames;
	if (TargetScriptArray[0]->IsRootCanvas())
	{
		auto renderModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderMode));
		renderModeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));
		switch (TargetScriptArray[0]->renderMode)
		{
		case ELGUIRenderMode::ScreenSpaceOverlay:
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
			break;
		case ELGUIRenderMode::WorldSpace:
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
			break;
		case ELGUIRenderMode::RenderTarget:
			break;
		}

		auto clipTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipType));
		clipTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));
		auto clipType = TargetScriptArray[0]->GetClipType();
		if (clipType == ELGUICanvasClipType::None)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
		}
		else if (clipType == ELGUICanvasClipType::Rect)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
		}
		else if (clipType == ELGUICanvasClipType::Texture)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
		}
	}
	else
	{
		needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderMode));
		needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
		if (TargetScriptArray[0]->GetRootCanvas() != nullptr)
		{
			if (TargetScriptArray[0]->GetRootCanvas()->renderMode == ELGUIRenderMode::WorldSpace)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
			}
		}

		auto overrideParametersHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, overrideParameters));
		overrideParametersHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));
		if (!TargetScriptArray[0]->GetOverrideDefaultMaterials())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, DefaultMaterials));
		}
		if (!TargetScriptArray[0]->GetOverridePixelPerfect())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
		}
		if (!TargetScriptArray[0]->GetOverrideDynamicPixelsPerUnit())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, dynamicPixelsPerUnit));
		}
		if (!TargetScriptArray[0]->GetOverrideClipType())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipType));
		}
		if (!TargetScriptArray[0]->GetOverrideAddionalShaderChannel())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, additionalShaderChannels));
		}

		if (TargetScriptArray[0]->GetOverrideClipType())
		{
			auto clipTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipType));
			clipTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));
			auto clipType = TargetScriptArray[0]->GetClipType();
			if (clipType == ELGUICanvasClipType::None)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
			}
			else if (clipType == ELGUICanvasClipType::Rect)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			}
			else if (clipType == ELGUICanvasClipType::Texture)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
			}
		}
		else
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
		}
	}

	if (TargetScriptArray[0]->GetWorld() != nullptr)
	{
		category.AddCustomRow(LOCTEXT("DrawcallInfo", "DrawcallInfo"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("DrawcallCount")))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ColorAndOpacity(FLinearColor(FColor::Green))
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(this, &FLGUICanvasCustomization::GetDrawcallInfo)
			.ToolTipText(this, &FLGUICanvasCustomization::GetDrawcallInfoTooltip)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ColorAndOpacity(FLinearColor(FColor::Green))
		]
		;
	}
	if (!needToHidePropertyNames.Contains(FName(TEXT("renderMode"))))
	{
		category.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderMode));//show before sortOrder
	}
	//category.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
	auto sortOrderHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, sortOrder));
	category.AddCustomRow(LOCTEXT("SortOrderManager", "SortOrderManager"))
		.NameContent()
		[
			sortOrderHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.Padding(2, 4)
			.FillWidth(5)
			[
				sortOrderHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 6)
			.FillWidth(2)
			[
				SNew(SButton)
				.Text(LOCTEXT("Up", "+"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.OnClicked_Lambda([=]()
				{
					sortOrderHandle->SetValue(TargetScriptArray[0]->GetSortOrder() + 1);
					return FReply::Handled(); 
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 6)
			.FillWidth(2)
			[
				SNew(SButton)
				.Text(LOCTEXT("Down", "-"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.OnClicked_Lambda([=]()
				{
					sortOrderHandle->SetValue(TargetScriptArray[0]->GetSortOrder() - 1);
					return FReply::Handled();
				})
			]
		];

	needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, sortOrder));
	for (auto item : needToHidePropertyNames)
	{
		DetailBuilder.HideProperty(item);
	}
}
void FLGUICanvasCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
FText FLGUICanvasCustomization::GetDrawcallInfo()const
{
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
	{
		int drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
		auto& allCanvas = LGUIManager::GetAllCanvas(TargetScriptArray[0]->GetWorld());
		int allDrawcallCount = 0;
		for (ULGUICanvas* canvasItem : allCanvas)
		{
			switch (TargetScriptArray[0]->renderMode)
			{
			case ELGUIRenderMode::WorldSpace:
			case ELGUIRenderMode::ScreenSpaceOverlay:
			{
				if (canvasItem->renderMode == TargetScriptArray[0]->renderMode)
				{
					allDrawcallCount += canvasItem->UIDrawcallList.Num();
				}
			}
			break;
			case ELGUIRenderMode::RenderTarget:
			{
				if (canvasItem->renderMode == ELGUIRenderMode::RenderTarget)
				{
					if (TargetScriptArray[0]->renderTarget == canvasItem->renderTarget && IsValid(canvasItem->renderTarget))
					{
						allDrawcallCount += canvasItem->UIDrawcallList.Num();
					}
				}
			}
			break;
			}
		}
		return FText::FromString(FString::Printf(TEXT("%d/%d"), drawcallCount, allDrawcallCount));
	}
	return FText::FromString(FString::Printf(TEXT("0/0")));
}
FText FLGUICanvasCustomization::GetDrawcallInfoTooltip()const
{
	int drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
	auto& allCanvas = LGUIManager::GetAllCanvas(TargetScriptArray[0]->GetWorld());
	int allDrawcallCount = 0;
	FString spaceText;
	switch (TargetScriptArray[0]->renderMode)
	{
	case ELGUIRenderMode::WorldSpace:
	{
		spaceText = TEXT("WorldSpace");
	}
	break;
	case ELGUIRenderMode::ScreenSpaceOverlay:
	{
		spaceText = TEXT("ScreenSpaceOverlay");
	}
	break;
	case ELGUIRenderMode::RenderTarget:
	{
		if (IsValid(TargetScriptArray[0]->renderTarget))
		{
			spaceText = FString::Printf(TEXT("RenderTarget(%s)"), *(TargetScriptArray[0]->renderTarget->GetName()));
		}
		else
		{
			spaceText = FString::Printf(TEXT("RenderTarget(NotValid)"));
		}
	}
	break;
	}
	for (ULGUICanvas* canvasItem : allCanvas)
	{
		switch (TargetScriptArray[0]->renderMode)
		{
		case ELGUIRenderMode::WorldSpace:
		case ELGUIRenderMode::ScreenSpaceOverlay:
		{
			if (canvasItem->renderMode == TargetScriptArray[0]->renderMode)
			{
				allDrawcallCount += canvasItem->UIDrawcallList.Num();
			}
		}
		break;
		case ELGUIRenderMode::RenderTarget:
		{
			if (canvasItem->renderMode == ELGUIRenderMode::RenderTarget)
			{
				if (TargetScriptArray[0]->renderTarget == canvasItem->renderTarget && IsValid(canvasItem->renderTarget))
				{
					allDrawcallCount += canvasItem->UIDrawcallList.Num();
				}
			}
		}
		break;
		}
	}
	FString tooltipStr = FString::Printf(TEXT("This canvas's drawcall count:%d, all canvas of %s drawcall count:%d"), drawcallCount, *spaceText, allDrawcallCount);
	return FText::FromString(tooltipStr);
}
#undef LOCTEXT_NAMESPACE