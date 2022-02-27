// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "MovieSceneSequencePlayer.h"
#include "LGUIPrefabSequenceComponent.generated.h"


class ULGUIPrefabSequence;
class ULGUIPrefabSequencePlayer;


/**
 * Movie scene animation embedded within an actor.
 */
UCLASS(Blueprintable, ClassGroup=Sequence, hidecategories=(Collision, Cooking, Activation), meta=(BlueprintSpawnableComponent))
class LGUI_API ULGUIPrefabSequenceComponent
	: public UActorComponent
{
public:
	GENERATED_BODY()

	ULGUIPrefabSequenceComponent(const FObjectInitializer& Init);

	ULGUIPrefabSequence* GetCurrentSequence() const
	{
		return GetSequenceByIndex(CurrentSequenceIndex);
	}

	ULGUIPrefabSequencePlayer* GetSequencePlayer() const 
	{
		return SequencePlayer;
	}

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPrefabSequence* GetSequenceByName(FName InName) const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPrefabSequence* GetSequenceByIndex(int32 InIndex) const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		const TArray<ULGUIPrefabSequence*>& GetSequenceArray() const { return SequenceArray; }
	void AddNewAnimation();
	void DeleteAnimationByIndex(int32 InIndex);
	void DuplicateAnimationByIndex(int32 InIndex);
	
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

protected:

	UPROPERTY(EditAnywhere, Category="Playback", meta=(ShowOnlyInnerProperties))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(EditAnywhere, Instanced, Category= LGUI)
		TArray<ULGUIPrefabSequence*> SequenceArray;
	UPROPERTY(EditAnywhere, Category = LGUI)
		int32 CurrentSequenceIndex = 0;

	UPROPERTY(transient, BlueprintReadOnly, Category= LGUI)
		ULGUIPrefabSequencePlayer* SequencePlayer;
};
