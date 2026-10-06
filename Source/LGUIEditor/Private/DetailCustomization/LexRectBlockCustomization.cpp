// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexRectBlockCustomization.h"
#include "LexUIEditorUtils.h"
#include "Core/Components/LexRectBlock.h"
#include "Utils/LexUIUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"
#include "Core/Components/LexWidget.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "LexRectBlockCustomization"
FLexRectBlockCustomization::FLexRectBlockCustomization()
{
}

FLexRectBlockCustomization::~FLexRectBlockCustomization()
{
	
}

TSharedRef<IDetailCustomization> FLexRectBlockCustomization::MakeInstance()
{
	return MakeShareable(new FLexRectBlockCustomization);
}
void FLexRectBlockCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<ULexRectBlock>(item.Get()))
		{
			TargetScriptArray.Add(TWeakObjectPtr<ULexRectBlock>(validItem));
			if (validItem->GetWorld() && validItem->GetWorld()->WorldType == EWorldType::Editor)
			{
				validItem->GetWidget()->MarkCanvasUpdate(true);
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}

	const FMargin OuterPadding(2, 0);
	const FMargin ContentPadding(2);
	auto CreateUnitSelector = [=](TSharedRef<IPropertyHandle> PropertyHandle) {
		return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		.Padding(OuterPadding)
		[
			SNew( SCheckBox )
			.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
			.ToolTipText(LOCTEXT("Value_Tooltip", "Use direct value"))
			.Padding(ContentPadding)
			.OnCheckStateChanged_Lambda([=](ECheckBoxState InCheckboxState){
				PropertyHandle->SetValue((uint8)ELexRectBlockUnitMode::Value);
				})
			.IsChecked_Lambda([=] {
				uint8 Value;
				PropertyHandle->GetValue(Value);
				return Value == (uint8)ELexRectBlockUnitMode::Value ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Value", "V"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		.Padding(OuterPadding)
		[
			SNew(SCheckBox)
			.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
			.ToolTipText(LOCTEXT("Percentage_Tooltip", "Use percentage of rect width and height"))
			.Padding(ContentPadding)
			.OnCheckStateChanged_Lambda([=](ECheckBoxState InCheckboxState) {
			PropertyHandle->SetValue((uint8)ELexRectBlockUnitMode::Percentage);
				})
			.IsChecked_Lambda([=] {
				uint8 Value;
				PropertyHandle->GetValue(Value);
				return Value == (uint8)ELexRectBlockUnitMode::Percentage ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Percentage", "%"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
		;
	};

	auto CreateNumericPropertyWithUnitMode = [](TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> UnitModePropertyHandle, bool EnableMinMax)
	{
		auto GetUnitMode = [=]()
		{
			ELexRectBlockUnitMode UnitMode = ELexRectBlockUnitMode::Value;
			UnitModePropertyHandle->GetValue(*(uint8*)&UnitMode);
			return UnitMode;
		};
		return
			SNew(SNumericEntryBox<float>)
			.MinValue(EnableMinMax ? 0 : TOptional<float>())
			.MaxValue_Lambda([=]()
			{
				if (!EnableMinMax)return TOptional<float>();
				return GetUnitMode() == ELexRectBlockUnitMode::Percentage ? 100 : TOptional<float>();
			})
			.AllowSpin(true)
			.MinSliderValue(EnableMinMax ? 0 : TOptional<float>())
			.MaxSliderValue_Lambda([=]()
			{
				if (!EnableMinMax)return TOptional<float>();
				return GetUnitMode() == ELexRectBlockUnitMode::Percentage ? 100 : TOptional<float>();
			})
			.OnValueChanged_Lambda([=](float Value)
			{
				Value = GetUnitMode() == ELexRectBlockUnitMode::Percentage ? Value * 0.01f : Value;
				PropertyHandle->SetValue(Value);
			})
			.Value_Lambda([=]()
			{
				float Value = 0;
				if (PropertyHandle->GetValue(Value) == FPropertyAccess::Success)
				{
					Value = GetUnitMode() == ELexRectBlockUnitMode::Percentage ? Value * 100 : Value;
					return Value;
				}
				return Value;
			});
	};

	auto CreateVectorPropertyWithUnitMode = [&](FName PropertyName, IDetailGroup& Group, FText PropertyDisplayName, const TAttribute<bool>& IsEnabledAttribute, bool EnableMinMax) {
		auto PropertyHandle = DetailBuilder.GetProperty(PropertyName);
		PropertyHandle->SetPropertyDisplayName(PropertyDisplayName);
		auto PropertyUnitHandle = DetailBuilder.GetProperty(FName(*(PropertyName.ToString() + TEXT("UnitMode"))));
		auto ValueHorizontalBox = SNew(SHorizontalBox);
		uint32 NumChildren = 0;
		PropertyHandle->GetNumChildren(NumChildren);
		if (NumChildren == 0)
		{
			ValueHorizontalBox->AddSlot()
			[
				CreateNumericPropertyWithUnitMode(PropertyHandle, PropertyUnitHandle, EnableMinMax)
			];
		}
		else
		{
			for (uint32 i = 0; i < NumChildren; i++)
			{
				ValueHorizontalBox->AddSlot()
				[
					CreateNumericPropertyWithUnitMode(PropertyHandle->GetChildHandle(i), PropertyUnitHandle, EnableMinMax)
				];
			}
		}
		Group.AddWidgetRow()
		.PropertyHandleList({ PropertyHandle })
		.IsEnabled(IsEnabledAttribute)
		.NameContent()
		[
			SNew(SBox)
			.MinDesiredWidth(1000)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					PropertyHandle->CreatePropertyNameWidget()
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					CreateUnitSelector(PropertyUnitHandle)
				]
			]
		]
		.ValueContent()
		[
			ValueHorizontalBox
		]
	;
	};

#define TO_TEXT(x) #x

#define AddPropertyRowToGroup(PropertyName, DisplayName, Group, IsEnabledAttribute)\
auto PropertyName##Handle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, PropertyName));\
PropertyName##Handle->SetPropertyDisplayName(LOCTEXT(TO_TEXT(PropertyName##_DisplayName), TO_TEXT(DisplayName)));\
Group.AddPropertyRow(PropertyName##Handle).IsEnabled(IsEnabledAttribute);

#define AddVectorPropertyRowToGroup(PropertyName, DisplayName, Group, IsEnabledAttribute, EnableMinMax)\
CreateVectorPropertyWithUnitMode(GET_MEMBER_NAME_CHECKED(ULexRectBlock, PropertyName), Group, LOCTEXT(TO_TEXT(PropertyName##_DisplayName), TO_TEXT(DisplayName)), IsEnabledAttribute, EnableMinMax);
	
	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LexUI");
	
	DetailBuilder.HideCategory(TEXT("LexUI-ProceduralRect"));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bUniformSetCornerRadius));

	auto UniformSetCornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bUniformSetCornerRadius));
	auto CornerRadiusUnitModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, CornerRadiusUnitMode));
	auto CornerRadiusHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, CornerRadius));
	auto CornerRadiusXHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, CornerRadius.X));
	auto CornerRadiusYHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, CornerRadius.Y));
	auto CornerRadiusZHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, CornerRadius.Z));
	auto CornerRadiusWHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, CornerRadius.W));
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

	LGUICategory.AddCustomRow(LOCTEXT("CornerRadius", "CornerRadius"), false)
	.PropertyHandleList({ CornerRadiusHandle, UniformSetCornerRadiusHandle, CornerRadiusUnitModeHandle })
	.OverrideResetToDefault(FResetToDefaultOverride::Create(TAttribute<bool>::CreateLambda([=]()
	{
		return UniformSetCornerRadiusHandle->CanResetToDefault() || CornerRadiusUnitModeHandle->CanResetToDefault() || CornerRadiusHandle->CanResetToDefault();
	}), FSimpleDelegate::CreateLambda([=]()
	{
		UniformSetCornerRadiusHandle->ResetToDefault();
		CornerRadiusUnitModeHandle->ResetToDefault();
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
					bool bUniformSetCornerRadius = (NewState == ECheckBoxState::Checked) ? true : false;
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
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				CreateUnitSelector(CornerRadiusUnitModeHandle)
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
			CreateNumericPropertyWithUnitMode(CornerRadiusXHandle, CornerRadiusUnitModeHandle, true)
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1)
		[
			SNew(SBox)
			.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
			[
				CreateNumericPropertyWithUnitMode(CornerRadiusYHandle, CornerRadiusUnitModeHandle, true)
			]
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1)
		[
			SNew(SBox)
			.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
			[
				CreateNumericPropertyWithUnitMode(CornerRadiusZHandle, CornerRadiusUnitModeHandle, true)
			]
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1)
		[
			SNew(SBox)
			.IsEnabled_Lambda(CornerRadiusPropertyIsEnabledFunction)
			[
				CreateNumericPropertyWithUnitMode(CornerRadiusWHandle, CornerRadiusUnitModeHandle, true)
			]
		]
	]
	;
	LGUICategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bSoftEdge)));

	//body
	auto EnableBodyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBody));
	EnableBodyHandle->SetPropertyDisplayName(LOCTEXT("EnableBody_DisplayName", "Body"));
	auto& BodyGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBody), EnableBodyHandle->GetPropertyDisplayName(), false, true);
	BodyGroup.HeaderProperty(EnableBodyHandle);
	{
		auto EnableBodyAttribute = TAttribute<bool>::CreateLambda([=]()
		{
			bool bEnable = false;
			EnableBodyHandle->GetValue(bEnable);
			return bEnable;
		});
		AddPropertyRowToGroup(BodyColor, Color, BodyGroup, EnableBodyAttribute);

		ELexRectBlockTextureMode BodyTextureMode;
		auto BodyTextureModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, BodyTextureMode));
		BodyTextureModeHandle->GetValue(*(uint8*)&BodyTextureMode);
		BodyTextureModeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {
			DetailBuilder.ForceRefreshDetails();
			}));
		auto BodyTextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, BodyTexture));
		BodyTextureHandle->SetPropertyDisplayName(LOCTEXT("BodyTexture_DisplayName", "Texture"));
		auto BodySpriteTextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, BodySpriteTexture));
		BodySpriteTextureHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=, this] {
			for (auto item : TargetScriptArray)
			{
				if (item.IsValid())
				{
					item->OnPreChangeSpriteProperty();
				}
			}
			}));
		BodySpriteTextureHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this] {
			for (auto item : TargetScriptArray)
			{
				if (item.IsValid())
				{
					item->OnPostChangeSpriteProperty();
				}
			}
			}));
		BodySpriteTextureHandle->SetPropertyDisplayName(LOCTEXT("BodySpriteTexture_DisplayName", "Sprite"));
		auto& TextureGroup = BodyTextureMode == ELexRectBlockTextureMode::Texture
			? BodyGroup.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, BodyTexture), BodyTextureHandle->GetPropertyDisplayName(), true)
			: BodyGroup.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, BodySpriteTexture), BodySpriteTextureHandle->GetPropertyDisplayName(), true)
			;
		auto TempBodyTextureHandle = BodyTextureMode == ELexRectBlockTextureMode::Texture ? BodyTextureHandle : BodySpriteTextureHandle;
		TextureGroup.HeaderRow()
			.PropertyHandleList({ BodyTextureModeHandle, BodyTextureHandle, BodySpriteTextureHandle })
			.OverrideResetToDefault(FResetToDefaultOverride::Create(TAttribute<bool>::CreateLambda([=]()
			{
				return BodyTextureModeHandle->CanResetToDefault() || BodyTextureHandle->CanResetToDefault() || BodySpriteTextureHandle->CanResetToDefault();
			}), FSimpleDelegate::CreateLambda([=]()
			{
				BodyTextureModeHandle->ResetToDefault();
				BodyTextureHandle->ResetToDefault();
				BodySpriteTextureHandle->ResetToDefault();
			})))
			.NameContent()
			[
				SNew(SBox)
				.MinDesiredWidth(1000)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						TempBodyTextureHandle->CreatePropertyNameWidget()
					]
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(EVerticalAlignment::VAlign_Center)
						.Padding(OuterPadding)
						[
							SNew( SCheckBox )
							.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
							.ToolTipText(LOCTEXT("Texture_Tooltip", "Use texture"))
							.Padding(ContentPadding)
							.OnCheckStateChanged_Lambda([=](ECheckBoxState InCheckboxState){
								BodyTextureModeHandle->SetValue((uint8)ELexRectBlockTextureMode::Texture);
								})
							.IsChecked_Lambda([=] {
								uint8 Value;
								BodyTextureModeHandle->GetValue(Value);
								return Value == (uint8)ELexRectBlockTextureMode::Texture ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								})
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Texture", "T"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(EVerticalAlignment::VAlign_Center)
						.Padding(OuterPadding)
						[
							SNew(SCheckBox)
							.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
							.ToolTipText(LOCTEXT("Sprite_Tooltip", "Use sprite"))
							.Padding(ContentPadding)
							.OnCheckStateChanged_Lambda([=](ECheckBoxState InCheckboxState) {
								BodyTextureModeHandle->SetValue((uint8)ELexRectBlockTextureMode::Sprite);
								})
							.IsChecked_Lambda([=] {
								uint8 Value;
								BodyTextureModeHandle->GetValue(Value);
								return Value == (uint8)ELexRectBlockTextureMode::Sprite ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								})
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Sprite", "S"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
				]
			]
			.ValueContent()
			[
				TempBodyTextureHandle->CreatePropertyValueWidget()
			]
			.IsEnabled(EnableBodyAttribute)
		;
		TextureGroup.AddWidgetRow()
			.ValueContent()
			[
				SNew(SButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SnapSize_Button", "Snap Size"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				.OnClicked_Lambda([=, this]()
				{
					GEditor->BeginTransaction(LOCTEXT("TextureSnapSize_Transaction", "UIProceduralRect texture snap size"));
					for (auto item : TargetScriptArray)
					{
						if (item.IsValid())
						{
							item->Modify();
							item->SetSizeFromBodyTexture();
							FLexUIUtils::NotifyPropertyChanged(item.Get(), ULexWidget::GetPropertyName_AnchorData());
							item->GetWidget()->MarkCanvasUpdate(true);
						}
					}
					GEditor->EndTransaction();
					return FReply::Handled();
				})
			]
			.IsEnabled(EnableBodyAttribute)
		;

		auto BodyTextureScaleModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, BodyTextureScaleMode));
		BodyTextureScaleModeHandle->SetPropertyDisplayName(LOCTEXT("BodyTextureScaleMode_DisplayName", "Scale Mode"));
		TextureGroup.AddPropertyRow(BodyTextureScaleModeHandle).IsEnabled(TAttribute<bool>::CreateLambda([=]()
		{
			UObject* BodyTexture = nullptr;
			TempBodyTextureHandle->GetValue(BodyTexture);
			return BodyTexture != nullptr && EnableBodyAttribute.Get();
		}));

		//gradient
		auto EnableBodyGradientHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBodyGradient));
		EnableBodyGradientHandle->SetPropertyDisplayName(LOCTEXT("EnableBodyGradient_DisplayName", "Gradient"));
		auto& BodyGradientGroup = BodyGroup.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBodyGradient), EnableBodyGradientHandle->GetPropertyDisplayName(), true);
		BodyGradientGroup.HeaderProperty(EnableBodyGradientHandle).IsEnabled(EnableBodyAttribute);
		{
			auto IsEnableGradientAttribute = TAttribute<bool>::CreateLambda([=]()
			{
				bool bEnable = false;
				EnableBodyGradientHandle->GetValue(bEnable);
				return bEnable && EnableBodyAttribute.Get();
			});
			AddPropertyRowToGroup(BodyGradientColor, Color, BodyGradientGroup, IsEnableGradientAttribute);
			AddVectorPropertyRowToGroup(BodyGradientCenter, Center, BodyGradientGroup, IsEnableGradientAttribute, false);
			AddVectorPropertyRowToGroup(BodyGradientRadius, Radius, BodyGradientGroup, IsEnableGradientAttribute, false);
			AddPropertyRowToGroup(BodyGradientRotation, Rotation, BodyGradientGroup, IsEnableGradientAttribute);
		}
	}

	//border
	auto BorderHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBorder));
	BorderHandle->SetPropertyDisplayName(LOCTEXT("bEnableBorder_DisplayName", "Border"));
	auto& BorderGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBorder), BorderHandle->GetPropertyDisplayName(), false, true);
	BorderGroup.HeaderProperty(BorderHandle);
	{
		auto IsEnableBorderAttribute = TAttribute<bool>::CreateLambda([=]()
		{
			bool bEnable = false;
			BorderHandle->GetValue(bEnable);
			return bEnable;
		});
		AddVectorPropertyRowToGroup(BorderWidth, Width, BorderGroup, IsEnableBorderAttribute, true);
		AddPropertyRowToGroup(BorderColor, Color, BorderGroup, IsEnableBorderAttribute);

		//gradient
		auto BorderGradientHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBorderGradient));
		BorderGradientHandle->SetPropertyDisplayName(LOCTEXT("bEnableBorderGradient_DisplayName", "Gradient"));
		auto& BorderGradientGroup = BorderGroup.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBorderGradient), BorderGradientHandle->GetPropertyDisplayName(), true);
		BorderGradientGroup.HeaderProperty(BorderGradientHandle).IsEnabled(IsEnableBorderAttribute);
		{
			auto IsEnableGradientAttribute = TAttribute<bool>::CreateLambda([=]()
			{
				bool bEnableGradient = false;
				BorderGradientHandle->GetValue(bEnableGradient);
				return bEnableGradient && IsEnableBorderAttribute.Get();
			});
			AddPropertyRowToGroup(BorderGradientColor, Color, BorderGradientGroup, IsEnableGradientAttribute);
			AddVectorPropertyRowToGroup(BorderGradientCenter, Center, BorderGradientGroup, IsEnableGradientAttribute, false);
			AddVectorPropertyRowToGroup(BorderGradientRadius, Radius, BorderGradientGroup, IsEnableGradientAttribute, false);
			AddPropertyRowToGroup(BorderGradientRotation, Rotation, BorderGradientGroup, IsEnableGradientAttribute);
		}
	}

	//inner shadow
	auto InnerShadowHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableInnerShadow));
	InnerShadowHandle->SetPropertyDisplayName(LOCTEXT("bEnableInnerShadow_DisplayName", "Inner Shadow"));
	auto& InnerShadowGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableBorder), InnerShadowHandle->GetPropertyDisplayName(), false, true);
	InnerShadowGroup.HeaderProperty(InnerShadowHandle);
	{
		auto IsEnabledAttribute = TAttribute<bool>::CreateLambda([=]()
		{
			bool bEnable = false;
			InnerShadowHandle->GetValue(bEnable);
			return bEnable;
		});
		AddPropertyRowToGroup(InnerShadowColor, Color, InnerShadowGroup, IsEnabledAttribute);
		AddVectorPropertyRowToGroup(InnerShadowSize, Size, InnerShadowGroup, IsEnabledAttribute, true);
		AddVectorPropertyRowToGroup(InnerShadowBlur, Blur, InnerShadowGroup, IsEnabledAttribute, true);
		AddPropertyRowToGroup(InnerShadowAngle, Angle, InnerShadowGroup, IsEnabledAttribute);
		AddVectorPropertyRowToGroup(InnerShadowDistance, Distance, InnerShadowGroup, IsEnabledAttribute, true);
	}

	//outer shadow
	auto OuterShadowHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableOuterShadow));
	OuterShadowHandle->SetPropertyDisplayName(LOCTEXT("EnableOuterShadow_DisplayName", "Outer Shadow"));
	auto& OuterShadowGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableOuterShadow), OuterShadowHandle->GetPropertyDisplayName(), false, true);
	OuterShadowGroup.HeaderProperty(OuterShadowHandle);
	{
		auto IsEnabledAttribute = TAttribute<bool>::CreateLambda([=]()
		{
			bool bEnable = false;
			OuterShadowHandle->GetValue(bEnable);
			return bEnable;
		});
		AddPropertyRowToGroup(OuterShadowColor, Color, OuterShadowGroup, IsEnabledAttribute);
		AddVectorPropertyRowToGroup(OuterShadowSize, Size, OuterShadowGroup, IsEnabledAttribute, true);
		AddVectorPropertyRowToGroup(OuterShadowBlur, Blur, OuterShadowGroup, IsEnabledAttribute, true);
		AddPropertyRowToGroup(OuterShadowAngle, Angle, OuterShadowGroup, IsEnabledAttribute);
		AddVectorPropertyRowToGroup(OuterShadowDistance, Distance, OuterShadowGroup, IsEnabledAttribute, true);
	}

	//radial fill
	auto RadialFillHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableRadialFill));
	RadialFillHandle->SetPropertyDisplayName(LOCTEXT("EnableRadialFill_DisplayName", "Radial Fill"));
	auto& RadialFillGroup = LGUICategory.AddGroup(GET_MEMBER_NAME_CHECKED(ULexRectBlock, bEnableRadialFill), RadialFillHandle->GetPropertyDisplayName(), false, true);
	RadialFillGroup.HeaderProperty(RadialFillHandle);
	{
		auto IsEnabledAttribute = TAttribute<bool>::CreateLambda([=]()
		{
			bool bEnable = false;
			RadialFillHandle->GetValue(bEnable);
			return bEnable;
		});
		AddVectorPropertyRowToGroup(RadialFillCenter, Center, RadialFillGroup, IsEnabledAttribute, false);
		AddPropertyRowToGroup(RadialFillRotation, Rotation, RadialFillGroup, IsEnabledAttribute);
		AddPropertyRowToGroup(RadialFillAngle, Angle, RadialFillGroup, IsEnabledAttribute);
	}

	auto TintColorHandle = DetailBuilder.GetProperty(ULexRectBlock::GetPropertyName_Color(), ULexVisual::StaticClass());
	TintColorHandle->SetPropertyDisplayName(LOCTEXT("TintColor", "Tint Color"));
	TintColorHandle->SetToolTipText(LOCTEXT("TintColorTooltip", "Known as \"Color\" property in other UI elements. This can tint all color of this UI element. Usually only set alpha value."));
	LGUICategory.AddProperty(TintColorHandle);
}
void FLexRectBlockCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE