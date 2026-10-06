// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexVisualBatchMeshCustomization.h"
#include "Core/Components/LexVisualBatchMesh.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "MaterialDomain.h"

#define LOCTEXT_NAMESPACE "LexVisualBatchMeshCustomization"
FLexVisualBatchMeshCustomization::FLexVisualBatchMeshCustomization()
{
}

FLexVisualBatchMeshCustomization::~FLexVisualBatchMeshCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexVisualBatchMeshCustomization::MakeInstance()
{
	return MakeShareable(new FLexVisualBatchMeshCustomization);
}
void FLexVisualBatchMeshCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULexVisualBatchMesh>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LexUI");
}
#undef LOCTEXT_NAMESPACE