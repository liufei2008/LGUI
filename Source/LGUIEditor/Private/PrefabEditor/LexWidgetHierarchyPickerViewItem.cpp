// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexWidgetHierarchyPickerViewItem.h"
#include "Styling/CoreStyle.h"
#include "LexUIPrefabEditor.h"
#include "ScopedTransaction.h"
#include "Editor.h"
#include "LGUIEditorModule.h"
#include "LGUIEditorStyle.h"
#include "Core/LexUIBehaviour.h"
#include "Core/Components/LexVisual.h"
#include "Core/Components/LexWidget.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "LexWidgetHierarchyPickerViewItem"

class SHoverBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHoverBox) {}
	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_EVENT(FSimpleDelegate, OnMouseEnter)
	SLATE_EVENT(FSimpleDelegate, OnMouseLeave)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs)
	{
		OnMouseEnterDelegate = InArgs._OnMouseEnter;
		OnMouseLeaveDelegate = InArgs._OnMouseLeave;

		ChildSlot
		[
			InArgs._Content.Widget
		];
	}

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (OnMouseEnterDelegate.IsBound())
			OnMouseEnterDelegate.Execute();
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		if (OnMouseLeaveDelegate.IsBound())
			OnMouseLeaveDelegate.Execute();
	}

private:
	FSimpleDelegate OnMouseEnterDelegate;
	FSimpleDelegate OnMouseLeaveDelegate;
};

void SLexWidgetHierarchyPickerViewItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, SLexWidgetHierarchyPickerView::DataType InModel
	, UClass* InObjectClass)
{
	Model = InModel;

	MenuBuilder = new FMenuBuilder(true, NULL, TSharedPtr<FExtender>(), false, &FCoreStyle::Get(), false);
	auto Widget = Model->Widget;
	MenuBuilder->BeginSection("WidgetSection", LOCTEXT("WidgetMenu", "Widget"));
	{
		if (Widget->IsA(InObjectClass))
		{
			MenuBuilder->AddMenuEntry(FText::FromString(FString::Printf(TEXT("%s (%s)"), *Widget->GetDisplayName(), *Widget->GetClass()->GetName())), FText::GetEmpty(), FSlateIconFinder::FindIconForClass(Widget->GetClass())
				, FUIAction(FExecuteAction::CreateLambda([=]()
				{
					InArgs._OnSelectObject.ExecuteIfBound(Widget.Get());
				})));
		}
	}
	MenuBuilder->EndSection();
	
	//widget subobjects
	{
		TArray<UObject*> SubObjects;
		ForEachObjectWithOuter(Widget.Get(), [&](UObject* SubObject)
		{
			if (SubObject->IsA(InObjectClass)
				&& !SubObject->IsA<ULexUIBehaviour>()//Component is handled below
				)
			{
				SubObjects.Add(SubObject);
			}
		}, false);
		if (SubObjects.Num() > 0)
		{
			MenuBuilder->BeginSection("SubObjectSection", LOCTEXT("SubObjectMenu", "SubObjects"));
			for (UObject* Object : SubObjects)
			{
				MenuBuilder->AddMenuEntry(FText::FromString(FString::Printf(TEXT("%s (%s)"), *Object->GetName(), *Object->GetClass()->GetName())), FText::GetEmpty(), FSlateIconFinder::FindIconForClass(Object->GetClass())
				, FUIAction(FExecuteAction::CreateLambda([=]()
				{
					InArgs._OnSelectObject.ExecuteIfBound(Object);
				})));
			}
			MenuBuilder->EndSection();
		}
	}
	
	MenuBuilder->BeginSection("ComponentsSection", LOCTEXT("ComponentsMenu", "Components"));
	auto Components = Widget->GetAllComponents();
	for (auto Component : Components)
	{
		if (Component->IsA(InObjectClass))
		{
			MenuBuilder->AddMenuEntry(FText::FromString(FString::Printf(TEXT("%s (%s)"), *Component->GetName(), *Component->GetClass()->GetName())), FText::GetEmpty(), FSlateIconFinder::FindIconForClass(Component->GetClass())
				, FUIAction(FExecuteAction::CreateLambda([=]()
				{
					InArgs._OnSelectObject.ExecuteIfBound(Component);
				})));
		}
		TArray<UObject*> SubObjects;
		ForEachObjectWithOuter(Component, [&](UObject* SubObject)
		{
			if (SubObject->IsA(InObjectClass))
			{
				SubObjects.Add(SubObject);
			}
		}, false);
		if (SubObjects.Num() > 0)
		{
			MenuBuilder->AddSubMenu(
				FText::FromString(FString::Printf(TEXT("%s (%s)"), *Component->GetName(), *Component->GetClass()->GetName())),
				FText::GetEmpty(), FNewMenuDelegate::CreateLambda([=](FMenuBuilder& SubMenuBuilder)
				{
					for (UObject* Object : SubObjects)
					{
						SubMenuBuilder.AddMenuEntry(FText::FromString(FString::Printf(TEXT("%s (%s)"), *Object->GetName(), *Object->GetClass()->GetName())), FText::GetEmpty(), FSlateIconFinder::FindIconForClass(Object->GetClass())
						, FUIAction(FExecuteAction::CreateLambda([=]()
						{
							InArgs._OnSelectObject.ExecuteIfBound(Object);
						})));
					}
				}), false, FSlateIconFinder::FindIconForClass(Component->GetClass()));
		}
	}
	MenuBuilder->EndSection();

	STableRow<SLexWidgetHierarchyPickerView::DataType>::Construct(
		STableRow<SLexWidgetHierarchyPickerView::DataType>::FArguments()
		.Padding(0.0f)
		.Content()
		[
			SAssignNew(MenuAnchor, SMenuAnchor)
			.Placement(MenuPlacement_MenuRight)
			[
				SNew(SHoverBox)
				.OnMouseEnter_Lambda([=, this]()
				{
					if (!MenuAnchor->IsOpen())
					{
						MenuAnchor->SetIsOpen(true, false);
					}
				})
				.OnMouseLeave_Lambda([=, this]()
				{
					
				})
				[
					SNew(SHorizontalBox)
					// Widget icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(2, 0)
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image_Lambda([=, this]()
						{
							if (Widget.IsValid())
							{
								if (Widget->GetVisual())
									return FSlateIconFinder::FindIconBrushForClass(Widget->GetVisual()->GetClass());
								return FSlateIconFinder::FindIconBrushForClass(ULexWidget::StaticClass());
							}
							return (const FSlateBrush*)nullptr;
						})
					]
					// Interaction icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(2, 0)
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image_Lambda([=, this]()
						{
							return FLGUIEditorModule::Get().GetInteractionIconBrush(Widget.Get());
						})
					]

					// Canvas
					+SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(2, 0)
					[
						SNew(SBox)
						.Visibility_Lambda([=, this]()
						{
							if (Widget.IsValid() && Widget->IsCanvasWidget())
							{
								return EVisibility::Visible;
							}
							return EVisibility::Collapsed;
						})
						[
							SNew(SBox)
							.WidthOverride(16)
							.HeightOverride(16)
							.Padding(FMargin(0))
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							[
								SNew(SImage)
								.Image(FLGUIEditorStyle::Get().GetBrush("CanvasMark"))
								.Visibility_Lambda([=, this]()
								{
									if (Widget->IsCanvasWidget())
									{
										return EVisibility::Visible;
									}
									return EVisibility::Hidden;
								})
								.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.4f))
							]
						]
					]			

					// Name of the widget
					+SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(2, 0, 0, 0)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([=]()
						{
							return FText::FromString(InModel->DisplayText);
						})
					]

					// Arrow
					+SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew( SBox )
						.Visibility_Lambda([=, this]()
						{
							return Model->ValidObjectArray.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed; 
						})
						.Padding(FMargin(7,0,0,0))
						[
							SNew( SImage )
							.Image( FAppStyle::Get().GetBrush( "Menu.SubMenuIndicator" ) )
						]
					]
				]
			]
			.OnGetMenuContent_Lambda([this]()
			{
				return MenuBuilder->MakeWidget();
			})
		],
		InOwnerTableView);
}

SLexWidgetHierarchyPickerViewItem::~SLexWidgetHierarchyPickerViewItem()
{
	delete MenuBuilder;
}

#undef LOCTEXT_NAMESPACE
