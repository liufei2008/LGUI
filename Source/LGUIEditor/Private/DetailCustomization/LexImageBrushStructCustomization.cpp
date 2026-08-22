// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexImageBrushStructCustomization.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "Core/LexUIImageBrush.h"
#include "Slate/SlateTextureAtlasInterface.h"
#include "TextureCompiler.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Core/LexUISpriteData_BaseObject.h"

#define LOCTEXT_NAMESPACE "LexImageBrushStructCustomization"

class SBrushResourceError : public SBorder
{
public:
	SLATE_BEGIN_ARGS( SBrushResourceError ) {}
	SLATE_DEFAULT_SLOT( FArguments, Content )
SLATE_END_ARGS()

void Construct( const FArguments& InArgs )
	{
		SBorder::Construct( SBorder::FArguments()
			.BorderBackgroundColor( FCoreStyle::Get().GetColor("ErrorReporting.BackgroundColor") )
			.BorderImage( FCoreStyle::Get().GetBrush("ErrorReporting.Box") )
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding( FMargin(3,0) )
			[
				InArgs._Content.Widget
			]
		);
	}
};

class SBrushResourceObjectBox : public SCompoundWidget
{
	SLATE_BEGIN_ARGS( SBrushResourceObjectBox ) {}
	
	SLATE_END_ARGS()

	struct FPropertyParams
	{
		TSharedPtr<IPropertyHandle> ResourceObjectProperty;
		TSharedPtr<IPropertyHandle> ImageSizeProperty;
		TSharedPtr<IPropertyHandle> DrawAsProperty;
		TSharedPtr<IPropertyHandle> UVRegionProperty;
	};

	void Construct(const FArguments& InArgs
		, const IStructCustomizationUtils* StructCustomizationUtils
		, const FPropertyParams& InParams)
	{
		ResourceObjectProperty = InParams.ResourceObjectProperty;
		ImageSizeProperty = InParams.ImageSizeProperty;
		DrawAsProperty = InParams.DrawAsProperty;
		UVRegionProperty = InParams.UVRegionProperty;

		FSimpleDelegate OnBrushResourceChangedDelegate = FSimpleDelegate::CreateSP(this, &SBrushResourceObjectBox::OnBrushResourceChanged);
		ResourceObjectProperty->SetOnPropertyValueChanged(OnBrushResourceChangedDelegate);

		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(InParams.ResourceObjectProperty)
				.ThumbnailPool(StructCustomizationUtils->GetThumbnailPool())
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding( 0.0f, 3.0f )
			[
				SAssignNew(ResourceError, SBrushResourceError )
				[
					SNew( SVerticalBox )
					+ SVerticalBox::Slot()
					.HAlign( HAlign_Left )
					[
						SAssignNew(IsEngineMaterialError, STextBlock)
						.Text( NSLOCTEXT("FSlateBrushStructCustomization", "IsEngineMaterialErrorText", "Assign a parent UI Material" ) )
					]
				]
			]
		];

		UpdateResourceError();
	}

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )override
	{
		UpdateResourceError();
	}

private:
	void OnBrushResourceChanged()
	{
		UObject* ResourceObject;
		FPropertyAccess::Result Result = ResourceObjectProperty->GetValue(ResourceObject);
		if ( Result == FPropertyAccess::Success )
		{
			TSharedPtr<IPropertyHandle> BrushHandle = ResourceObjectProperty->GetParentHandle();

			TArray<void*> RawBrushData;
			BrushHandle->AccessRawData(RawBrushData);
			for (int32 BrushIndex = 0; BrushIndex < RawBrushData.Num(); BrushIndex++)
			{
				auto TemporaryBrush = static_cast<FLexUIImageBrush*>(RawBrushData[BrushIndex]);
				if (TemporaryBrush)
				{
				}
			}

			using ImageSizeType = decltype(FLexUIImageBrush::ImageSize);
			ImageSizeType CachedTextureSize = ImageSizeType::ZeroVector;

			TArray<void*> ImageSizeRawData;
			ImageSizeProperty->AccessRawData(ImageSizeRawData);
			if ( ImageSizeRawData.Num() > 0 && ImageSizeRawData[0] != nullptr )
			{
				CachedTextureSize = *static_cast<ImageSizeType*>( ImageSizeRawData[0] );
			}

			using UVRegionType = decltype(FLexUIImageBrush::UVRegion);
			UVRegionType CachedUVRegion;
			TArray<void*> UVRegionRawData;
			UVRegionProperty->AccessRawData(UVRegionRawData);
			if (UVRegionRawData.Num() > 0 && UVRegionRawData[0] != nullptr)
			{
				CachedUVRegion = *static_cast<UVRegionType*>(UVRegionRawData[0]);
			}

			if (auto BrushTexture = Cast<UTexture2D>(ResourceObject) )
			{
				if ( BrushTexture->IsDefaultTexture() )
				{
					UTexture* const BaseTexture = BrushTexture;
					// GetSizeX/Y will return the incorrect value if this texture is being compiled so we need to wait for it here
					FTextureCompilingManager::Get().FinishCompilation( MakeArrayView(&BaseTexture, 1) );
				}
				CachedTextureSize = ImageSizeType(BrushTexture->GetSizeX(), BrushTexture->GetSizeY());
				CachedUVRegion = UVRegionType(0, 0, 1, 1);
			}
			else if (auto AtlasedTextureObject = Cast<ISlateTextureAtlasInterface>(ResourceObject) )
			{
				auto AtlasData = AtlasedTextureObject->GetSlateAtlasData();
				CachedTextureSize = ImageSizeType(AtlasData.GetSourceDimensions());
				CachedUVRegion = FVector4f(AtlasData.StartUV.X, AtlasData.StartUV.Y
					, AtlasData.StartUV.X + AtlasData.SizeUV.X, AtlasData.StartUV.Y + AtlasData.SizeUV.Y);
			}
			else if (auto LexSprite = Cast<ULexUISpriteData_BaseObject>(ResourceObject))
			{
				CachedTextureSize.X = LexSprite->GetSpriteInfo().GetSourceWidth();
				CachedTextureSize.Y = LexSprite->GetSpriteInfo().GetSourceHeight();
				CachedUVRegion = UVRegionType(0, 0, 1, 1);
			}
			else
			{
				CachedUVRegion = UVRegionType(0, 0, 1, 1);
			}

			// Update the image size to match that of the incoming new texture.
			// TODO: Should we always do this?  Or should we avoid doing it if there's already some 'set value'
			// problem is we don't have a way to track that right now.
			ImageSizeProperty->SetValueFromFormattedString(FString::Format(TEXT("(X={0},Y={1})"), {CachedTextureSize.X, CachedTextureSize.Y}));

			UVRegionProperty->SetValueFromFormattedString(FString::Format(TEXT("(X={0},Y={1},Z={2},W={3})")
				, {CachedUVRegion.X, CachedUVRegion.Y, CachedUVRegion.Z, CachedUVRegion.W}));

			// When you assign a resource object, if the current draw type is 'None' we go ahead and update it to 'Image'.
			// Also update ResourceName to be null (Object name will be used), & set ImageType
			if (ResourceObject)
			{
				TArray<FString> OutPerObjectValues;
				DrawAsProperty->GetPerObjectValues(OutPerObjectValues);

				TArray<FString> NewPerObjectValues;
				for (int32 ObjectIndex = 0; ObjectIndex < OutPerObjectValues.Num(); ObjectIndex++)
				{
					FString& ExistingValue = OutPerObjectValues[ObjectIndex];
					NewPerObjectValues.Add(ExistingValue == TEXT("None") ? TEXT("Image") : ExistingValue);
				}

				DrawAsProperty->SetPerObjectValues(NewPerObjectValues);
			}
		}
	}

	void UpdateResourceError()
	{
		UObject* Resource = nullptr;

		if( ResourceObjectProperty->GetValue(Resource) == FPropertyAccess::Success && Resource && Resource->IsA<UMaterialInterface>() )
		{
			UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>( Resource );
			UMaterial* BaseMaterial = MaterialInterface->GetBaseMaterial();
			if( BaseMaterial && !BaseMaterial->IsUIMaterial() )
			{
				ResourceError->SetVisibility( EVisibility::Visible );

				// Special engine materials cannot change domain. This typically occurs when
				// the user creates or assigns a material instance with no parent material.
				// In this case, we warn the user rather than offer to change the domain.
				if (BaseMaterial->bUsedAsSpecialEngineMaterial)
				{
					IsEngineMaterialError->SetVisibility( EVisibility::Visible );
				}
				else
				{
					IsEngineMaterialError->SetVisibility( EVisibility::Collapsed );
				}
			}
			else
			{
				ResourceError->SetVisibility( EVisibility::Collapsed );
			}
		}
		else if( ResourceError->GetVisibility() != EVisibility::Collapsed )
		{
			ResourceError->SetVisibility( EVisibility::Collapsed );
		}
	}

