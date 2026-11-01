// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "ComponentTransformDetails.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "EditorStyleSet.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "UObject/UnrealType.h"
#include "Misc/ConfigCacheIni.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SRotatorInputBox.h"
#include "Editor/UnrealEdEngine.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "DetailLayoutBuilder.h"
#include "ScopedTransaction.h"
#include "IPropertyUtilities.h"
#include "Math/UnitConversion.h"
#include "Widgets/Input/NumericUnitTypeInterface.inl"
#include "Settings/EditorProjectSettings.h"
#include "HAL/PlatformApplicationMisc.h"

#include "Core/Components/LexWidget.h"

#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "LGUIComponentTransformDetails"

FComponentTransformDetails::FComponentTransformDetails(const TArray<TWeakObjectPtr<ULexWidget>>& InSelectedObjects, IDetailLayoutBuilder* DetailBuilder)
	: TNumericUnitTypeInterface(GetDefault<UEditorProjectAppearanceSettings>()->bDisplayUnitsOnComponentTransforms ? EUnit::Centimeters : EUnit::Unspecified)
	, SelectedObjects(InSelectedObjects)
	, NotifyHook(DetailBuilder->GetPropertyUtilities()->GetNotifyHook())
{
	GConfig->GetBool(TEXT("SelectionDetails"), TEXT("PreserveScaleRatio"), bPreserveScaleRatio, GEditorPerProjectIni);
}

TSharedRef<SWidget> FComponentTransformDetails::BuildTransformFieldLabel( ETransformField::Type TransformField )
{
	FText Label;
	switch( TransformField )
	{
	case ETransformField::Rotation:
		Label = LOCTEXT( "RotationLabel", "Rotation");
		break;
	case ETransformField::Scale:
		Label = LOCTEXT( "ScaleLabel", "Scale" );
		break;
	case ETransformField::Location:
	default:
		Label = LOCTEXT("LocationLabel", "Location");
		break;
	}


	TSharedRef<SWidget> NameContent =
		SNew(STextBlock)
		.Text(Label)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		;

	if(TransformField == ETransformField::Scale)
	{
		NameContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				NameContent
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				// Add a checkbox to toggle between preserving the ratio of x,y,z components of scale when a value is entered
				SNew(SCheckBox)
				.IsChecked(this, &FComponentTransformDetails::IsPreserveScaleRatioChecked)
				.OnCheckStateChanged(this, &FComponentTransformDetails::OnPreserveScaleRatioToggled)
				.Style(FAppStyle::Get(), "TransparentCheckBox")
				.ToolTipText(LOCTEXT("PreserveScaleToolTip", "When locked, scales uniformly based on the current xyz scale values so the object maintains its shape in each direction when scaled"))
				[
					SNew(SImage)
					.Image(this, &FComponentTransformDetails::GetPreserveScaleRatioImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];
	}

	return NameContent;
}
bool FComponentTransformDetails::OnCanCopy( ETransformField::Type TransformField ) const
{
	// We can only copy values if the whole field is set.  If multiple values are defined we do not copy since we are unable to determine the value
	switch (TransformField)
	{
	case ETransformField::Location:
		return CachedLocation.IsSet();
		break;
	case ETransformField::Rotation:
		return CachedRotation.IsSet();
		break;
	case ETransformField::Scale:
		return CachedScale.IsSet();
		break;
	default:
		return false;
		break;
	}
}

void FComponentTransformDetails::OnCopy( ETransformField::Type TransformField )
{
	CacheTransform();

	FString CopyStr;
	switch (TransformField)
	{
	case ETransformField::Location:
		CopyStr = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), CachedLocation.X.GetValue(), CachedLocation.Y.GetValue(), CachedLocation.Z.GetValue());
		break;
	case ETransformField::Rotation:
		CopyStr = FString::Printf(TEXT("(Pitch=%f,Yaw=%f,Roll=%f)"), CachedRotation.Y.GetValue(), CachedRotation.Z.GetValue(), CachedRotation.X.GetValue());
		break;
	case ETransformField::Scale:
		CopyStr = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), CachedScale.X.GetValue(), CachedScale.Y.GetValue(), CachedScale.Z.GetValue());
		break;
	default:
		break;
	}

	if( !CopyStr.IsEmpty() )
	{
		FPlatformApplicationMisc::ClipboardCopy( *CopyStr );
	}
}

