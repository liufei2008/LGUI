// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/UILayoutBaseCustomization.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "UILayoutBaseCustomization"

TSharedRef<IDetailCustomization> FUILayoutBaseCustomization::MakeInstance()
{
	return MakeShareable(new FUILayoutBaseCustomization);
}
void FUILayoutBaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUILayoutBase>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		if (TargetScriptPtr->GetWorld()->WorldType == EWorldType::Editor)
		{
			TargetScriptPtr->OnRebuildLayout();
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	if (AActor* owner = TargetScriptPtr->GetOwner())
	{
		auto layoutClassArray = owner->GetComponentsByClass(UUILayoutBase::StaticClass());
		if (layoutClassArray.Num() > 1)
		{
			IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
			category.AddCustomRow(FText::FromString(TEXT("Warning")))
				.WholeRowContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Multiple UILayout component on same actor, this may cause some problem!"))))
					.AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(FLinearColor(FColor(255,0,0,255))))
				];
		}
	}
}
#undef LOCTEXT_NAMESPACE