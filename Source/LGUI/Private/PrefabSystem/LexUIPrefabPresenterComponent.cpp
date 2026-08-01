// Copyright 2019-Present LexLiu. All Rights Reserved.


#include "PrefabSystem/LexUIPrefabPresenterComponent.h"

#include "LGUI.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"
#include "Event/LexEventSystem.h"
#include "PrefabSystem/LexUIPrefab.h"

#define LOCTEXT_NAMESPACE "LexWidgetPresenterComponent"

ULexUIPrefabPresenterComponent::ULexUIPrefabPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULexUIPrefabPresenterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULexUIPrefabPresenterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ULexUIPrefabPresenterComponent::LoadWidget()
{
	if (LoadedWidget.IsValid())
	{
		LoadedWidget->DestroyWidget();
		LoadedWidget = nullptr;
	}
#if WITH_EDITOR
	if (this->GetName().Contains(TEXT("SKEL_")) || this->GetName().Contains(TEXT("TRASH_")))
	{
		UE_LOG(LGUI, Warning, TEXT("Skip LoadPrefab for %s because it's a temp object for blueprint compiling!"), *this->GetName());
		return;
	}
#endif
	if (IsValid(WidgetPrefab))
	{
		if (auto World = GetWorld())
		{
			LoadedWidget = WidgetPrefab->LoadPrefab(World, nullptr, [this](ULexWidget* RootWidget)
			{
				if (auto Canvas = RootWidget->GetComponent<ULexCanvas>())
				{
					RootWidget->RemoveComponent(Canvas);
				}
				RootCanvas = RootWidget->AddComponentByTemplate<ULexCanvas>(CanvasTemplate);
				RootCanvas->AttachToWidgetPresenterComponent(this);
			});
			LoadedWidget->CalculateObjectToWorldTransform(true);
#if WITH_EDITOR
			TArray<ULexWidget*> AllLoadedWidgets;
			ULexWidget::CollectChildrenWidgets(LoadedWidget.Get(), AllLoadedWidgets, true);
			if (World->WorldType == EWorldType::Editor)
			{
				for (auto Widget : AllLoadedWidgets)
				{
					//set transient in edit mode because we don't want to save these widgets in level, not set in game mode because no need to
					//skip EditorPreview mode because we need full transactional
					Widget->SetFlags(RF_Transient);
				}
			}
			OverallVersionMD5 = WidgetPrefab->GenerateOverallVersionMD5();//store version for auto update
#endif
		}
	}
#if WITH_EDITOR
	if (!bIsSpawnFromFactory)//if spawn from prefab-factory then the "CheckNecessaryObjects" is handled from there
	{
		auto World = GetWorld();
		if (World && World->WorldType != EWorldType::EditorPreview && !World->IsGameWorld())//Edit mode and not BlueprintEditorPreview
		{
			ULexUIManagerObject::AddOneShotTickFunction([WeakThis = MakeWeakObjectPtr(this)]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->CheckNecessaryObjects();
					MarkNeedCheckNecessaryObjects();
				}
			}, 1);
		}
	}
#endif
}

#if WITH_EDITOR
void ULexUIPrefabPresenterComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.MemberProperty != nullptr)
	{
		auto PropertyName = PropertyChangedEvent.GetMemberPropertyName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUIPrefabPresenterComponent, WidgetPrefab))
		{
			LoadWidget();
		}
	}
}

void ULexUIPrefabPresenterComponent::CheckPrefabVersion()
{
	if (IsValid(WidgetPrefab))
	{
		if (OverallVersionMD5 != WidgetPrefab->GenerateOverallVersionMD5())
		{
			LoadWidget();
		}
	}
	else
	{
		if (LoadedWidget.IsValid())
		{
			LoadedWidget->DestroyWidget();
			LoadedWidget = nullptr;
		}
	}
}

#endif

void ULexUIPrefabPresenterComponent::SetPrefab(ULexUIPrefab* Value)
{
	if (WidgetPrefab != Value)
	{
		WidgetPrefab = Value;
		LoadWidget();
	}
}

#undef LOCTEXT_NAMESPACE
