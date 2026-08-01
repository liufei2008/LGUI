// Copyright 2019-Present LexLiu. All Rights Reserved.


#include "XMLSupport/LexUIMLPresenterComponent.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"
#include "LGUI.h"
#include "Core/LexUIManager.h"
#include "XMLSupport/LexUIMLBehaviour.h"

ULexUIMLPresenterComponent::ULexUIMLPresenterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


void ULexUIMLPresenterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULexUIMLPresenterComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (auto World = GetWorld())
	{
		if (!World->IsGameWorld())
		{
			RegisterEditorFocusCheck();
		}
	}
#endif
}

void ULexUIMLPresenterComponent::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	UnregisterEditorFocusCheck();
#endif
}

void ULexUIMLPresenterComponent::LoadWidget()
{
	if (LoadedWidget.IsValid())
	{
		LoadedWidget->DestroyWidget();
		LoadedWidget = nullptr;
	}
#if WITH_EDITOR
	if (this->GetName().Contains(TEXT("SKEL_")) || this->GetName().Contains(TEXT("TRASH_")))
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d - Skip LoadWidget for %s (temp object)"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetName());
		return;
	}
#endif
	if (!XAMLScriptClass)return;

	auto World = GetWorld();
	if (!World) return;

	auto Behaviour = ULexUIMLBehaviour::CreateByClass(XAMLScriptClass, World, nullptr, nullptr, false, nullptr);
	if (!Behaviour)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d - Failed to load Widget with class: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *XAMLScriptClass->GetPathName());
		return;
	}
	LoadedWidget = Behaviour->GetWidget();

	// Swap canvas: remove built-in canvas from loaded widget, replace with CanvasTemplate
	if (auto Canvas = LoadedWidget->GetComponent<ULexCanvas>())
	{
		LoadedWidget->RemoveComponent(Canvas);
	}
	RootCanvas = LoadedWidget->AddComponentByTemplate<ULexCanvas>(CanvasTemplate);
	RootCanvas->AttachToWidgetPresenterComponent(this);

	LoadedWidget->CalculateObjectToWorldTransform(true);

#if WITH_EDITOR
	TArray<ULexWidget*> AllLoadedWidgets;
	ULexWidget::CollectChildrenWidgets(LoadedWidget.Get(), AllLoadedWidgets, true);
	if (World->WorldType == EWorldType::Editor)
	{
		for (auto Widget : AllLoadedWidgets)
		{
			Widget->SetFlags(RF_Transient);
		}
	}
#endif

#if WITH_EDITOR
	if (!bIsSpawnFromFactory)//if spawn from factory then the "CheckNecessaryObjects" is handled from there
	{
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
#include "Misc/SecureHash.h"
void ULexUIMLPresenterComponent::RegisterEditorFocusCheck()
{
	if (!EditorFocusHandle.IsValid())
	{
		EditorFocusHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().AddWeakLambda(this, [this](const bool bApplicationIsActive)
		{
			if (bApplicationIsActive)
			{
				CheckAndReloadWidget();
			}
		});
	}
}

void ULexUIMLPresenterComponent::UnregisterEditorFocusCheck()
{
	if (EditorFocusHandle.IsValid())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().Remove(EditorFocusHandle);
		EditorFocusHandle.Reset();
	}
}

void ULexUIMLPresenterComponent::CheckAndReloadWidget()
{
	const FString FilePath = GetXAMLFilePath();
	if (FilePath.IsEmpty()) return;

	FMD5Hash Hash = FMD5Hash::HashFile(*FilePath);
	const FString CurrentMD5 = Hash.IsValid() ? FString::FromBlob(Hash.GetBytes(), Hash.GetSize()) : FString();
	if (CurrentMD5.IsEmpty()) return;

	if (CachedFileMD5.IsEmpty())
	{
		CachedFileMD5 = CurrentMD5;
		return;
	}

	if (CachedFileMD5 != CurrentMD5)
	{
		UE_LOG(LGUI, Log, TEXT("[%s].%d - Source file changed, reloading: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *FilePath);
		CachedFileMD5 = CurrentMD5;
		ReloadWidget();
	}
}
#endif

void ULexUIMLPresenterComponent::SetScriptClass(TSubclassOf<ULexUIMLBehaviour> Value)
{
	if (XAMLScriptClass != Value)
	{
		XAMLScriptClass = Value;
		LoadWidget();
	}
}

FString ULexUIMLPresenterComponent::GetXAMLFilePath() const
{
	if (XAMLScriptClass)
	{
		FString RelativePath;
		ULexUIMLResource* Resource = nullptr;
		GetDefault<ULexUIMLBehaviour>(XAMLScriptClass)->GetUIMLData(RelativePath, Resource);
		if (!RelativePath.IsEmpty())
		{
			return FPaths::ProjectContentDir() / RelativePath;
		}
	}
	return FString();
}


