// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "ComponentTransformDetails.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Textures/SlateIcon.h"
#include "EditorStyleSet.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "UObject/UnrealType.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Misc/ConfigCacheIni.h"
#include "SlateOptMacros.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Editor/UnrealEdEngine.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SRotatorInputBox.h"
#include "ScopedTransaction.h"
#include "IPropertyUtilities.h"
#include "Math/UnitConversion.h"
#include "Widgets/Input/NumericUnitTypeInterface.inl"
#include "Settings/EditorProjectSettings.h"
#include "HAL/PlatformApplicationMisc.h"

#include "Core/ActorComponent/UIItem.h"

#include "DetailCategoryBuilder.h"
#include "Algo/Transform.h"

#define LOCTEXT_NAMESPACE "LGUIComponentTransformDetails"

class FScopedSwitchWorldForObject
{
public:
	FScopedSwitchWorldForObject( UObject* Object )
		: PrevWorld( NULL )
	{
		bool bRequiresPlayWorld = false;
		if( GUnrealEd->PlayWorld && !GIsPlayInEditorWorld )
		{
			UPackage* ObjectPackage = Object->GetOutermost();
			bRequiresPlayWorld = ObjectPackage->HasAnyPackageFlags(PKG_PlayInEditor);
		}

		if( bRequiresPlayWorld )
		{
			PrevWorld = SetPlayInEditorWorld( GUnrealEd->PlayWorld );
		}
	}

	~FScopedSwitchWorldForObject()
	{
		if( PrevWorld )
		{
			RestoreEditorWorld( PrevWorld );
		}
	}

private:
	UWorld* PrevWorld;
};

static USceneComponent* GetSceneComponentFromDetailsObject(UObject* InObject)
{
	AActor* Actor = Cast<AActor>(InObject);
	if (Actor)
	{
		return Actor->GetRootComponent();
	}

	return Cast<USceneComponent>(InObject);
}

FComponentTransformDetails::FComponentTransformDetails( const TArray< TWeakObjectPtr<UUIItem> >& InSelectedObjects, const FSelectedActorInfo& InSelectedActorInfo, IDetailLayoutBuilder& DetailBuilder )
	: TNumericUnitTypeInterface(GetDefault<UEditorProjectAppearanceSettings>()->bDisplayUnitsOnComponentTransforms ? EUnit::Centimeters : EUnit::Unspecified)
	, SelectedActorInfo( InSelectedActorInfo )
	, SelectedObjects( InSelectedObjects )
	, NotifyHook( DetailBuilder.GetPropertyUtilities()->GetNotifyHook() )
	, bPreserveScaleRatio( false )
	, bEditingRotationInUI( false )
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
				.IsEnabled(this, &FComponentTransformDetails::GetIsEnabled)
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
	const USceneComponent* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return;

	UClass* SceneComponentClass = USceneComponent::StaticClass();
		
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
		.PropertyHandleList({ GeneratePropertyHandle(USceneComponent::GetRelativeLocationPropertyName(), ChildrenBuilder) })
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
			.IsEnabled( this, &FComponentTransformDetails::GetIsEnabled )
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
		.PropertyHandleList({ GeneratePropertyHandle(USceneComponent::GetRelativeRotationPropertyName(), ChildrenBuilder) })
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
			.IsEnabled( this, &FComponentTransformDetails::GetIsEnabled )
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
		.PropertyHandleList({ GeneratePropertyHandle(USceneComponent::GetRelativeScale3DPropertyName(), ChildrenBuilder) })
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
			.IsEnabled( this, &FComponentTransformDetails::GetIsEnabled )
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

void FComponentTransformDetails::Tick( float DeltaTime ) 
{
	CacheTransform();
	/*if (!FixedDisplayUnits.IsSet())
	{
		CacheCommonLocationUnits();
	}*/
}

void FComponentTransformDetails::CacheCommonLocationUnits()
{
	float LargestValue = 0.f;
	if (CachedLocation.X.IsSet() && CachedLocation.X.GetValue() > LargestValue)
	{
		LargestValue = CachedLocation.X.GetValue();
	}
	if (CachedLocation.Y.IsSet() && CachedLocation.Y.GetValue() > LargestValue)
	{
		LargestValue = CachedLocation.Y.GetValue();
	}
	if (CachedLocation.Z.IsSet() && CachedLocation.Z.GetValue() > LargestValue)
	{
		LargestValue = CachedLocation.Z.GetValue();
	}

	SetupFixedDisplay(LargestValue);
}

TSharedPtr<IPropertyHandle> FComponentTransformDetails::GeneratePropertyHandle(FName PropertyName, IDetailChildrenBuilder& ChildrenBuilder)
{
	// Try finding the property handle in the details panel's property map first.
	IDetailLayoutBuilder& LayoutBuilder = ChildrenBuilder.GetParentCategory().GetParentLayout();
	TSharedPtr<IPropertyHandle> PropertyHandle = LayoutBuilder.GetProperty(PropertyName, USceneComponent::StaticClass());
	if (!PropertyHandle || !PropertyHandle->IsValidHandle())
	{
		// If it wasn't found, add a collapsed row which contains the property node.
		TArray<UObject*> SceneComponents;
		Algo::Transform(SelectedObjects, SceneComponents, [](TWeakObjectPtr<UObject> Obj) { return GetSceneComponentFromDetailsObject(Obj.Get()); });
		PropertyHandle = LayoutBuilder.AddObjectPropertyData(SceneComponents, PropertyName);
		//CachedHandlesObjects.Append(SceneComponents);
	}

	//PropertyHandles.Add(PropertyHandle);
	return PropertyHandle;
}

bool FComponentTransformDetails::GetIsEnabled() const
{
	return !GEditor->HasLockedActors() || SelectedActorInfo.NumSelected == 0;
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
	const USceneComponent* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return false;
	FVector targetLocation = FVector::ZeroVector;
	if (!IsLocationXEnable())
	{
		targetLocation.X = Archetype->GetRelativeLocation().X;
	}
	if (!IsLocationYEnable())
	{
		targetLocation.Y = Archetype->GetRelativeLocation().Y;
	}
	if (!IsLocationZEnable())
	{
		targetLocation.Z = Archetype->GetRelativeLocation().Z;
	}
	return Archetype->GetRelativeLocation() != targetLocation;
}

void FComponentTransformDetails::OnLocationResetClicked()
{
	const FText TransactionName = LOCTEXT("ResetLocation", "Reset Location");
	FScopedTransaction Transaction(TransactionName);

	UUIItem* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return;
	FVector targetLocation = FVector::ZeroVector;
	if (!IsLocationXEnable())
	{
		targetLocation.X = Archetype->GetRelativeLocation().X;
	}
	if (!IsLocationYEnable())
	{
		targetLocation.Y = Archetype->GetRelativeLocation().Y;
	}
	if (!IsLocationZEnable())
	{
		targetLocation.Z = Archetype->GetRelativeLocation().Z;
	}

	OnSetTransform(ETransformField::Location, EAxisList::All, targetLocation, true);
}

bool FComponentTransformDetails::GetRotationResetVisibility() const
{
	const USceneComponent* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return false;
	return Archetype->GetRelativeRotation().Euler() != FVector::ZeroVector;
}

void FComponentTransformDetails::OnRotationResetClicked()
{
	const FText TransactionName = LOCTEXT("ResetRotation", "Reset Rotation");
	FScopedTransaction Transaction(TransactionName);

	UUIItem* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return;

	OnSetTransform(ETransformField::Rotation, EAxisList::All, FVector::ZeroVector, true);
}

bool FComponentTransformDetails::GetScaleResetVisibility() const
{
	const USceneComponent* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return false;
	return Archetype->GetRelativeScale3D() != FVector::OneVector;
}

