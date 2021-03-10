﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

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

#define LOCTEXT_NAMESPACE "LGUISceneOutlinerInfoColumn"

namespace LGUISceneOutliner
{
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
		static FName LGUIInfoID("LGUIInfo");
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

		TSharedRef<SLGUISceneOutlinerButton> result = SNew(SLGUISceneOutlinerButton)
			.ButtonStyle(FLGUIEditorStyle::Get(), "EmptyButton")
			.ContentPadding(FMargin(0))
			.HasDownArrow(false)
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
							.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)))
							.ToolTipText(FText::FromString(FString(TEXT("This actor have LGUICanvas. The number is the drawcall count of this canvas."))))
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
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetCanvasIconVisibility, weakTreeItem)
							.ToolTipText(FText::FromString(FString(TEXT("This actor have LGUICanvas. The number is the drawcall count of this canvas."))))
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
							.Image(FLGUIEditorStyle::Get().GetBrush("PrefabMarkWhite"))
							.ColorAndOpacity(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconColor, weakTreeItem)
							.Visibility(this, &FLGUISceneOutlinerInfoColumn::GetPrefabIconVisibility, weakTreeItem)
							.ToolTipText(FText::FromString(FString(TEXT("This actor is part of a LGUIPrefab."))))
						]
					]
				]
			]
			.MenuContent()
			[
				FLGUIEditorModule::Instance->MakeEditorToolsMenu(true)
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

	FText FLGUISceneOutlinerInfoColumn::GetTextForItem(TWeakPtr<SceneOutliner::ITreeItem> TreeItem) const
	{
		auto Item = TreeItem.Pin();
		return Item.IsValid() ? FText::FromString(Item->Get(FGetInfo(*this))) : FText::GetEmpty();
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetPrefabIconVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			return LGUIEditorTools::GetPrefabActor_WhichManageThisActor(actor) != nullptr ? EVisibility::Visible : EVisibility::Hidden;
		}
		else
		{
			return EVisibility::Hidden;
		}
	}
	EVisibility FLGUISceneOutlinerInfoColumn::GetDownArrowVisibility(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			bool isPrefab = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(actor) != nullptr;
			return isPrefab ? EVisibility::Hidden : EVisibility::Visible;
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
	FText FLGUISceneOutlinerInfoColumn::GetDrawcallInfo(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		int drawcallCount = 0;
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			drawcallCount = LGUIEditorTools::GetCanvasDrawcallCount(actor);
		}
		return FText::FromString(FString::Printf(TEXT("%d"), drawcallCount));
	}
	FSlateColor FLGUISceneOutlinerInfoColumn::GetPrefabIconColor(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		FColor resultColor = FColor::White;
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			auto prefabActor = LGUIEditorTools::GetPrefabActor_WhichManageThisActor(actor);
			if (prefabActor != nullptr)
			{
				resultColor = prefabActor->GetPrefabComponent()->IdentityColor;
			}
		}
		return FSlateColor(resultColor);
	}

	bool FLGUISceneOutlinerInfoColumn::CanShowPrefabIcon(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem) const
	{
		if (AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem)))
		{
			auto prefabComp = actor->FindComponentByClass<ULGUIPrefabHelperComponent>();
			return IsValid(prefabComp);
		}
		else
		{
			return false;
		}
	}

	AActor* FLGUISceneOutlinerInfoColumn::GetActorFromTreeItem(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		auto Item = TreeItem.Pin();
		if (Item.IsValid())
		{
			TSharedPtr<SceneOutliner::FActorTreeItem> ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);

			if (ActorTreeItem.IsValid() && ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
			{
				if (ActorTreeItem->Actor->GetWorld())
				{
					return Cast<AActor>(ActorTreeItem->Actor.Get());
				}
			}
		}
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE