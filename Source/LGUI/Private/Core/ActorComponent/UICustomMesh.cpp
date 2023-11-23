// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UICustomMesh.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/UIGeometry.h"
#include "Core/LGUICustomMesh.h"
#include "Core/ActorComponent/UITextureBase.h"

#define LOCTEXT_NAMESPACE "UICustomMesh"

UUICustomMesh::UUICustomMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UUICustomMesh::SupportDrawcallBatching()const
{
	if (IsValid(CustomMesh))
	{
		return CustomMesh->SupportDrawcallBatching();
	}
	return true;
}
void UUICustomMesh::OnBeforeCreateOrUpdateGeometry()
{

}
UTexture* UUICustomMesh::GetTextureToCreateGeometry()
{
	return UUITextureBase::GetDefaultWhiteTexture();
}
void UUICustomMesh::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (IsValid(CustomMesh))
	{
		CustomMesh->UIGeo = &InGeo;
		CustomMesh->OnFillMesh(this, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
	}
}

void UUICustomMesh::BeginPlay()
{
	Super::BeginPlay();
}
void UUICustomMesh::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
}

#if WITH_EDITOR
bool UUICustomMesh::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();

	}

	return Super::CanEditChange(InProperty);
}
void UUICustomMesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (auto Property = PropertyChangedEvent.MemberProperty)
	{
		auto PropertyName = Property->GetFName();
	}
}
#endif

void UUICustomMesh::SetCustomMesh(ULGUICustomMesh* Value)
{
	if (CustomMesh != Value)
	{
		CustomMesh = Value;
		MarkAllDirty();
	}
}

#undef LOCTEXT_NAMESPACE
