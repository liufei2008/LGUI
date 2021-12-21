// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUICanvasCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Engine/TextureRenderTarget2D.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

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
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth));
			break;
		case ELGUIRenderMode::WorldSpace:
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, pixelPerfect));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, renderTarget));
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth));
			break;
		case ELGUIRenderMode::WorldSpace_LGUI:
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

		if (!TargetScriptArray[0]->GetOverrideBlendDepth())
		{
			needToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth));
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
				if (TargetScript->IsRenderByLGUIRendererOrUERenderer())
				{
					spaceText = TEXT("World Space - LGUI Renderer");
				}
				else
				{
					spaceText = TEXT("World Space - UE Renderer");
				}
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

const TArray<TWeakObjectPtr<ULGUICanvas>>& GetAllCanvasArray(UWorld* InWorld)
{
	if (IsValid(InWorld))
	{
#if WITH_EDITOR
		if (!InWorld->IsGameWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				return ULGUIEditorManagerObject::Instance->GetCanvasArray();
			}
		}
		else
#endif
		{
			if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(InWorld))
			{
				return LGUIManagerActor->GetCanvasArray();
			}
		}
	}
	static TArray<TWeakObjectPtr<ULGUICanvas>> staticArray;//just for the return value
	return staticArray;
}

FText FLGUICanvasCustomization::GetDrawcallInfo()const
{
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
	{
		int drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
		auto& allCanvas = GetAllCanvasArray(TargetScriptArray[0]->GetWorld());
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
	auto& allCanvas = GetAllCanvasArray(TargetScriptArray[0]->GetWorld());
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