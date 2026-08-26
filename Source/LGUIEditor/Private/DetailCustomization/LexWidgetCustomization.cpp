// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexWidgetCustomization.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "IDetailGroup.h"
#include "LGUIEditorStyle.h"
#include "Editor.h"
#include "Widget/ComponentTransformDetails.h"
#include "Widget/AnchorPreviewWidget.h"
#include "PropertyCustomizationHelpers.h"
#include "HAL/PlatformApplicationMisc.h"
#include "LexUIEditorUtils.h"
#include "LexUIEditorTools.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "UnrealEdGlobals.h"
#include "Core/Components/LexCanvas.h"
#include "Editor/UnrealEdEngine.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"
#include "Utils/LexUIUtils.h"

#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "UIItemComponentDetails"

class SLexWidgetSubObjectWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLexWidgetSubObjectWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<IPropertyHandle> InPropertyHandle, bool InEditable)
	{
		PropertyHandle = InPropertyHandle;
		auto VisualPropertyValueWidget = InPropertyHandle->CreatePropertyValueWidget();
		if (!InEditable)
		{
			VisualPropertyValueWidget->SetEnabled(false);
		}
		ChildSlot
		[
			VisualPropertyValueWidget
		];
	}

	virtual FReply OnMouseButtonUp(
		const FGeometry& MyGeometry,
		const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			OpenContextMenu(MouseEvent);
			return FReply::Handled();
		}

		return FReply::Unhandled();
	}

private:
	TSharedPtr<IPropertyHandle> PropertyHandle;
	static TWeakObjectPtr<UObject> CopiedObject;
	
	void OpenContextMenu(const FPointerEvent& MouseEvent)
	{
		FMenuBuilder MenuBuilder(true, nullptr);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("CopyProps", "Copy all properties"),
			LOCTEXT("CopyProps_Tooltip", "You copy all properties of this object then paste it to others"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([=, this]()
			{
				UObject* Object = nullptr;
				PropertyHandle->GetValue(Object);
				if (Object)
				{
					CopiedObject = Object;
				}
			}), FCanExecuteAction::CreateLambda([=, this]()
			{
				UObject* Object = nullptr;
				PropertyHandle->GetValue(Object);
				return Object != nullptr;
			}))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("PasteProps", "Paste all properties"),
			LOCTEXT("PasteProps_Tooltip", "You paste all properties of copied object to this"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([=, this]()
			{
				UObject* Object = nullptr;
				PropertyHandle->GetValue(Object);
				if (Object)
				{
					UEngine::FCopyPropertiesForUnrelatedObjectsParams Options;
					Options.bNotifyObjectReplacement = true;
					UEditorEngine::CopyPropertiesForUnrelatedObjects(CopiedObject.Get(), Object, Options);
					Object->PostReinitProperties();
				}
			}), FCanExecuteAction::CreateLambda([=, this]()
			{
				UObject* Object = nullptr;
				PropertyHandle->GetValue(Object);
				return Object != nullptr && CopiedObject.IsValid();
			}))
		);

		FSlateApplication::Get().PushMenu(
			AsShared(),
			FWidgetPath(),
			MenuBuilder.MakeWidget(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);
	}
};
TWeakObjectPtr<UObject> SLexWidgetSubObjectWidget::CopiedObject = nullptr;


FLexWidgetCustomization::FLexWidgetCustomization()
{
	
}
FLexWidgetCustomization::~FLexWidgetCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexWidgetCustomization::MakeInstance()
{
	return MakeShareable(new FLexWidgetCustomization);
}

FText FLexWidgetCustomization::GetAnchorsTooltipText()const
{
	return GetLayoutControlAnchorValue().AnyControl() ? LOCTEXT("ChangeAnchor_Tooltip", "Change anchor") : LOCTEXT("AnchorIsControlledByLayout", "Anchor is controlled by layout");
}

void FLexWidgetCustomization::ForceUpdateUI()
{
	for (auto item : TargetScriptArray)
	{
		if (item.IsValid())
		{
			item->MarkCanvasUpdate(true);
		}
	}
}

void FLexWidgetCustomization::CustomizeDetails( const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder )
{
	DetailBuilder = InDetailBuilder;
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder->GetObjectsBeingCustomized(TargetObjects);
	TargetScriptArray.Empty();
	bool bIsSubPrefab = false;
	for (auto Item : TargetObjects)
	{
		if (auto ValidItem = Cast<ULexWidget>(Item.Get()))
		{
			TargetScriptArray.Add(ValidItem);
			if (ValidItem->GetWorld() != nullptr)
			{
				if (ValidItem->GetWorld()->WorldType == EWorldType::Editor)
				{
					if (auto PrefabHelper = ULexUIPrefabHelperObject::GetPrefabHelperObject_WhichManageThisWidget(ValidItem))
					{
						bIsSubPrefab = PrefabHelper->IsWidgetBelongsToSubPrefab(ValidItem);
					}
					ValidItem->MarkCanvasUpdate(true);
				}
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}

	IDetailCategoryBuilder& LGUICategory = DetailBuilder->EditCategory("LGUI");
	DetailBuilder->HideCategory("TransformCommon");
	IDetailCategoryBuilder& TransformCategory = DetailBuilder->EditCategory("LGUITransform", LOCTEXT("LGUI-Transform", "LGUI-Transform"), ECategoryPriority::Transform);

	//base
	// {
	// 	auto uiActiveHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, bIsUIActive));
	// 	uiActiveHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this] {
	// 		ForceUpdateUI();
	// 	}));
	// }

	DetailBuilder->HideProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData));

	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, bWidgetActive));
	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, RenderOpacity));
	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, bIgnoreParentRenderOpacity));
	auto Clipping_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, Clipping));
	auto& ClippingGroup = LGUICategory.AddGroup(TEXT("ClippingGroup"), LOCTEXT("ClippingGroup", "Clipping"));
	ClippingGroup.HeaderProperty(Clipping_PH);
	{
		DetailBuilder->HideProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, bUniformSetClippingCornerRadius));
		DetailBuilder->HideProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingCornerRadius));

		auto UniformSetCornerRadiusHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, bUniformSetClippingCornerRadius));
		auto CornerRadiusHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingCornerRadius));
		auto CornerRadiusXHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingCornerRadius.X));
		auto CornerRadiusYHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingCornerRadius.Y));
		auto CornerRadiusZHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingCornerRadius.Z));
		auto CornerRadiusWHandle = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingCornerRadius.W));
		auto CornerRadiusPropertyIsEnabledFunction = [=] {
			bool bUniformSetCornerRadius = false;
			UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
			return !bUniformSetCornerRadius;
		};

		CornerRadiusXHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			bool bUniformSetCornerRadius = false;
			UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
			if (bUniformSetCornerRadius)
			{
				float CornerRadiusX;
				CornerRadiusXHandle->GetValue(CornerRadiusX);
				CornerRadiusYHandle->SetValue(CornerRadiusX);
				CornerRadiusZHandle->SetValue(CornerRadiusX);
				CornerRadiusWHandle->SetValue(CornerRadiusX);
			}
			}));

		ClippingGroup.AddWidgetRow()
		.PropertyHandleList({ CornerRadiusHandle, UniformSetCornerRadiusHandle })
		.OverrideResetToDefault(FResetToDefaultOverride::Create(TAttribute<bool>::CreateLambda([=]()
		{
			return UniformSetCornerRadiusHandle->CanResetToDefault() || CornerRadiusHandle->CanResetToDefault();
		}), FSimpleDelegate::CreateLambda([=]()
		{
			UniformSetCornerRadiusHandle->ResetToDefault();
			CornerRadiusHandle->ResetToDefault();
		})))
		.NameContent()
		[
			SNew(SBox)
			.MinDesiredWidth(1000)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(CornerRadiusHandle->GetPropertyDisplayName())
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([=] {
						bool bUniformSetCornerRadius = false;
						UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
						return bUniformSetCornerRadius ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
					.OnCheckStateChanged_Lambda([=](ECheckBoxState NewState){
						bool bUniformSetCornerRadius = NewState == ECheckBoxState::Checked;
						UniformSetCornerRadiusHandle->SetValue(bUniformSetCornerRadius);
						})
					.Style(FAppStyle::Get(), "TransparentCheckBox")
					.ToolTipText(LOCTEXT("UniformSetCornerRadiusToolTip", "When locked, corner radius will all set with x value"))
					[
						SNew(SImage)
						.Image_Lambda([=] {
							bool bUniformSetCornerRadius = false;
							UniformSetCornerRadiusHandle->GetValue(bUniformSetCornerRadius);
							return bUniformSetCornerRadius ? FAppStyle::GetBrush(TEXT("Icons.Lock")) : FAppStyle::GetBrush(TEXT("Icons.Unlock"));
							})
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1)
			[
				CornerRadiusXHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1)
			[
				SNew(SBox)
				.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
				[
					CornerRadiusYHandle->CreatePropertyValueWidget()
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1)
			[
				SNew(SBox)
				.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
				[
					CornerRadiusZHandle->CreatePropertyValueWidget()
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1)
			[
				SNew(SBox)
				.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
				[
					CornerRadiusWHandle->CreatePropertyValueWidget()
				]
			]
		]
		;
	}
	auto ClippingMargin_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingMargin));
	ClippingGroup.AddPropertyRow(ClippingMargin_PH);

	//anchor, width, height
	{
		auto AnchorData_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData));
		auto AnchorMin_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchorMin));
		auto AnchorMax_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchorMax));
		auto AnchoredPosition_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchoredPosition));
		auto SizeDelta_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.SizeDelta));
		FVector2D AnchorMin, AnchorMax;
		AnchorMin_PH->GetValue(AnchorMin);
		AnchorMax_PH->GetValue(AnchorMax);

		//anchors preset menu
		FVector2D anchorItemSize(42, 42);
		float itemBasePadding = 8;
		FMargin AnchorLabelMargin = FMargin(4, 2);
		FMargin AnchorValueMargin = FMargin(2, 2);

		auto MakeAnchorLabelWidget = [&](int AnchorLabelIndex) {
			return
				SNew(SBox)
				.Padding(AnchorLabelMargin)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &FLexWidgetCustomization::GetAnchorLabelText, AnchorMin_PH, AnchorMax_PH, AnchorLabelIndex)
					.ToolTipText(this, &FLexWidgetCustomization::GetAnchorLabelTooltipText, AnchorMin_PH, AnchorMax_PH, AnchorLabelIndex)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			;
		};
		auto DetailBuilderPtr = &DetailBuilder;
		auto MakeAnchorValueWidget = [=, this](int AnchorValueIndex) {
			return
				SNew(SBox)
				.Padding(AnchorValueMargin)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					//GetAnchorPropertyHandle(DetailBuilderPtr, AnchorMinHandle, AnchorMaxHandle, AnchorValueIndex)->CreatePropertyValueWidget()
					SNew(SNumericEntryBox<float>)
					.AllowSpin(true)
					.Delta(1.0f)
					.LinearDeltaSensitivity(1)
					.MinValue(TOptional<float>())
					.MaxValue(TOptional<float>())
					.MinSliderValue(TOptional<float>())
					.MaxSliderValue(TOptional<float>())
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.UndeterminedString( NSLOCTEXT( "PropertyEditor", "MultipleValues", "Multiple Values") )
					.Value(this, &FLexWidgetCustomization::GetAnchorValue, AnchorData_PH, AnchorValueIndex)
					.OnValueChanged(this, &FLexWidgetCustomization::OnAnchorValueChanged, AnchorData_PH, AnchorValueIndex)
					.OnValueCommitted(this, &FLexWidgetCustomization::OnAnchorValueCommitted, AnchorData_PH, AnchorValueIndex)
					.OnBeginSliderMovement(this, &FLexWidgetCustomization::OnAnchorValueSliderMovementBegin)
					.OnEndSliderMovement(this, &FLexWidgetCustomization::OnAnchorValueSliderMovementEnd, AnchorData_PH, AnchorValueIndex)
					.IsEnabled(this, &FLexWidgetCustomization::IsAnchorValueEnable, AnchorData_PH, AnchorValueIndex)
				]
			;
		};
		auto MakeAnchorPreviewWidget = [=, this](LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign HAlign, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign VAlign) {
			return
				SNew(LGUIAnchorPreviewWidget::SAnchorPreviewWidget, anchorItemSize)
				.BasePadding(itemBasePadding)
				.SelectedHAlign(this, &FLexWidgetCustomization::GetAnchorHAlign, AnchorMin_PH, AnchorMax_PH)
				.SelectedVAlign(this, &FLexWidgetCustomization::GetAnchorVAlign, AnchorMin_PH, AnchorMax_PH)
				.PersistentHAlign(HAlign)
				.PersistentVAlign(VAlign)
				.ButtonEnable(true)
				.OnAnchorChange(this, &FLexWidgetCustomization::OnSelectAnchor, AnchorData_PH)
			;
		};//@todo: auto refresh SAnchorPreviewWidget when change from AnchorMinMax

		auto SplitLineColor = FLinearColor(0.5f, 0.5f, 0.5f);
		TransformCategory.AddCustomRow(LOCTEXT("Anchor","Anchor"))
		.CopyAction(FUIAction
		(
			FExecuteAction::CreateSP(this, &FLexWidgetCustomization::OnCopyAnchor),
			FCanExecuteAction::CreateSP(this, &FLexWidgetCustomization::OnCanCopyAnchor)
		))
		.PasteAction(FUIAction
		(
			FExecuteAction::CreateSP(this, &FLexWidgetCustomization::OnPasteAnchor),
			FCanExecuteAction::CreateSP(this, &FLexWidgetCustomization::OnCanPasteAnchor)
		))
		.PropertyHandleList({AnchorData_PH})
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SBox)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(0)
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(1)
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorValueWidget(0)
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorValueWidget(1)
					]
				]

				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(2)
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(3)
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.IsEnabled_Lambda([=, this]()
					{
						if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
						{
							auto Widget = TargetScriptArray[0];
							if (Widget->IsCanvasWidget() && Widget->GetRenderCanvas() != nullptr && Widget->GetRenderCanvas()->IsRenderToScreenSpace())//is root canvas, and is render to screen space
							{
								return false;
							}
						}
						return true;
					})
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.FillWidth(0.5f)
						[
							MakeAnchorValueWidget(2)
						]
						+SHorizontalBox::Slot()
						.FillWidth(0.5f)
						[
							MakeAnchorValueWidget(3)
						]
					]
				]
			]
		]
		.NameContent()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				.Visibility(this, &FLexWidgetCustomization::GetAnchorPresetButtonVisibility)
				[
					SNew(SComboButton)
					.ContentPadding(8)
					.HasDownArrow(false)
					.ToolTipText(this, &FLexWidgetCustomization::GetAnchorsTooltipText)
					.ButtonStyle(FLGUIEditorStyle::Get(), "AnchorButton")
					.ButtonContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.Padding(FMargin(0, 0))
							[
								SNew(SBox)
								.Padding(FMargin(0, 0))
								.HAlign(EHorizontalAlignment::HAlign_Center)
								[
									SNew(STextBlock)
									.Text(this, &FLexWidgetCustomization::GetHAlignText, AnchorMin_PH, AnchorMax_PH)
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
							]
							+SVerticalBox::Slot()
							.Padding(FMargin(0, 0))
							.AutoHeight()
							[
								TargetScriptArray[0]->GetParent() != nullptr
								?
								SNew(SBox)
								[
									SNew(LGUIAnchorPreviewWidget::SAnchorPreviewWidget, FVector2D(40, 40))
									.BasePadding(0)
									.ButtonEnable(false)
									.PersistentHAlign(this, &FLexWidgetCustomization::GetAnchorHAlign, AnchorMin_PH, AnchorMax_PH)
									.PersistentVAlign(this, &FLexWidgetCustomization::GetAnchorVAlign, AnchorMin_PH, AnchorMax_PH)
									//.SelectedHAlign(this, &FUIItemCustomization::GetAnchorHAlign, AnchorMinHandle, AnchorMaxHandle)
									//.SelectedVAlign(this, &FUIItemCustomization::GetAnchorVAlign, AnchorMinHandle, AnchorMaxHandle)
								]
								:
								SNew(SBox)
							]
						]
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.Padding(FMargin(0, 0))
							.HAlign(EHorizontalAlignment::HAlign_Center)
							[
								SNew(STextBlock)
								.Text(this, &FLexWidgetCustomization::GetVAlignText, AnchorMin_PH, AnchorMax_PH)
								.Font(IDetailLayoutBuilder::GetDetailFont())
								.Justification(ETextJustify::Center)
								.RenderTransformPivot(FVector2D(0, 0.5f))
								.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(90)), FVector2D(-12, -10)))
							]
						]
					]
					.MenuContent()
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBorder)
							.Padding(4)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("AnchorPresets", "Anchor Presets"))
								]
								+SVerticalBox::Slot()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("AnchorPresetsHelperKeys", "Shift: Also set pivot		Alt: Also set position"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
							]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SOverlay)
									+SOverlay::Slot()
									[
										SNew(SUniformGridPanel)
										+SUniformGridPanel::Slot(1, 0)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										+SUniformGridPanel::Slot(2, 0) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										+SUniformGridPanel::Slot(3, 0) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										+SUniformGridPanel::Slot(4, 0) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										//Top
										+SUniformGridPanel::Slot(0, 1)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(1, 1)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(2, 1) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(3, 1) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(4, 1) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										//Center
										+SUniformGridPanel::Slot(0, 2)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(1, 2)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(2, 2) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(3, 2) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(4, 2) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										//Bottom
										+SUniformGridPanel::Slot(0, 3)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(1, 3)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(2, 3) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(3, 3) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(4, 3) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										//Bottom stretch
										+SUniformGridPanel::Slot(0, 4)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(1, 4)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(2, 4) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(3, 4) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(4, 4) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
									]
									//split line
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.HAlign(EHorizontalAlignment::HAlign_Left)
										[
											SNew(SBox)
											.WidthOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.HAlign(EHorizontalAlignment::HAlign_Right)
												.WidthOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.HAlign(EHorizontalAlignment::HAlign_Right)
										[
											SNew(SBox)
											.WidthOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.HAlign(EHorizontalAlignment::HAlign_Left)
												.WidthOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.VAlign(EVerticalAlignment::VAlign_Top)
										[
											SNew(SBox)
											.HeightOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.VAlign(EVerticalAlignment::VAlign_Bottom)
												.HeightOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.VAlign(EVerticalAlignment::VAlign_Bottom)
										[
											SNew(SBox)
											.HeightOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.VAlign(EVerticalAlignment::VAlign_Top)
												.HeightOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
								]
							]
						]
					]
				]
			]
		]
		;

		IDetailGroup& AnchorGroup = TransformCategory.AddGroup(FName("Anchors"), LOCTEXT("AnchorsGroup", "Anchors"));

		IDetailPropertyRow& AnchorMinProperty = AnchorGroup.AddPropertyRow(DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchorMin)));
		if (!this->IsAnchorEditable())
		{
			AnchorMinProperty.IsEnabled(false);
			AnchorMinProperty.ToolTip(LOCTEXT("ControlledByLayoutTip", "This property is controlled by layout"));
		}

		IDetailPropertyRow& AnchorMaxProperty = AnchorGroup.AddPropertyRow(DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchorMax)));
		if (!this->IsAnchorEditable())
		{
			AnchorMaxProperty.IsEnabled(false);
			AnchorMaxProperty.ToolTip(LOCTEXT("ControlledByLayoutTip", "This property is controlled by layout"));
		}

		auto& AnchorRawDataGroup = TransformCategory.AddGroup(FName("AnchorsRawData"), LOCTEXT("AnchorsRawData", "AnchorsRawData"), true);
		AnchorRawDataGroup.AddWidgetRow()
		.WholeRowContent()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AnchorRawDataWarning", "Normally do not edit these!"))
				.ColorAndOpacity(FLinearColor(FColor::Yellow))
				.AutoWrapText(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
		;
		auto& AnchoredPositionProperty = AnchorRawDataGroup.AddPropertyRow(DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchoredPosition)));
		auto& SizeDeltaProperty = AnchorRawDataGroup.AddPropertyRow(DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.SizeDelta)));
		AnchoredPositionProperty.IsEnabled(this->IsAnchorEditable());
		SizeDeltaProperty.IsEnabled(this->IsAnchorEditable());
	}
	//pivot
	auto Pivot_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.Pivot));
	auto& PivotPropertyRow = TransformCategory.AddProperty(Pivot_PH);
	PivotPropertyRow.IsEnabled(this->IsAnchorEditable());
	Pivot_PH->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPrePivotChange(Pivot_PH);
		}));
	Pivot_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPivotChanged(Pivot_PH);
		}));
	Pivot_PH->SetOnChildPropertyValuePreChange(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPrePivotChange(Pivot_PH);
		}));
	Pivot_PH->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPivotChanged(Pivot_PH);
		}));

	//location rotation scale
	const FSelectedActorInfo& selectedActorInfo = DetailBuilder->GetDetailsViewSharedPtr()->GetSelectedActorInfo();
	TSharedRef<FComponentTransformDetails> transformDetails = MakeShareable(new FComponentTransformDetails(TargetScriptArray, selectedActorInfo, DetailBuilder.Get()));
	TransformCategory.AddCustomBuilder(transformDetails);
	
	//SiblingIndex
	{
		auto SiblingIndex_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, SiblingIndex));
		DetailBuilder->HideProperty(SiblingIndex_PH);
		SiblingIndex_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this] {
			ForceUpdateUI();
			}));
		auto SiblingIndexWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			[
				SiblingIndex_PH->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("IncreaseHierarchyOrder_Tooltip", "Move order up"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.IsEnabled_Static(FLexUIEditorUtils::IsEnabledOnProperty, SiblingIndex_PH)
				.OnClicked(this, &FLexWidgetCustomization::OnClickIncreaseOrDecreaseSiblingIndex, true, SiblingIndex_PH)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncreaseHierarchyOrder", "+"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("DecreaseHierarchyOrder_Tooltip", "Move order down"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.IsEnabled_Static(FLexUIEditorUtils::IsEnabledOnProperty, SiblingIndex_PH)
				.OnClicked(this, &FLexWidgetCustomization::OnClickIncreaseOrDecreaseSiblingIndex, false, SiblingIndex_PH)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DecreaseHierarchyOrder", "-"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			];
		
		LGUICategory.AddProperty(SiblingIndex_PH, EPropertyLocation::Advanced).IsEnabled(false);//not editable inside PrefabEditor, because we can drag-drop inside it
		LGUICategory.AddProperty(DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, FlattenHierarchyIndex)), EPropertyLocation::Advanced);
	}
		
	//displayName
	auto DisplayName_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, DisplayName));
	LGUICategory.AddProperty(DisplayName_PH);

	//LayoutContainer
	{
		auto LayoutContainer_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, LayoutContainer));
		UObject* LayoutContainer = nullptr;
		LayoutContainer_PH->GetValue(LayoutContainer);
		auto& LayoutCategory = DetailBuilder->EditCategory("LayoutContainer");
		LayoutCategory.AddCustomRow(LOCTEXT("LayoutContainerRow", "LayoutContainer"))
			.NameContent()
			[
				LayoutContainer_PH->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SLexWidgetSubObjectWidget, LayoutContainer_PH, !bIsSubPrefab)
			]
			.OverrideResetToDefault(FResetToDefaultOverride::Create(
				TAttribute<bool>::CreateLambda([LayoutContainer_PH, bIsSubPrefab]()
				{
					return !bIsSubPrefab && LayoutContainer_PH->CanResetToDefault();
				}),
				FSimpleDelegate::CreateLambda([LayoutContainer_PH]()
				{
					LayoutContainer_PH->ResetToDefault();
				})
			));
		LayoutCategory.AddExternalObjects({ LayoutContainer }, EPropertyLocation::Default
			, FAddPropertyParams().HideRootObjectNode(true).CreateCategoryNodes(false));
		DetailBuilder->HideProperty(LayoutContainer_PH);
	}

	//LayoutSelf
	{
		auto LayoutSelf_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, LayoutSelf));
		UObject* LayoutSelf = nullptr;
		LayoutSelf_PH->GetValue(LayoutSelf);
		auto& LayoutSelfCategory = DetailBuilder->EditCategory("LayoutSelf");
		LayoutSelfCategory.AddCustomRow(LOCTEXT("LayoutSelfRow", "LayoutSelf"))
			.NameContent()
			[
				LayoutSelf_PH->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SLexWidgetSubObjectWidget, LayoutSelf_PH, !bIsSubPrefab)
			]
			.OverrideResetToDefault(FResetToDefaultOverride::Create(
				TAttribute<bool>::CreateLambda([LayoutSelf_PH, bIsSubPrefab]()
				{
					return !bIsSubPrefab && LayoutSelf_PH->CanResetToDefault();
				}),
				FSimpleDelegate::CreateLambda([LayoutSelf_PH]()
				{
					LayoutSelf_PH->ResetToDefault();
				})
			));
		LayoutSelfCategory.AddExternalObjects({ LayoutSelf }, EPropertyLocation::Default
			, FAddPropertyParams().HideRootObjectNode(true).CreateCategoryNodes(false));
		DetailBuilder->HideProperty(LayoutSelf_PH);
	}

	//visual
	{
		auto Visual_PH = DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, Visual));
		UObject* Visual = nullptr;
		Visual_PH->GetValue(Visual);
		IDetailCategoryBuilder& VisualCategory = DetailBuilder->EditCategory("Visual");
		VisualCategory.AddCustomRow(LOCTEXT("VisualRow", "Visual"))
			.NameContent()
			[
				Visual_PH->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SLexWidgetSubObjectWidget, Visual_PH, !bIsSubPrefab)
			]
			.OverrideResetToDefault(FResetToDefaultOverride::Create(
				TAttribute<bool>::CreateLambda([Visual_PH, bIsSubPrefab]()
				{
					return !bIsSubPrefab && Visual_PH->CanResetToDefault();
				}),
				FSimpleDelegate::CreateLambda([Visual_PH]()
				{
					Visual_PH->ResetToDefault();
				})
			));
		VisualCategory.AddExternalObjects({ Visual }, EPropertyLocation::Common
			, FAddPropertyParams().HideRootObjectNode(true).CreateCategoryNodes(false));
		DetailBuilder->HideProperty(Visual_PH);
	}
}

void FLexWidgetCustomization::OnPrePivotChange(TSharedPtr<IPropertyHandle> PivotPH)
{
	AnchorOffset.Left = TargetScriptArray[0]->GetAnchorOffsetLeft();
	AnchorOffset.Top = TargetScriptArray[0]->GetAnchorOffsetTop();
	AnchorOffset.Right = TargetScriptArray[0]->GetAnchorOffsetRight();
	AnchorOffset.Bottom = TargetScriptArray[0]->GetAnchorOffsetBottom();
}
void FLexWidgetCustomization::OnPivotChanged(TSharedPtr<IPropertyHandle> PivotPH)
{
	TargetScriptArray[0]->SetAnchorOffsetLeft(AnchorOffset.Left);
	TargetScriptArray[0]->SetAnchorOffsetTop(AnchorOffset.Top);
	TargetScriptArray[0]->SetAnchorOffsetRight(AnchorOffset.Right);
	TargetScriptArray[0]->SetAnchorOffsetBottom(AnchorOffset.Bottom);
}

FReply FLexWidgetCustomization::OnClickIncreaseOrDecreaseSiblingIndex(bool IncreaseOrDecrease, TSharedRef<IPropertyHandle> HierarchyIndexHandle)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FReply::Handled();

	//hierarchy index could affect other items
	GEditor->BeginTransaction(LOCTEXT("ChangeHierarchyIndex_Transaction", "Change LexUI Hierarchy Index"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
		if (auto Parent = Item->GetParent())
		{
			for (auto Child : Parent->Children)
			{
				Child->Modify();
			}
		}
	}

	for (auto& Item : TargetScriptArray)
	{
		HierarchyIndexHandle->SetValue(Item->SiblingIndex + (IncreaseOrDecrease ? 1 : -1));
		//notify others
		if (auto Parent = Item->GetParent())
		{
			for (auto Child : Parent->Children)
			{
				auto HierarchyIndexProperty = FindFProperty<FIntProperty>(ULexWidget::StaticClass(), GET_MEMBER_NAME_CHECKED(ULexWidget, SiblingIndex));
				check(HierarchyIndexProperty != nullptr);
				FLexUIUtils::NotifyPropertyChanged(Child, HierarchyIndexProperty);
			}
		}
	}
	GEditor->EndTransaction();

	return FReply::Handled();
}

EVisibility FLexWidgetCustomization::GetAnchorPresetButtonVisibility()const
{
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
	{
		return TargetScriptArray[0]->GetParent() != nullptr ? EVisibility::Visible : EVisibility::Hidden;
	}
	return EVisibility::Hidden;
}

bool FLexWidgetCustomization::OnCanCopyAnchor()const
{
	return TargetScriptArray.Num() == 1;
}
#define BEGIN_LGUI_AnchorData_CLIPBOARD TEXT("Begin LGUI AnchorData")
bool FLexWidgetCustomization::OnCanPasteAnchor()const
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	return PastedText.StartsWith(BEGIN_LGUI_AnchorData_CLIPBOARD);
}
void FLexWidgetCustomization::OnCopyAnchor()
{
	if (TargetScriptArray.Num() == 1)
	{
		auto script = TargetScriptArray[0];
		if (script.IsValid())
		{
			auto AnchorData = script->GetAnchorData();
			auto CopiedText = FString::Printf(TEXT("%s, PivotX=%f, PivotY=%f\
, AnchorMinX=%f, AnchorMinY=%f, AnchorMaxX=%f, AnchorMaxY=%f\
, AnchoredPositionX=%f, AnchoredPositionY=%f\
, SizeDeltaX=%f, SizeDeltaY=%f")
, BEGIN_LGUI_AnchorData_CLIPBOARD
, AnchorData.Pivot.X
, AnchorData.Pivot.Y
, AnchorData.AnchorMin.X
, AnchorData.AnchorMin.Y
, AnchorData.AnchorMax.X
, AnchorData.AnchorMax.Y
, AnchorData.AnchoredPosition.X
, AnchorData.AnchoredPosition.Y
, AnchorData.SizeDelta.X
, AnchorData.SizeDelta.Y
);
			FPlatformApplicationMisc::ClipboardCopy(*CopiedText);
		}
	}
}
void FLexWidgetCustomization::OnPasteAnchor()
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	if (PastedText.StartsWith(BEGIN_LGUI_AnchorData_CLIPBOARD))
	{
		FLexUIAnchorData AnchorData;
		FParse::Value(*PastedText, TEXT("PivotX="), AnchorData.Pivot.X);
		FParse::Value(*PastedText, TEXT("PivotY="), AnchorData.Pivot.Y);
		FParse::Value(*PastedText, TEXT("AnchorMinX="), AnchorData.AnchorMin.X);
		FParse::Value(*PastedText, TEXT("AnchorMinY="), AnchorData.AnchorMin.Y);
		FParse::Value(*PastedText, TEXT("AnchorMaxX="), AnchorData.AnchorMax.X);
		FParse::Value(*PastedText, TEXT("AnchorMaxY="), AnchorData.AnchorMax.Y);
		FParse::Value(*PastedText, TEXT("AnchoredPositionX="), AnchorData.AnchoredPosition.X);
		FParse::Value(*PastedText, TEXT("AnchoredPositionY="), AnchorData.AnchoredPosition.Y);
		FParse::Value(*PastedText, TEXT("SizeDeltaX="), AnchorData.SizeDelta.X);
		FParse::Value(*PastedText, TEXT("SizeDeltaY="), AnchorData.SizeDelta.Y);
		auto AnchorData_PH = DetailBuilder->GetProperty(ULexWidget::GetPropertyName_AnchorData());
		AnchorData_PH->NotifyPreChange();
		for (auto item : TargetScriptArray)
		{
			if (item.IsValid())
			{
				item->SetAnchorData(AnchorData);
				item->MarkPackageDirty();
			}
		}
		AnchorData_PH->NotifyPostChange(EPropertyChangeType::ValueSet);
		ForceUpdateUI();
		DetailBuilder->ForceRefreshDetails();
	}
}

void FLexWidgetCustomization::OnCopyHierarchyIndex()
{
	if (TargetScriptArray.Num() > 0)
	{
		if (TargetScriptArray[0].IsValid())
		{
			FPlatformApplicationMisc::ClipboardCopy(*FString::Printf(TEXT("%d"), TargetScriptArray[0]->GetSiblingIndex()));
		}
	}
}
void FLexWidgetCustomization::OnPasteHierarchyIndex(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	if (PastedText.IsNumeric())
	{
		int value = FCString::Atoi(*PastedText);
		PropertyHandle->SetValue(value);
	}
}

bool FLexWidgetCustomization::IsAnchorEditable()const
{
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
	{
		auto Widget = TargetScriptArray[0];
		if (Widget->GetParent() != nullptr)return true;//not root
		if (Widget->IsCanvasWidget() && Widget->GetRenderCanvas() != nullptr && Widget->GetRenderCanvas()->IsRenderToScreenSpace())//is root canvas, and is render to screen space
		{
			return false;
		}
	}
	return true;
}

TSharedPtr<IPropertyHandle> FLexWidgetCustomization::GetAnchorPropertyHandle(
	TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int Index) const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return nullptr;

	FVector2D AnchorMinValue;
	FVector2D AnchorMaxValue;
	if (AnchorMinHandle->GetValue(AnchorMinValue) == FPropertyAccess::Success
		&& AnchorMaxHandle->GetValue(AnchorMaxValue) == FPropertyAccess::Success)
	{
		switch (Index)
		{
		case 0://anchored position y, stretch left
			if (AnchorMinValue.X == AnchorMaxValue.X)
				return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchoredPosition.X));
			return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, CacheAnchorOffsetLeft));
		case 1://anchored position z, stretch top
			if (AnchorMinValue.Y == AnchorMaxValue.Y)
				return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.AnchoredPosition.Y));
			return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, CacheAnchorOffsetTop));
		case 2://width, stretch right
			if (AnchorMinValue.X == AnchorMaxValue.X)
				return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.SizeDelta.X));
			return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, CacheAnchorOffsetRight));
		case 3://height, stretch bottom
			if (AnchorMinValue.Y == AnchorMaxValue.Y)
				return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData.SizeDelta.Y));
			return DetailBuilder->GetProperty(GET_MEMBER_NAME_CHECKED(ULexWidget, CacheAnchorOffsetBottom));
		}
	}
	return nullptr;
}

FText FLexWidgetCustomization::GetHAlignText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	if (AnchorMinValue.X == AnchorMaxValue.X)
	{
		if (AnchorMinValue.X == 0)
		{
			return LOCTEXT("AnchorLeft", "Left");
		}
		else if (AnchorMinValue.X == 0.5f)
		{
			return LOCTEXT("AnchorCenter", "Center");
		}
		else if (AnchorMinValue.X == 1.0f)
		{
			return LOCTEXT("AnchorRight", "Right");
		}
		else
		{
			return LOCTEXT("AnchorCustom", "Custom");
		}
	}
	else if (AnchorMinValue.X == 0.0f && AnchorMaxValue.X == 1.0f)
	{
		return LOCTEXT("AnchorStretch", "Stretch");
	}
	else
	{
		return LOCTEXT("AnchorCustom", "Custom");
	}
}
FText FLexWidgetCustomization::GetVAlignText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	if (AnchorMinValue.Y == AnchorMaxValue.Y)
	{
		if (AnchorMinValue.Y == 0)
		{
			return LOCTEXT("AnchorBottom", "Bottom");
		}
		else if (AnchorMinValue.Y == 0.5f)
		{
			return LOCTEXT("AnchorMiddle", "Middle");
		}
		else if (AnchorMinValue.Y == 1.0f)
		{
			return LOCTEXT("AnchorTop", "Top");
		}
		else
		{
			return LOCTEXT("AnchorCustom", "Custom");
		}
	}
	else if (AnchorMinValue.Y == 0.0f && AnchorMaxValue.Y == 1.0f)
	{
		return LOCTEXT("AnchorStretch", "Stretch");
	}
	else
	{
		return LOCTEXT("AnchorCustom", "Custom");
	}
}

FText FLexWidgetCustomization::GetAnchorLabelText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int LabelIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	switch (LabelIndex)
	{
	case 0://anchored position y, stretch left
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return LOCTEXT("AnchoredPositionX", "PosY");
		}
		else
		{
			return LOCTEXT("AnchoredLeft", "Left");
		}
	}
	break;
	case 1://anchored position z, stretch top
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return LOCTEXT("AnchoredPositionY", "PosZ");
		}
		else
		{
			return LOCTEXT("AnchoredTop", "Top");
		}
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return LOCTEXT("Width", "Width");
		}
		else
		{
			return LOCTEXT("AnchoredRight", "Right");
		}
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return LOCTEXT("Height", "Height");
		}
		else
		{
			return LOCTEXT("AnchoredBottom", "Bottom");
		}
	}
	break;
	}
	return LOCTEXT("AnchorError", "Error");
}

FText FLexWidgetCustomization::GetAnchorLabelTooltipText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int LabelTooltipIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	switch (LabelTooltipIndex)
	{
	default:
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return FText::Format(LOCTEXT("AnchoredPositionX_Tooltip", "Horizontal anchored position. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetAnchoredPosition)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetAnchoredPosition)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredLeft_Tooltip", "Calculated distance to parent's left anchor point. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetAnchorOffsetLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetAnchorOffsetLeft)));
		}
	}
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return FText::Format(LOCTEXT("AnchoredPositionY_Tooltip", "Vertical anchored position. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetAnchoredPosition)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetAnchoredPosition)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredTop_Tooltip", "Calculated distance to parent's top anchor point. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetAnchorOffsetLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetAnchorOffsetLeft)));
		}
	}
	case 2://width, stretch right
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return FText::Format(LOCTEXT("Width_Tooltip", "Horizontal size. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetWidth)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetWidth)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredRight_Tooltip", "Calculated distance to parent's right anchor point. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetAnchorOffsetLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetAnchorOffsetLeft)));
		}
	}
	case 3://height, stretch bottom
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return FText::Format(LOCTEXT("Height_Tooltip", "Vertical size. Related function: {0} / {1}"), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetHeight)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetHeight)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredBottom_Tooltip", "Calculated distance to parent's bottom anchor point. Related function: {0} / {0}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, GetAnchorOffsetLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(ULexWidget, SetAnchorOffsetLeft)));
		}
	}
	}
}

LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign FLexWidgetCustomization::GetAnchorHAlign(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None;

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None;
	if (AnchorMinValue.X == AnchorMaxValue.X)
	{
		if (AnchorMinValue.X == 0)
		{
			AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left;
		}
		else if (AnchorMinValue.X == 0.5f)
		{
			AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center;
		}
		else if (AnchorMinValue.X == 1.0f)
		{
			AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right;
		}
	}
	else if (AnchorMinValue.X == 0.0f && AnchorMaxValue.X == 1.0f)
	{
		AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch;
	}
	return AnchorHAlign;
}
LGUIAnchorPreviewWidget::UIAnchorVerticalAlign FLexWidgetCustomization::GetAnchorVAlign(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None;

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	LGUIAnchorPreviewWidget::UIAnchorVerticalAlign AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None;
	if (AnchorMinValue.Y == AnchorMaxValue.Y)
	{
		if (AnchorMinValue.Y == 0)
		{
			AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom;
		}
		else if (AnchorMinValue.Y == 0.5f)
		{
			AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle;
		}
		else if (AnchorMinValue.Y == 1.0f)
		{
			AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top;
		}
	}
	else if (AnchorMinValue.Y == 0.0f && AnchorMaxValue.Y == 1.0f)
	{
		AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch;
	}
	return AnchorVAlign;
}

