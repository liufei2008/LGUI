// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexWidgetDetailPropertyExtensionHandler.h"

#include "DetailLayoutBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "LexWidgetHierarchyPickerView.h"
#include "Widgets/SBoxPanel.h"
#include "Core/LexUIBehaviour.h"
#include "Core/Components/LexWidget.h"
#include "Core/Components/LexWidgetSubObjectBehaviour.h"

#define LOCTEXT_NAMESPACE "LexWidgetDetailPropertyExtensionHandler"

FLexWidgetDetailPropertyExtensionHandler::FLexWidgetDetailPropertyExtensionHandler(UWorld* InWorld)
{
	World = InWorld;
}

bool FLexWidgetDetailPropertyExtensionHandler::IsPropertyExtendable(const UClass* ObjectClass, const IPropertyHandle& PropertyHandle) const
{
	return true;
}

void FLexWidgetDetailPropertyExtensionHandler::ExtendWidgetRow(FDetailWidgetRow& InWidgetRow, const IDetailLayoutBuilder& InDetailBuilder, const UClass* InObjectClass,	TSharedPtr<IPropertyHandle> InPropertyHandle)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	InDetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() != 1)return;
	if (!ObjectsBeingCustomized[0].IsValid())return;
	auto ObjectProperty = CastField<FObjectPropertyBase>(InPropertyHandle->GetProperty());
	if (!ObjectProperty)return;
	if (CastField<FClassProperty>(ObjectProperty) != nullptr)return;//skip class property
	auto ObjectClass = ObjectProperty->PropertyClass;
	if (!ObjectClass->IsChildOf(ULexWidget::StaticClass())
		&& !ObjectClass->IsChildOf(ULexWidgetSubObjectBehaviour::StaticClass())
		&& !ObjectClass->IsChildOf(ULexUIBehaviour::StaticClass())
		)return;
	if (ObjectProperty->HasAnyPropertyFlags(CPF_PersistentInstance))
		return;
	UObject* Object = nullptr;
	if (InPropertyHandle->GetValue(Object) != FPropertyAccess::Success)return;
	auto WeakObject = MakeWeakObjectPtr(Object);
	InPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&InDetailBuilder]()
	{
		InDetailBuilder.GetPropertyUtilities()->RequestForceRefresh();
	}));

	auto NoneObjectText = LOCTEXT("None", "None");
	auto GetText = [=, this]()
	{
		if (!WeakObject.IsValid())return NoneObjectText;
		if (auto Widget = Cast<ULexWidget>(WeakObject.Get()))
		{
			return FText::FromString(Widget->GetDisplayName());
		}
		else
		{
			if (auto OuterWidget = WeakObject->GetTypedOuter<ULexWidget>())
			{
				return FText::FromString(OuterWidget->GetDisplayName());
			}
			return NoneObjectText;
		}
	};
	auto GetTooltipText = [=, this]()
	{
		if (!WeakObject.IsValid())return NoneObjectText;
		ULexWidget* Widget = nullptr;
		FString PathStr;
		if (auto CastWidget = Cast<ULexWidget>(WeakObject.Get()))
		{
			Widget = CastWidget;
		}
		else
		{
			Widget = WeakObject->GetTypedOuter<ULexWidget>();
			if (!IsValid(Widget))return NoneObjectText;
			if (!Widget->HasRegistered() && !Widget->GetParent() && !Widget->GetRenderCanvas())//could be destroyed in editor
			{
				PathStr = "None." + WeakObject->GetPathName(Widget);
			}
			else
			{
				PathStr = "." + WeakObject->GetPathName(Widget);
			}
		}
		while (Widget && !Widget->IsRootWidgetInHierarchy())
		{
			PathStr = "/" + Widget->GetDisplayName() + PathStr;
			Widget = Widget->GetParent();
		}
		return FText::FromString(PathStr);
	};
	// For container (array/set/map) element rows the default detail row decoration would normally provide insert/delete/duplicate buttons.
	TSharedPtr<IPropertyUtilities> PropertyUtilities = InDetailBuilder.GetPropertyUtilities();
	TSharedPtr<SWidget> ContainerActionWidget;
	TSharedPtr<IPropertyHandle> ParentHandle = InPropertyHandle->GetParentHandle();
	if (ParentHandle.IsValid() && InPropertyHandle->GetNumOuterObjects() == 1)
	{
		const int32 ElementIndex = InPropertyHandle->GetIndexInArray();
		if (TSharedPtr<IPropertyHandleArray> ArrayHandle = ParentHandle->AsArray())
		{
			FProperty* ArrayProperty = ParentHandle->GetProperty();
			if (ArrayProperty != nullptr && !ArrayProperty->HasAnyPropertyFlags(CPF_EditFixedSize))
			{
				FExecuteAction InsertAction;
				FExecuteAction DuplicateAction;
				FExecuteAction DeleteAction;
				if (PropertyUtilities.IsValid())
				{
					InsertAction = FExecuteAction::CreateLambda([PropertyUtilities, ArrayHandle, ElementIndex]()
					{
						PropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([ArrayHandle, ElementIndex]()
						{
							ArrayHandle->Insert(ElementIndex);
						}));
					});
					DeleteAction = FExecuteAction::CreateLambda([PropertyUtilities, ArrayHandle, ElementIndex]()
					{
						PropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([ArrayHandle, ElementIndex]()
						{
							ArrayHandle->DeleteItem(ElementIndex);
						}));
					});
					if (!ParentHandle->HasMetaData(TEXT("NoElementDuplicate")))
					{
						DuplicateAction = FExecuteAction::CreateLambda([PropertyUtilities, ArrayHandle, ElementIndex]()
						{
							PropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([ArrayHandle, ElementIndex]()
							{
								ArrayHandle->DuplicateItem(ElementIndex);
							}));
						});
					}
				}
				ContainerActionWidget = PropertyCustomizationHelpers::MakeInsertDeleteDuplicateButton(InsertAction, DeleteAction, DuplicateAction);
			}
		}
		else if (TSharedPtr<IPropertyHandleSet> SetHandle = ParentHandle->AsSet())
		{
			FProperty* SetProperty = ParentHandle->GetProperty();
			if (SetProperty != nullptr && !SetProperty->HasAnyPropertyFlags(CPF_EditFixedSize) && PropertyUtilities.IsValid())
			{
				ContainerActionWidget = PropertyCustomizationHelpers::MakeDeleteButton(
					FSimpleDelegate::CreateLambda([PropertyUtilities, SetHandle, ElementIndex]()
					{
						PropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([SetHandle, ElementIndex]()
						{
							SetHandle->DeleteItem(ElementIndex);
						}));
					}));
			}
		}
		else if (TSharedPtr<IPropertyHandleMap> MapHandle = ParentHandle->AsMap())
		{
			FProperty* MapProperty = ParentHandle->GetProperty();
			if (MapProperty != nullptr && !MapProperty->HasAnyPropertyFlags(CPF_EditFixedSize) && PropertyUtilities.IsValid())
			{
				ContainerActionWidget = PropertyCustomizationHelpers::MakeDeleteButton(
					FSimpleDelegate::CreateLambda([PropertyUtilities, MapHandle, ElementIndex]()
					{
						PropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([MapHandle, ElementIndex]()
						{
							MapHandle->DeleteItem(ElementIndex);
						}));
					}));
			}
		}
	}

	TSharedRef<SBox> PickerBox = SNew(SBox)
		.IsEnabled_Lambda([=]()
		{
			return InPropertyHandle->IsEditable();
		})
		[
			SNew(SBox)
			.MinDesiredWidth(125)
			.Padding(0, 4)
			[
				SAssignNew(PickerButton, SComboButton)
				.HasDownArrow(true)
				.ToolTipText_Lambda(GetTooltipText)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text_Lambda(GetText)
				]
				.MenuContent()
				[
					SNew(SBox)
					.Padding(4, 4)
					[
						SNew(SLexWidgetHierarchyPickerView, World.Get(), ObjectClass)
						.OnSelectItem_Lambda([=, this](UObject* InItem)
						{
							InPropertyHandle->SetValueFromFormattedString(InItem->GetPathName());
							PickerButton->SetIsOpen(false);
						})
					]
				]
			]
		];

	if (ContainerActionWidget.IsValid())
	{
		InWidgetRow.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				PickerBox
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 1.0f, 0.0f, 1.0f)
			[
				SNew(SBox)
				.IsEnabled_Lambda([=]()
				{
					return InPropertyHandle->IsEditable();
				})
				[
					ContainerActionWidget.ToSharedRef()
				]
			]
		];
	}
	else
	{
		InWidgetRow.ValueContent()
		[
			SNew(SBox)
			.WidthOverride(5000)
			[
				PickerBox
			]
		];
	}
}

#undef LOCTEXT_NAMESPACE