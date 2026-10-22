// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "MovieSceneSequencePlayer.h"
#include "Core/LexUIBehaviour.h"
#include "LexUIPrefabSequenceComponent.generated.h"


class ULexUIPrefabSequence;
class ULexUIPrefabSequencePlayer;

/**
 * Movie scene animation embedded within LexUIPrefab.
 */
UCLASS(Blueprintable, ClassGroup=LGUI, meta=(BlueprintSpawnableComponent), DisplayName="LexUI Prefab Sequence Component")
class LGUI_API ULexUIPrefabSequenceComponent
	: public ULexUIBehaviour
{
public:
	GENERATED_BODY()

	ULexUIPrefabSequenceComponent();

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ULexUIPrefabSequence* GetSequenceByDisplayName(const FString& InName) const;
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ULexUIPrefabSequence* GetSequenceByIndex(int32 InIndex) const;
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		const TArray<ULexUIPrefabSequence*>& GetSequenceArray() const { return SequenceArray; }
	/** Init SequencePlayer with current sequence. */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void InitSequencePlayer();
	/** Find animation in SequenceArray by Index, then set it to SequencePlayer. */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetSequenceByIndex(int32 InIndex);
	/** Find animation in SequenceArray by Name, then set it to SequencePlayer */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetSequenceByDisplayName(const FString& InName);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		int32 GetCurrentSequenceIndex()const { return CurrentSequenceIndex; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ULexUIPrefabSequence* GetCurrentSequence() const { return GetSequenceByIndex(CurrentSequenceIndex); }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ULexUIPrefabSequencePlayer* GetSequencePlayer() const { return SequencePlayer; }

	ULexUIPrefabSequence* AddNewAnimation();
	bool DeleteAnimationByIndex(int32 InIndex);
	ULexUIPrefabSequence* DuplicateAnimationByIndex(int32 InIndex);
	
	virtual void Awake()override;
	virtual void OnDestroy() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams)override;
	virtual void PreSave(class FObjectPreSaveContext SaveContext)override;
	virtual void PostDuplicate(bool bDuplicateForPIE)override;
	virtual void PostInitProperties()override;
	virtual void PostLoad()override;

	void FixEditorHelpers();
#endif
protected:

	UPROPERTY(EditAnywhere, Category = "Playback", meta=(ShowOnlyInnerProperties))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "Playback")
		TArray<TObjectPtr<ULexUIPrefabSequence>> SequenceArray;
	UPROPERTY(EditAnywhere, Category = "Playback")
		int32 CurrentSequenceIndex = 0;

	UPROPERTY(transient)
		TObjectPtr<ULexUIPrefabSequencePlayer> SequencePlayer;
};