void FLexWidgetCustomization::OnSelectAnchor(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign HorizontalAlign, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign VerticalAlign, TSharedRef<IPropertyHandle> AnchorData_PH)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return;

	bool ShiftPressed = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	bool AltPressed = FSlateApplication::Get().GetModifierKeys().IsAltDown();

	GEditor->BeginTransaction(LOCTEXT("ChangeAnchor_Transaction", "Change LGUI Anchor"));
	for (auto& UIItem : TargetScriptArray)
	{
		UIItem->Modify();
	}
	
	auto AnchorMin_PH = AnchorData_PH->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMin));
	auto AnchorMax_PH = AnchorData_PH->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMax));
	auto AnchoredPosition_PH = AnchorData_PH->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchoredPosition));
	auto SizeDelta_PH = AnchorData_PH->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, SizeDelta));
	auto Pivot_PH = AnchorData_PH->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, Pivot));

	AnchorMin_PH->NotifyPreChange();
	AnchorMax_PH->NotifyPreChange();
	AnchoredPosition_PH->NotifyPreChange();
	SizeDelta_PH->NotifyPreChange();
	if (ShiftPressed)
	{
		Pivot_PH->NotifyPreChange();
	}
	
	for (auto& Widget : TargetScriptArray)
	{
		FVector2D DesiredPivot = Widget->GetPivot();
		auto AnchorMin = Widget->GetAnchorMin();
		auto AnchorMax = Widget->GetAnchorMax();
		switch (HorizontalAlign)
		{
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None:
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left:
		{
			DesiredPivot.X = 0;
			AnchorMin.X = AnchorMax.X = 0;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center:
		{
			DesiredPivot.X = 0.5f;
			AnchorMin.X = AnchorMax.X = 0.5f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right:
		{
			DesiredPivot.X = 1.0f;
			AnchorMin.X = AnchorMax.X = 1.0f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch:
		{
			DesiredPivot.X = 0.5f;
			AnchorMin.X = 0;
			AnchorMax.X = 1.0f;
		}
		break;
		}
		switch (VerticalAlign)
		{
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None:
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top:
		{
			DesiredPivot.Y = 1.0f;
			AnchorMin.Y = AnchorMax.Y = 1;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle:
		{
			DesiredPivot.Y = 0.5f;
			AnchorMin.Y = AnchorMax.Y = 0.5f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom:
		{
			DesiredPivot.Y = 0.0f;
			AnchorMin.Y = AnchorMax.Y = 0.0f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch:
		{
			DesiredPivot.Y = 0.5f;
			AnchorMin.Y = 0;
			AnchorMax.Y = 1.0f;
		}
		break;
		}
		auto PrevRelativeLocation = Widget->GetRelativeLocation();
		auto PrevWidth = Widget->GetWidth();
		auto PrevHeight = Widget->GetHeight();
		Widget->SetAnchorData(FLexUIAnchorData{Widget->GetPivot(), AnchorMin, AnchorMax, Widget->GetAnchoredPosition(), Widget->GetSizeDelta()});
		Widget->MarkAllDirtyRecursive();
		Widget->SetWidth(PrevWidth);
		Widget->SetHeight(PrevHeight);
		Widget->SetRelativeLocation(PrevRelativeLocation);
		if (AltPressed)
		{
			switch (HorizontalAlign)
			{
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left:
			{
				Widget->SetHorizontalAnchoredPosition(-Widget->GetLocalSpaceLeft());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center:
			{
				Widget->SetHorizontalAnchoredPosition(Widget->GetWidth() * (Widget->GetPivot().X - 0.5f));
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right:
			{
				Widget->SetHorizontalAnchoredPosition(-Widget->GetLocalSpaceRight());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch:
			{
				Widget->SetAnchorOffsetLeft(0);
				Widget->SetAnchorOffsetRight(0);
			}
				break;
			}
			switch (VerticalAlign)
			{
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top:
			{
				Widget->SetVerticalAnchoredPosition(-Widget->GetLocalSpaceTop());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle:
			{
				Widget->SetVerticalAnchoredPosition(Widget->GetHeight() * (Widget->GetPivot().Y - 0.5f));
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom:
			{
				Widget->SetVerticalAnchoredPosition(-Widget->GetLocalSpaceBottom());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch:
			{
				Widget->SetAnchorOffsetBottom(0);
				Widget->SetAnchorOffsetTop(0);
			}
				break;
			}
		}
		if (ShiftPressed)
		{
			FMargin PrevAnchorAsMargin(Widget->GetAnchorOffsetLeft(), Widget->GetAnchorOffsetTop(), Widget->GetAnchorOffsetRight(), Widget->GetAnchorOffsetBottom());
			Widget->SetPivot(DesiredPivot);
			Widget->SetAnchorOffsetLeft(PrevAnchorAsMargin.Left);
			Widget->SetAnchorOffsetRight(PrevAnchorAsMargin.Right);
			Widget->SetAnchorOffsetBottom(PrevAnchorAsMargin.Bottom);
			Widget->SetAnchorOffsetTop(PrevAnchorAsMargin.Top);
		}
	}
	
	AnchorMin_PH->NotifyPostChange(EPropertyChangeType::ValueSet);
	AnchorMax_PH->NotifyPostChange(EPropertyChangeType::ValueSet);
	AnchoredPosition_PH->NotifyPostChange(EPropertyChangeType::ValueSet);
	SizeDelta_PH->NotifyPostChange(EPropertyChangeType::ValueSet);
	if (ShiftPressed)
	{
		Pivot_PH->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
	
	TargetScriptArray[0]->MarkCanvasUpdate(true);
	DetailBuilder->ForceRefreshDetails();
	GEditor->EndTransaction();
}

FLexLayoutControlAnchorData FLexWidgetCustomization::GetLayoutControlAnchorValue()const
{
	FLexLayoutControlAnchorData Result;
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return Result;

	auto Widget = TargetScriptArray[0];
	if (Widget.IsValid())
	{
		if (Widget->LayoutContainer)
		{
			Result = Widget->LayoutContainer->GetLayoutControlAnchor(Widget.Get());
		}
		if (Widget->LayoutSelf)
		{
			auto LayoutSelfResult = Widget->LayoutSelf->GetLayoutControlAnchor(Widget.Get());
			Result.Or(LayoutSelfResult);
		}
		if (auto Parent = Widget->GetParent())
		{
			if (auto ParentLayout = Parent->GetLayoutContainer())
			{
				auto ParentResult = ParentLayout->GetLayoutControlAnchor(Widget.Get());
				Result.Or(ParentResult);
			}
		}
	}
	return Result;
}

bool FLexWidgetCustomization::IsAnchorControlledByMultipleLayout(TMap<EAnchorControlledByLayoutType, TArray<UObject*>>& Result)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return false;
	return false;
}

bool FLexWidgetCustomization::GetLayoutControlHorizontalAnchoredPosition()const
{
	return GetLayoutControlAnchorValue().bCanControlHorizontalPosition;
}
bool FLexWidgetCustomization::GetLayoutControlVerticalAnchoredPosition()const
{
	return GetLayoutControlAnchorValue().bCanControlVerticalPosition;
}
bool FLexWidgetCustomization::GetLayoutControlHorizontalSizeDelta()const
{
	return GetLayoutControlAnchorValue().bCanControlHorizontalSize;
}
bool FLexWidgetCustomization::GetLayoutControlVerticalSizeDelta()const
{
	return GetLayoutControlAnchorValue().bCanControlVerticalSize;
}

TOptional<float> FLexWidgetCustomization::GetAnchorValue(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return TOptional<float>();

	auto AnchorMinHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMin));
	auto AnchorMaxHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMax));
	auto AnchoredPositionHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchoredPosition));
	auto SizeDeltaHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, SizeDelta));

	FVector2D AnchorMinValue;
	auto AnchorMinValueAccessResult = AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	auto AnchorMaxValueAccessResult = AnchorMaxHandle->GetValue(AnchorMaxValue);
	FVector2D AnchoredPosition;
	auto AnchoredPositionAccessResult = AnchoredPositionHandle->GetValue(AnchoredPosition);
	FVector2D SizeDelta;
	auto SizeDeltaAccessResult = SizeDeltaHandle->GetValue(SizeDelta);

	switch (AnchorValueIndex)
	{
	default:
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<ULexWidget> Item)->float {
				if (AnchorMinValue.X == AnchorMaxValue.X)
				{
					return Item->GetHorizontalAnchoredPosition();
				}
				else
				{
					return Item->GetAnchorOffsetLeft();
				}
			};
			if (AnchoredPositionAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (AnchoredPositionAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<ULexWidget> Item)->float {
				if (AnchorMinValue.Y == AnchorMaxValue.Y)
				{
					return Item->GetVerticalAnchoredPosition();
				}
				else
				{
					return Item->GetAnchorOffsetTop();
				}
			};
			if (AnchoredPositionAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (AnchoredPositionAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<ULexWidget> Item)->float {
				if (AnchorMinValue.X == AnchorMaxValue.X)
				{
					return Item->GetSizeDelta().X;
				}
				else
				{
					return Item->GetAnchorOffsetRight();
				}
			};
			if (SizeDeltaAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (SizeDeltaAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<ULexWidget> Item)->float {
				if (AnchorMinValue.Y == AnchorMaxValue.Y)
				{
					return Item->GetSizeDelta().Y;
				}
				else
				{
					return Item->GetAnchorOffsetBottom();
				}
			};
			if (SizeDeltaAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (SizeDeltaAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	}
}
void FLexWidgetCustomization::ApplyValueChanged(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex, bool Commited)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return;

	auto AnchorMinHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMin));
	auto AnchorMaxHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMax));
	auto AnchoredPositionHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchoredPosition));
	auto SizeDeltaHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, SizeDelta));

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);
	
	auto AnchoredPosition_PH = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchoredPosition));
	auto AnchoredPositionX_PH = AnchoredPosition_PH->GetChildHandle(0);
	auto AnchoredPositionY_PH = AnchoredPosition_PH->GetChildHandle(1);
	auto SizeDelta_PH = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, SizeDelta));
	auto SizeDeltaX_PH = SizeDelta_PH->GetChildHandle(0);
	auto SizeDeltaY_PH = SizeDelta_PH->GetChildHandle(1);
	auto RelativeLocation_PH = DetailBuilder->GetProperty(USceneComponent::GetRelativeLocationPropertyName());
	bool AnchoredPositionX = false, AnchoredPositionY = false, SizeDeltaX = false, SizeDeltaY = false;
	auto NotifyPropertyChange = [=, &AnchoredPositionX, &AnchoredPositionY, &SizeDeltaX, &SizeDeltaY](bool PreOrPostChange)
	{
		if (AnchoredPositionX)
		{
			if (PreOrPostChange)
			{
				AnchoredPositionY_PH->NotifyPreChange();
				RelativeLocation_PH->NotifyPreChange();
			}
			else
			{
				AnchoredPositionX_PH->NotifyPostChange(Commited ? EPropertyChangeType::ValueSet : EPropertyChangeType::Interactive);
				RelativeLocation_PH->NotifyPostChange(Commited ? EPropertyChangeType::ValueSet : EPropertyChangeType::Interactive);
			}
		}
		if (AnchoredPositionY)
		{
			if (PreOrPostChange)
			{
				AnchoredPositionY_PH->NotifyPreChange();
				RelativeLocation_PH->NotifyPreChange();
			}
			else
			{
				AnchoredPositionY_PH->NotifyPostChange(Commited ? EPropertyChangeType::ValueSet : EPropertyChangeType::Interactive);
				RelativeLocation_PH->NotifyPostChange(Commited ? EPropertyChangeType::ValueSet : EPropertyChangeType::Interactive);
			}
		}
		if (SizeDeltaX)
		{
			if (PreOrPostChange) SizeDeltaX_PH->NotifyPreChange();
			else SizeDeltaX_PH->NotifyPostChange(Commited ? EPropertyChangeType::ValueSet : EPropertyChangeType::Interactive);
		}
		if (SizeDeltaY)
		{
			if (PreOrPostChange) SizeDeltaY_PH->NotifyPreChange();
			else SizeDeltaY_PH->NotifyPostChange(Commited ? EPropertyChangeType::ValueSet : EPropertyChangeType::Interactive);
		}
	};
	
	switch (AnchorValueIndex)
	{
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			AnchoredPositionX = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetHorizontalAnchoredPosition(Value);
			}
			NotifyPropertyChange(false);
		}
		else
		{
			AnchoredPositionX = true;
			SizeDeltaX = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorOffsetLeft(Value);
			}
			NotifyPropertyChange(false);
		}
	}
	break;
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			AnchoredPositionY = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetVerticalAnchoredPosition(Value);
			}
			NotifyPropertyChange(false);
		}
		else
		{
			AnchoredPositionY = true;
			SizeDeltaY = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorOffsetTop(Value);
			}
			NotifyPropertyChange(false);
		}
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			SizeDeltaX = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetWidth(Value);
			}
			NotifyPropertyChange(false);
		}
		else
		{
			AnchoredPositionX = true;
			SizeDeltaX = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorOffsetRight(Value);
			}
			NotifyPropertyChange(false);
		}
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			SizeDeltaY = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetHeight(Value);
			}
			NotifyPropertyChange(false);
		}
		else
		{
			AnchoredPositionY = true;
			SizeDeltaY = true;
			NotifyPropertyChange(true);
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorOffsetBottom(Value);
			}
			NotifyPropertyChange(false);
		}
	}
	break;
	}

	GUnrealEd->UpdatePivotLocationForSelection();
	GUnrealEd->SetPivotMovedIndependently(false);
	// Redraw
	GUnrealEd->RedrawLevelEditingViewports();
}
void FLexWidgetCustomization::OnAnchorValueChanged(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	ApplyValueChanged(Value, AnchorHandle, AnchorValueIndex, false);
}
void FLexWidgetCustomization::OnAnchorValueCommitted(float Value, ETextCommit::Type commitType, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	GEditor->BeginTransaction(LOCTEXT("ChangeWidgetAnchor_Transaction", "Change Widget Anchor"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
	ApplyValueChanged(Value, AnchorHandle, AnchorValueIndex, true);
	GEditor->EndTransaction();
}

void FLexWidgetCustomization::OnAnchorValueSliderMovementBegin()
{
	GEditor->BeginTransaction(LOCTEXT("SlideChangeWidgetAnchor_Transaction", "Change Widget Anchor"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
}

void FLexWidgetCustomization::OnAnchorValueSliderMovementEnd(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	//ApplyValueChanged(Value, AnchorHandle, AnchorValueIndex);
	GEditor->EndTransaction();
}

bool FLexWidgetCustomization::IsAnchorValueEnable(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return false;

	auto AnchorMinHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMin));
	auto AnchorMaxHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchorMax));
	auto AnchoredPositionHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, AnchoredPosition));
	auto SizeDeltaHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLexUIAnchorData, SizeDelta));

	FVector2D AnchorMinValue;
	auto AnchorMinValueAccessResult = AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	auto AnchorMaxValueAccessResult = AnchorMaxHandle->GetValue(AnchorMaxValue);
	FVector2D AnchoredPosition;
	auto AnchoredPositionAccessResult = AnchoredPositionHandle->GetValue(AnchoredPosition);
	FVector2D SizeDelta;
	auto SizeDeltaAccessResult = SizeDeltaHandle->GetValue(SizeDelta);

	switch (AnchorValueIndex)
	{
	default:
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& AnchoredPositionAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.X == AnchorMaxValue.X)
			{
				return !GetLayoutControlHorizontalAnchoredPosition();
			}
			else
			{
				return !GetLayoutControlHorizontalAnchoredPosition() && !GetLayoutControlHorizontalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& AnchoredPositionAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.Y == AnchorMaxValue.Y)
			{
				return !GetLayoutControlVerticalAnchoredPosition();
			}
			else
			{
				return !GetLayoutControlVerticalAnchoredPosition() && !GetLayoutControlVerticalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& SizeDeltaAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.X == AnchorMaxValue.X)
			{
				return !GetLayoutControlHorizontalSizeDelta();
			}
			else
			{
				return !GetLayoutControlHorizontalAnchoredPosition() && !GetLayoutControlHorizontalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& SizeDeltaAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.Y == AnchorMaxValue.Y)
			{
				return !GetLayoutControlVerticalSizeDelta();
			}
			else
			{
				return !GetLayoutControlVerticalAnchoredPosition() && !GetLayoutControlVerticalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	}
}


#undef LOCTEXT_NAMESPACE