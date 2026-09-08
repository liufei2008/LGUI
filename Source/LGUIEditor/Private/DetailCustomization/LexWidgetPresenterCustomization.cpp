// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexWidgetPresenterCustomization.h"

#include "DetailCategoryBuilder.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Core/LexWidgetPresenterComponent.h"
#include "Core/Components/LexCanvas.h"
#include "Window/LexUIWidgetInspector.h"

#define LOCTEXT_NAMESPACE "LexWidgetPresenterBaseCustomization"
FLexWidgetPresenterCustomization::FLexWidgetPresenterCustomization()
{
}

FLexWidgetPresenterCustomization::~FLexWidgetPresenterCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexWidgetPresenterCustomization::MakeInstance()
{
	return MakeShareable(new FLexWidgetPresenterCustomization);
}
void FLexWidgetPresenterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptArray.Empty();
	for (auto Item : TargetObjects)
	{
		if (auto ValidItem = Cast<ULexWidgetPresenterComponent>(Item.Get()))
		{
			TargetScriptArray.Add(ValidItem);
		}
	}
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	auto TargetWorld = TargetScriptArray[0]->GetWorld();

	auto& Category = DetailBuilder.EditCategory("LexWidgetPresenter");

	// ReloadWidget button
	Category.AddCustomRow(LOCTEXT("ReloadWidget", "ReloadWidget"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReloadWidget", "ReloadWidget"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([=, this]()
			{
				for (auto& Target : TargetScriptArray)
				{
					if (Target.IsValid())
					{
						Target->ReloadWidget();
					}
				}
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ReloadWidgetBtn", "Reload"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];

	TSharedPtr<FTabManager> HostTabManager = nullptr;
	if (auto DetailsView = DetailBuilder.GetDetailsViewSharedPtr())
	{
		HostTabManager = DetailsView->GetHostTabManager();
	}
	if (!HostTabManager.IsValid())
	{
		HostTabManager = FGlobalTabmanager::Get();
	}

	bool bIsExternalTabAlreadyOpened = false;

	TSharedPtr<SDockTab> ExistingTab = HostTabManager->FindExistingLiveTab(FLGUIEditorModule:: LexUIWidgetInspectorTabName);
	if (ExistingTab.IsValid())
	{
		auto WidgetInspector = StaticCastSharedRef<SLexUIWidgetInspector>(ExistingTab->GetContent());
		bIsExternalTabAlreadyOpened = TargetWorld != nullptr && WidgetInspector->GetWorld() == TargetWorld;
	}
	Category.AddCustomRow(FText())
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("WidgetInspector", "WidgetInspector"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Visibility_Lambda([=, this]()
			{
				if (TargetScriptArray.Num() > 0 && TargetScriptArray[0] != nullptr && TargetScriptArray[0]->GetWorld() != nullptr)
				{
					return EVisibility::Visible;
				}
				return EVisibility::Collapsed;
			})
			.OnClicked_Lambda([=, this]()
			{
				if (auto Tab = FGlobalTabmanager::Get()->TryInvokeTab(FLGUIEditorModule::LexUIWidgetInspectorTabName))
				{
					StaticCastSharedRef<SLexUIWidgetInspector>(Tab->GetContent())->AssignWorld(TargetScriptArray[0]->GetWorld());
				}
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(bIsExternalTabAlreadyOpened ? LOCTEXT("OpenWidgetInspector", "Focus Tab") : LOCTEXT("OpenWidgetInspector", "Open in Tab"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];

	//canvas template
	{
		auto CanvasTemplate_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidgetPresenterComponent, CanvasTemplate));
		ULexCanvas* CanvasTemplate = nullptr;
		CanvasTemplate_PH->GetValue(*(UObject**)&CanvasTemplate);
		ELexRenderMode RenderMode = ELexRenderMode::WorldSpace;
		if (CanvasTemplate != nullptr)
		{
			RenderMode = CanvasTemplate->GetRenderMode();
		}
		auto& CanvasTemplateCategory = DetailBuilder.EditCategory("CanvasTemplate");
		if (RenderMode == ELexRenderMode::WorldSpace || RenderMode == ELexRenderMode::WorldSpace_LexUI)
		{
			CanvasTemplateCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexWidgetPresenterComponent, RootSizeForWorldSpaceWidget));
		}
		else
		{
			DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexWidgetPresenterComponent, RootSizeForWorldSpaceWidget));
		}
		auto CanvasTemplateRow = CanvasTemplateCategory.AddExternalObjects({ CanvasTemplate }, EPropertyLocation::Default
			, FAddPropertyParams().HideRootObjectNode(true).CreateCategoryNodes(true));
		CanvasTemplateRow->ShouldAutoExpand(true);
		CanvasTemplateRow->Visibility(TAttribute<EVisibility>::CreateSPLambda(this, [=]()
		{
			return (TargetWorld && TargetWorld->IsGameWorld()) ? EVisibility::Collapsed : EVisibility::Visible;
		}));
		DetailBuilder.HideProperty(CanvasTemplate_PH);
	}
}
void FLexWidgetPresenterCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE