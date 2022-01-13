// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIGeometryModifierBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"

UUIGeometryModifierBase::UUIGeometryModifierBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIGeometryModifierBase::BeginPlay()
{
	Super::BeginPlay();
	
}
void UUIGeometryModifierBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
}
UUIBatchGeometryRenderable* UUIGeometryModifierBase::GetRenderableUIItem()
{
	if(!renderableUIItem.IsValid())
	{
		if (auto actor = GetOwner())
		{
			TInlineComponentArray<class UUIBatchGeometryRenderable*> components;
			actor->GetComponents(components);
			if (components.Num() > 1)
			{
				for (auto comp : components)
				{
					if (comp->GetFName() == componentName)
					{
						renderableUIItem = (UUIBatchGeometryRenderable*)comp;
						break;
					}
				}
				if (!renderableUIItem.IsValid())
				{
					UE_LOG(LGUI, Warning, TEXT("[UUIGeometryModifierBase::GetRenderableUIItem]Cannot find component of name:%s, will use first one."), *(componentName.ToString()));
					renderableUIItem = (UUIBatchGeometryRenderable*)components[0];
				}
			}
			else if(components.Num() == 1)
			{
				renderableUIItem = (UUIBatchGeometryRenderable*)components[0];
			}
		}
	}
	return renderableUIItem.Get();
}
#if WITH_EDITOR
void UUIGeometryModifierBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GetOwner())
	{
		if (auto rootComp = GetOwner()->GetRootComponent())
		{
			if (auto uiRenderable = Cast<UUIItem>(rootComp))
			{
				uiRenderable->EditorForceUpdate();
			}
		}
	}
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIGeometryModifierBase, componentName))
		{
			//remove from old
			RemoveFromUIBatchGeometry();
			//add to new
			AddToUIBatchGeometry();
		}
	}
}
#endif

void UUIGeometryModifierBase::OnRegister()
{
	Super::OnRegister();
	AddToUIBatchGeometry();
}
void UUIGeometryModifierBase::OnUnregister()
{
	Super::OnUnregister();
	RemoveFromUIBatchGeometry();
}

void UUIGeometryModifierBase::RemoveFromUIBatchGeometry()
{
	if (renderableUIItem.IsValid())
	{
		renderableUIItem->RemoveGeometryModifier(this);
		renderableUIItem->MarkTriangleDirty();
		renderableUIItem = nullptr;
	}
}
void UUIGeometryModifierBase::AddToUIBatchGeometry()
{
	if (GetRenderableUIItem() != nullptr)
	{
		renderableUIItem->AddGeometryModifier(this);
		renderableUIItem->MarkTriangleDirty();
	}
}