void FComponentTransformDetails::OnPaste( ETransformField::Type TransformField )
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);

	switch (TransformField)
	{
		case ETransformField::Location:
		{
			FVector Location;
			if (Location.InitFromString(PastedText))
			{
				FScopedTransaction Transaction(LOCTEXT("PasteLocation", "Paste Location"));
				OnSetTransform(ETransformField::Location, EAxisList::All, Location, true);
			}
		}
		break;
	case ETransformField::Rotation:
		{
			FRotator Rotation;
			PastedText.ReplaceInline(TEXT("Pitch="), TEXT("P="));
			PastedText.ReplaceInline(TEXT("Yaw="), TEXT("Y="));
			PastedText.ReplaceInline(TEXT("Roll="), TEXT("R="));
			if (Rotation.InitFromString(PastedText))
			{
				FScopedTransaction Transaction(LOCTEXT("PasteRotation", "Paste Rotation"));
				OnSetTransform(ETransformField::Rotation, EAxisList::All, Rotation.Euler(), true);
			}
		}
		break;
	case ETransformField::Scale:
		{
			FVector Scale;
			if (Scale.InitFromString(PastedText))
			{
				FScopedTransaction Transaction(LOCTEXT("PasteScale", "Paste Scale"));
				OnSetTransform(ETransformField::Scale, EAxisList::All, Scale, true);
			}
		}
		break;
	default:
		break;
	}
}

FUIAction FComponentTransformDetails::CreateCopyAction( ETransformField::Type TransformField ) 
{
	return
		FUIAction
		(
			FExecuteAction::CreateSP(const_cast<FComponentTransformDetails*>(this), &FComponentTransformDetails::OnCopy, TransformField ),
			FCanExecuteAction::CreateSP(const_cast<FComponentTransformDetails*>(this), &FComponentTransformDetails::OnCanCopy, TransformField )
		);
}

FUIAction FComponentTransformDetails::CreatePasteAction( ETransformField::Type TransformField ) 
{
	return 
		 FUIAction( FExecuteAction::CreateSP(const_cast<FComponentTransformDetails*>(this), &FComponentTransformDetails::OnPaste, TransformField ) );
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FComponentTransformDetails::GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder )
{
	if (SelectedObjects.Num() <= 0)return;
	const auto* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return;

	FSlateFontInfo FontInfo = IDetailLayoutBuilder::GetDetailFont();

	// Location
	{
		TSharedPtr<INumericTypeInterface<FVector::FReal>> TypeInterface;
		if( FUnitConversion::Settings().ShouldDisplayUnits() )
		{
			TypeInterface = SharedThis(this);
		}

		TSharedPtr<SNumericVectorInputBox<FVector::FReal>> LocationWidget;
		ChildrenBuilder.AddCustomRow( LOCTEXT("LocationFilter", "Location") )
		.CopyAction( CreateCopyAction( ETransformField::Location ) )
		.PasteAction( CreatePasteAction( ETransformField::Location ) )
		.OverrideResetToDefault(FResetToDefaultOverride::Create(TAttribute<bool>(this, &FComponentTransformDetails::GetLocationResetVisibility), FSimpleDelegate::CreateSP(this, &FComponentTransformDetails::OnLocationResetClicked)))
		.PropertyHandleList({ GeneratePropertyHandle(TEXT("RelativeLocation"), ChildrenBuilder) })
		.NameContent()
		.VAlign(VAlign_Center)
		[
			BuildTransformFieldLabel( ETransformField::Location )
		]
		.ValueContent()
		.MinDesiredWidth(125.0f * 3.0f)
		.MaxDesiredWidth(125.0f * 3.0f)
		.VAlign( VAlign_Center )
		[
			SAssignNew(LocationWidget, SNumericVectorInputBox<FVector::FReal>)
			.X( this, &FComponentTransformDetails::GetLocationX )
			.Y( this, &FComponentTransformDetails::GetLocationY )
			.Z( this, &FComponentTransformDetails::GetLocationZ )
			.bColorAxisLabels( true )
			.OnXChanged(this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Location, EAxisList::X, false)
			.OnYChanged(this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Location, EAxisList::Y, false)
			.OnZChanged(this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Location, EAxisList::Z, false)
			.OnXCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Location, EAxisList::X, true )
			.OnYCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Location, EAxisList::Y, true )
			.OnZCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Location, EAxisList::Z, true )
			.Font( FontInfo )
			.TypeInterface( TypeInterface )
			.AllowSpin(SelectedObjects.Num() == 1)
			.SpinDelta(1)
			.OnBeginSliderMovement(this, &FComponentTransformDetails::OnBeginLocationSlider)
			.OnEndSliderMovement(this, &FComponentTransformDetails::OnEndLocationSlider)
		];
		//Disable Y&Z for UIItem
		{
			auto Child = LocationWidget->GetChildren()->GetChildAt(0);
			auto HorizontalBox = StaticCastSharedRef<SHorizontalBox>(Child);
			HorizontalBox->GetSlot(1).GetWidget()->SetEnabled(false);
			HorizontalBox->GetSlot(2).GetWidget()->SetEnabled(false);
		}
	}
	
	// Rotation
	{
		TSharedPtr<INumericTypeInterface<FVector::FReal>> TypeInterface;
		if( FUnitConversion::Settings().ShouldDisplayUnits() )
		{
			TypeInterface = MakeShareable( new TNumericUnitTypeInterface<FVector::FReal>(EUnit::Degrees) );
		}

		ChildrenBuilder.AddCustomRow( LOCTEXT("RotationFilter", "Rotation") )
		.CopyAction( CreateCopyAction(ETransformField::Rotation) )
		.PasteAction( CreatePasteAction(ETransformField::Rotation) )
		.OverrideResetToDefault(FResetToDefaultOverride::Create(TAttribute<bool>(this, &FComponentTransformDetails::GetRotationResetVisibility), FSimpleDelegate::CreateSP(this, &FComponentTransformDetails::OnRotationResetClicked)))
		.PropertyHandleList({ GeneratePropertyHandle(TEXT("RelativeRotation"), ChildrenBuilder) })
		.NameContent()
		.VAlign(VAlign_Center)
		[
			BuildTransformFieldLabel(ETransformField::Rotation)
		]
		.ValueContent()
		.MinDesiredWidth(125.0f * 3.0f)
		.MaxDesiredWidth(125.0f * 3.0f)
		.VAlign( VAlign_Center )
		[
			SNew(SNumericRotatorInputBox<FRotator::FReal>)
			.AllowSpin( SelectedObjects.Num() == 1 ) 
			.Roll( this, &FComponentTransformDetails::GetRotationX )
			.Pitch( this, &FComponentTransformDetails::GetRotationY )
			.Yaw( this, &FComponentTransformDetails::GetRotationZ )
			.bColorAxisLabels( true )
			.OnBeginSliderMovement( this, &FComponentTransformDetails::OnBeginRotationSlider )
			.OnEndSliderMovement( this, &FComponentTransformDetails::OnEndRotationSlider )
			.OnRollChanged( this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Rotation, EAxisList::X, false )
			.OnPitchChanged( this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Rotation, EAxisList::Y, false )
			.OnYawChanged( this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Rotation, EAxisList::Z, false )
			.OnRollCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Rotation, EAxisList::X, true )
			.OnPitchCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Rotation, EAxisList::Y, true )
			.OnYawCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Rotation, EAxisList::Z, true )
			.TypeInterface(TypeInterface)
			.Font( FontInfo )
		];
	}
	
	// Scale
	{
		ChildrenBuilder.AddCustomRow( LOCTEXT("ScaleFilter", "Scale") )
		.CopyAction( CreateCopyAction(ETransformField::Scale) )
		.PasteAction( CreatePasteAction(ETransformField::Scale) )
		.OverrideResetToDefault(FResetToDefaultOverride::Create(TAttribute<bool>(this, &FComponentTransformDetails::GetScaleResetVisibility), FSimpleDelegate::CreateSP(this, &FComponentTransformDetails::OnScaleResetClicked)))
		.PropertyHandleList({ GeneratePropertyHandle(TEXT("RelativeScale"), ChildrenBuilder) })
		.NameContent()
		.VAlign(VAlign_Center)
		[
			BuildTransformFieldLabel(ETransformField::Scale)
		]
		.ValueContent()
		.MinDesiredWidth(125.0f * 3.0f)
		.MaxDesiredWidth(125.0f * 3.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SNumericVectorInputBox<FVector::FReal>)
			.X( this, &FComponentTransformDetails::GetScaleX )
			.Y( this, &FComponentTransformDetails::GetScaleY )
			.Z( this, &FComponentTransformDetails::GetScaleZ )
			.bColorAxisLabels( true )
			.OnXChanged( this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Scale, EAxisList::X, false )
			.OnYChanged( this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Scale, EAxisList::Y, false )
			.OnZChanged( this, &FComponentTransformDetails::OnSetTransformAxis, ETextCommit::Default, ETransformField::Scale, EAxisList::Z, false )
			.OnXCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Scale, EAxisList::X, true )
			.OnYCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Scale, EAxisList::Y, true )
			.OnZCommitted( this, &FComponentTransformDetails::OnSetTransformAxis, ETransformField::Scale, EAxisList::Z, true )
			.Font( FontInfo )
			.AllowSpin( SelectedObjects.Num() == 1 )
			.SpinDelta( 0.0025f )
			.OnBeginSliderMovement( this, &FComponentTransformDetails::OnBeginScaleSlider )
			.OnEndSliderMovement(this, &FComponentTransformDetails::OnEndScaleSlider)
		];
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FComponentTransformDetails::Tick(float DeltaTime)
{
	CacheTransform();
}

ULexWidget* FComponentTransformDetails::GetFirstSelectedWidget() const
{
	return SelectedObjects.Num() > 0 ? SelectedObjects[0].Get() : nullptr;
}

bool FComponentTransformDetails::IsRootWidget() const
{
	const ULexWidget* Widget = GetFirstSelectedWidget();
	return IsValid(Widget) && Widget->GetParent() == nullptr;
}

TSharedPtr<IPropertyHandle> FComponentTransformDetails::GeneratePropertyHandle(FName PropertyName, IDetailChildrenBuilder& ChildrenBuilder)
{
	// Try finding the property handle in the details panel's property map first.
	IDetailLayoutBuilder& LayoutBuilder = ChildrenBuilder.GetParentCategory().GetParentLayout();
	TSharedPtr<IPropertyHandle> PropertyHandle = LayoutBuilder.GetProperty(PropertyName, ULexWidget::StaticClass());
	if (!PropertyHandle || !PropertyHandle->IsValidHandle())
	{
		// If it wasn't found, add a collapsed row which contains the property node.
		TArray<UObject*> Widgets;
		Widgets.Reserve(SelectedObjects.Num());
		for (const auto& ObjectPtr : SelectedObjects)
		{
			if (ObjectPtr.IsValid())
			{
				Widgets.Add(ObjectPtr.Get());
			}
		}
		PropertyHandle = LayoutBuilder.AddObjectPropertyData(Widgets, PropertyName);
	}
	return PropertyHandle;
}

