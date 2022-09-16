// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "SceneOutliner/LGUISceneOutlinerActorLabelColumn.h"
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
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "SceneOutlinerStandaloneTypes.h"
#include "Core/ActorComponent/UIItem.h"
#include "Widgets/Input/SHyperlink.h"
#include "Styling/StyleColors.h"

#define LOCTEXT_NAMESPACE "LGUISceneOutlinerInfoColumn"

namespace LGUISceneOutliner
{
	TSharedRef<ISceneOutlinerColumn> FLGUISceneOutlinerActorLabelColumn::MakeInstance(ISceneOutliner& SceneOutliner)
	{
		return MakeShareable(new FLGUISceneOutlinerActorLabelColumn(SceneOutliner));
	}


	FLGUISceneOutlinerActorLabelColumn::FLGUISceneOutlinerActorLabelColumn(ISceneOutliner& InSceneOutliner)
		: WeakSceneOutliner(StaticCastSharedRef<ISceneOutliner>(InSceneOutliner.AsShared()))
	{
		
	}

	FLGUISceneOutlinerActorLabelColumn::~FLGUISceneOutlinerActorLabelColumn()
	{
	}
	FName FLGUISceneOutlinerActorLabelColumn::GetID()
	{
		static FName LGUIInfoID("LGUILabel");
		return LGUIInfoID;
	}

	FName FLGUISceneOutlinerActorLabelColumn::GetColumnID()
	{
		return GetID();
	}

	SHeaderRow::FColumn::FArguments FLGUISceneOutlinerActorLabelColumn::ConstructHeaderRowColumn()
	{
		return SHeaderRow::Column(GetColumnID())
			.DefaultLabel(LOCTEXT("LGUIColumeHeader", "LGUI"))
			.DefaultTooltip(LOCTEXT("LGUIColumeHeader_Tooltip", "LGUI ActorLabel"))
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			;
	}

	FText FLGUISceneOutlinerActorLabelColumn::GetTextForItem(TWeakPtr<ISceneOutlinerTreeItem> TreeItem) const
	{
		ISceneOutlinerTreeItem* Item = TreeItem.Pin().Get();
		if (Item)
		{
			if (const FActorTreeItem* ActorItem = Item->CastTo<FActorTreeItem>())
			{
				if (AActor* Actor = ActorItem->Actor.Get())
				{
					return Actor ? FText::FromString(Actor->GetName()) : FText::GetEmpty();
				}
			}
		}
		return FText::FromString(TEXT("null"));
	}

	EVisibility FLGUISceneOutlinerActorLabelColumn::GetColumnDataVisibility(bool bIsClassHyperlink) const
	{
		return bIsClassHyperlink ? EVisibility::Visible : EVisibility::Collapsed;
	}


	const TSharedRef< SWidget > FLGUISceneOutlinerActorLabelColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
	{
		auto SceneOutliner = WeakSceneOutliner.Pin();
		check(SceneOutliner.IsValid());

		TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

		TSharedRef<STextBlock> MainText = SNew(STextBlock)
			.Text(this, &FLGUISceneOutlinerActorLabelColumn::GetTextForItem, TWeakPtr<ISceneOutlinerTreeItem>(TreeItem))
			.HighlightText(SceneOutliner->GetFilterHighlightText())
			.ColorAndOpacity(FSlateColor::UseSubduedForeground());

		HorizontalBox->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(12, 0, 12, 0)
			[
				MainText
			];

		TSharedPtr<SWidget> Hyperlink = ConstructClassHyperlink(*TreeItem);
		if (Hyperlink.IsValid())
		{
			// If we got a hyperlink, disable hide default text, and show the hyperlink
			MainText->SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FLGUISceneOutlinerActorLabelColumn::GetColumnDataVisibility, false)));
			Hyperlink->SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FLGUISceneOutlinerActorLabelColumn::GetColumnDataVisibility, true)));

			HorizontalBox->AddSlot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(8, 0, 0, 0)
				[
					// Make sure that the hyperlink shows as black (by multiplying black * desired color) when selected so it is readable against the orange background even if blue/green/etc... normally
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("NoBorder"))
				.ForegroundColor_Static([](TWeakPtr<const STableRow<FSceneOutlinerTreeItemPtr>> WeakRow)->FSlateColor {
				auto TableRow = WeakRow.Pin();
				return TableRow.IsValid() && TableRow->IsSelected() ? FStyleColors::ForegroundHover : FSlateColor::UseStyle();
			}, TWeakPtr<const STableRow<FSceneOutlinerTreeItemPtr>>(StaticCastSharedRef<const STableRow<FSceneOutlinerTreeItemPtr>>(Row.AsShared())))
				[
					Hyperlink.ToSharedRef()
				]
				];
		}

		return HorizontalBox;
	}


	TSharedPtr<SWidget> FLGUISceneOutlinerActorLabelColumn::ConstructClassHyperlink(ISceneOutlinerTreeItem& TreeItem)
	{
		if (const FActorTreeItem* ActorItem = TreeItem.CastTo<FActorTreeItem>())
		{
			if (AActor* Actor = ActorItem->Actor.Get())
			{
				if (ULGUIPrefabHelperObject* PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(Actor))
				{
					//if (PrefabHelperObject->IsActorBelongsToSubPrefab(Actor))
					if (ULGUIPrefab* Prefab = PrefabHelperObject->GetSubPrefabAsset(Actor))
					{
						return GetSourceLink(Prefab, Actor);
					}
				}
			}
		}
		return nullptr;
	}

	TSharedRef<SWidget> FLGUISceneOutlinerActorLabelColumn::GetSourceLink(ULGUIPrefab* Prefab, const TWeakObjectPtr<UObject> ObjectWeakPtr)
	{
		const FText BlueprintFormat = NSLOCTEXT("SourceHyperlink", "EditPrefab", "Edit {0}");
		const FText CodeFormat = NSLOCTEXT("SourceHyperlink", "GoToCode", "Open {0}");

		return GetSourceLinkFormatted(Prefab, ObjectWeakPtr, BlueprintFormat, CodeFormat);
	}

	TSharedRef<SWidget> FLGUISceneOutlinerActorLabelColumn::GetSourceLinkFormatted(ULGUIPrefab* Prefab, const TWeakObjectPtr<UObject> ObjectWeakPtr, const FText& BlueprintFormat, const FText& CodeFormat)
	{
		TSharedRef<SWidget> SourceHyperlink = SNew(SSpacer);

		if (Prefab)
		{
			struct Local
			{
				static void OnEditBlueprintClicked(TWeakObjectPtr<ULGUIPrefab> InPrefab, TWeakObjectPtr<UObject> InAsset)
				{
					ULGUIPrefab* PrefabAsset = InPrefab.Get();
					if (IsValid(PrefabAsset))
					{
						UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
						AssetEditorSubsystem->OpenEditorForAsset(PrefabAsset);
					}
				}
			};

			TWeakObjectPtr<ULGUIPrefab> PrefabPtr = Prefab;

			SourceHyperlink = SNew(SHyperlink)
				.Style(FEditorStyle::Get(), "Common.GotoBlueprintHyperlink")
				.OnNavigate_Static(&Local::OnEditBlueprintClicked, PrefabPtr, ObjectWeakPtr)
				.Text(FText::Format(BlueprintFormat, FText::FromString(Prefab->GetName())))
				.ToolTipText(NSLOCTEXT("SourceHyperlink", "EditBlueprint_ToolTip", "Click to edit the blueprint"));
		}

		return SourceHyperlink;
	}


}

#undef LOCTEXT_NAMESPACE