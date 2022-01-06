// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LGUIBaseEventData.generated.h"

/** event execute type */
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIEventFireType :uint8
{
	/** event will call on trace target actor and all component of the actor */
	TargetActorAndAllItsComponents,
	/** event will call only on trace target */
	OnlyTargetComponent,
	/** event will call only on trace target actor */
	OnlyTargetActor,
};

UENUM(BlueprintType, Category = LGUI)
enum class EPointerEventType :uint8
{
	Click = 0,
	Enter = 1,
	Exit = 2,
	Down = 3,
	Up = 4,
	BeginDrag = 5,
	Drag = 6,
	EndDrag = 7,
	Scroll = 8,
	DragDrop = 11,
	Select = 12,
	Deselect = 13,
	Navigate = 14,
};
UENUM(BlueprintType, Category = LGUI)
enum class EMouseButtonType :uint8
{
	Left,Middle,Right,
	/** UserDefinedX is for custom defined input buttun type */
	UserDefined1,
	UserDefined2,
	UserDefined3,
	UserDefined4,
	UserDefined5,
	UserDefined6,
	UserDefined7,
	UserDefined8,
};
UCLASS(BlueprintType, classGroup = LGUI)
class LGUI_API ULGUIBaseEventData :public UObject
{
	GENERATED_BODY()
public:
	/** current selected component. when call Deselect interface, this is also the new selected component*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		USceneComponent* selectedComponent = nullptr;
	/** event type*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		EPointerEventType eventType = EPointerEventType::Click;

	ELGUIEventFireType selectedComponentEventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (LGUIEventData)", CompactNodeTitle = ".", BlueprintAutocast), Category = "LGUI") virtual FString ToString()const 
	{
		return TEXT("");
	};
};
