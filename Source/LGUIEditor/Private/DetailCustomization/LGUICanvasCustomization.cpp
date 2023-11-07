// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUICanvasCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/Actor/LGUIManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Engine/TextureRenderTarget2D.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"

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

	auto renderModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderMode));
	renderModeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));

	if (TargetScriptArray[0]->GetActualRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
	{
		if (auto world = TargetScriptArray[0]->GetWorld())
		{
			if (auto LGUIManager = ULGUIManagerWorldSubsystem::GetInstance(world))
			{
				auto& CanvasArray = LGUIManager->GetCanvasArray(ELGUIRenderMode::ScreenSpaceOverlay);
				int ScreenSpaceRootCanvasCount = 0;
				for (auto item : CanvasArray)
				{
					if (item.IsValid())
					{
						if (item->IsRootCanvas())
						{
							ScreenSpaceRootCanvasCount++;
						}
					}
				}
				if (ScreenSpaceRootCanvasCount > 1)
				{
					auto errMsg = FText::Format(LOCTEXT("MultipleScreenSpaceLGUICanvasError", "[{0}].{1} Detect multiple LGUICanvas renderred with ScreenSpaceOverlay mode, this is not allowed! There should be only one ScreenSpace UI in a world!")
					, FText::FromString(ANSI_TO_TCHAR(__FUNCTION__)), __LINE__);
					LGUIEditorUtils::ShowError(&DetailBuilder, errMsg);
				}
			}
		}
	}

	//if (TargetScriptArray[0]->GetWorld())
	//{
	//	if (!TargetScriptArray[0]->GetWorld()->IsGameWorld())
	//	{
	//		TargetScriptArray[0]->MarkCanvasUpdate();
	//	}
	//}
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	TArray<FName> needToHidePropertyNames;

	if (TargetScriptArray[0]->GetWorld() != nullptr)
	{
		category.AddCustomRow(LOCTEXT("DrawcallInfo", "DrawcallInfo"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DrawcallCountLabel", "DrawcallCount"))
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

	auto OverrideSortingHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, bOverrideSorting));
	bool bOverrideSorting;
	OverrideSortingHandle->GetValue(bOverrideSorting);
	category.AddProperty(OverrideSortingHandle);
	OverrideSortingHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));

	if (bOverrideSorting)
	{
		category.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, sortOrder)));
		//sortOrder info
		{
			category.AddCustomRow(LOCTEXT("SortOrderInfo", "SortOrderInfo"))
			.WholeRowContent()
			.MinDesiredWidth(500)
			[
				SNew(SBox)
				.HeightOverride(20)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(this, &FLGUICanvasCustomization::GetSortOrderInfo, TargetScriptArray[0])
					.AutoWrapText(true)
				]
			]
			;
		}
	}
	else
	{
		needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, sortOrder));
	}

	auto clipTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipType));
	clipTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));
	auto clipType = TargetScriptArray[0]->GetClipType();

	if (TargetScriptArray[0]->IsRootCanvas())
	{
		switch (TargetScriptArray[0]->renderMode)
		{
		case ELGUIRenderMode::ScreenSpaceOverlay:
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, depthFade));
			break;
		case ELGUIRenderMode::WorldSpace:
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, depthFade));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, bEnableDepthTest));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, previewWithLGUIRenderer));
			break;
		case ELGUIRenderMode::WorldSpace_LGUI:
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, bEnableDepthTest));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, previewWithLGUIRenderer));
			break;
		case ELGUIRenderMode::RenderTarget:
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, depthFade));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, previewWithLGUIRenderer));
			break;
		}

		if (clipType == ELGUICanvasClipType::None)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip));
		}
		else if (clipType == ELGUICanvasClipType::Rect)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip));
		}
		else if (clipType == ELGUICanvasClipType::Texture)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip));
		}
		else if (clipType == ELGUICanvasClipType::Custom)
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold));
		}
	}
	else
	{
		needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderMode));
		needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
		needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, bEnableDepthTest));
		needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, previewWithLGUIRenderer));
		if (TargetScriptArray[0]->GetRootCanvas() != nullptr)
		{
			if (
				TargetScriptArray[0]->GetRootCanvas()->renderMode == ELGUIRenderMode::WorldSpace
				|| TargetScriptArray[0]->GetRootCanvas()->renderMode == ELGUIRenderMode::WorldSpace_LGUI
				)
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
			if (clipType == ELGUICanvasClipType::None)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip));
			}
			else if (clipType == ELGUICanvasClipType::Rect)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip));
			}
			else if (clipType == ELGUICanvasClipType::Texture)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip));
			}
			else if (clipType == ELGUICanvasClipType::Custom)
			{
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
				needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold));
			}
		}
		else
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip));
		}

		if (!TargetScriptArray[0]->GetOverrideBlendDepth())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth));
		}
		if (!TargetScriptArray[0]->GetOverrideDepthFade())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, depthFade));
		}
	}

	if (!needToHidePropertyNames.Contains(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderMode)))
	{
		category.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderMode));
	}
	if (!needToHidePropertyNames.Contains(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect)))
	{
		category.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
	}
	if (!needToHidePropertyNames.Contains(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipType)))
	{
		//category.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipType));
		IDetailGroup& ClipTypeGroup = category.AddGroup(FName(TEXT("ClipType")), LOCTEXT("ClipType", "ClipType"));
		ClipTypeGroup.HeaderProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipType)));
		if (clipType == ELGUICanvasClipType::None)
		{
			
		}
		else if (clipType == ELGUICanvasClipType::Rect)
		{
			ClipTypeGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipFeather)));
			ClipTypeGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipRectOffset)));
			ClipTypeGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, inheritRectClip)));
		}
		else if (clipType == ELGUICanvasClipType::Texture)
		{
			auto ClipTextureProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTexture));
			ClipTextureProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUICanvasCustomization::ForceRefresh, &DetailBuilder));
			ClipTypeGroup.AddPropertyRow(ClipTextureProperty);
			ClipTypeGroup.AddWidgetRow()
				.ValueContent()
				[
					SNew(SBox)
					.IsEnabled(this, &FLGUICanvasCustomization::IsFixClipTextureEnabled, ClipTextureProperty)
					[
						SNew(SButton)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						.OnClicked(this, &FLGUICanvasCustomization::OnClickFixClipTextureSetting, ClipTextureProperty)
						.ToolTipText(LOCTEXT("FixTextureForHitTest_Tooltip", "\
By default we can't access texture's pixel data, which is required for line trace.\
Click this button to fix it by change texture settings.\
"))
						[
							SNew(STextBlock)
							.Text(LOCTEXT("FixTextureSettingsForHitTest", "Fix texture settings for hit test"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
				]
			;
			ClipTypeGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, clipTextureHitTestThreshold)));
		}
		else if (clipType == ELGUICanvasClipType::Custom)
		{
			ClipTypeGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, customClip)));
		}
	}

	for (auto item : needToHidePropertyNames)
	{
		DetailBuilder.HideProperty(item);
	}
}

