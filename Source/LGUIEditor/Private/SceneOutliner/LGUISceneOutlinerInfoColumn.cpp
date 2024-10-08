﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "SceneOutliner/LGUISceneOutlinerInfoColumn.h"
#include "LGUIEditorModule.h"
#include "ActorTreeItem.h"
#include "SortHelper.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorStyle.h"
#include "SceneOutliner/LGUISceneOutlinerButton.h"
#include "LGUIEditorTools.h"
#include "SortHelper.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "SceneOutlinerStandaloneTypes.h"
#include "Core/ActorComponent/UIItem.h"

#define LOCTEXT_NAMESPACE "LGUISceneOutlinerInfoColumn"

namespace LGUISceneOutliner
{
	TSharedRef<ISceneOutlinerColumn> FLGUISceneOutlinerInfoColumn::MakeInstance(ISceneOutliner& SceneOutliner)
	{
		return MakeShareable(new FLGUISceneOutlinerInfoColumn(SceneOutliner));
	}


	FLGUISceneOutlinerInfoColumn::FLGUISceneOutlinerInfoColumn(ISceneOutliner& InSceneOutliner)
		: WeakSceneOutliner(StaticCastSharedRef<ISceneOutliner>(InSceneOutliner.AsShared()))
	{
		
	}

	FLGUISceneOutlinerInfoColumn::~FLGUISceneOutlinerInfoColumn()
	{
	}
	FName FLGUISceneOutlinerInfoColumn::GetID()
	{
		static FName LGUIInfoID("LGUI");
		return LGUIInfoID;
	}

	FName FLGUISceneOutlinerInfoColumn::GetColumnID()
	{
		return GetID();
	}

	SHeaderRow::FColumn::FArguments FLGUISceneOutlinerInfoColumn::ConstructHeaderRowColumn()
	{
		return SHeaderRow::Column(GetColumnID())
			.DefaultLabel(LOCTEXT("LGUIColumeHeader", "LGUI"))
			.DefaultTooltip(LOCTEXT("LGUIColumeHeader_Tooltip", "LGUI functions"))
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			;
	}

