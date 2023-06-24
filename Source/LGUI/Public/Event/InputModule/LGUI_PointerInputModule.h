// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "Event/LGUIPointerEventData.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "LGUI_PointerInputModule.generated.h"

class ULGUIBaseRaycaster;

UCLASS(Abstract)
class LGUI_API ULGUI_PointerInputModule : public ULGUIBaseInputModule
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)TObjectPtr<class ULGUIEventSystem> eventSystem = nullptr;
	bool CheckEventSystem();
	struct FHitResultContainerStruct
	{
		FHitResult hitResult;
		ELGUIEventFireType eventFireType = ELGUIEventFireType::TargetActorAndAllItsComponents;

		FVector rayOrigin = FVector(0, 0, 0), rayDirection = FVector(1, 0, 0), rayEnd = FVector(1, 0, 0);

		ULGUIBaseRaycaster* raycaster = nullptr;

		TArray<USceneComponent*> hoverArray;
	};

	void ProcessPointerEvent(ULGUIPointerEventData* pointerEventData, bool pointerHitAnything, const FHitResultContainerStruct& hitResult, bool& outIsHitSomething, FHitResult& outHitResult);
	bool LineTrace(ULGUIPointerEventData* InPointerEventData, FHitResultContainerStruct& hitResult);
	TArray<FHitResultContainerStruct> multiHitResult;//temp array for hit result
	void ProcessPointerEnterExit(ULGUIPointerEventData* pointerEventData, USceneComponent* oldObj, USceneComponent* newObj, ELGUIEventFireType enterFireType);
	/** find a commont root actor of two actors. return nullptr if no common root */
	AActor* FindCommonRoot(AActor* actorA, AActor* actorB);

	bool Navigate(ELGUINavigationDirection direction, ULGUIPointerEventData* InPointerEventData, FHitResultContainerStruct& hitResult);
	void ProcessInputForNavigation();
	void ClearEventByID(int pointerID);
	bool CanHandleInterface(USceneComponent* targetComp, UClass* targetInterfaceClass, ELGUIEventFireType eventFireType);
	USceneComponent* GetEventHandle(USceneComponent* targetComp, UClass* targetInterfaceClass, ELGUIEventFireType eventFireType);
	void DeselectIfSelectionChanged(USceneComponent* currentPressed, ULGUIBaseEventData* eventData);
public:
	virtual void ClearEvent()override;

	/** input for gamepad or keyboard navigation */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigation(ELGUINavigationDirection direction, bool pressOrRelease, int pointerID);
	/** input for gamepad or keyboard press and release */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputTriggerForNavigation(bool triggerPress, int pointerID);
};