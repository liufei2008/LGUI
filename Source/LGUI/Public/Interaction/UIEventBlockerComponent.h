// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Event/LGUIPointerEnterExitInterface.h"
#include "Event/LGUIPointerDownUpInterface.h"
#include "Event/LGUIPointerClickInterface.h"
#include "Event/LGUIPointerDragInterface.h"
#include "Event/LGUIPointerDragEnterExitInterface.h"
#include "Event/LGUIPointerDragDropInterface.h"
#include "Event/LGUIPointerScrollInterface.h"
#include "Event/LGUIPointerSelectDeselectInterface.h"

#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Components/ActorComponent.h"
#include "UIEventBlockerComponent.generated.h"

//use this component to stop LGUIPointerEvent bubble up
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEventBlockerComponent : public UActorComponent
	, public ILGUIPointerEnterExitInterface
	, public ILGUIPointerDownUpInterface
	, public ILGUIPointerClickInterface
	, public ILGUIPointerDragInterface
	, public ILGUIPointerDragEnterExitInterface
	, public ILGUIPointerDragDropInterface
	, public ILGUIPointerScrollInterface
	, public ILGUIPointerSelectDeselectInterface
{
	GENERATED_BODY()

public:
	UUIEventBlockerComponent();
protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	UPROPERTY(EditAnywhere, Category = "UIEventBlocker") bool AllowEventBubbleUp = false;
	virtual bool OnPointerEnter_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerExit_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDragEnter_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDragExit_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDragDrop_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerScroll_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData)override;
};
