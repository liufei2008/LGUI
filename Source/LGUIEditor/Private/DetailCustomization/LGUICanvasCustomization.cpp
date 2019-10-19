// Copyright 2019 LexLiu. All Rights Reserved.

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
	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptArray[0]);

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
		uint8 renderMode;
		renderModeHandle->GetValue(renderMode);
		if (renderMode == (uint8)ELGUIRenderMode::ScreenSpaceOverlay)
		{
			
		}
		else if (renderMode == (uint8)ELGUIRenderMode::WorldSpace)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
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
		if (TargetScriptArray[0]->GetRootCanvas()->renderMode == ELGUIRenderMode::WorldSpace)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
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

	for (auto item : needToHidePropertyNames)
	{
		DetailBuilder.HideProperty(item);
	}

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
void FLGUICanvasCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
FText FLGUICanvasCustomization::GetDrawcallInfo()const
{
	int drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
	auto& allCanvas = LGUIManager::GetAllCanvas(TargetScriptArray[0]->GetWorld());
	int allDrawcallCount = 0;
	for (ULGUICanvas* canvasItem : allCanvas)
	{
		if (TargetScriptArray[0]->IsScreenSpaceOverlayUI() == canvasItem->IsScreenSpaceOverlayUI())
		{
			allDrawcallCount += canvasItem->UIDrawcallList.Num();
		}
	}
	return FText::FromString(FString::Printf(TEXT("%d/%d"), drawcallCount, allDrawcallCount));
}
FText FLGUICanvasCustomization::GetDrawcallInfoTooltip()const
{
	int drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
	auto& allCanvas = LGUIManager::GetAllCanvas(TargetScriptArray[0]->GetWorld());
	int allDrawcallCount = 0;
	for (ULGUICanvas* canvasItem : allCanvas)
	{
		if (TargetScriptArray[0]->IsScreenSpaceOverlayUI() == canvasItem->IsScreenSpaceOverlayUI())
		{
			allDrawcallCount += canvasItem->UIDrawcallList.Num();
		}
	}
	FString tooltipStr = FString::Printf(TEXT("This canvas's drawcall count:%d, all canvas of %s drawcall count:%d"), drawcallCount, (TargetScriptArray[0]->IsScreenSpaceOverlayUI() ? TEXT("screen space") : TEXT("world space")), allDrawcallCount);
	return FText::FromString(tooltipStr);
}
#undef LOCTEXT_NAMESPACE