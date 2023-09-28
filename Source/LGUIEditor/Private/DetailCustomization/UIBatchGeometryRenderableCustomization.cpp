// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIBatchGeometryRenderableCustomization.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "MaterialShared.h"

#define LOCTEXT_NAMESPACE "UIBatchGeometryRenderableCustomization"
FUIBatchGeometryRenderableCustomization::FUIBatchGeometryRenderableCustomization()
{
}

FUIBatchGeometryRenderableCustomization::~FUIBatchGeometryRenderableCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUIBatchGeometryRenderableCustomization::MakeInstance()
{
	return MakeShareable(new FUIBatchGeometryRenderableCustomization);
}
void FUIBatchGeometryRenderableCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIBatchGeometryRenderable>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LGUI");

	auto CustomUIMaterialHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIBatchGeometryRenderable, CustomUIMaterial));
	CustomUIMaterialHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, &DetailBuilder] {
		DetailBuilder.ForceRefreshDetails();
		}));

	LGUICategory.AddProperty(CustomUIMaterialHandle);
	UMaterialInterface* CustomUIMaterial = nullptr;
	CustomUIMaterialHandle->GetValue((UObject*&)CustomUIMaterial);
	if (CustomUIMaterial)
	{
		if (auto Mat = CustomUIMaterial->GetMaterial())
		{
			if (Mat->MaterialDomain != EMaterialDomain::MD_Surface)
			{
				LGUICategory.AddCustomRow(LOCTEXT("MaterialDomainErrorTipRow", "MaterialDomainErrorTip"))
					.WholeRowContent()
					.MinDesiredWidth(500)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("MaterialDomainErrorTip", "CustomUIMaterial should use Surface domain!"))
						.ColorAndOpacity(FLinearColor(FColor::Yellow))
						.AutoWrapText(true)
					]
					;
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE