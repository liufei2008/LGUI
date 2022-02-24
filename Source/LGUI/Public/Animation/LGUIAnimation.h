// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "MovieSceneSequence.h"
#include "Animation/LGUIAnimationBinding.h"
#include "MovieScene.h"
#include "LGUIAnimation.generated.h"

class UMovieScene;
class UUserWidget;


UCLASS(DefaultToInstanced)
class LGUI_API ULGUIMovieScene : public UMovieScene
{
	GENERATED_UCLASS_BODY()
public:

	/** call by ActorSerializtion, after load all property */
	void PostLoadMovieProperties();
};


/**
 *  Todo: Runtime play LGUIAnimaton, not finish!!
 */
UCLASS(BlueprintType)
class LGUI_API ULGUIAnimation : public UMovieSceneSequence
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
	/**
	 * Get a placeholder animation.
	 *
	 * @return Placeholder animation.
	 */
	static ULGUIAnimation* GetNullAnimation();

	/** @return The friendly name of the animation */
	 const FString& GetDisplayLabel() const
	{
		return DisplayLabel;
	}

	/** Sets the friendly name of the animation to display in the editor */
	 void SetDisplayLabel(const FString& InDisplayLabel);

	/** Returns the DisplayLabel if set, otherwise the object name */
	 virtual FText GetDisplayName() const override;
#endif

	/**
	 * Get the start time of this animation.
	 *
	 * @return Start time in seconds.
	 * @see GetEndTime
	 */
	UFUNCTION(BlueprintCallable, Category="Animation")
	 float GetStartTime() const;

	/**
	 * Get the end time of this animation.
	 *
	 * @return End time in seconds.
	 * @see GetStartTime
	 */
	UFUNCTION(BlueprintCallable, Category="Animation")
	 float GetEndTime() const;

	// zachma todo: AnimationPlay Event
	///** Fires when the widget animation starts playing. */
	//UPROPERTY(BlueprintAssignable, Category = "Animation")
	//	FOnWidgetAnimationPlaybackStatusChanged OnAnimationStarted;

	///** Fires when the widget animation is finished. */
	//UPROPERTY(BlueprintAssignable, Category = "Animation")
	//	FOnWidgetAnimationPlaybackStatusChanged OnAnimationFinished;

	//UFUNCTION(BlueprintCallable, Category = Animation, meta=(BlueprintInternalUseOnly = "TRUE"))
	//void BindToAnimationStarted(UUserWidget* Widget, FWidgetAnimationDynamicEvent Delegate);

	//UFUNCTION(BlueprintCallable, Category = Animation, meta = (BlueprintInternalUseOnly = "TRUE"))
	//void UnbindFromAnimationStarted(UUserWidget* Widget, FWidgetAnimationDynamicEvent Delegate);

	//UFUNCTION(BlueprintCallable, Category = Animation, meta = (BlueprintInternalUseOnly = "TRUE"))
	//void UnbindAllFromAnimationStarted(UUserWidget* Widget);

	//UFUNCTION(BlueprintCallable, Category = Animation, meta = (BlueprintInternalUseOnly = "TRUE"))
	//void BindToAnimationFinished(UUserWidget* Widget, FWidgetAnimationDynamicEvent Delegate);

	//UFUNCTION(BlueprintCallable, Category = Animation, meta = (BlueprintInternalUseOnly = "TRUE"))
	//void UnbindFromAnimationFinished(UUserWidget* Widget, FWidgetAnimationDynamicEvent Delegate);

	//UFUNCTION(BlueprintCallable, Category = Animation, meta = (BlueprintInternalUseOnly = "TRUE"))
	//void UnbindAllFromAnimationFinished(UUserWidget* Widget);

public:

	// UMovieSceneAnimation overrides
	virtual void BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context) override;
	virtual bool CanPossessObject(UObject& Object, UObject* InPlaybackContext) const override;
	virtual UMovieScene* GetMovieScene() const override;
	virtual UObject* GetParentObject(UObject* Object) const override;
	virtual void UnbindPossessableObjects(const FGuid& ActorGuid) override;
	virtual void LocateBoundObjects(const FGuid& ActorGuid, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const override;
	virtual UObject* CreateDirectorInstance(IMovieScenePlayer& Player) override;
#if WITH_EDITOR
	virtual ETrackSupport IsTrackSupported(TSubclassOf<class UMovieSceneTrack> InTrackClass) const override;
#endif
	// ~UMovieSceneAnimation overrides

	//~ Begin UObject Interface. 
	virtual bool IsPostLoadThreadSafe() const override;
	//~ End UObject Interface

	/** Get Animation bindings of the animation */
	const TArray<FLGUIAnimationBinding>& GetBindings() const { return AnimationBindings; }

	/** Remove Animation Binding */
	 void RemoveBinding(const UObject& PossessedObject);
	 void RemoveBinding(const FLGUIAnimationBinding& Binding);

public:

	/** Pointer to the movie scene that controls this animation. */
	UPROPERTY(Instanced)
	UMovieScene* MovieScene;

	/** 动画引用的Actor的绑定信息, GUID->ObjectName之间的映射 */
	UPROPERTY()
	TArray<FLGUIAnimationBinding> AnimationBindings;

private:

	/** The friendly name for this animation displayed in the designer. */
	UPROPERTY()
	FString DisplayLabel;
};