private:
	TSharedPtr<IPropertyHandle> ResourceObjectProperty;
	TSharedPtr<IPropertyHandle> ImageSizeProperty;
	TSharedPtr<IPropertyHandle> DrawAsProperty;
	TSharedPtr<IPropertyHandle> UVRegionProperty;
	TSharedPtr<SBrushResourceError> ResourceError;
	TSharedPtr<STextBlock> IsEngineMaterialError;
};


TSharedRef<IPropertyTypeCustomization> FLexImageBrushStructCustomization::MakeInstance()
{
	return MakeShareable(new FLexImageBrushStructCustomization);
}
void FLexImageBrushStructCustomization::CustomizeHeader( TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils )
{
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		StructPropertyHandle->CreatePropertyValueWidget()
	];
}
void FLexImageBrushStructCustomization::CustomizeChildren( TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils )
{
	ImageSizeProperty = StructPropertyHandle->GetChildHandle( TEXT("ImageSize") );
	DrawAsProperty = StructPropertyHandle->GetChildHandle( TEXT("DrawAs") );
	TSharedPtr<IPropertyHandle> MarginProperty = StructPropertyHandle->GetChildHandle( TEXT("Margin") );
	TSharedPtr<IPropertyHandle> UVRegionProperty = StructPropertyHandle->GetChildHandle( TEXT("UVRegion") );
	TSharedPtr<IPropertyHandle> TintProperty = StructPropertyHandle->GetChildHandle( TEXT("TintColor") );
	auto PixelsPerUnitMultiplierProperty = StructPropertyHandle->GetChildHandle( TEXT("PixelsPerUnitMultiplier") );
	auto FlipModeProperty = StructPropertyHandle->GetChildHandle( TEXT("FlipMode") );
	ResourceObjectProperty = StructPropertyHandle->GetChildHandle( TEXT("ResourceObject") );
	ImageTypeProperty = StructPropertyHandle->GetChildHandle(TEXT("ImageType"));

	ResourceObjectProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSPLambda(this, [&StructCustomizationUtils]
	{
		StructCustomizationUtils.GetPropertyUtilities()->RequestForceRefresh();
	}));
	DrawAsProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateSPLambda(this, [&StructCustomizationUtils]
	{
		StructCustomizationUtils.GetPropertyUtilities()->RequestForceRefresh();
	}));
	
	FDetailWidgetRow& ResourceObjectRow = StructBuilder.AddProperty(ResourceObjectProperty.ToSharedRef()).CustomWidget();

	SBrushResourceObjectBox::FPropertyParams Params;
	Params.ResourceObjectProperty = ResourceObjectProperty;
	Params.ImageSizeProperty = ImageSizeProperty;
	Params.DrawAsProperty = DrawAsProperty;
	Params.UVRegionProperty = UVRegionProperty;

	ResourceObjectRow
		.NameContent()
		[
			ResourceObjectProperty->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250.0f)
		.MaxDesiredWidth(0.0f)
		[
			SNew(SBrushResourceObjectBox, &StructCustomizationUtils, Params)
		];

	// Add the image size property with custom reset delegates that also affect the child properties (the components)
	const bool bOverrideDefaultOnVectorChildren = true;
	StructBuilder.AddProperty( ImageSizeProperty.ToSharedRef() )
	.OverrideResetToDefault(FResetToDefaultOverride::Create(
		FIsResetToDefaultVisible::CreateSP(this, &FLexImageBrushStructCustomization::IsImageSizeResetToDefaultVisible),
		FResetToDefaultHandler::CreateSP(this, &FLexImageBrushStructCustomization::OnImageSizeResetToDefault),
		bOverrideDefaultOnVectorChildren));

	StructBuilder.AddProperty( TintProperty.ToSharedRef() );
	StructBuilder.AddProperty( DrawAsProperty.ToSharedRef() );
	StructBuilder.AddProperty( MarginProperty.ToSharedRef() )
	.Visibility( TAttribute<EVisibility>::CreateSP(this, &FLexImageBrushStructCustomization::GetMarginPropertyVisibility ) );
	StructBuilder.AddProperty(UVRegionProperty.ToSharedRef())
	.Visibility( TAttribute<EVisibility>::CreateSP(this, &FLexImageBrushStructCustomization::GetUVRegionPropertyVisibility ) );
	StructBuilder.AddProperty(PixelsPerUnitMultiplierProperty.ToSharedRef())
	.Visibility(TAttribute<EVisibility>::CreateSP(this, &FLexImageBrushStructCustomization::GetPixelsPerUnitMultiplierPropertyVisibility));
	StructBuilder.AddProperty(FlipModeProperty.ToSharedRef());
}

EVisibility FLexImageBrushStructCustomization::GetMarginPropertyVisibility() const
{
	uint8 DrawAsType;
	FPropertyAccess::Result Result = DrawAsProperty->GetValue( DrawAsType );

	UObject* ResourceObject = nullptr;
	ResourceObjectProperty->GetValue(ResourceObject);
	
	if (Cast<ULexUISpriteData_BaseObject>(ResourceObject) != nullptr)
	{
		return EVisibility::Collapsed;
	}
	return (Result == FPropertyAccess::MultipleValues || DrawAsType == ESlateBrushDrawType::Box || DrawAsType == ESlateBrushDrawType::Border) ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility FLexImageBrushStructCustomization::GetUVRegionPropertyVisibility()const
{
	UObject* ResourceObject = nullptr;
	FPropertyAccess::Result Result = ResourceObjectProperty->GetValue(ResourceObject);

	if (Cast<ULexUISpriteData_BaseObject>(ResourceObject) != nullptr)
	{
		return EVisibility::Collapsed;
	}
	return (Result == FPropertyAccess::MultipleValues || Cast<ISlateTextureAtlasInterface>(ResourceObject) == nullptr) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility FLexImageBrushStructCustomization::GetPixelsPerUnitMultiplierPropertyVisibility() const
{
	uint8 DrawAsType;
	FPropertyAccess::Result Result = DrawAsProperty->GetValue( DrawAsType );
	
	return (Result == FPropertyAccess::MultipleValues || DrawAsType == ESlateBrushDrawType::Box || DrawAsType == ESlateBrushDrawType::Border) ? EVisibility::Visible : EVisibility::Collapsed;
}

bool FLexImageBrushStructCustomization::IsImageSizeResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	UObject* ResourceObject;
	if (FPropertyAccess::Success == ResourceObjectProperty->GetValue(ResourceObject) && ResourceObject)
	{
		// get texture size from ResourceObjectProperty and compare to image size prop value
		auto SizeDefault = GetDefaultImageSize();

		FVector2f Size = FVector2f::ZeroVector;
		TArray<void*> ImageSizeRawData;
		ImageSizeProperty->AccessRawData(ImageSizeRawData);
		if ( ImageSizeRawData.Num() > 0 && ImageSizeRawData[0] != nullptr )
		{
			Size = *static_cast<FVector2f*>( ImageSizeRawData[0] );
		}

		if (PropertyHandle->GetProperty() == ImageSizeProperty->GetProperty())
		{
			// reseting the whole vector
			return SizeDefault != Size;
		}
		else if (PropertyHandle->GetProperty() == ImageSizeProperty->GetChildHandle(0)->GetProperty())	// X
		{
			// reseting the vector.X
			return SizeDefault.X != Size.X;
		}
		else if (PropertyHandle->GetProperty() == ImageSizeProperty->GetChildHandle(1)->GetProperty())	// Y
		{
			// reseting the vector.Y
			return SizeDefault.Y != Size.Y;
		}

		ensureMsgf(false, TEXT("Property handle mismatch in brush size FVector2D struct"));
		return false;
	}

	// Fall back to default handler
	return PropertyHandle->DiffersFromDefault();
}

void FLexImageBrushStructCustomization::OnImageSizeResetToDefault(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	UObject* ResourceObject;
	if (FPropertyAccess::Success == ResourceObjectProperty->GetValue(ResourceObject) && ResourceObject)
	{
		// Set image size prop value to the texture size in ResourceObjectProperty
		auto SizeDefault = GetDefaultImageSize();

		if (PropertyHandle->GetProperty() == ImageSizeProperty->GetProperty())
		{
			// reseting the whole vector
			PropertyHandle->SetValueFromFormattedString(FString::Format(TEXT("(X={0},Y={1})"), {SizeDefault.X, SizeDefault.Y}));
		}
		else if (PropertyHandle->GetProperty() == ImageSizeProperty->GetChildHandle(0)->GetProperty())	// X
		{
			// reseting the vector.X
			PropertyHandle->SetValue(SizeDefault.X);
		}
		else if (PropertyHandle->GetProperty() == ImageSizeProperty->GetChildHandle(1)->GetProperty())	// Y
		{
			// reseting the vector.Y
			PropertyHandle->SetValue(SizeDefault.Y);
		}
		else
		{
			ensureMsgf(false, TEXT("Property handle mismatch in brush size FVector2D struct"));
		}
	}	
	else
	{
		// Fall back to default handler. 
		PropertyHandle->ResetToDefault();
	}
}

FVector2f FLexImageBrushStructCustomization::GetDefaultImageSize() const
{
	// Custom default behavior using the texture's size, if one is set as the resource object
	UObject* ResourceObject;
	if (FPropertyAccess::Success == ResourceObjectProperty->GetValue(ResourceObject))
	{
		if (auto Texture = Cast<UTexture2D>(ResourceObject) )
		{
			if ( Texture->IsDefaultTexture() )
			{
				// GetSizeX/Y will return the incorrect value if this texture is being compiled so we need to wait for it here
				UTexture* const BaseTexture = Texture;
				FTextureCompilingManager::Get().FinishCompilation(MakeArrayView(&BaseTexture, 1));
			}
			return FVector2f(Texture->GetSizeX(), Texture->GetSizeY());
		}
		else if (auto AtlasTextureObject = Cast<ISlateTextureAtlasInterface>(ResourceObject) )
		{
			return FVector2f(AtlasTextureObject->GetSlateAtlasData().GetSourceDimensions());
		}
		else if (auto LexSprite = Cast<ULexUISpriteData_BaseObject>(ResourceObject))
		{
			return FVector2f(LexSprite->GetSpriteInfo().GetSourceWidth(), LexSprite->GetSpriteInfo().GetSourceHeight());
		}
	}

	// Fall back on the standard default size for brush images
	return FVector2f(SlateBrushDefs::DefaultImageSize, SlateBrushDefs::DefaultImageSize);
}

#undef LOCTEXT_NAMESPACE