void FComponentTransformDetails::OnScaleResetClicked()
{
	const FText TransactionName = LOCTEXT("ResetScale", "Reset Scale");
	FScopedTransaction Transaction(TransactionName);

	UUIItem* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return;

	OnSetTransform(ETransformField::Scale, EAxisList::All, FVector(1.0f), true);
}

void FComponentTransformDetails::CacheTransform()
{
	FVector CurLoc;
	FRotator CurRot;
	FVector CurScale;

	for( int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex )
	{
		TWeakObjectPtr<UUIItem> ObjectPtr = SelectedObjects[ObjectIndex];
		if( ObjectPtr.IsValid() )
		{
			UUIItem* Object = ObjectPtr.Get();
			USceneComponent* SceneComponent = Object;

			FVector Loc;
			FRotator Rot;
			FVector Scale;
			if( SceneComponent )
			{
				Loc = SceneComponent->GetRelativeLocation();
				FRotator* FoundRotator = ObjectToRelativeRotationMap.Find(SceneComponent);
				Rot = (bEditingRotationInUI && !Object->IsTemplate() && FoundRotator) ? *FoundRotator : SceneComponent->GetRelativeRotation();
				Scale = SceneComponent->GetRelativeScale3D();

				if( ObjectIndex == 0 )
				{
					// Cache the current values from the first actor to see if any values differ among other actors
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
					const bool bAllValuesDiffer = !CachedLocation.IsSet() && !CachedRotation.IsSet() && !CachedScale.IsSet();
					if( bAllValuesDiffer )
					{
						break;
					}
				}
			}
		}
	}
}

bool FComponentTransformDetails::IsLocationYEnable()const
{
	if (SelectedObjects.Num() > 0)
	{
		TWeakObjectPtr<UUIItem> uiItem = SelectedObjects[0];
		if (uiItem.IsValid())
		{
			if (uiItem->GetParentUIItem() == nullptr)
			{
				return true;
			}
			return false;
		}
	}
	return false;
}
bool FComponentTransformDetails::IsLocationZEnable()const
{
	if (SelectedObjects.Num() > 0)
	{
		TWeakObjectPtr<UUIItem> uiItem = SelectedObjects[0];
		if (uiItem.IsValid())
		{
			if (uiItem->GetParentUIItem() == nullptr)
			{
				return true;
			}
			return false;
		}
	}
	return false;
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
	FProperty* AxisProperty = nullptr;
	
	switch (TransformField)
	{
	case ETransformField::Location:
		TransactionText = LOCTEXT("OnSetLocation", "Set Location");
		ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), TEXT("RelativeLocation"));
		
		// Only set axis property for single axis set
		if (Axis == EAxisList::X)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FVector>::Get(), GET_MEMBER_NAME_CHECKED(FVector, X));
		}
		else if (Axis == EAxisList::Y)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FVector>::Get(), GET_MEMBER_NAME_CHECKED(FVector, Y));
		}
		else if (Axis == EAxisList::Z)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FVector>::Get(), GET_MEMBER_NAME_CHECKED(FVector, Z));
		}
		break;
	case ETransformField::Rotation:
		TransactionText = LOCTEXT("OnSetRotation", "Set Rotation");
		ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), TEXT("RelativeRotation"));
		
		// Only set axis property for single axis set
		if (Axis == EAxisList::X)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FRotator>::Get(), GET_MEMBER_NAME_CHECKED(FRotator, Roll));
		}
		else if (Axis == EAxisList::Y)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FRotator>::Get(), GET_MEMBER_NAME_CHECKED(FRotator, Pitch));
		}
		else if (Axis == EAxisList::Z)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FRotator>::Get(), GET_MEMBER_NAME_CHECKED(FRotator, Yaw));
		}
		break;
	case ETransformField::Scale:
		TransactionText = LOCTEXT("OnSetScale", "Set Scale");
		ValueProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), TEXT("RelativeScale3D"));

		// If keep scale is set, don't set axis property
		if (!bPreserveScaleRatio && Axis == EAxisList::X)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FVector>::Get(), GET_MEMBER_NAME_CHECKED(FVector, X));
		}
		else if (!bPreserveScaleRatio && Axis == EAxisList::Y)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FVector>::Get(), GET_MEMBER_NAME_CHECKED(FVector, Y));
		}
		else if (!bPreserveScaleRatio && Axis == EAxisList::Z)
		{
			AxisProperty = FindFProperty<FFloatProperty>(TBaseStructure<FVector>::Get(), GET_MEMBER_NAME_CHECKED(FVector, Z));
		}
		break;
	default:
		return;
	}

	bool bBeganTransaction = false;
	TArray<UObject*> ModifiedObjects;

	FPropertyChangedEvent PropertyChangedEvent(ValueProperty, !bCommitted ? EPropertyChangeType::Interactive : EPropertyChangeType::ValueSet, MakeArrayView(ModifiedObjects));
	FEditPropertyChain PropertyChain;

	if (AxisProperty)
	{
		PropertyChain.AddHead(AxisProperty);
	}
	PropertyChain.AddHead(ValueProperty);
	FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		TWeakObjectPtr<UUIItem> ObjectPtr = SelectedObjects[ObjectIndex];
		if (ObjectPtr.IsValid())
		{
			UUIItem* Object = ObjectPtr.Get();
			UUIItem* SceneComponent = Object;
			if (SceneComponent)
			{
				AActor* EditedActor = SceneComponent->GetOwner();
				const bool bIsEditingTemplateObject = Object->IsTemplate();

				FVector OldComponentValue;
				FVector NewComponentValue;

				switch (TransformField)
				{
				case ETransformField::Location:
					OldComponentValue = SceneComponent->GetRelativeLocation();
					break;
				case ETransformField::Rotation:
					// Pull from the actual component or from the cache
					OldComponentValue = SceneComponent->GetRelativeRotation().Euler();
					if (bEditingRotationInUI && !bIsEditingTemplateObject && ObjectToRelativeRotationMap.Find(SceneComponent))
					{
						OldComponentValue = ObjectToRelativeRotationMap.Find(SceneComponent)->Euler();
					}
					break;
				case ETransformField::Scale:
					OldComponentValue = SceneComponent->GetRelativeScale3D();
					break;
				}

				//NewComponentValue = GetAxisFilteredVector(Axis, NewValue, OldComponentValue);
				NewComponentValue = NewValue;

				// If we're committing during a rotation edit then we need to force it
				if (OldComponentValue != NewComponentValue || (bCommitted && bEditingRotationInUI))
				{
					if (!bBeganTransaction && bCommitted)
					{
						// Begin a transaction the first time an actors rotation is about to change.
						// NOTE: One transaction per change, not per actor
						GEditor->BeginTransaction(TransactionText);
						bBeganTransaction = true;
					}

					FScopedSwitchWorldForObject WorldSwitcher(Object);

					if (bCommitted)
					{
						if (!bIsEditingTemplateObject)
						{
							// Broadcast the first time an actor is about to move
							GEditor->BroadcastBeginObjectMovement(*SceneComponent);
							if (EditedActor && EditedActor->GetRootComponent() == SceneComponent)
							{
								GEditor->BroadcastBeginObjectMovement(*EditedActor);
							}
						}

						if (SceneComponent->HasAnyFlags(RF_DefaultSubObject))
						{
							// Default subobjects must be included in any undo/redo operations
							SceneComponent->SetFlags(RF_Transactional);
						}

						// Have to downcast here because of function overloading and inheritance not playing nicely
						// We don't call PreEditChange for non commit changes because most classes implement the version that doesn't check the interaction type
						((UObject*)SceneComponent)->PreEditChange(PropertyChain);
						if (EditedActor && EditedActor->GetRootComponent() == SceneComponent)
						{
							((UObject*)EditedActor)->PreEditChange(PropertyChain);
						}
					}

					if (NotifyHook)
					{
						//NotifyHook->NotifyPreChange(ValueProperty);
					}

					switch (TransformField)
					{
					case ETransformField::Location:
						{
							if (!bIsEditingTemplateObject)
							{
								// Update local cache for restoring later
								ObjectToRelativeRotationMap.FindOrAdd(SceneComponent) = SceneComponent->GetRelativeRotation();
							}

							SceneComponent->SetRelativeLocation(NewComponentValue);

							// Also forcibly set it as the cache may have changed it slightly
							SceneComponent->GetRelativeLocation_DirectMutable() = NewComponentValue;
							CachedLocation.Set(NewComponentValue);

							// If it's a template, propagate the change out to any current instances of the object
							if (bIsEditingTemplateObject)
							{
								TSet<USceneComponent*> UpdatedInstances;
								FComponentEditorUtils::PropagateDefaultValueChange(SceneComponent, ValueProperty, OldComponentValue, NewComponentValue, UpdatedInstances);
							}

							break;
						}
					case ETransformField::Rotation:
						{
							FRotator NewRotation = FRotator::MakeFromEuler(NewComponentValue);

							if (!bIsEditingTemplateObject)
							{
								// Update local cache for restoring later
								ObjectToRelativeRotationMap.FindOrAdd(SceneComponent) = NewRotation;
							}

							SceneComponent->SetRelativeRotationExact(NewRotation);
							CachedRotation.Set(NewRotation);

							// If it's a template, propagate the change out to any current instances of the object
							if (bIsEditingTemplateObject)
							{
								TSet<USceneComponent*> UpdatedInstances;
								FComponentEditorUtils::PropagateDefaultValueChange(SceneComponent, ValueProperty, FRotator::MakeFromEuler(OldComponentValue), NewRotation, UpdatedInstances);
							}

							break;
						}
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
									Ratio = OldComponentValue.X == 0.0f ? NewComponentValue.X : NewComponentValue.X / OldComponentValue.X;
									NewComponentValue.Y *= Ratio;
									NewComponentValue.Z *= Ratio;
									break;
								case EAxisList::Y:
									Ratio = OldComponentValue.Y == 0.0f ? NewComponentValue.Y : NewComponentValue.Y / OldComponentValue.Y;
									NewComponentValue.X *= Ratio;
									NewComponentValue.Z *= Ratio;
									break;
								case EAxisList::Z:
									Ratio = OldComponentValue.Z == 0.0f ? NewComponentValue.Z : NewComponentValue.Z / OldComponentValue.Z;
									NewComponentValue.X *= Ratio;
									NewComponentValue.Y *= Ratio;
								default:
									// Do nothing, this set multiple axis at once
									break;
								}
							}

							SceneComponent->SetRelativeScale3D(NewComponentValue);

							// If it's a template, propagate the change out to any current instances of the object
							if (bIsEditingTemplateObject)
							{
								TSet<USceneComponent*> UpdatedInstances;
								FComponentEditorUtils::PropagateDefaultValueChange(SceneComponent, ValueProperty, OldComponentValue, NewComponentValue, UpdatedInstances);
							}

							break;
						}
					}

					ModifiedObjects.Add(Object);
				}
			}
		}
	}

	if (ModifiedObjects.Num())
	{
		for (UObject* Object : ModifiedObjects)
		{
			USceneComponent* SceneComponent = GetSceneComponentFromDetailsObject(Object);
			USceneComponent* OldSceneComponent = SceneComponent;

			if (SceneComponent)
			{
				AActor* EditedActor = SceneComponent->GetOwner();
				FString SceneComponentPath = SceneComponent->GetPathName(EditedActor);
				
				if (bCommitted)
				{
					// This can invalidate OldSceneComponent
					// We don't call PostEditChange for non commit changes because most classes implement the version that doesn't check the interaction type
					OldSceneComponent->PostEditChangeChainProperty(PropertyChangedChainEvent);
				}
				else
				{
					SnapshotTransactionBuffer(OldSceneComponent);
				}

				SceneComponent = FindObject<USceneComponent>(EditedActor, *SceneComponentPath);

				if (EditedActor && EditedActor->GetRootComponent() == SceneComponent)
				{
					if (bCommitted)
					{
						EditedActor->PostEditChangeChainProperty(PropertyChangedChainEvent);
						SceneComponent = FindObject<USceneComponent>(EditedActor, *SceneComponentPath);
					}
					else
					{
						SnapshotTransactionBuffer(EditedActor);
					}
				}
				
				if (!Object->IsTemplate())
				{
					if (TransformField == ETransformField::Rotation || TransformField == ETransformField::Location)
					{
						FRotator* FoundRotator = ObjectToRelativeRotationMap.Find(OldSceneComponent);

						if (FoundRotator)
						{
							FQuat OldQuat = FoundRotator->GetDenormalized().Quaternion();
							FQuat NewQuat = SceneComponent->GetRelativeRotation().GetDenormalized().Quaternion();

							if (OldQuat.Equals(NewQuat))
							{
								// Need to restore the manually set rotation as it was modified by quat conversion
								SceneComponent->SetRelativeRotation(*FoundRotator);
							}
						}
					}

					if (bCommitted)
					{
						// Broadcast when the actor is done moving
						GEditor->BroadcastEndObjectMovement(*SceneComponent);
						if (EditedActor && EditedActor->GetRootComponent() == SceneComponent)
						{
							GEditor->BroadcastEndObjectMovement(*EditedActor);
						}
					}
				}
			}
		}

		if (NotifyHook)
		{
			//NotifyHook->NotifyPostChange(PropertyChangedEvent, ValueProperty);
		}
	}

	if (bCommitted && bBeganTransaction)
	{
		GEditor->EndTransaction();
		CacheTransform();
	}

	GUnrealEd->UpdatePivotLocationForSelection();
	GUnrealEd->SetPivotMovedIndependently(false);
	// Redraw
	GUnrealEd->RedrawLevelEditingViewports();
}