const FSlateBrush* FComponentTransformDetails::GetPreserveScaleRatioImage() const
{
	return bPreserveScaleRatio ? FAppStyle::GetBrush(TEXT("Icons.Lock")) : FAppStyle::GetBrush(TEXT("Icons.Unlock"));
}

ECheckBoxState FComponentTransformDetails::IsPreserveScaleRatioChecked() const
{
	return bPreserveScaleRatio ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FComponentTransformDetails::OnPreserveScaleRatioToggled( ECheckBoxState NewState )
{
	bPreserveScaleRatio = (NewState == ECheckBoxState::Checked) ? true : false;
	GConfig->SetBool(TEXT("SelectionDetails"), TEXT("PreserveScaleRatio"), bPreserveScaleRatio, GEditorPerProjectIni);
}

bool FComponentTransformDetails::GetLocationResetVisibility() const
{
	const ULexWidget* Widget = GetFirstSelectedWidget();
	if (!IsValid(Widget))return false;
	FVector TargetLocation = FVector::ZeroVector;
	if (!IsRootWidget())//Y/Z offset of a child widget is controlled by anchors
	{
		TargetLocation.Y = Widget->GetRelativeLocation().Y;
		TargetLocation.Z = Widget->GetRelativeLocation().Z;
	}
	return Widget->GetRelativeLocation() != TargetLocation;
}

void FComponentTransformDetails::OnLocationResetClicked()
{
	const FText TransactionName = LOCTEXT("ResetLocation", "Reset Location");
	FScopedTransaction Transaction(TransactionName);

	ULexWidget* Widget = GetFirstSelectedWidget();
	if (!IsValid(Widget))return;
	FVector TargetLocation = FVector::ZeroVector;
	if (!IsRootWidget())//Y/Z offset of a child widget is controlled by anchors
	{
		TargetLocation.Y = Widget->GetRelativeLocation().Y;
		TargetLocation.Z = Widget->GetRelativeLocation().Z;
	}

	OnSetTransform(ETransformField::Location, EAxisList::All, TargetLocation, true);
}

bool FComponentTransformDetails::GetRotationResetVisibility() const
{
	const ULexWidget* Widget = GetFirstSelectedWidget();
	if (!IsValid(Widget))return false;
	return Widget->GetRelativeRotation().Euler() != FVector::ZeroVector;
}

void FComponentTransformDetails::OnRotationResetClicked()
{
	const FText TransactionName = LOCTEXT("ResetRotation", "Reset Rotation");
	FScopedTransaction Transaction(TransactionName);

	if (!IsValid(GetFirstSelectedWidget()))return;

	OnSetTransform(ETransformField::Rotation, EAxisList::All, FVector::ZeroVector, true);
}

bool FComponentTransformDetails::GetScaleResetVisibility() const
{
	const ULexWidget* Widget = GetFirstSelectedWidget();
	if (!IsValid(Widget))return false;
	return Widget->GetRelativeScale() != FVector::OneVector;
}

void FComponentTransformDetails::OnScaleResetClicked()
{
	const FText TransactionName = LOCTEXT("ResetScale", "Reset Scale");
	FScopedTransaction Transaction(TransactionName);

	if (!IsValid(GetFirstSelectedWidget()))return;

	OnSetTransform(ETransformField::Scale, EAxisList::All, FVector(1.0f), true);
}

void FComponentTransformDetails::CacheTransform()
{
	FVector CurLoc = FVector(EForceInit::ForceInitToZero);
	FRotator CurRot = FRotator(EForceInit::ForceInitToZero);
	FVector CurScale = FVector(EForceInit::ForceInitToZero);

	for( int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex )
	{
		ULexWidget* Widget = SelectedObjects[ObjectIndex].Get();
		if( !IsValid(Widget) )continue;

		const FVector Loc = Widget->GetRelativeLocation();
		FRotator* FoundRotator = ObjectToRelativeRotationMap.Find(Widget);
		const FRotator Rot = (bEditingRotationInUI && !Widget->IsTemplate() && FoundRotator) ? *FoundRotator : Widget->GetRelativeRotation().Rotator();
		const FVector Scale = Widget->GetRelativeScale();

		if( ObjectIndex == 0 )
		{
			// Cache the current values from the first widget to see if any values differ among other widgets
			CurLoc = Loc;
			CurRot = Rot;
			CurScale = Scale;

			CachedLocation.Set( Loc );
			CachedRotation.Set( Rot );
			CachedScale.Set( Scale );
		}
		else if( CurLoc != Loc || CurRot != Rot || CurScale != Scale )
		{
			// Check which values differ and unset the different values
			CachedLocation.X = Loc.X == CurLoc.X && CachedLocation.X.IsSet() ? Loc.X : TOptional<FVector::FReal>();
			CachedLocation.Y = Loc.Y == CurLoc.Y && CachedLocation.Y.IsSet() ? Loc.Y : TOptional<FVector::FReal>();
			CachedLocation.Z = Loc.Z == CurLoc.Z && CachedLocation.Z.IsSet() ? Loc.Z : TOptional<FVector::FReal>();

			CachedRotation.X = Rot.Roll == CurRot.Roll && CachedRotation.X.IsSet() ? Rot.Roll : TOptional<FVector::FReal>();
			CachedRotation.Y = Rot.Pitch == CurRot.Pitch && CachedRotation.Y.IsSet() ? Rot.Pitch : TOptional<FVector::FReal>();
			CachedRotation.Z = Rot.Yaw == CurRot.Yaw && CachedRotation.Z.IsSet() ? Rot.Yaw : TOptional<FVector::FReal>();

			CachedScale.X = Scale.X == CurScale.X && CachedScale.X.IsSet() ? Scale.X : TOptional<FVector::FReal>();
			CachedScale.Y = Scale.Y == CurScale.Y && CachedScale.Y.IsSet() ? Scale.Y : TOptional<FVector::FReal>();
			CachedScale.Z = Scale.Z == CurScale.Z && CachedScale.Z.IsSet() ? Scale.Z : TOptional<FVector::FReal>();

			// If all values are unset all values are different and we can stop looking
			if( !CachedLocation.IsSet() && !CachedRotation.IsSet() && !CachedScale.IsSet() )
			{
				break;
			}
		}
	}
}

FVector FComponentTransformDetails::GetAxisFilteredVector(EAxisList::Type Axis, const FVector& NewValue, const FVector& OldValue)
{
	return FVector((Axis & EAxisList::X) ? NewValue.X : OldValue.X,
		(Axis & EAxisList::Y) ? NewValue.Y : OldValue.Y,
		(Axis & EAxisList::Z) ? NewValue.Z : OldValue.Z);
}

void FComponentTransformDetails::OnSetTransform(ETransformField::Type TransformField, EAxisList::Type Axis, FVector NewValue, bool bCommitted)
{
	if (!bCommitted && SelectedObjects.Num() > 1)
	{
		// Ignore interactive changes when we have more than one selected object
		return;
	}

	FText TransactionText;
	FProperty* ValueProperty = nullptr;
	switch (TransformField)
	{
	case ETransformField::Location:
		TransactionText = LOCTEXT("OnSetLocation", "Set Location");
		ValueProperty = FindFProperty<FProperty>(ULexWidget::StaticClass(), TEXT("RelativeLocation"));
		break;
	case ETransformField::Rotation:
		TransactionText = LOCTEXT("OnSetRotation", "Set Rotation");
		ValueProperty = FindFProperty<FProperty>(ULexWidget::StaticClass(), TEXT("RelativeRotation"));
		break;
	case ETransformField::Scale:
		TransactionText = LOCTEXT("OnSetScale", "Set Scale");
		ValueProperty = FindFProperty<FProperty>(ULexWidget::StaticClass(), TEXT("RelativeScale"));
		break;
	default:
		return;
	}

	bool bBeganTransaction = false;
	bool bModified = false;

	FPropertyChangedEvent PropertyChangedEvent(ValueProperty, !bCommitted ? EPropertyChangeType::Interactive : EPropertyChangeType::ValueSet);

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		ULexWidget* Widget = SelectedObjects[ObjectIndex].Get();
		if (!IsValid(Widget))continue;

		const bool bIsEditingTemplateObject = Widget->IsTemplate();

		FVector OldValue;
		switch (TransformField)
		{
		case ETransformField::Location:
			OldValue = Widget->GetRelativeLocation();
			break;
		case ETransformField::Rotation:
			// Pull from the actual widget or from the cache
			OldValue = Widget->GetRelativeRotation().Euler();
			if (bEditingRotationInUI && !bIsEditingTemplateObject && ObjectToRelativeRotationMap.Find(Widget))
			{
				OldValue = ObjectToRelativeRotationMap.Find(Widget)->Euler();
			}
			break;
		case ETransformField::Scale:
			OldValue = Widget->GetRelativeScale();
			break;
		default:
			OldValue = FVector::ZeroVector;
			break;
		}

		// If we're committing during a rotation edit then we need to force it
		if (OldValue == NewValue && !(bCommitted && bEditingRotationInUI))continue;

		if (!bBeganTransaction && bCommitted)
		{
			// NOTE: One transaction per change, not per object
			GEditor->BeginTransaction(TransactionText);
			bBeganTransaction = true;
		}

		if (bCommitted)
		{
			if (Widget->HasAnyFlags(RF_DefaultSubObject))
			{
				// Default subobjects must be included in any undo/redo operations
				Widget->SetFlags(RF_Transactional);
			}

			// We don't call PreEditChange for non commit changes because most classes implement the version that doesn't check the interaction type
			Widget->PreEditChange(ValueProperty);
		}

		if (NotifyHook)
		{
			NotifyHook->NotifyPreChange(ValueProperty);
		}

		switch (TransformField)
		{
		case ETransformField::Location:
			{
				if (!bIsEditingTemplateObject)
				{
					// Update local cache for restoring later
					ObjectToRelativeRotationMap.FindOrAdd(Widget) = Widget->GetRelativeRotation().Rotator();
				}

				Widget->SetRelativeLocation(NewValue);
				CachedLocation.Set(NewValue);
			}
			break;
		case ETransformField::Rotation:
			{
				const FRotator NewRotation = FRotator::MakeFromEuler(NewValue);

				if (!bIsEditingTemplateObject)
				{
					// Update local cache for restoring later
					ObjectToRelativeRotationMap.FindOrAdd(Widget) = NewRotation;
				}

				Widget->SetRelativeRotation(NewRotation.Quaternion());
				CachedRotation.Set(NewRotation);
			}
			break;
		case ETransformField::Scale:
			{
				if (bPreserveScaleRatio)
				{
					// If we set a single axis, scale the others
					float Ratio = 0.0f;
					switch (Axis)
					{
					case EAxisList::X:
						// Account for the previous scale being zero.  Just set to the new value in that case?
						Ratio = OldValue.X == 0.0f ? NewValue.X : NewValue.X / OldValue.X;
						NewValue.Y *= Ratio;
						NewValue.Z *= Ratio;
						break;
					case EAxisList::Y:
						Ratio = OldValue.Y == 0.0f ? NewValue.Y : NewValue.Y / OldValue.Y;
						NewValue.X *= Ratio;
						NewValue.Z *= Ratio;
						break;
					case EAxisList::Z:
						Ratio = OldValue.Z == 0.0f ? NewValue.Z : NewValue.Z / OldValue.Z;
						NewValue.X *= Ratio;
						NewValue.Y *= Ratio;
						break;
					default:
						// Do nothing, this set multiple axis at once
						break;
					}
				}

				Widget->SetRelativeScale(NewValue);
			}
			break;
		}

		bModified = true;
	}

	if (bModified && NotifyHook)
	{
		NotifyHook->NotifyPostChange(PropertyChangedEvent, ValueProperty);
	}

	if (bCommitted && bBeganTransaction)
	{
		GEditor->EndTransaction();
		CacheTransform();
	}

	if (bModified)
	{
		// Redraw
		GUnrealEd->RedrawLevelEditingViewports();
	}
}

