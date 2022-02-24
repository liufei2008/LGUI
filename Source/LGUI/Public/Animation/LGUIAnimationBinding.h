// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Misc/Guid.h"
#include "LGUIAnimationBinding.generated.h"



/**
 * UIAnimation中 ActorGUID和Actor的映射
 */
USTRUCT()
struct LGUI_API FLGUIAnimationBinding
{
	GENERATED_USTRUCT_BODY()

	// UI ActorName, 可能会发生变化
	UPROPERTY()
	FName ActorName;

	UPROPERTY()
	FGuid ActorGuid;

	// is root Actor?
	UPROPERTY()
	bool bIsRootActor;

	// Actor statable unique id, make relation with GUID and UIActorID
	UPROPERTY()
	uint32 UIActorID;

public:

	/**
	 * 根据WidgetName从AnimComp中找到对应的UIBaseActor
	 */
	UObject* FindRuntimeObject(class UUIAnimationComp& AnimComp, const class ULGUIAnimation* Animation) const;

	/**
	 * Compares two widget animation bindings for equality.
	 *
	 * @param X The first binding to compare.
	 * @param Y The second binding to compare.
	 * @return true if the bindings are equal, false otherwise.
	 */
	friend bool operator==(const FLGUIAnimationBinding& X, const FLGUIAnimationBinding& Y)
	{
		return (X.ActorName == Y.ActorName) && (X.ActorGuid == Y.ActorGuid) && (X.bIsRootActor == Y.bIsRootActor);
	}

	/**
	 * Serializes a widget animation binding from/to the given archive.
	 *
	 * @param Ar The archive to serialize to/from.
	 * @param Binding the binding to serialize.
	 */
	friend FArchive& operator<<(FArchive& Ar, FLGUIAnimationBinding& Binding)
	{
		Ar << Binding.ActorName;
		Ar << Binding.ActorGuid;
		Ar << Binding.bIsRootActor;
		Ar << Binding.UIActorID;
		return Ar;
	}

	friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIAnimationBinding& Binding)
	{
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		Record << SA_VALUE(TEXT("ActorName"), Binding.ActorName);
		Record << SA_VALUE(TEXT("ActorGuid"), Binding.ActorGuid);
		Record << SA_VALUE(TEXT("bIsRootActor"), Binding.bIsRootActor);
		Record << SA_VALUE(TEXT("UIActorID"), Binding.UIActorID);
	}
};