void FComponentTransformDetails::OnSetTransformAxis(FVector::FReal NewValue, ETextCommit::Type CommitInfo, ETransformField::Type TransformField, EAxisList::Type Axis, bool bCommitted)
{
	if (SelectedObjects.Num() <= 0)return;
	UUIItem* Archetype = SelectedObjects[0].Get();
	if (!IsValid(Archetype))return;
	switch (TransformField)
	{
	case ETransformField::Location:
	{
		FVector NewVector = GetAxisFilteredVector(Axis, FVector(NewValue), Archetype->GetRelativeLocation());
		OnSetTransform(TransformField, Axis, NewVector, bCommitted);
	}
	break;
	case ETransformField::Rotation:
	{
		FVector NewVector = GetAxisFilteredVector(Axis, FVector(NewValue), Archetype->GetRelativeRotation().Euler());
		OnSetTransform(TransformField, Axis, NewVector, bCommitted);
	}
	break;
	case ETransformField::Scale:
	{
		FVector NewVector = GetAxisFilteredVector(Axis, FVector(NewValue), Archetype->GetRelativeScale3D());
		OnSetTransform(TransformField, Axis, NewVector, bCommitted);
	}
	break;
	}
}

void FComponentTransformDetails::BeginSliderTransaction(FText ActorTransaction, FText ComponentTransaction) const
{
	bool bBeganTransaction = false;
	for (TWeakObjectPtr<UObject> ObjectPtr : SelectedObjects)
	{
		if (ObjectPtr.IsValid())
		{
			UObject* Object = ObjectPtr.Get();

			// Start a new transaction when a slider begins to change
			// We'll end it when the slider is released
			// NOTE: One transaction per change, not per actor
			if (!bBeganTransaction)
			{
				if (Object->IsA<AActor>())
				{
					GEditor->BeginTransaction(ActorTransaction);
				}
				else
				{
					GEditor->BeginTransaction(ComponentTransaction);
				}

				bBeganTransaction = true;
			}

			USceneComponent* SceneComponent = GetSceneComponentFromDetailsObject(Object);
			if (SceneComponent)
			{
				FScopedSwitchWorldForObject WorldSwitcher(Object);

				if (SceneComponent->HasAnyFlags(RF_DefaultSubObject))
				{
					// Default subobjects must be included in any undo/redo operations
					SceneComponent->SetFlags(RF_Transactional);
				}

				// Call modify but not PreEdit, we don't do the proper "Edit" until it's committed
				SceneComponent->Modify();
			}
		}
	}

	// Just in case we couldn't start a new transaction for some reason
	if (!bBeganTransaction)
	{
		GEditor->BeginTransaction(ActorTransaction);
	}
}