void FComponentTransformDetails::OnSetTransformAxis(FVector::FReal NewValue, ETextCommit::Type CommitInfo, ETransformField::Type TransformField, EAxisList::Type Axis, bool bCommitted)
{
	ULexWidget* Widget = GetFirstSelectedWidget();
	if (!IsValid(Widget))return;
	switch (TransformField)
	{
	case ETransformField::Location:
	{
		FVector NewVector = GetAxisFilteredVector(Axis, FVector(NewValue), Widget->GetRelativeLocation());
		OnSetTransform(TransformField, Axis, NewVector, bCommitted);
	}
	break;
	case ETransformField::Rotation:
	{
		FVector NewVector = GetAxisFilteredVector(Axis, FVector(NewValue), Widget->GetRelativeRotation().Euler());
		OnSetTransform(TransformField, Axis, NewVector, bCommitted);
	}
	break;
	case ETransformField::Scale:
	{
		FVector NewVector = GetAxisFilteredVector(Axis, FVector(NewValue), Widget->GetRelativeScale());
		OnSetTransform(TransformField, Axis, NewVector, bCommitted);
	}
	break;
	}
}

void FComponentTransformDetails::BeginSliderTransaction(const FText& TransactionName) const
{
	// Start a new transaction when a slider begins to change, we'll end it when the slider is released
	// NOTE: One transaction per change, not per object
	GEditor->BeginTransaction(TransactionName);

	for (const auto& ObjectPtr : SelectedObjects)
	{
		ULexWidget* Widget = ObjectPtr.Get();
		if (!IsValid(Widget))continue;

		if (Widget->HasAnyFlags(RF_DefaultSubObject))
		{
			// Default subobjects must be included in any undo/redo operations
			Widget->SetFlags(RF_Transactional);
		}

		// Call modify but not PreEdit, we don't do the proper "Edit" until it's committed
		Widget->Modify();
	}
}

