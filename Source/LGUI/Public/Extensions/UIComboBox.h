// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Event/LGUIPointerSelectDeselectInterface.h"
#include "UIComboBox.generated.h"

/*
@param InSelectIndex Selected item index
@param InSelectItem Selected item string
*/
DECLARE_DYNAMIC_DELEGATE_TwoParams(FUIComboBoxSelectDynamicDelegate, int32, InSelectIndex, FString, InSelectItem);
DECLARE_DELEGATE_TwoParams(FUIComboBoxSelectDelegate, const int32&, const FString&);

UENUM(BlueprintType)
enum class EComboBoxPosition : uint8
{
	Top,Middle,Bottom,
};

UCLASS( ClassGroup=(LGUI), Blueprintable, meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIComboBox : public UActorComponent, public ILGUIPointerSelectDeselectInterface
{
	GENERATED_BODY()

public:	
	UUIComboBox();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUIPanelActor* _RootUIActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUIBaseActor* _SrcItemActor;
	TArray<class UUIComboBoxItem*> _CreatedItemArray;
	FUIComboBoxSelectDelegate _SelectionChangeCallback;
	void CreateFromArray_Internal(const TArray<FString>& InItemNameArray, const int32& InSelectedItemIndex, const FUIComboBoxSelectDelegate& InCallback);
public:
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "InSelectedItemIndex,InPosition"), Category = LGUI)
		static void CreateComboBoxFromArray(const TArray<FString>& InItemNameArray, const FUIComboBoxSelectDynamicDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InSelectedItemIndex = -1, EComboBoxPosition InPosition = EComboBoxPosition::Top);
	static void CreateComboBoxFromArray(const TArray<FString>& InItemNameArray, const FUIComboBoxSelectDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InSelectedItemIndex = -1, EComboBoxPosition InPosition = EComboBoxPosition::Top);
	void OnClickItem(const int32& InItemIndex, const FString& InItemName);

	virtual bool OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData);
	virtual bool OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData);
};
