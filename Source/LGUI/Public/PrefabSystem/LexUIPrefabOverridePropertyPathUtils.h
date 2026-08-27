// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexUIPrefab.h"

class FEditPropertyChain;

namespace LexUIPrefabSystem
{
	enum class EPropertyPathResolveResult : uint8
	{
		/** Leaf resolved. */
		Success,
		/** The path crosses something we deliberately don't address; caller should fall back to a whole member override. */
		FallbackToWholeMember,
		/** A segment no longer exists on the type. */
		Invalid,
	};

	struct LGUI_API FLexUIPrefabOverridePropertyPathUtils
	{
		/**
		 * Walk InSegments from InStruct down to the leaf.
		 * OutLeafContainer is the container that directly owns OutLeafProperty, so the pair can be handed straight to
		 * ULexUIPrefabHelperObject::RevertPrefabPropertyValue / ApplyPrefabPropertyValue.
		 * Pass InContainer == nullptr to validate the segments against the type only.
		 */
		static EPropertyPathResolveResult Resolve(UStruct* InStruct, void* InContainer, const TArray<FName>& InSegments
			, FProperty*& OutLeafProperty, void*& OutLeafContainer);
		/** Type-only overload, for validation and UI labels. */
		static EPropertyPathResolveResult ResolveProperty(UStruct* InStruct, const TArray<FName>& InSegments, FProperty*& OutLeafProperty);

		/** Display text like "Anchor Data > Anchor Min > X". Falls back to raw names for segments that no longer resolve. */
		static FText GetDisplayText(UStruct* InStruct, const TArray<FName>& InSegments);

		/**
		 * RelativeLocation/Rotation/Scale and AnchorData are two representations of the same state and are recomputed
		 * from each other on attach and on property change, so a partial override of them would be overwritten by the
		 * recomputation. Such properties must stay whole-member.
		 */
		static bool ShouldForceWholeMemberOverride(const UObject* InObject, FName InRootName);

#if WITH_EDITOR
		/**
		 * Build the segment chain of the property currently being edited, from the active member node down to the
		 * active (leaf) node. Leaves OutSegments empty if the chain has an unexpected shape, which means "fall back".
		 */
		static void BuildFromEditPropertyChain(const FEditPropertyChain& InChain, TArray<FName>& OutSegments);
#endif
	};
}
