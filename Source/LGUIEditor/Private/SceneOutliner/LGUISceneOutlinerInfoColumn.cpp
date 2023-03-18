﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "SceneOutliner/LGUISceneOutlinerInfoColumn.h"
#include "LGUIEditorModule.h"
#include "SceneOutlinerVisitorTypes.h"
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
#include "PrefabSystem/LGUIPrefabHelperActor.h"
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

	struct FGetInfo : SceneOutliner::TTreeItemGetter<FString>
	{
		FGetInfo(const FLGUISceneOutlinerInfoColumn& InColumn)
			: WeakColumn(StaticCastSharedRef<const FLGUISceneOutlinerInfoColumn>(InColumn.AsShared()))
		{}

		virtual FString Get(const SceneOutliner::FActorTreeItem& ActorItem) const override
		{
			if (!WeakColumn.IsValid())
			{
				return FString();
			}

			AActor* Actor = ActorItem.Actor.Get();
			if (!Actor)
			{
				return FString();
			}

			const FLGUISceneOutlinerInfoColumn& Column = *WeakColumn.Pin();

			return Column.GetTextForActor(Actor);
		}


		/** Weak reference to the sequencer info column */
		TWeakPtr< const FLGUISceneOutlinerInfoColumn > WeakColumn;
	};

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
			.FixedWidth(40)
			;
	}

	const TSharedRef< SWidget > FLGUISceneOutlinerInfoColumn::ConstructRowWidget(SceneOutliner::FTreeItemRef TreeItem, const STableRow<SceneOutliner::FTreeItemPtr>& Row)
	{
		auto SceneOutliner = WeakSceneOutliner.Pin();
		check(SceneOutliner.IsValid());

		auto weakTreeItem = TWeakPtr<SceneOutliner::ITreeItem>(TreeItem);
		AActor* actor = GetActorFromTreeItem(weakTreeItem);
		if (actor == nullptr)
		{
			return SNew(SBox);
		}
		auto ActorClassName = actor->GetClass()->GetFName();
		if (
			ActorClassName == TEXT("Landscape")
			|| ActorClassName == TEXT("LandscapeStreamingProxy")
			|| ActorClassName == TEXT("WorldDataLayers")
			|| ActorClassName == TEXT("WorldPartitionMiniMap")
			)
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
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetCanvasIconVisibility, weakTreeItem)
							.ColorAndOpacity(this, &FLGUISceneOutlinerInfoColumn::GetDrawcallIconColor, weakTreeItem)
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
							.Text(this, &FLGUISceneOutlinerInfoColumn::GetDrawcallInfo, weakTreeItem)
							.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Green)))
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetDrawcallCountVisibility, weakTreeItem)
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
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetDownArrowVisibility, weakTreeItem)
							.Image(FEditorStyle::GetBrush("ComboButton.Arrow"))
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
							.Image(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconImage, weakTreeItem)
							.ColorAndOpacity(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconColor, weakTreeItem)
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconVisibility, weakTreeItem)
							.ToolTipText(this, &FLGUISceneOutlinerInfoColumn::GetPrefabTooltip, weakTreeItem)
						]
					]
				]
			]
			.MenuContent()
			[
				FLGUIEditorModule::Get().MakeEditorToolsMenu(false, false, false, false, false, false)
			];

		result->_TreeItemActor = actor;

		return result;
	}

	void FLGUISceneOutlinerInfoColumn::PopulateSearchStrings(const SceneOutliner::ITreeItem& Item, TArray< FString >& OutSearchStrings) const
	{
		OutSearchStrings.Add(Item.GetDisplayString());
	}

	void FLGUISceneOutlinerInfoColumn::SortItems(TArray<SceneOutliner::FTreeItemPtr>& OutItems, const EColumnSortMode::Type SortMode) const
	{
		//SceneOutliner::FSortHelper<FString>()
		//	.Primary(FGetInfo(*this), SortMode)
		//	.Sort(OutItems);
		if (SortMode == EColumnSortMode::None)return;

		OutItems.Sort([this, SortMode](const SceneOutliner::FTreeItemPtr& A, const SceneOutliner::FTreeItemPtr& B)
		{
			auto CommonCompare = [&] {
				auto AStr = SceneOutliner::FNumericStringWrapper(A->GetDisplayString());
				auto BStr = SceneOutliner::FNumericStringWrapper(B->GetDisplayString());
				return AStr > BStr;
			};
			bool result = true;
			AActor* ActorA = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(A));
			AActor* ActorB = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(B));
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
						if (A->GetTypeSortPriority() != B->GetTypeSortPriority())
						{
							result = A->GetTypeSortPriority() > B->GetTypeSortPriority();
						}
						else
						{
							result = CommonCompare();
						}
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

	FString FLGUISceneOutlinerInfoColumn::GetTextForActor(AActor* InActor) const
	{
		if (!InActor->IsPendingKillPending())
		{

		}
		return TEXT("");
	}

	EVisibility FLGUISceneOutlinerInfoColumn::GetPrefabIconVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
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
	EVisibility FLGUISceneOutlinerInfoColumn::GetDownArrowVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			return GetPrefabIconVisibility(TreeItem) == EVisibility::Visible ? EVisibility::Hidden : EVisibility::Visible;
		}
		else
		{
			return EVisibility::Hidden;
		}
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetCanvasIconVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			return LGUIEditorTools::IsCanvasActor(actor) ? EVisibility::Visible : EVisibility::Hidden;
		}
		return EVisibility::Hidden;
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetDrawcallCountVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			return LGUIEditorTools::IsCanvasActor(actor) ? EVisibility::Visible : EVisibility::Hidden;
		}
		return EVisibility::Hidden;
	}
	FSlateColor FLGUISceneOutlinerInfoColumn::GetDrawcallIconColor(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			return LGUIEditorTools::IsCanvasActor(actor) ? FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.4f)) : FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
		return FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	}
	FText FLGUISceneOutlinerInfoColumn::GetDrawcallInfo(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		int drawcallCount = 0;
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			drawcallCount = LGUIEditorTools::GetDrawcallCount(actor);
		}
		return FText::FromString(FString::Printf(TEXT("%d"), drawcallCount));
	}
	const FSlateBrush* FLGUISceneOutlinerInfoColumn::GetPrefabIconImage(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
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
			}
		}
		return FLGUIEditorStyle::Get().GetBrush("PrefabMarkWhite");
	}
	FSlateColor FLGUISceneOutlinerInfoColumn::GetPrefabIconColor(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
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
	FText FLGUISceneOutlinerInfoColumn::GetPrefabTooltip(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
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

	AActor* FLGUISceneOutlinerInfoColumn::GetActorFromTreeItem(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		auto Item = TreeItem.Pin();
		if (Item.IsValid())
		{
			switch (Item->GetTypeSortPriority())
			{
			case SceneOutliner::ETreeItemSortOrder::Actor:
			{
				TSharedPtr<SceneOutliner::FActorTreeItem> ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
				if (ActorTreeItem.IsValid() && ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
				{
					if (ActorTreeItem->Actor->GetWorld())
					{
						return ActorTreeItem->Actor.Get();
					}
				}
			}
			break;
			}
		}
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE