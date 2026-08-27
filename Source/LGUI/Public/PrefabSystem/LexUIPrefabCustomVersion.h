// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Identifies the prefab blob format stored in ULexUIPrefab::BinaryData / BinaryDataForBuild.
 *
 * That blob is written to a bare FBufferArchive and read back from a bare FMemoryReader, so no version travels with it.
 * Seeding this custom version on those two archives is what lets the otherwise version-blind operator<< of the save
 * data structs branch on ELexUIPrefabVersion.
 *
 * Deliberately NOT registered through FCustomVersionRegistration: the value is seeded from the asset's own
 * PrefabVersion field, not from the global registry.
 */
struct FLexUIPrefabCustomVersion
{
	static const FGuid GUID;
};
