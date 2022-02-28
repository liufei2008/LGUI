// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/LGUILifeCycleBehaviour.h"
#include "MovieSceneSequencePlayer.h"
#include "LGUIPrefabSequenceComponent.generated.h"


class ULGUIPrefabSequence;
class ULGUIPrefabSequencePlayer;

/**
 * Movie scene animation embedded within LGUIPrefab.
 */
UCLASS(Blueprintable, ClassGroup=LGUI, hidecategories=(Collision, Cooking, Activation), meta=(BlueprintSpawnableComponent))
class LGUI_API ULGUIPrefabSequenceComponent
	: public ULGUILifeCycleBehaviour
{
public:
	GENERATED_BODY()

	ULGUIPrefabSequenceComponent();

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
	/** Init SequencePlayer and play animation with CurrentSequenceIndex */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void Play();
	/** Find animation in SequenceArray with InIndex, then init SequencePlayer and play animation */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void PlaySequenceByIndex(int32 InIndex);
	/** Find animation in SequenceArray with InName, then init SequencePlayer and play animation */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void PlaySequenceByName(FName InName);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetCurrentSequenceIndex()const { return CurrentSequenceIndex; }
	/** Set CurrentSequenceIndex, then call Play to play animation with the index */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetCurrentSequenceIndex(int32 InIndex);

	ULGUIPrefabSequence* AddNewAnimation();
	bool DeleteAnimationByIndex(int32 InIndex);
	ULGUIPrefabSequence* DuplicateAnimationByIndex(int32 InIndex);
	
	virtual void Awake()override;
	virtual void Start() override;
	virtual void OnDestroy() override;
	virtual void Update(float DeltaTime) override;

protected:

	UPROPERTY(EditAnywhere, Category="Playback", meta=(ShowOnlyInnerProperties))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(VisibleAnywhere, Instanced, Category= Playback)
		TArray<ULGUIPrefabSequence*> SequenceArray;
	UPROPERTY(EditAnywhere, Category = Playback)
		int32 CurrentSequenceIndex = 0;

	UPROPERTY(transient, BlueprintReadOnly, Category= Playback)
		ULGUIPrefabSequencePlayer* SequencePlayer;
};
