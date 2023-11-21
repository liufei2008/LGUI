// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRichTextImageData.h"
#include "LGUI.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/LGUIRichTextImageData.h"
#include "Extensions/UISpriteSequencePlayer.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManager.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Core/LGUISpriteData_BaseObject.h"

#if WITH_EDITOR
void ULGUIRichTextImageData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnDataChange.Broadcast();
}
#endif

void ULGUIRichTextImageData::SetImageMap(const TMap<FName, FLGUIRichTextImageItemData>& value)
{
	imageMap = value;
	OnDataChange.Broadcast();
}
void ULGUIRichTextImageData::SetAnimationFps(float value)
{
	animationFps = value;
	OnDataChange.Broadcast();
}
void ULGUIRichTextImageData::BroadcastOnDataChange()
{
	OnDataChange.Broadcast();
}

void ULGUIRichTextImageData::CreateOrUpdateObject(UUIItem* parent, const TArray<FUIText_RichTextImageTag>& imageTagData, TArray<UUIItem*>& createdImageObjectArray, bool listImageObjectInEditorOutliner)
{
	//destroy extra
	while (createdImageObjectArray.Num() > imageTagData.Num())
	{
		auto lastIndex = createdImageObjectArray.Num() - 1;
		auto imageObj = createdImageObjectArray[lastIndex];
		LGUIUtils::DestroyActorWithHierarchy(imageObj->GetOwner());
		createdImageObjectArray.RemoveAt(lastIndex);
	}
	//create more
	while (createdImageObjectArray.Num() < imageTagData.Num())
	{
		auto spriteActor = parent->GetWorld()->SpawnActor<AUISpriteActor>();
		spriteActor->SetFlags(EObjectFlags::RF_Transient);
		spriteActor->GetUISprite()->AttachToComponent(parent, FAttachmentTransformRules::KeepRelativeTransform);
		createdImageObjectArray.Push(spriteActor->GetUISprite());
	}
	//apply data
	for (int i = 0; i < imageTagData.Num(); i++)
	{
		auto imageObj = (UUISprite*)createdImageObjectArray[i];
#if WITH_EDITOR
		imageObj->GetOwner()->SetActorLabel(FString::Printf(TEXT("[%s]"), *imageTagData[i].TagName.ToString()));
		if (!parent->GetWorld()->IsGameWorld())//set it only in edit mode
		{
			auto bListedInSceneOutliner_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bListedInSceneOutliner"));
			bListedInSceneOutliner_Property->SetPropertyValue_InContainer(imageObj->GetOwner(), listImageObjectInEditorOutliner);
		}
#endif
		if (auto imageItemPtr = imageMap.Find(imageTagData[i].TagName))
		{
			auto& spriteFrames = imageItemPtr->frames;
			auto sequencePlayerComp = imageObj->GetOwner()->FindComponentByClass<UUISpriteSequencePlayer>();
			ULGUISpriteData_BaseObject* sprite = nullptr;
			if (spriteFrames.Num() == 0)
			{
				imageObj->SetSprite(nullptr, false);
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
					sequencePlayerComp = NewObject<UUISpriteSequencePlayer>(imageObj->GetOwner());
					sequencePlayerComp->SetSnapSpriteSize(false);
					sequencePlayerComp->RegisterComponent();
					imageObj->GetOwner()->AddInstanceComponent(sequencePlayerComp);
				}
				sequencePlayerComp->SetSpriteSequence(spriteFrames);
				sequencePlayerComp->SetFps(imageItemPtr->overrideAnimationFps < 0 ? animationFps : imageItemPtr->overrideAnimationFps);
				if (parent->GetWorld()->IsGameWorld())
				{
					sequencePlayerComp->Play();
				}
			}
			imageObj->SetColor(imageTagData[i].TintColor);
			imageObj->SetAnchoredPosition(imageTagData[i].Position);
			auto referenceWidth = imageTagData[i].Size;
			float referenceHeight;
			if (sprite != nullptr)
			{
				imageObj->SetSprite(sprite, false);
				referenceHeight = (float)sprite->GetSpriteInfo().height / (float)sprite->GetSpriteInfo().width * referenceWidth;
			}
			else
			{
				referenceHeight = referenceWidth;
			}
			imageObj->SetSizeDelta(FVector2D(referenceWidth, referenceHeight));
		}
		else
		{
			imageObj->SetColor(imageTagData[i].TintColor);
			imageObj->SetAnchoredPosition(imageTagData[i].Position);
			imageObj->SetSizeDelta(FVector2D(imageTagData[i].Size, imageTagData[i].Size));
		}
	}
#if WITH_EDITOR
	if (!parent->GetWorld()->IsGameWorld())//refresh on editor
	{
		ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();
	}
#endif
}
