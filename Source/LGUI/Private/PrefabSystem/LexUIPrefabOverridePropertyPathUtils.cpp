// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LexUIPrefabOverridePropertyPathUtils.h"
#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "Core/Components/LexWidget.h"

#define LOCTEXT_NAMESPACE "LexUIPrefabOverridePropertyPath"

bool FLexUIPrefabOverridePropertyPath::IsSameOrChildOf(const FLexUIPrefabOverridePropertyPath& InOther)const
{
	if (InOther.Segments.Num() > Segments.Num())return false;
	for (int32 i = 0; i < InOther.Segments.Num(); i++)
	{
		if (Segments[i] != InOther.Segments[i])return false;
	}
	return true;
}
FString FLexUIPrefabOverridePropertyPath::ToString()const
{
	TArray<FString> SegmentStrings;
	SegmentStrings.Reserve(Segments.Num());
	for (auto& Segment : Segments)
	{
		SegmentStrings.Add(Segment.ToString());
	}
	return FString::Join(SegmentStrings, TEXT("."));
}

namespace LexUIPrefabSystem
{
	EPropertyPathResolveResult FLexUIPrefabOverridePropertyPathUtils::Resolve(UStruct* InStruct, void* InContainer, const TArray<FName>& InSegments
		, FProperty*& OutLeafProperty, void*& OutLeafContainer)
	{
		OutLeafProperty = nullptr;
		OutLeafContainer = nullptr;
		if (InStruct == nullptr || InSegments.Num() == 0)return EPropertyPathResolveResult::Invalid;

		UStruct* CurrentStruct = InStruct;
		void* CurrentContainer = InContainer;
		for (int32 i = 0; i < InSegments.Num(); i++)
		{
			auto Property = FindFProperty<FProperty>(CurrentStruct, InSegments[i]);
			if (Property == nullptr)return EPropertyPathResolveResult::Invalid;

			if (i == InSegments.Num() - 1)//leaf
			{
				if (LexUIPrefab_ShouldSkipProperty(Property))return EPropertyPathResolveResult::Invalid;
				OutLeafProperty = Property;
				OutLeafContainer = CurrentContainer;
				return EPropertyPathResolveResult::Success;
			}

			//Intermediate segments must be plain structs. Containers are out of scope because the editor's property
			//chain omits element indices (FPropertyNode::BuildPropertyChain skips array element nodes), so there is no
			//way to tell which element an override refers to.
			if (Property->ArrayDim > 1)return EPropertyPathResolveResult::FallbackToWholeMember;
			auto StructProperty = CastField<FStructProperty>(Property);
			if (StructProperty == nullptr)return EPropertyPathResolveResult::FallbackToWholeMember;

			if (CurrentContainer != nullptr)
			{
				CurrentContainer = StructProperty->ContainerPtrToValuePtr<void>(CurrentContainer);
			}
			CurrentStruct = StructProperty->Struct;
		}
		return EPropertyPathResolveResult::Invalid;
	}

	EPropertyPathResolveResult FLexUIPrefabOverridePropertyPathUtils::ResolveProperty(UStruct* InStruct, const TArray<FName>& InSegments, FProperty*& OutLeafProperty)
	{
		void* UnusedContainer = nullptr;
		return Resolve(InStruct, nullptr, InSegments, OutLeafProperty, UnusedContainer);
	}

	FText FLexUIPrefabOverridePropertyPathUtils::GetDisplayText(UStruct* InStruct, const TArray<FName>& InSegments)
	{
		TArray<FText> Parts;
		Parts.Reserve(InSegments.Num());
		UStruct* CurrentStruct = InStruct;
		for (auto& Segment : InSegments)
		{
			FProperty* Property = CurrentStruct != nullptr ? FindFProperty<FProperty>(CurrentStruct, Segment) : nullptr;
			if (Property != nullptr)
			{
				Parts.Add(Property->GetDisplayNameText());
				auto StructProperty = CastField<FStructProperty>(Property);
				CurrentStruct = StructProperty != nullptr ? StructProperty->Struct : nullptr;
			}
			else
			{
				Parts.Add(FText::FromName(Segment));
				CurrentStruct = nullptr;
			}
		}
		return FText::Join(LOCTEXT("PropertyPathSeparator", " > "), Parts);
	}

	bool FLexUIPrefabOverridePropertyPathUtils::ShouldForceWholeMemberOverride(const UObject* InObject, FName InRootName)
	{
		if (InObject == nullptr)return false;
		if (!InObject->IsA<ULexWidget>())return false;
		return InRootName == ULexWidget::GetPropertyName_RelativeLocation()
			|| InRootName == ULexWidget::GetPropertyName_RelativeRotation()
			|| InRootName == ULexWidget::GetPropertyName_RelativeScale()
			;
	}

#if WITH_EDITOR
	void FLexUIPrefabOverridePropertyPathUtils::BuildFromEditPropertyChain(const FEditPropertyChain& InChain, TArray<FName>& OutSegments)
	{
		OutSegments.Reset();
		auto MemberNode = InChain.GetActiveMemberNode();
		auto ActiveNode = InChain.GetActiveNode();
		if (MemberNode == nullptr || ActiveNode == nullptr)return;

		bool bReachedActiveNode = false;
		for (auto Node = MemberNode; Node != nullptr; Node = Node->GetNextNode())
		{
			auto Property = Node->GetValue();
			if (Property == nullptr)
			{
				OutSegments.Reset();
				return;
			}
			OutSegments.Add(Property->GetFName());
			if (Node == ActiveNode)
			{
				bReachedActiveNode = true;
				break;
			}
		}
		if (!bReachedActiveNode)//active node is not below the active member node, shape we don't understand
		{
			OutSegments.Reset();
		}
	}
#endif
}

#undef LOCTEXT_NAMESPACE
