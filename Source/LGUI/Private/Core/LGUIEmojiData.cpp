// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIEmojiData.h"
#include "LGUI.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/LGUIEmojiData.h"
#include "Extensions/UISpriteSequencePlayer.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"

void ULGUIEmojiData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnDataChange.Broadcast();
}

void ULGUIEmojiData::CreateOrUpdateEmojiObject(UUIItem* parent, const TArray<FUIText_RichTextEmojiTag>& emojiTagData, TArray<UUIItem*>& createdEmojiObjectArray, bool listEmojiObjectInOutliner)
{
	//destroy extra
	while (createdEmojiObjectArray.Num() > emojiTagData.Num())
	{
		auto lastIndex = createdEmojiObjectArray.Num() - 1;
		auto emojiObj = createdEmojiObjectArray[lastIndex];
		LGUIUtils::DestroyActorWithHierarchy(emojiObj->GetOwner());
		createdEmojiObjectArray.RemoveAt(lastIndex);
	}
	//create more
	while (createdEmojiObjectArray.Num() < emojiTagData.Num())
	{
		auto spriteActor = parent->GetWorld()->SpawnActor<AUISpriteActor>();
		spriteActor->SetFlags(EObjectFlags::RF_Transient);
		spriteActor->GetUISprite()->AttachToComponent(parent, FAttachmentTransformRules::KeepRelativeTransform);
		createdEmojiObjectArray.Push(spriteActor->GetUISprite());
	}
	//apply data
	for (int i = 0; i < emojiTagData.Num(); i++)
	{
		auto emojiObj = (UUISprite*)createdEmojiObjectArray[i];
#if WITH_EDITOR
		emojiObj->GetOwner()->SetActorLabel(FString::Printf(TEXT("[%s]"), *emojiTagData[i].TagName.ToString()));
		if (!parent->GetWorld()->IsGameWorld())//set it only in edit mode
		{
			auto bListedInSceneOutliner_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bListedInSceneOutliner"));
			bListedInSceneOutliner_Property->SetPropertyValue_InContainer(emojiObj->GetOwner(), listEmojiObjectInOutliner);
		}
#endif
		if (auto emojiItemPtr = emojiMap.Find(emojiTagData[i].TagName))
		{
			auto& spriteFrames = emojiItemPtr->frames;
			auto sequencePlayerComp = emojiObj->GetOwner()->FindComponentByClass<UUISpriteSequencePlayer>();
			ULGUISpriteData_BaseObject* sprite = nullptr;
			if (spriteFrames.Num() == 0)
			{
				emojiObj->SetSprite(nullptr, false);
				if (IsValid(sequencePlayerComp))
				{
					sequencePlayerComp->DestroyComponent();
				}
			}
			else if (spriteFrames.Num() == 1)
			{
				sprite = spriteFrames[0];
				if (IsValid(sequencePlayerComp))
				{
					sequencePlayerComp->DestroyComponent();
				}
			}
			else
			{
				sprite = spriteFrames[0];
				if (!IsValid(sequencePlayerComp))
				{
					sequencePlayerComp = NewObject<UUISpriteSequencePlayer>(emojiObj->GetOwner());
					sequencePlayerComp->SetSnapSpriteSize(false);
					sequencePlayerComp->RegisterComponent();
					emojiObj->GetOwner()->AddInstanceComponent(sequencePlayerComp);
				}
				sequencePlayerComp->SetSpriteSequence(spriteFrames);
				sequencePlayerComp->SetFps(emojiItemPtr->overrideAnimationFps < 0 ? animationFps : emojiItemPtr->overrideAnimationFps);
				if (parent->GetWorld()->IsGameWorld())
				{
					sequencePlayerComp->Play();
				}
			}
			emojiObj->SetColor(emojiTagData[i].TintColor);
			emojiObj->SetAnchoredPosition(emojiTagData[i].Position);
			auto referenceWidth = emojiTagData[i].Size;
			float referenceHeight;
			if (sprite != nullptr)
			{
				emojiObj->SetSprite(sprite, false);
				referenceHeight = (float)sprite->GetSpriteInfo().height / (float)sprite->GetSpriteInfo().width * referenceWidth;
			}
			else
			{
				referenceHeight = referenceWidth;
			}
			emojiObj->SetSizeDelta(FVector2D(referenceWidth, referenceHeight));
		}
	}
#if WITH_EDITOR
	if (!parent->GetWorld()->IsGameWorld())//refresh on editor
	{
		ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged();
	}
#endif
}
