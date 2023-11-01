// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabFactory.h"
#include "LGUIEditorModule.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabFactory"


ULGUIPrefabFactory::ULGUIPrefabFactory()
{
	SupportedClass = ULGUIPrefab::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

class FAssetClassParentFilter : public IClassViewerFilter
{
public:
	FAssetClassParentFilter()
		: DisallowedClassFlags(CLASS_None), bDisallowBlueprintBase(false)
	{}

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	/** Disallow blueprint base classes. */
	bool bDisallowBlueprintBase;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bAllowed = !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InClass->CanCreateAssetOfClass()
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		if (bAllowed && bDisallowBlueprintBase)
		{
			if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
			{
				return false;
			}
		}

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		if (bDisallowBlueprintBase)
		{
			return false;
		}

		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};
bool ULGUIPrefabFactory::ConfigureProperties()
{
	// Null the CurveClass so we can get a clean class
	RootActorClass = nullptr;

	// Load the classviewer module to display a class picker
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

	TSharedPtr<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
	Options.ClassFilters.Add(Filter.ToSharedRef());

	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists;
	Filter->AllowedChildrenOfClasses.Add(AActor::StaticClass());

	const FText TitleText = LOCTEXT("CreatePrefabOptions", "Pick Class as Root-Actor");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, AActor::StaticClass());

	if (bPressedOk)
	{
		RootActorClass = ChosenClass;
	}

	return bPressedOk;
}
UObject* ULGUIPrefabFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (SourcePrefab != nullptr)//prefab variant
	{
		ULGUIPrefab* NewAsset = NewObject<ULGUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
		NewAsset->bIsPrefabVariant = true;
		ULGUIPrefabHelperObject* HelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage());
		HelperObject->PrefabAsset = NewAsset;
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLGUISubPrefabData> SubPrefabMap;
		HelperObject->LoadedRootActor = SourcePrefab->LoadPrefabWithExistingObjects(ULGUIPrefabManagerObject::GetPreviewWorldForPrefabPackage(), nullptr, MapGuidToObject, SubPrefabMap);
		FLGUISubPrefabData SubPrefabData;
		SubPrefabData.PrefabAsset = SourcePrefab;
		SubPrefabData.MapGuidToObject = MapGuidToObject;
		for (auto& KeyValue : MapGuidToObject)
		{
			auto GuidInParent = FGuid::NewGuid();
			SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParent, KeyValue.Key);
			HelperObject->MapGuidToObject.Add(GuidInParent, KeyValue.Value);
		}
		HelperObject->SubPrefabMap.Add(HelperObject->LoadedRootActor, SubPrefabData);
		HelperObject->SavePrefab();
		HelperObject->LoadedRootActor->Destroy();
		HelperObject->ConditionalBeginDestroy();
		return NewAsset;
	}
	else
	{
		ULGUIPrefab* NewAsset = NewObject<ULGUIPrefab>(InParent, Class, Name, Flags | RF_Transactional);
		NewAsset->bIsPrefabVariant = false;
		ULGUIPrefabHelperObject* HelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage());
		HelperObject->PrefabAsset = NewAsset;
		HelperObject->LoadedRootActor = ULGUIPrefabManagerObject::GetPreviewWorldForPrefabPackage()->SpawnActor<AActor>(RootActorClass);
		HelperObject->LoadedRootActor->SetActorLabel(TEXT("RootActor"));
		if (!HelperObject->LoadedRootActor->GetRootComponent())
		{
			USceneComponent* RootComponent = NewObject<USceneComponent>(HelperObject->LoadedRootActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
			RootComponent->Mobility = EComponentMobility::Movable;
			RootComponent->bVisualizeComponent = false;

			HelperObject->LoadedRootActor->SetRootComponent(RootComponent);
			RootComponent->RegisterComponent();
			HelperObject->LoadedRootActor->AddInstanceComponent(RootComponent);
		}
		HelperObject->SavePrefab();
		HelperObject->LoadedRootActor->Destroy();
		HelperObject->ConditionalBeginDestroy();
		return NewAsset;
	}
}

#undef LOCTEXT_NAMESPACE
