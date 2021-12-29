// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
		.AllowEditing(true)//@todo: not allow editing for sub prefab
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
				//@todo: only show this on sub prefab actor
				SNew(SComboButton)
				.HasDownArrow(true)
				.ToolTipText(LOCTEXT("PrefabOverride", "PrefabOverrideProperties"))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OverrideButton", "Prefab Override Properties"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
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
			.AutoHeight()
			[
				DetailsView.ToSharedRef()
			]
		];
}

SLGUIPrefabEditorDetails::~SLGUIPrefabEditorDetails()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
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

		for (const TSharedPtr<FSCSEditorTreeNode> Node : SelectedNodes)
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