FReply FLGUICanvasCustomization::OnClickFixClipTextureSetting(TSharedRef<IPropertyHandle> ClipTextureHandle)
{
	UObject* ClipTextureObject = nullptr;
	ClipTextureHandle->GetValue(ClipTextureObject);
	if (IsValid(ClipTextureObject))
	{
		auto clipTexture = Cast<UTexture2D>(ClipTextureObject);
		if (clipTexture->CompressionSettings != TextureCompressionSettings::TC_EditorIcon
			|| clipTexture->MipGenSettings != TextureMipGenSettings::TMGS_NoMipmaps
			)
		{
			clipTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
			clipTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			clipTexture->UpdateResource();
			clipTexture->Modify();
		}
	}

	return FReply::Handled();
}
bool FLGUICanvasCustomization::IsFixClipTextureEnabled(TSharedRef<IPropertyHandle> ClipTextureHandle)const
{
	UObject* ClipTextureObject = nullptr;
	ClipTextureHandle->GetValue(ClipTextureObject);
	if (IsValid(ClipTextureObject))
	{
		auto clipTexture = Cast<UTexture2D>(ClipTextureObject);
		if (clipTexture->CompressionSettings != TextureCompressionSettings::TC_EditorIcon
			|| clipTexture->MipGenSettings != TextureMipGenSettings::TMGS_NoMipmaps
			)
		{
			return true;
		}
	}
	return false;
}

void FLGUICanvasCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
FText FLGUICanvasCustomization::GetSortOrderInfo(TWeakObjectPtr<ULGUICanvas> TargetScript)const
{
	if (TargetScript.IsValid())
	{
		if (auto world = TargetScript->GetWorld())
		{
			if (auto LGUIManager = ULGUIManagerWorldSubsystem::GetInstance(world))
			{
				FText spaceText;
				if (TargetScript->IsRenderToScreenSpace())
				{
					spaceText = LOCTEXT("ScreenSpaceOverlay", "ScreenSpaceOverlay");
				}
				else if (TargetScript->IsRenderToWorldSpace())
				{
					if (TargetScript->IsRenderByLGUIRendererOrUERenderer())
					{
						spaceText = LOCTEXT("World Space - LGUI Renderer", "World Space - LGUI Renderer");
					}
					else
					{
						spaceText = LOCTEXT("World Space - UE Renderer", "World Space - UE Renderer");
					}
				}
				else if (TargetScript->IsRenderToRenderTarget())
				{
					if (IsValid(TargetScript->renderTarget))
					{
						spaceText = FText::Format(LOCTEXT("RenderTarget({0})", "RenderTarget({0})"), FText::FromString(TargetScript->renderTarget->GetName()));
					}
					else
					{
						spaceText = LOCTEXT("RenderTarget(NotValid)", "RenderTarget(NotValid)");
					}
				}

				auto renderMode = TargetScript->GetActualRenderMode();
				auto& itemList = LGUIManager->GetCanvasArray(renderMode);
				int sortOrderCount = 0;
				for (auto item : itemList)
				{
					if (!item.IsValid())continue;
					if (item == TargetScript)continue;

					if (item->GetSortOrder() == TargetScript->GetSortOrder())
						sortOrderCount++;
				}
				auto depthInfo = FText::Format(LOCTEXT("CanvasSortOrderTip", "All LGUICanvas of {0} with same SortOrder count: {1}\n"), spaceText, sortOrderCount);
				return depthInfo;
			}
		}
	}
	return FText::GetEmpty();
}

FText FLGUICanvasCustomization::GetDrawcallInfo()const
{
	auto LGUIManager = ULGUIManagerWorldSubsystem::GetInstance(TargetScriptArray[0]->GetWorld());
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid() && LGUIManager)
	{
		auto& allCanvas = LGUIManager->GetCanvasArray(TargetScriptArray[0]->GetRenderMode());
		int allDrawcallCount = 0;
		for (auto& canvasItem : allCanvas)
		{
			if (TargetScriptArray[0]->GetActualRenderMode() == ELGUIRenderMode::RenderTarget)
			{
				if (TargetScriptArray[0]->renderTarget == canvasItem->renderTarget && IsValid(canvasItem->renderTarget))
				{
					allDrawcallCount += canvasItem->GetDrawcallCount();
				}
			}
			else
			{
				allDrawcallCount += canvasItem->GetDrawcallCount();
			}
		}
		return FText::FromString(FString::Printf(TEXT("%d/%d"), TargetScriptArray[0]->GetDrawcallCount(), allDrawcallCount));
	}
	return FText::FromString(FString::Printf(TEXT("0/0")));
}
FText FLGUICanvasCustomization::GetDrawcallInfoTooltip()const
{
	FString spaceText;
	switch (TargetScriptArray[0]->GetActualRenderMode())
	{
	case ELGUIRenderMode::ScreenSpaceOverlay:
		spaceText = TEXT("ScreenSpaceOverlay");
		break;
	case ELGUIRenderMode::WorldSpace:
		spaceText = TEXT("WorldSpace UE Renderer");
		break;
	case ELGUIRenderMode::WorldSpace_LGUI:
		spaceText = TEXT("WorldSpace LGUI Renderer");
		break;
	case ELGUIRenderMode::RenderTarget:
		if (IsValid(TargetScriptArray[0]->renderTarget))
		{
			spaceText = FString::Printf(TEXT("RenderTarget(%s)"), *(TargetScriptArray[0]->renderTarget->GetName()));
		}
		else
		{
			spaceText = FString::Printf(TEXT("RenderTarget(NotValid)"));
		}
		break;
	}

	if (auto LGUIManager = ULGUIManagerWorldSubsystem::GetInstance(TargetScriptArray[0]->GetWorld()))
	{
		auto& allCanvas = LGUIManager->GetCanvasArray(TargetScriptArray[0]->GetActualRenderMode());
		int allDrawcallCount = 0;
		for (auto& canvasItem : allCanvas)
		{
			if (TargetScriptArray[0]->GetActualRenderMode() == ELGUIRenderMode::RenderTarget)
			{
				if (TargetScriptArray[0]->renderTarget == canvasItem->renderTarget && IsValid(canvasItem->renderTarget))
				{
					allDrawcallCount += canvasItem->GetDrawcallCount();
				}
			}
			else
			{
				allDrawcallCount += canvasItem->GetDrawcallCount();
			}
		}
		auto tooltipStr = FText::Format(LOCTEXT("DrawcallInfoTooltip", "This canvas's drawcall count:{0}, all canvas of {1} drawcall count:{2}")
			, TargetScriptArray[0]->GetDrawcallCount(), FText::FromString(spaceText), allDrawcallCount);
		return tooltipStr;
	}
	return FText::GetEmpty();
}
void FLGUICanvasCustomization::OnCopySortOrder()
{
	if (TargetScriptArray.Num() > 0)
	{
		if (TargetScriptArray[0].IsValid())
		{
			FPlatformApplicationMisc::ClipboardCopy(*FString::Printf(TEXT("%d"), TargetScriptArray[0]->GetSortOrder()));
		}
	}
}
void FLGUICanvasCustomization::OnPasteSortOrder(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	if (PastedText.IsNumeric())
	{
		int value = FCString::Atoi(*PastedText);
		PropertyHandle->SetValue(value);
	}
}
#undef LOCTEXT_NAMESPACE