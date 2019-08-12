// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIRootCustomization.h"
#include "Window/LGUIEditorTools.h"
#include "Window/LGUIScreenSpaceUIViewer.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "UIRootCustomization"

TSharedRef<IDetailCustomization> FUIRootCustomization::MakeInstance()
{
	return MakeShareable(new FUIRootCustomization);
}
void FUIRootCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIRoot>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	
	auto snapModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIRoot, UISnapMode));
	snapModeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	uint8 snapMode;
	snapModeHandle->GetValue(snapMode);
	TArray<FName> needToHidePropertyNameArray;
	if (snapMode == (uint8)(LGUISnapMode::WorldSpace))
	{
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, UIScaleMode));
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, PreferredHeight));
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, PreferredWidth));
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, DistanceToCamera));
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, SceneCapture));
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, SnapRenderTargetToViewportSize));
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, RenderTargetSizeMultiply));
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, SnapPixel));
	}
	else if (snapMode == (uint8)(LGUISnapMode::SnapToViewTargetCamera) || snapMode == (uint8)(LGUISnapMode::SnapToSceneCapture))
	{
		if (snapMode == (uint8)(LGUISnapMode::SnapToViewTargetCamera))
		{
			needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, SceneCapture));
		}
		auto scaleModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIRoot, UIScaleMode));
		scaleModeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		uint8 scaleMode;
		scaleModeHandle->GetValue(scaleMode);
		if (scaleMode == (uint8)(LGUIScaleMode::ScaleWithScreenWidth))
		{
			needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, PreferredHeight));
		}
		else if (scaleMode == (uint8)(LGUIScaleMode::ScaleWithScreenHeight))
		{
			needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, PreferredWidth));
		}
		else if (scaleMode == (uint8)(LGUIScaleMode::ConstantPixelSize))
		{
			needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, PreferredHeight));
			needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, PreferredWidth));
		}

		if (snapMode != (uint8)(LGUISnapMode::SnapToSceneCapture))
		{
			needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, SnapRenderTargetToViewportSize));
			needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, RenderTargetSizeMultiply));
		}
	}

	auto useOverrideMaterialsHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIRoot, UseOverrideMaterials));
	useOverrideMaterialsHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	bool useOverrideMaterials;
	useOverrideMaterialsHandle->GetValue(useOverrideMaterials);
	if (!useOverrideMaterials)
	{
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, OverrideMaterials));
	}

	auto useFirstPawnAsUIOwnerHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIRoot, UseFirstPawnAsUIOwner));
	useFirstPawnAsUIOwnerHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	bool useFirstPawnAsUIOwner;
	useFirstPawnAsUIOwnerHandle->GetValue(useFirstPawnAsUIOwner);
	if (useFirstPawnAsUIOwner)
	{
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, UIOwner));
	}

	auto snapRenderTargetToViewportSizeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIRoot, SnapRenderTargetToViewportSize));
	snapRenderTargetToViewportSizeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] {DetailBuilder.ForceRefreshDetails(); }));
	bool snapRenderTargetToViewportSize;
	snapRenderTargetToViewportSizeHandle->GetValue(snapRenderTargetToViewportSize);
	if (!snapRenderTargetToViewportSize)
	{
		needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(UUIRoot, RenderTargetSizeMultiply));
	}

	for (auto item : needToHidePropertyNameArray)
	{
		DetailBuilder.HideProperty(item);
	}

	if (snapMode == (uint8)(LGUISnapMode::SnapToSceneCapture))
	{
		IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");
		lguiCategory.AddCustomRow(LOCTEXT("OpenScreenSpaceUIViewer", "Open Screen Space UI Viewer"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("OpenScreenSpaceUIViewer", "Open Screen Space UI Viewer"))
			.OnClicked_Lambda([=] {
			auto renderTargetTexture = TargetScriptPtr->GetPreviewRenderTarget();
			SLGUIScreenSpaceUIViewer::CurrentScreenSpaceUIRenderTarget = renderTargetTexture;
			SLGUIScreenSpaceUIViewer::CurrentUIRoot = TargetScriptPtr.Get();
			ULGUIEditorToolsAgentObject::OpenScreenSpaceUIViewer_Impl();
			return FReply::Handled();
			})
		]
		;
	}
}
#undef LOCTEXT_NAMESPACE