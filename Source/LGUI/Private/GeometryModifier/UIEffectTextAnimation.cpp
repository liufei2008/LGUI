// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectTextAnimation.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"


UUIEffectTextAnimation::UUIEffectTextAnimation()
{
	PrimaryComponentTick.bCanEverTick = false;
}
bool UUIEffectTextAnimation::CheckUIText()
{
	if (IsValid(uiText))return true;
	if (auto uiRenderable = GetUIRenderable())
	{
		uiText = Cast<UUIText>(uiRenderable);
		if (IsValid(uiText))
		{
			return true;
		}
	}
	return false;
}
void UUIEffectTextAnimation::BeginPlay()
{
	Super::BeginPlay();
	for (auto propertyItem : properties)
	{
		if (IsValid(propertyItem))
		{
			propertyItem->Init();
		}
	}
}
void UUIEffectTextAnimation::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	for (auto propertyItem : properties)
	{
		if (IsValid(propertyItem))
		{
			propertyItem->Deinit();
		}
	}
}

#if WITH_EDITOR
void UUIEffectTextAnimation::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		auto PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIEffectTextAnimation, selectorOffset))
		{
			if (IsValid(selector))
			{
				selector->SetOffset(selectorOffset);
			}
		}
	}
}
#endif

void UUIEffectTextAnimation::ModifyUIGeometry(
	UIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	if (!CheckUIText())return;
	if (InGeometry.vertices.Num() <= 0)return;
	if (InTriangleChanged || InUVChanged || InColorChanged || InVertexPositionChanged)
	{
		if (IsValid(selector))
		{
			if (selector->Select(uiText, selection))
			{
				if (InGeometry.vertices.Num() <= 0)return;
				for (auto propertyItem : properties)
				{
					if (IsValid(propertyItem))
					{
						propertyItem->ApplyProperty(uiText, selection, &InGeometry);
					}
				}
			}
		}
	}
}
UUIText* UUIEffectTextAnimation::GetUIText()
{
	CheckUIText();
	return uiText;
}
UUIEffectTextAnimation_Property* UUIEffectTextAnimation::GetProperty(int index)const
{
	if (index >= properties.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIEffectTextAnimation::GetProperty]index:%d out of range:%d"), index, properties.Num());
		return nullptr;
	}
	return properties[index];
}
void UUIEffectTextAnimation::SetSelector(UUIEffectTextAnimation_Selector* value)
{
	if (selector != value)
	{
		selector = value;
		if (CheckUIText())
		{
			uiText->MarkVerticesDirty(true, true, true, true);
		}
	}
}
void UUIEffectTextAnimation::SetProperties(const TArray<UUIEffectTextAnimation_Property*>& value)
{
	properties = value;
	if (CheckUIText())
	{
		uiText->MarkVerticesDirty(true, true, true, true);
	}
}
void UUIEffectTextAnimation::SetProperty(int index, UUIEffectTextAnimation_Property* value)
{
	if (index >= properties.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIEffectTextAnimation::SetProperty]index:%d out of range:%d"), index, properties.Num());
		return;
	}
	if (properties[index] != value)
	{
		properties[index] = value;
		if (CheckUIText())
		{
			uiText->MarkVerticesDirty(true, true, true, true);
		}
	}
}

UUIText* UUIEffectTextAnimation_Selector::GetUIText()const
{
	GetUIEffectTextAnimation();
	return UIEffectTextAnimation.IsValid() ? UIEffectTextAnimation->GetUIText() : nullptr;
}

UUIEffectTextAnimation* UUIEffectTextAnimation_Selector::GetUIEffectTextAnimation()const
{
	if (!UIEffectTextAnimation.IsValid())
	{
		if (auto outter = this->GetOuter())
		{
			UIEffectTextAnimation = Cast<UUIEffectTextAnimation>(outter);
		}
	}
	return UIEffectTextAnimation.Get();
}

void UUIEffectTextAnimation_Selector::SetOffset(float value)
{
	if (offset != value)
	{
		offset = value;
		if (auto uiText = GetUIText())
		{
			uiText->MarkVertexPositionDirty();
		}
	}
}

float UUIEffectTextAnimation::GetSelectorOffset()const
{
	if (IsValid(selector))
	{
		selectorOffset = selector->GetOffset();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIEffectTextAnimation::GetSelectorOffset]selector is null!"));
	}
	return selectorOffset;
}

void UUIEffectTextAnimation::SetSelectorOffset(float value)
{
	if (IsValid(selector))
	{
		selector->SetOffset(value);
		selectorOffset = value;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIEffectTextAnimation::SetSelectorOffset]selector is null!"));
	}
}

UUIText* UUIEffectTextAnimation_Property::GetUIText()
{
	if (auto outter = this->GetOuter())
	{
		if (auto uiTextAnimation = Cast<UUIEffectTextAnimation>(outter))
		{
			return uiTextAnimation->GetUIText();
		}
	}
	return nullptr;
}
void UUIEffectTextAnimation_Property::MarkUITextPositionDirty()
{
	if (auto uiText = GetUIText())
	{
		uiText->MarkVertexPositionDirty();
	}
}