	const TSharedRef< SWidget > FLGUISceneOutlinerInfoColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
	{
		auto SceneOutliner = WeakSceneOutliner.Pin();
		check(SceneOutliner.IsValid());

		AActor* actor = GetActorFromTreeItem(TreeItem);
		if (actor == nullptr)
		{
			return SNew(SBox);
		}
		if (!LGUIEditorTools::IsActorCompatibleWithLGUIToolsMenu(actor))
		{
			return SNew(SBox);
		}

		auto bIsRootAgentActor = FLGUIPrefabEditor::ActorIsRootAgent(actor);
		TSharedRef<SLGUISceneOutlinerButton> result = SNew(SLGUISceneOutlinerButton)
			.ButtonStyle(FLGUIEditorStyle::Get(), "EmptyButton")
			.ContentPadding(FMargin(0))
			.HasDownArrow(false)
			.OnComboBoxOpened(FOnComboBoxOpened::CreateLambda([=]() {//@todo: make it a callback
				FLGUIEditorModule::Get().OnOutlinerSelectionChange();
				}))
			.Visibility(bIsRootAgentActor ? EVisibility::HitTestInvisible : EVisibility::Visible)
			.ButtonContent()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(SOverlay)
					+SOverlay::Slot()//canvas icon
					[
						SNew(SBox)
						.WidthOverride(16)
						.HeightOverride(16)
						.Padding(FMargin(0))
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SImage)
							.Image(FLGUIEditorStyle::Get().GetBrush("CanvasMark"))
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetCanvasIconVisibility, TreeItem)
							.ColorAndOpacity(this, &FLGUISceneOutlinerInfoColumn::GetDrawcallIconColor, TreeItem)
							.ToolTipText(LOCTEXT("CanvasMarkTip", "This actor have LGUICanvas. The number is the drawcall count of this canvas."))
						]
					]
					+SOverlay::Slot()//drawcall count
					[
						SNew(SBox)
						.WidthOverride(16)
						.HeightOverride(16)
						.Padding(FMargin(0))
						.HAlign(EHorizontalAlignment::HAlign_Left)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.ShadowColorAndOpacity(FLinearColor::Black)
							.ShadowOffset(FVector2D(1, 1))
							.Text(this, &FLGUISceneOutlinerInfoColumn::GetDrawcallInfo, TreeItem)
							.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Green)))
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetDrawcallCountVisibility, TreeItem)
							.ToolTipText(LOCTEXT("DrawcallCountTip", "The number is the drawcall count generated by this LGUICanvas."))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
				]
				+SHorizontalBox::Slot()
				[
					SNew(SOverlay)
					+SOverlay::Slot()//down arrow
					[
						SNew(SBox)
						.Visibility(bIsRootAgentActor ? EVisibility::Hidden : EVisibility::Visible)
						.WidthOverride(8)
						.HeightOverride(8)
						.Padding(FMargin(0))
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SImage)
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetDownArrowVisibility, TreeItem)
							.Image(FAppStyle::GetBrush("ComboButton.Arrow"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
					+SOverlay::Slot()//prefab
					[
						SNew(SBox)
						.WidthOverride(16)
						.HeightOverride(16)
						.Padding(FMargin(0))
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SImage)
							.Image(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconImage, TreeItem)
							.ColorAndOpacity(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconColor, TreeItem)
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconVisibility, TreeItem)
							.ToolTipText(this, &FLGUISceneOutlinerInfoColumn::GetPrefabTooltip, TreeItem)
						]
					]
					//+SOverlay::Slot()//prefab+
					//[
					//	SNew(SBox)
					//	.WidthOverride(16)
					//	.HeightOverride(16)
					//	.Padding(FMargin(0))
					//	.HAlign(EHorizontalAlignment::HAlign_Center)
					//	.VAlign(EVerticalAlignment::VAlign_Center)
					//	[
					//		SNew(SImage)
					//		.Image(this, &FLGUISceneOutlinerInfoColumn::GetPrefabPlusIconImage, TreeItem)
					//		.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetPrefabPlusIconVisibility, TreeItem)
					//	]
					//]
				]
			]
			.MenuContent()
			[
				FLGUIEditorModule::Get().MakeEditorToolsMenu(false, false, false, false, false, false)
			];

		result->_TreeItemActor = actor;

		return result;
	}

	void FLGUISceneOutlinerInfoColumn::PopulateSearchStrings(const ISceneOutlinerTreeItem& Item, TArray< FString >& OutSearchStrings) const
	{
		OutSearchStrings.Add(Item.GetDisplayString());
	}

	void FLGUISceneOutlinerInfoColumn::SortItems(TArray<FSceneOutlinerTreeItemPtr>& OutItems, const EColumnSortMode::Type SortMode) const
	{
		if (SortMode == EColumnSortMode::None)return;

		OutItems.Sort([this, SortMode](FSceneOutlinerTreeItemPtr A, FSceneOutlinerTreeItemPtr B)
		{
			auto CommonCompare = [&] {
				auto AStr = SceneOutliner::FNumericStringWrapper(A->GetDisplayString());
				auto BStr = SceneOutliner::FNumericStringWrapper(B->GetDisplayString());
				return AStr > BStr;
			};
			bool result = true;
			AActor* ActorA = GetActorFromTreeItem(A.ToSharedRef());
			AActor* ActorB = GetActorFromTreeItem(B.ToSharedRef());
			if (ActorA != nullptr && ActorB != nullptr)
			{
				USceneComponent* RootCompA = ActorA->GetRootComponent();
				USceneComponent* RootCompB = ActorB->GetRootComponent();
				if (RootCompA != nullptr && RootCompB != nullptr)
				{
					UUIItem* UIItemA = Cast<UUIItem>(RootCompA);
					UUIItem* UIItemB = Cast<UUIItem>(RootCompB);
					if (UIItemA != nullptr && UIItemB != nullptr)
					{
						if (UIItemA->GetHierarchyIndex() == UIItemB->GetHierarchyIndex())
						{
							result = ActorA->GetActorLabel().Compare(ActorB->GetActorLabel()) > 0;
						}
						else
						{
							result = UIItemA->GetHierarchyIndex() > UIItemB->GetHierarchyIndex();
						}
					}
					else
					{
						result = CommonCompare();
					}
				}
				else
				{
					result = CommonCompare();
				}
			}
			else
			{
				result = CommonCompare();
			}
			return SortMode == EColumnSortMode::Ascending ? !result : result;
		});
	}

	EVisibility FLGUISceneOutlinerInfoColumn::GetPrefabIconVisibility(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TreeItem))
		{
			if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(actor))
			{
				if (PrefabHelperObject->IsActorBelongsToSubPrefab(actor))//is sub prefab
				{
					return EVisibility::Visible;
				}
				else
				{
					if (PrefabHelperObject->IsActorBelongsToMissingSubPrefab(actor))
					{
						return EVisibility::Visible;
					}
					else
					{
						return EVisibility::Hidden;
					}
				}
			}
		}
		return EVisibility::Hidden;
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetDownArrowVisibility(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TreeItem))
		{
			return GetPrefabIconVisibility(TreeItem) == EVisibility::Visible ? EVisibility::Hidden : EVisibility::Visible;
		}
		else
		{
			return EVisibility::Hidden;
		}
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetCanvasIconVisibility(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TreeItem))
		{
			return LGUIEditorTools::IsCanvasActor(actor) ? EVisibility::Visible : EVisibility::Hidden;
		}
		return EVisibility::Hidden;
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetDrawcallCountVisibility(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TreeItem))
		{
			return LGUIEditorTools::IsCanvasActor(actor) ? EVisibility::Visible : EVisibility::Hidden;
		}
		return EVisibility::Hidden;
	}
	auto ActorIsPrefabPlus(AActor* Actor)
	{
		if (auto ParentActor = Actor->GetAttachParentActor())
		{
			auto PrefabHelperObjectForParent = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(ParentActor);
			auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(Actor);
			if (PrefabHelperObject != nullptr && PrefabHelperObjectForParent != nullptr)
			{
				if (!PrefabHelperObject->IsActorBelongsToSubPrefab(Actor) && PrefabHelperObjectForParent->IsActorBelongsToSubPrefab(ParentActor))
				{
					return true;
				}
			}
		}
		return false;
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetPrefabPlusIconVisibility(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (AActor* Actor = GetActorFromTreeItem(TreeItem))
		{
			if (ActorIsPrefabPlus(Actor))
				return EVisibility::Visible;
		}
		return EVisibility::Hidden;
	}
	FSlateColor FLGUISceneOutlinerInfoColumn::GetDrawcallIconColor(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TreeItem))
		{
			return LGUIEditorTools::IsCanvasActor(actor) ? FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.4f)) : FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
		return FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	}
	FText FLGUISceneOutlinerInfoColumn::GetDrawcallInfo(FSceneOutlinerTreeItemRef TreeItem)const
	{
		int drawcallCount = 0;
		if (AActor* actor = GetActorFromTreeItem(TreeItem))
		{
			drawcallCount = LGUIEditorTools::GetDrawcallCount(actor);
		}
		return FText::FromString(FString::Printf(TEXT("%d"), drawcallCount));
	}
	const FSlateBrush* FLGUISceneOutlinerInfoColumn::GetPrefabIconImage(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (auto actor = GetActorFromTreeItem(TreeItem))
		{
			if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(actor))
			{
				if (!PrefabHelperObject->IsActorBelongsToSubPrefab(actor))//is sub prefab
				{
					if (PrefabHelperObject->IsActorBelongsToMissingSubPrefab(actor))
					{
						return FLGUIEditorStyle::Get().GetBrush("PrefabMarkBroken");
					}
				}
				else
				{
					if (PrefabHelperObject->GetSubPrefabAsset(actor)->GetIsPrefabVariant())
					{
						return FLGUIEditorStyle::Get().GetBrush("PrefabVariantMarkWhite");
					}
				}
			}
		}
		return FLGUIEditorStyle::Get().GetBrush("PrefabMarkWhite");
	}
	const FSlateBrush* FLGUISceneOutlinerInfoColumn::GetPrefabPlusIconImage(FSceneOutlinerTreeItemRef TreeItem)const
	{
		return FLGUIEditorStyle::Get().GetBrush("PrefabPlusMarkWhite");
	}
	FSlateColor FLGUISceneOutlinerInfoColumn::GetPrefabIconColor(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (auto actor = GetActorFromTreeItem(TreeItem))
		{
			if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(actor))
			{
				if (PrefabHelperObject->IsActorBelongsToSubPrefab(actor))//is sub prefab
				{
					return FSlateColor(PrefabHelperObject->GetSubPrefabData(actor).EditorIdentifyColor);
				}
				else
				{
					if (PrefabHelperObject->IsActorBelongsToMissingSubPrefab(actor))
					{
						return FSlateColor(FColor::White);
					}
				}
			}
		}
		return FSlateColor(FColor::Green);
	}
	FText FLGUISceneOutlinerInfoColumn::GetPrefabTooltip(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (auto actor = GetActorFromTreeItem(TreeItem))
		{
			if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(actor))
			{
				if (!PrefabHelperObject->IsActorBelongsToSubPrefab(actor))//is sub prefab
				{
					if (PrefabHelperObject->IsActorBelongsToMissingSubPrefab(actor))
					{
						return LOCTEXT("PrefabMarkBrokenTip", "This actor was part of a LGUIPrefab, but the prefab asset is missing!");
					}
				}
			}
		}
		return LOCTEXT("PrefabMarkWhiteTip", "This actor is part of a LGUIPrefab.");
	}

	AActor* FLGUISceneOutlinerInfoColumn::GetActorFromTreeItem(FSceneOutlinerTreeItemRef TreeItem)const
	{
		if (auto ActorTreeItem = TreeItem->CastTo<FActorTreeItem>())
		{
			if (ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
			{
				if (ActorTreeItem->Actor->GetWorld())
				{
					return ActorTreeItem->Actor.Get();
				}
			}
		}
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE