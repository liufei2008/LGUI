// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIPanel.h"
#include "UIPanelActor.generated.h"

UCLASS(ShowCategories(Rendering))
class LGUI_API AUIPanelActor : public AUIBaseActor
{
	GENERATED_BODY()
	
public:	
	AUIPanelActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIPanel; }
	FORCEINLINE UUIPanel* GetUIPanel()const { return UIPanel; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUIPanel* UIPanel;
	
};
