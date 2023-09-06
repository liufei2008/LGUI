// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "Event/LGUIPointerEventData.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "LGUI_PointerInputModule.generated.h"

class ULGUIBaseRaycaster;
class ULGUIEventSystem;

UCLASS(Abstract)
class LGUI_API ULGUI_PointerInputModule : public ULGUIBaseInputModule
{
	GENERATED_BODY()

public:
	static void ProcessPointerEvent(ULGUIEventSystem* eventSystem, ULGUIPointerEventData* pointerEventData, bool pointerHitAnything, const FLGUIHitResult& hitResult, bool& outIsHitSomething, FHitResult& outHitResult);
protected:
	UPROPERTY(Transient)TObjectPtr<ULGUIEventSystem> eventSystem = nullptr;
	bool CheckEventSystem();

	bool LineTrace(ULGUIPointerEventData* InPointerEventData, FLGUIHitResult& hitResult);
	TArray<FLGUIHitResult> multiHitResult;//temp array for hit result
	static void ProcessPointerEnterExit(ULGUIEventSystem* eventSystem, ULGUIPointerEventData* pointerEventData, USceneComponent* oldObj, USceneComponent* newObj, ELGUIEventFireType enterFireType);
	/** find a commont root actor of two actors. return nullptr if no common root */
	static AActor* FindCommonRoot(AActor* actorA, AActor* actorB);

	bool Navigate(ELGUINavigationDirection direction, ULGUIPointerEventData* InPointerEventData, FLGUIHitResult& hitResult);
	void ProcessInputForNavigation();
	void ProcessInputForNavigation(ULGUIPointerEventData* InPointerEventData);
	void ClearEventByID(int pointerID);
	static bool CanHandleInterface(USceneComponent* targetComp, UClass* targetInterfaceClass, ELGUIEventFireType eventFireType);
	static USceneComponent* GetEventHandle(USceneComponent* targetComp, UClass* targetInterfaceClass, ELGUIEventFireType eventFireType);
	static void DeselectIfSelectionChanged(ULGUIEventSystem* eventSystem, USceneComponent* currentPressed, ULGUIBaseEventData* eventData);
public:
	virtual void ClearEvent()override;

	/** input for gamepad or keyboard navigation */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigation(ELGUINavigationDirection direction, bool pressOrRelease, int pointerID);
	/** input for gamepad or keyboard press and release */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputTriggerForNavigation(bool triggerPress, int pointerID);
};