void FComponentTransformDetails::OnBeginRotationSlider()
{
	FText ActorTransaction = LOCTEXT("OnSetRotation", "Set Rotation");
	FText ComponentTransaction = LOCTEXT("OnSetRotation_ComponentDirect", "Modify Component(s)");
	BeginSliderTransaction(ActorTransaction, ComponentTransaction);
	
	bEditingRotationInUI = true;
	bIsSliderTransaction = true;

	for (TWeakObjectPtr<UObject> ObjectPtr : SelectedObjects)
	{
		if (ObjectPtr.IsValid())
		{
			UObject* Object = ObjectPtr.Get();

			USceneComponent* SceneComponent = GetSceneComponentFromDetailsObject(Object);
			if (SceneComponent)
			{
				FScopedSwitchWorldForObject WorldSwitcher(Object);

				// Add/update cached rotation value prior to slider interaction
				ObjectToRelativeRotationMap.FindOrAdd(SceneComponent) = SceneComponent->GetRelativeRotation();
			}
		}
	}
}

void FComponentTransformDetails::OnEndRotationSlider(FVector::FReal NewValue)
{
	// Commit gets called right before this, only need to end the transaction
	bEditingRotationInUI = false;
	bIsSliderTransaction = false;
	GEditor->EndTransaction();
}

