// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUISceneOutlinerInfoColumn.h"
#include "LGUIEditorModule.h"
#include "SceneOutlinerVisitorTypes.h"
#include "ActorTreeItem.h"
#include "SortHelper.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorStyle.h"
#include "LGUISceneOutlinerButton.h"

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
		static FName LGUIPrefabID("LGUIPrefab");
		return LGUIPrefabID;
	}

	FName FLGUISceneOutlinerInfoColumn::GetColumnID()
	{
		return GetID();
	}

	SHeaderRow::FColumn::FArguments FLGUISceneOutlinerInfoColumn::ConstructHeaderRowColumn()
	{
		return SHeaderRow::Column(GetColumnID())
			.DefaultLabel(LOCTEXT("LGUIColumeHeader", "UI"))
			.DefaultTooltip(LOCTEXT("LGUIColumeHeader_Tooltip", "LGUI functions"))
			.FixedWidth(20)
			;
	}

	const TSharedRef< SWidget > FLGUISceneOutlinerInfoColumn::ConstructRowWidget(SceneOutliner::FTreeItemRef TreeItem, const STableRow<SceneOutliner::FTreeItemPtr>& Row)
	{
		auto SceneOutliner = WeakSceneOutliner.Pin();
		check(SceneOutliner.IsValid());

		AActor* actor = GetActorFromTreeItem(TWeakPtr<SceneOutliner::ITreeItem>(TreeItem));
		if (actor == nullptr)
		{
			return SNew(SBox);
		}
		CurrentWorld = actor->GetWorld();

		TSharedRef<SLGUISceneOutlinerButton> result = SNew(SLGUISceneOutlinerButton)
			.ButtonStyle(FLGUIEditorStyle::Get(), "EmptyButton")
			.ContentPadding(FMargin(0))
			//.HasDownArrow(false)
			.ButtonContent()
			[
				SNew(SBox)
				.WidthOverride(16)
				.HeightOverride(16)
				.Padding(FMargin(0))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
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
		SceneOutliner::FSortHelper<FString>()
			.Primary(FGetInfo(*this), SortMode)
			.Sort(OutItems);
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

	bool FLGUISceneOutlinerInfoColumn::CanShowPrefabIcon(AActor* InActor) const
	{
		auto prefabComp = InActor->FindComponentByClass<ULGUIPrefabHelperComponent>();
		return IsValid(prefabComp);
	}

	AActor* FLGUISceneOutlinerInfoColumn::GetActorFromTreeItem(const TWeakPtr<SceneOutliner::ITreeItem> TreeItem)const
	{
		auto Item = TreeItem.Pin();
		if (Item.IsValid())
		{
			TSharedPtr<SceneOutliner::FActorTreeItem> ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);

			if (ActorTreeItem.IsValid() && ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
			{
				if (ActorTreeItem->Actor->GetWorld() && ActorTreeItem->Actor->GetWorld()->WorldType == EWorldType::Editor)
				{
					return Cast<AActor>(ActorTreeItem->Actor.Get());
				}
			}
		}
		return nullptr;
	}

	ALGUIPrefabActor* FLGUISceneOutlinerInfoColumn::GetPrefabActor_WhichManageThisActor(AActor* InActor)const
	{
		for (TActorIterator<ALGUIPrefabActor> ActorItr(InActor->GetWorld()); ActorItr; ++ActorItr)
		{
			auto prefabActor = *ActorItr;
			if (prefabActor->GetPrefabComponent()->AllLoadedActorArray.Contains(InActor))
			{
				if (InActor->IsAttachedTo(prefabActor))
				{
					return prefabActor;
				}
			}
		}
		return nullptr;
	}

	bool FLGUISceneOutlinerInfoColumn::IsInsidePrefabActor(AActor* InActor)const
	{
		return GetPrefabActor_WhichManageThisActor(InActor) != nullptr;
	}

	void FLGUISceneOutlinerInfoColumn::GotoPrefabActor(AActor* InActor)
	{
		ALGUIPrefabActor* PrefabActor = nullptr;
		for (TActorIterator<ALGUIPrefabActor> ActorItr(InActor->GetWorld()); ActorItr; ++ActorItr)
		{
			auto itemActor = *ActorItr;
			if (itemActor->GetPrefabComponent()->AllLoadedActorArray.Contains(InActor))
			{
				PrefabActor = itemActor;
				break;
			}
		}
		if (PrefabActor != nullptr)
		{
			GEditor->SelectNone(true, false);
			GEditor->SelectActor(PrefabActor, true, true);
		}
	}
}

#undef LOCTEXT_NAMESPACE