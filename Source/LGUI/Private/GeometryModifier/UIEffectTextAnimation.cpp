﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	if (auto uiRenderable = GetRenderableUIItem())
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
void UUIEffectTextAnimation::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
	bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged
	)
{
	if (!CheckUIText())return;
	if (InGeometry->vertices.Num() <= 0)return;
	if (uvChanged || colorChanged || vertexPositionChanged)
	{
		if (IsValid(selector))
		{
			if (selector->Select(uiText, selection))
			{
				if (InGeometry->vertices.Num() <= 0)return;
				for (auto propertyItem : properties)
				{
					if (IsValid(propertyItem))
					{
						propertyItem->ApplyProperty(uiText, selection, InGeometry);
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
			uiText->MarkTriangleDirty();
		}
	}
}
void UUIEffectTextAnimation::SetProperties(const TArray<UUIEffectTextAnimation_Property*>& value)
{
	if (properties != value)
	{
		properties = value;
		if (CheckUIText())
		{
			uiText->MarkTriangleDirty();
		}
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
			uiText->MarkTriangleDirty();
		}
	}
}

UUIText* UUIEffectTextAnimation_Selector::GetUIText()
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
		return selector->GetOffset();
	}
	UE_LOG(LGUI, Warning, TEXT("[UUIEffectTextAnimation::GetSelectorOffset]selector is null!"));
	return 0;
}

void UUIEffectTextAnimation::SetSelectorOffset(float value)
{
	if (IsValid(selector))
	{
		return selector->SetOffset(value);
	}
	UE_LOG(LGUI, Warning, TEXT("[UUIEffectTextAnimation::SetSelectorOffset]selector is null!"));
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