void FComponentTransformDetails::OnBeginLocationSlider()
{
	bIsSliderTransaction = true;
	FText ActorTransaction = LOCTEXT("OnSetLocation", "Set Location");
	FText ComponentTransaction = LOCTEXT("OnSetLocation_ComponentDirect", "Modify Component Location");
	BeginSliderTransaction(ActorTransaction, ComponentTransaction);
}

void FComponentTransformDetails::OnEndLocationSlider(FVector::FReal NewValue)
{
	bIsSliderTransaction = false;
	GEditor->EndTransaction();
}

void FComponentTransformDetails::OnBeginScaleSlider()
{
	// Assumption: slider isn't usable if multiple objects are selected
	//SliderScaleRatio.X = CachedScale.X.GetValue();
	//SliderScaleRatio.Y = CachedScale.Y.GetValue();
	//SliderScaleRatio.Z = CachedScale.Z.GetValue();

	bIsSliderTransaction = true;
	FText ActorTransaction = LOCTEXT("OnSetScale", "Set Scale");
	FText ComponentTransaction = LOCTEXT("OnSetScale_ComponentDirect", "Modify Component Scale");
	BeginSliderTransaction(ActorTransaction, ComponentTransaction);
}

void FComponentTransformDetails::OnEndScaleSlider(FVector::FReal NewValue)
{
	bIsSliderTransaction = false;
	GEditor->EndTransaction();
}

#undef LOCTEXT_NAMESPACE
