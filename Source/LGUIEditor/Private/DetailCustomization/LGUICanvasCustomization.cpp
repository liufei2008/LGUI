// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUICanvasCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LGUIEditorPCH.H"

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
			TArray<TWeakObjectPtr<ULGUICanvas>> allCanvasArray;
			if (!TargetScriptArray[0]->GetWorld()->IsGameWorld())
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					allCanvasArray = ULGUIEditorManagerObject::Instance->GetCanvasArray();
				}
			}
			else
			{
				if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(world))
				{
					allCanvasArray = LGUIManagerActor->GetCanvasArray();
				}
			}
			int screenSpaceCanvasCount = 0;
			for (auto item : allCanvasArray)
			{
				if (item.IsValid())
				{
					if (item->IsRootCanvas())
					{
						if (item->GetWorld() == world)
						{
							if (item->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
							{
								screenSpaceCanvasCount++;
							}
						}
					}
				}
			}
			if (screenSpaceCanvasCount > 1)
			{
				auto errMsg = FString::Printf(TEXT("Detect multiply LGUICanvas renderred with ScreenSpaceOverlay mode, this is not allowed! There should be only one ScreenSpace UI in a world!"));
				LGUIEditorUtils::ShowError(&DetailBuilder, errMsg);
			}
		}
	}

	if (TargetScriptArray[0]->GetWorld())
	{
		if (!TargetScriptArray[0]->GetWorld()->IsGameWorld())
		{
			TargetScriptArray[0]->MarkCanvasUpdate();
			TargetScriptArray[0]->MarkRebuildAllDrawcall();
		}
	}
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	TArray<FName> needToHidePropertyNames;
	if (TargetScriptArray[0]->IsRootCanvas())
	{
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

		if (!TargetScriptArray[0]->GetOverrideOwnerNoSee())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, ownerNoSee));
		}
		if (!TargetScriptArray[0]->GetOverrideOnlyOwnerSee())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, onlyOwnerSee));
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

	auto sortOrderHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvas, sortOrder));
	category.AddCustomRow(LOCTEXT("SortOrderManager", "SortOrderManager"))
		.CopyAction(FUIAction(
			FExecuteAction::CreateSP(this, &FLGUICanvasCustomization::OnCopySortOrder)
		))
		.PasteAction(FUIAction(
			FExecuteAction::CreateSP(this, &FLGUICanvasCustomization::OnPasteSortOrder, sortOrderHandle)
		))
		.NameContent()
		[
			sortOrderHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(5)
			[
				sortOrderHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(2)
			[
				SNew(SBox)
				.HeightOverride(18)
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
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(2)
			[
				SNew(SBox)
				.HeightOverride(18)
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
			]
		];

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
FText FLGUICanvasCustomization::GetSortOrderInfo(TWeakObjectPtr<ULGUICanvas> TargetScript)const
{
	if (TargetScript.IsValid())
	{
		if (auto world = TargetScript->GetWorld())
		{
			TArray<TWeakObjectPtr<ULGUICanvas>> itemList;
			if (world->IsGameWorld())
			{
				if (auto instance = ALGUIManagerActor::GetLGUIManagerActorInstance(world))
				{
					itemList = instance->GetCanvasArray();
				}
			}
			else
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					itemList = ULGUIEditorManagerObject::Instance->GetCanvasArray();
				}
			}

			FString spaceText;
			if (TargetScript->IsRenderToScreenSpace())
			{
				spaceText = TEXT("ScreenSpaceOverlay");
			}
			else if (TargetScript->IsRenderToWorldSpace())
			{
				spaceText = TEXT("WorldSpace");
			}
			else if (TargetScript->IsRenderToRenderTarget())
			{
				if (IsValid(TargetScript->renderTarget))
				{
					spaceText = FString::Printf(TEXT("RenderTarget(%s)"), *(TargetScript->renderTarget->GetName()));
				}
				else
				{
					spaceText = FString::Printf(TEXT("RenderTarget(NotValid)"));
				}
			}

			auto renderMode = TargetScript->GetActualRenderMode();
			int sortOrderCount = 0;
			for (auto item : itemList)
			{
				if (!item.IsValid())continue;
				if (item->GetWorld() != world)continue;
				if (item == TargetScript)continue;
				if (renderMode != item->GetActualRenderMode())continue;

				if (item->GetSortOrder() == TargetScript->GetSortOrder())
					sortOrderCount++;
			}
			auto depthInfo = FString::Printf(TEXT("All LGUICanvas of %s with same SortOrder count:%d\n"), *spaceText, sortOrderCount);
			return FText::FromString(depthInfo);
		}
	}
	return LOCTEXT("", "");
}
FText FLGUICanvasCustomization::GetDrawcallInfo()const
{
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
	{
		int drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
		auto& allCanvas = TargetScriptArray[0]->GetAllCanvasArray();
		int allDrawcallCount = 0;
		auto world = TargetScriptArray[0]->GetWorld();
		for (auto canvasItem : allCanvas)
		{
			if (TargetScriptArray[0]->IsRenderToScreenSpace() || TargetScriptArray[0]->IsRenderToWorldSpace())
			{
				if (canvasItem->IsRenderToScreenSpace() == TargetScriptArray[0]->IsRenderToScreenSpace()
					|| canvasItem->IsRenderToWorldSpace() == TargetScriptArray[0]->IsRenderToWorldSpace()
					)
				{
					if (canvasItem->GetWorld() == world)
					{
						allDrawcallCount += canvasItem->UIDrawcallList.Num();
					}
				}
			}
			else if (TargetScriptArray[0]->IsRenderToRenderTarget())
			{
				if (canvasItem->IsRenderToRenderTarget())
				{
					if (TargetScriptArray[0]->renderTarget == canvasItem->renderTarget && IsValid(canvasItem->renderTarget))
					{
						if (canvasItem->GetWorld() == world)
						{
							allDrawcallCount += canvasItem->UIDrawcallList.Num();
						}
					}
				}
			}
		}
		return FText::FromString(FString::Printf(TEXT("%d/%d"), drawcallCount, allDrawcallCount));
	}
	return FText::FromString(FString::Printf(TEXT("0/0")));
}
FText FLGUICanvasCustomization::GetDrawcallInfoTooltip()const
{
	int drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
	auto& allCanvas = TargetScriptArray[0]->GetAllCanvasArray();
	int allDrawcallCount = 0;
	FString spaceText;
	if (TargetScriptArray[0]->IsRenderToScreenSpace())
	{
		spaceText = TEXT("ScreenSpaceOverlay");
	}
	else if (TargetScriptArray[0]->IsRenderToWorldSpace())
	{
		spaceText = TEXT("WorldSpace");
	}
	else if (TargetScriptArray[0]->IsRenderToRenderTarget())
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

	auto world = TargetScriptArray[0]->GetWorld();
	for (auto canvasItem : allCanvas)
	{
		if (TargetScriptArray[0]->IsRenderToScreenSpace() || TargetScriptArray[0]->IsRenderToWorldSpace())
		{
			if (canvasItem->IsRenderToScreenSpace() == TargetScriptArray[0]->IsRenderToScreenSpace()
				|| canvasItem->IsRenderToWorldSpace() == TargetScriptArray[0]->IsRenderToWorldSpace()
				)
			{
				if (canvasItem->GetWorld() == world)
				{
					allDrawcallCount += canvasItem->UIDrawcallList.Num();
				}
			}
		}
		else if (TargetScriptArray[0]->IsRenderToRenderTarget())
		{
			if (canvasItem->IsRenderToRenderTarget())
			{
				if (TargetScriptArray[0]->renderTarget == canvasItem->renderTarget && IsValid(canvasItem->renderTarget))
				{
					if (canvasItem->GetWorld() == world)
					{
						allDrawcallCount += canvasItem->UIDrawcallList.Num();
					}
				}
			}
		}
	}
	FString tooltipStr = FString::Printf(TEXT("This canvas's drawcall count:%d, all canvas of %s drawcall count:%d"), drawcallCount, *spaceText, allDrawcallCount);
	return FText::FromString(tooltipStr);
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