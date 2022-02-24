// Copyright 2019-2021 Tencent All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "UIAnimationComp.generated.h"

class ULGUIAnimationPlayer;
class ULGUIAnimation;

/**
 * 挂在UIRootActor上面
 * 管理若干个LGUIAnimation对象
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIAnimationComp : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UUIAnimationComp();
	
	// ~ ActorInterface Begin
	virtual void BeginPlay() override;
	// ~ ActorInterface End

	/** Called when a sequence player is finished playing an animation */
	virtual void OnAnimationStartedPlaying(ULGUIAnimationPlayer& Player);

	/** Called when a sequence player is finished playing an animation */
	virtual void OnAnimationFinishedPlaying(ULGUIAnimationPlayer& Player);

	UPROPERTY(EditAnywhere)
	int32 TestValue = 3;
	
	// UI动画资源
	UPROPERTY(VisibleAnywhere, Instanced)
	TArray<ULGUIAnimation*> Animations;
	
};
