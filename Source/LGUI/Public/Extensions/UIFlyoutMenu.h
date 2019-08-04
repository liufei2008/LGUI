// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Event/LGUIPointerSelectDeselectInterface.h"
#include "UIFlyoutMenu.generated.h"

/*
@param InSelectIndex Selected item index
@param InSelectItem Selected item string
*/
DECLARE_DYNAMIC_DELEGATE_TwoParams(FUIFlyoutMenuSelectDynamicDelegate, int32, InSelectIndex, FString, InSelectItem);
DECLARE_DELEGATE_TwoParams(FUIFlyoutMenuSelectDelegate, const int32&, const FString&);

UENUM(BlueprintType)
enum class EFlyoutMenuVerticalPosition : uint8
{
	Top,Bottom,
};
UENUM(BlueprintType)
enum class EFlyoutMenuHorizontalAlignment : uint8
{
	Left, Center, Right
};

UCLASS( ClassGroup=(LGUI), Blueprintable, meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIFlyoutMenu : public UActorComponent, public ILGUIPointerSelectDeselectInterface
{
	GENERATED_BODY()

public:	
	UUIFlyoutMenu();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUIPanelActor* _RootUIActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUIBaseActor* _SrcItemActor;
	TArray<class UUIFlyoutMenuItem*> _CreatedItemArray;
	FUIFlyoutMenuSelectDelegate _SelectionChangeCallback;
	void CreateFromArray_Internal(const TArray<FString>& InItemNameArray, const FUIFlyoutMenuSelectDelegate& InCallback);
public:
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "InWidth,InVerticalPosition,InHorizontalAlign"), Category = LGUI)
		static void CreateFlyoutMenuFromArray(const TArray<FString>& InItemNameArray, const FUIFlyoutMenuSelectDynamicDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InWidth = 200, EFlyoutMenuVerticalPosition InVerticalPosition = EFlyoutMenuVerticalPosition::Top, EFlyoutMenuHorizontalAlignment InHorizontalAlign = EFlyoutMenuHorizontalAlignment::Center);
	static void CreateFlyoutMenuFromArray(const TArray<FString>& InItemNameArray, const FUIFlyoutMenuSelectDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InWidth = 200, EFlyoutMenuVerticalPosition InVerticalPosition = EFlyoutMenuVerticalPosition::Top, EFlyoutMenuHorizontalAlignment InHorizontalAlign = EFlyoutMenuHorizontalAlignment::Center);
	void OnClickItem(const int32& InItemIndex, const FString& InItemName);

	virtual bool OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData);
	virtual bool OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData);
};
