// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorDetails.h"
#include "Engine/Selection.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "SSCSEditor.h"
#include "ISCSEditorUICustomization.h"
#include "GameFramework/Actor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Misc/NotifyHook.h"
#include "LGUIPrefabEditor.h"
#include "DetailLayoutBuilder.h"
#include "LGUIPrefabOverrideDataViewer.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIEditorTools.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorDetailTab"

class LGUISCSEditorUICustomization : public ISCSEditorUICustomization
{
	virtual bool HideBlueprintButtons() const override { return true; }
};

void SLGUIPrefabEditorDetails::Construct(const FArguments& Args, TSharedPtr<FLGUIPrefabEditor> InPrefabEditor)
{
	PrefabEditorPtr = InPrefabEditor;

	USelection::SelectionChangedEvent.AddRaw(this, &SLGUIPrefabEditorDetails::OnEditorSelectionChanged);

    FPropertyEditorModule& PropPlugin = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bUpdatesFromSelection = true;
    DetailsViewArgs.bLockable = true;
    DetailsViewArgs.NotifyHook = GUnrealEd;
    DetailsViewArgs.ViewIdentifier = NAME_None;
    DetailsViewArgs.bCustomNameAreaLocation = true;
    DetailsViewArgs.bCustomFilterAreaLocation = false;
    DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ComponentsAndActorsUseNameArea;
    DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.bAllowSearch = true;
    //DetailsViewArgs.HostCommandList = InCommandList;

    DetailsView = PropPlugin.CreateDetailView(DetailsViewArgs);
    DetailsView->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateSP(this, &SLGUIPrefabEditorDetails::IsPropertyReadOnly));

	SCSEditor = SNew(SSCSEditor)
		.EditorMode(EComponentEditorMode::ActorInstance)
		.AllowEditing(this, &SLGUIPrefabEditorDetails::IsSSCSEditorAllowEditing)
		.ActorContext(this, &SLGUIPrefabEditorDetails::GetActorContext)
		.OnSelectionUpdated(this, &SLGUIPrefabEditorDetails::OnSCSEditorTreeViewSelectionChanged)
		.OnItemDoubleClicked(this, &SLGUIPrefabEditorDetails::OnSCSEditorTreeViewItemDoubleClicked);

	
	TSharedPtr<ISCSEditorUICustomization> Customization = MakeShared<LGUISCSEditorUICustomization>();
	SCSEditor->SetUICustomization(Customization);

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(2, 2))
			.AutoHeight()
			[
				DetailsView->GetNameAreaWidget().ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(2, 2))
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility(this, &SLGUIPrefabEditorDetails::GetPrefabButtonVisibility)
				.IsEnabled(this, &SLGUIPrefabEditorDetails::IsPrefabButtonEnable)
				.HeightOverride(this, &SLGUIPrefabEditorDetails::GetPrefabButtonHeight)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(4, 0))
					[
						SNew(SBox)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("PrefabFunctions", "Prefab"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.Padding(FMargin(2, 0))
					[
						SNew(SButton)
						.OnClicked_Lambda([=]() {
							PrefabEditorPtr.Pin()->OpenSubPrefab(CachedActor.Get());
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("OpenPrefab", "Open"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.Padding(FMargin(2, 0))
					[
						SNew(SButton)
						.OnClicked_Lambda([=]() {
							PrefabEditorPtr.Pin()->SelectSubPrefab(CachedActor.Get());
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SelectPrefab", "Select"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					.Padding(FMargin(2, 0))
					[
						SNew(SComboButton)
						.HasDownArrow(true)
						.ToolTipText(LOCTEXT("PrefabOverride", "Edit override parameters for this prefab"))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("OverrideButton", "Prefab Override Properties"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
						.MenuContent()
						[
							SNew(SBox)
							.Padding(FMargin(4, 4))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										.AutoWidth()
										[
											SAssignNew(OverrideParameterEditor, SLGUIPrefabOverrideDataViewer, PrefabEditorPtr.Pin()->GetPrefabManagerObject())
											.AfterRevertPrefab_Lambda([=](ULGUIPrefab* PrefabAsset) {
												RefreshOverrideParameter();
												})
											.AfterApplyPrefab_Lambda([=](ULGUIPrefab* PrefabAsset){
												RefreshOverrideParameter();
												LGUIEditorTools::RefreshLevelLoadedPrefab(PrefabAsset);
												LGUIEditorTools::RefreshOnSubPrefabChange(PrefabAsset);
												LGUIEditorTools::RefreshOpenedPrefabEditor(PrefabAsset);
												})
										]
									]
								]
							]
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 2))
			.AutoHeight()
			[
				SCSEditor.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 2))
			[
				DetailsView.ToSharedRef()
			]
		];
}

SLGUIPrefabEditorDetails::~SLGUIPrefabEditorDetails()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
}

bool SLGUIPrefabEditorDetails::IsPrefabButtonEnable()const
{
	if (PrefabEditorPtr.IsValid() && CachedActor.IsValid())
	{
		return PrefabEditorPtr.Pin()->ActorIsSubPrefabRoot(CachedActor.Get());
	}
	return false;
}

FOptionalSize SLGUIPrefabEditorDetails::GetPrefabButtonHeight()const
{
	return IsPrefabButtonEnable() ? 26 : 0;
}

EVisibility SLGUIPrefabEditorDetails::GetPrefabButtonVisibility()const
{
	return IsPrefabButtonEnable() ? EVisibility::Visible : EVisibility::Hidden;
}

bool SLGUIPrefabEditorDetails::IsSSCSEditorAllowEditing()const
{
	if (PrefabEditorPtr.IsValid() && CachedActor.IsValid())
	{
		return !PrefabEditorPtr.Pin()->ActorBelongsToSubPrefab(CachedActor.Get());
	}
	return true;
}

void SLGUIPrefabEditorDetails::RefreshOverrideParameter()
{
	FLGUISubPrefabData SubPrefabData = PrefabEditorPtr.Pin()->GetSubPrefabDataForActor(CachedActor.Get());
	OverrideParameterEditor->RefreshDataContent(SubPrefabData.ObjectOverrideParameterArray, nullptr);
}

AActor* SLGUIPrefabEditorDetails::GetActorContext() const
{
	return CachedActor.Get();
}

void SLGUIPrefabEditorDetails::OnEditorSelectionChanged(UObject* Object)
{
	// Make sure the selection set that changed is relevant to us
	USelection* Selection = Cast<USelection>(Object);
	if (Selection)
	{
		if (AActor* Actor = Selection->GetTop<AActor>())
		{
			if (Actor->GetWorld() != PrefabEditorPtr.Pin()->GetWorld())
			{
				return;
			}

			CachedActor = Actor;
			RefreshOverrideParameter();
			if (SCSEditor)
			{
				SCSEditor->UpdateTree();
			}
		}

		TArray<UObject*> SeletedObjectList;
		for (int32 i = 0; i < Selection->Num(); i++)
		{
			UObject* SeletedObject = Selection->GetSelectedObject(i);
			if (SeletedObject)
			{
				if (SeletedObject->GetWorld() != PrefabEditorPtr.Pin()->GetWorld())
				{
					continue;
				}

				SeletedObjectList.Add(SeletedObject);
			}
		}

		if (SeletedObjectList.Num() > 0)
		{
			if (DetailsView)
			{
				DetailsView->SetObjects(SeletedObjectList);
			}
		}
	}
}

void SLGUIPrefabEditorDetails::OnSCSEditorTreeViewSelectionChanged(const TArray<TSharedPtr<class FSCSEditorTreeNode> >& SelectedNodes)
{
	if (SelectedNodes.Num() > 0)
	{
		TArray<UObject*> SelectedObjects;

		for (const TSharedPtr<FSCSEditorTreeNode>& Node : SelectedNodes)
		{
			if (Node.IsValid())
			{
				UObject* Object = const_cast<UObject*>(Node->GetObject<UObject>());
				if (Object)
				{
					SelectedObjects.Add(Object);
				}
			}
		}

		if (SelectedObjects.Num() > 0 && DetailsView.IsValid())
		{
			DetailsView->SetObjects(SelectedObjects);
		}
	}
}

void SLGUIPrefabEditorDetails::OnSCSEditorTreeViewItemDoubleClicked(const TSharedPtr<class FSCSEditorTreeNode> ClickedNode)
{

}

bool SLGUIPrefabEditorDetails::IsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent)
{
	return false;
}

#undef LOCTEXT_NAMESPACE