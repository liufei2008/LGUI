// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "PrefabSystem/ILGUIPrefabInterface.h"
#include "MovieSceneSequencePlayer.h"
#include "LGUIComponentReference.h"
#include "LGUIPrefabSequenceComponent.generated.h"


class ULGUIPrefabSequence;
class ULGUIPrefabSequencePlayer;

/**
 * Movie scene animation embedded within LGUIPrefab.
 */
UCLASS(Blueprintable, ClassGroup=LGUI, hidecategories=(Collision, Cooking, Activation), meta=(BlueprintSpawnableComponent))
class LGUI_API ULGUIPrefabSequenceComponent
	: public UActorComponent, public ILGUIPrefabInterface
{
public:
	GENERATED_BODY()

	ULGUIPrefabSequenceComponent();

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPrefabSequence* GetSequenceByName(FName InName) const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPrefabSequence* GetSequenceByIndex(int32 InIndex) const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		const TArray<ULGUIPrefabSequence*>& GetSequenceArray() const { return SequenceArray; }
	/** Init SequencePlayer with current sequence. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InitSequencePlayer();
	/** Find animation in SequenceArray by Index, then set it to SequencePlayer. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSequenceByIndex(int32 InIndex);
	/** Find animation in SequenceArray by Name, then set it to SequencePlayer */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSequenceByName(FName InName);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetCurrentSequenceIndex()const { return CurrentSequenceIndex; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPrefabSequence* GetCurrentSequence() const { return GetSequenceByIndex(CurrentSequenceIndex); }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPrefabSequencePlayer* GetSequencePlayer() const { return SequencePlayer; }

	ULGUIPrefabSequence* AddNewAnimation();
	bool DeleteAnimationByIndex(int32 InIndex);
	ULGUIPrefabSequence* DuplicateAnimationByIndex(int32 InIndex);
	
	virtual void BeginPlay()override;
	// Begin ILGUIPrefabInterface
	virtual void Awake_Implementation()override;
	// End ILGUIPrefabInterface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams)override;
	virtual void PreSave(class FObjectPreSaveContext SaveContext)override;
	virtual void PostDuplicate(bool bDuplicateForPIE)override;
	virtual void PostInitProperties()override;
	virtual void PostLoad()override;

	void FixEditorHelpers();
	UBlueprint* GetSequenceBlueprint()const;
#endif
	UObject* GetSequenceBlueprintInstance()const { return SequenceEventHandler.GetComponent(); }
protected:

	UPROPERTY(EditAnywhere, Category="Playback", meta=(ShowOnlyInnerProperties))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(VisibleAnywhere, Instanced, Category= Playback)
		TArray<TObjectPtr<ULGUIPrefabSequence>> SequenceArray;
	UPROPERTY(EditAnywhere, Category = Playback)
		int32 CurrentSequenceIndex = 0;
	/**
	 * Use a Blueprint component to handle callback for event track.
	 * Not working: Add event in prefab the event can work no problem, but if close editor and open again, the event not fire at all.
	 */
	UPROPERTY(/*EditAnywhere, Category = Playback*/)
		FLGUIComponentReference SequenceEventHandler;

	UPROPERTY(transient)
		TObjectPtr<ULGUIPrefabSequencePlayer> SequencePlayer;
};