void FComponentTransformDetails::OnBeginRotationSlider()
{
	BeginSliderTransaction(LOCTEXT("OnSetRotation", "Set Rotation"));

	bEditingRotationInUI = true;

	// Add/update cached rotation value prior to slider interaction
	for (const auto& ObjectPtr : SelectedObjects)
	{
		ULexWidget* Widget = ObjectPtr.Get();
		if (!IsValid(Widget))continue;

		ObjectToRelativeRotationMap.FindOrAdd(Widget) = Widget->GetRelativeRotation().Rotator();
	}
}

void FComponentTransformDetails::OnEndRotationSlider(FVector::FReal NewValue)
{
	// Commit gets called right before this, only need to end the transaction
	bEditingRotationInUI = false;
	GEditor->EndTransaction();
}

void FComponentTransformDetails::OnBeginLocationSlider()
{
	BeginSliderTransaction(LOCTEXT("OnSetLocation", "Set Location"));
}

void FComponentTransformDetails::OnEndLocationSlider(FVector::FReal NewValue)
{
	GEditor->EndTransaction();
}

void FComponentTransformDetails::OnBeginScaleSlider()
{
	BeginSliderTransaction(LOCTEXT("OnSetScale", "Set Scale"));
}

void FComponentTransformDetails::OnEndScaleSlider(FVector::FReal NewValue)
{
	GEditor->EndTransaction();
}

#undef LOCTEXT_NAMESPACE
