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
    //DetailsView->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateSP(this, &SLGUIPrefabEditorDetails::IsPropertyReadOnly));


	ComponentsBox = SNew(SBox)
		.Visibility(EVisibility::Visible);

	SCSEditor = SNew(SSCSEditor)
		.EditorMode(EComponentEditorMode::ActorInstance)
		.AllowEditing(true)
		.ActorContext(this, &SLGUIPrefabEditorDetails::GetActorContext)
		.OnSelectionUpdated(this, &SLGUIPrefabEditorDetails::OnSCSEditorTreeViewSelectionChanged)
		.OnItemDoubleClicked(this, &SLGUIPrefabEditorDetails::OnSCSEditorTreeViewItemDoubleClicked);

	
	TSharedPtr<ISCSEditorUICustomization> Customization = MakeShared<LGUISCSEditorUICustomization>();
	SCSEditor->SetUICustomization(Customization);

	ComponentsBox->SetContent(SCSEditor.ToSharedRef());

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0.0f, 0.0f, 0.0f, 2.0f)
			.AutoHeight()
			[
				DetailsView->GetNameAreaWidget().ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 10, 0, 0))
			.FillHeight(0.35f)
			[
				ComponentsBox.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 10, 0, 0))
			.FillHeight(0.65f)
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

#undef LOCTEXT_NAMESPACE