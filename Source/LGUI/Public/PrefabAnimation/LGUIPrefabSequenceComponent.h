// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Core/LGUILifeCycleBehaviour.h"
#include "MovieSceneSequencePlayer.h"
#include "LGUIPrefabSequenceComponent.generated.h"


class ULGUIPrefabSequence;
class ULGUIPrefabSequencePlayer;

/**
 * Only use this as blueprint, because MovieScene need blueprint to handle event callback.
 * Not working yet.
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUIPrefabSequenceBlueprint : public UObject
{
	GENERATED_BODY()
public:
	/** Called when LGUIPrefabSequenceComponent process Start */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"), Category = "LGUI")
		void ReceiveInit(class ULGUIPrefabSequenceComponent* InComp);
};

/**
 * Movie scene animation embedded within LGUIPrefab.
 */
UCLASS(Blueprintable, ClassGroup=LGUI, hidecategories=(Collision, Cooking, Activation), meta=(BlueprintSpawnableComponent))
class LGUI_API ULGUIPrefabSequenceComponent
	: public ULGUILifeCycleBehaviour
	, public IMovieSceneSequenceActor
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
	
	virtual void Awake()override;
	virtual void Start() override;
	virtual void OnDestroy() override;
	virtual void Update(float DeltaTime) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams)override;
	virtual void PreSave(class FObjectPreSaveContext SaveContext)override;
	virtual void PostDuplicate(bool bDuplicateForPIE)override;
	virtual void PostInitProperties()override;
	virtual void PostLoad()override;

	void FixEditorHelpers();
	UBlueprint* GetSequenceBlueprint()const;
	UObject* GetSequenceBlueprintInstance()const { return SequenceBlueprint; }
#endif
	// IMovieSceneSequenceActor interface
	virtual void TickFromSequenceTickManager(float DeltaSeconds) override;
protected:

	UPROPERTY(EditAnywhere, Category="Playback", meta=(ShowOnlyInnerProperties))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(VisibleAnywhere, Instanced, Category= Playback)
		TArray<TObjectPtr<ULGUIPrefabSequence>> SequenceArray;
	UPROPERTY(EditAnywhere, Category = Playback)
		int32 CurrentSequenceIndex = 0;
	/**
	 * Create a blueprint that use LGUIPrefabSequenceBlueprint as parent class and use it here.
	 * Not working yet.
	 */
	UPROPERTY(/*EditAnywhere, Category = Playback, Instanced*/)
		TObjectPtr<ULGUIPrefabSequenceBlueprint> SequenceBlueprint = nullptr;

	UPROPERTY(transient)
		TObjectPtr<ULGUIPrefabSequencePlayer> SequencePlayer;
};
