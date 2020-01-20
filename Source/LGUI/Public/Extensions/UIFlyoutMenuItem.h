// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Event/LGUIPointerClickInterface.h"
#include "UIFlyoutMenuItem.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIFlyoutMenuItem : public UActorComponent, public ILGUIPointerClickInterface
{
	GENERATED_BODY()

public:	
	UUIFlyoutMenuItem();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUIBaseActor* _RootUIActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUITextActor* _TextActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUISpriteActor* _HighlightSpriteActor;
	class UUIFlyoutMenu* _Manager;
	int32 _Index;
public:
	void Init(class UUIFlyoutMenu* InManager, const int32 InItemIndex, const FString& InItemName);
	void SetSelectionState(const bool& InSelect);
	virtual bool OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)override;
};
