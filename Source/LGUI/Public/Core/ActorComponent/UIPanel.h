// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "Layout/Margin.h"
#include "UIPanel.generated.h"

UENUM(BlueprintType)
enum class UIPanelClipType :uint8
{
	None		 		UMETA(DisplayName = "None"),
	Rect		 		UMETA(DisplayName = "Rect"),
	Texture				UMETA(DisplayName = "Texture"),
};

UENUM(BlueprintType, meta = (Bitflags))
enum class UIPanelAdditionalChannelType :uint8
{
	//Lit shader may need this
	Normal,
	//Lit and normalMap may need this
	Tangent,
	//Additional textureCoordinate at uv1(first uv is uv0, which is used by LGUI. Second uv is uv1)
	UV1,
	UV2,
	UV3,
};
ENUM_CLASS_FLAGS(UIPanelAdditionalChannelType)

class UUIRenderable;

//this class is only for old version property holder. please replace this component with UIItem and LGUICanvas
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUIPanel : public UUIItem
{
	GENERATED_BODY()

public:	
	UUIPanel(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

	UPROPERTY(EditAnywhere, Category = LGUI)
		UIPanelClipType clipType = UIPanelClipType::None;
	UPROPERTY(EditAnywhere, Category = LGUI)
		FVector2D clipFeather = FVector2D(4, 4);
	UPROPERTY(EditAnywhere, Category = LGUI)
		FMargin clipRectOffset = FMargin(0);
	UPROPERTY(EditAnywhere, Category = LGUI)
		UTexture* clipTexture = nullptr;
	//if inherit parent's rect clip value. only valid if self is RectClip
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool inheritRectClip = true;

	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bOwnerNoSee = false;
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bOnlyOwnerSee = false;

	//The amount of pixels per unit to use for dynamically created bitmaps in the UI, such as UIText. 
	//But!!! Do not set this value too large if you already have large font size of UIText, because that will result in extreamly large texture! 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float dynamicPixelsPerUnit = 1.0f;

	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "UIPanelAdditionalChannelType"))
		int8 additionalShaderChannels = 0;
};
