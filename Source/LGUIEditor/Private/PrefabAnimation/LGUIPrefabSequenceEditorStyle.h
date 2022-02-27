// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"


class FLGUIPrefabSequenceEditorStyle
	: public FSlateStyleSet
{
public:
	FLGUIPrefabSequenceEditorStyle()
		: FSlateStyleSet("LGUIPrefabSequenceEditorStyle")
	{
		const FVector2D Icon16x16(16.0f, 16.0f);
		SetContentRoot(FPaths::EnginePluginsDir() / TEXT("MovieScene/LGUIPrefabSequence/Content"));

		Set("ClassIcon.LGUIPrefabSequence", new FSlateImageBrush(RootToContentDir(TEXT("LGUIPrefabSequence_16x.png")), Icon16x16));
		Set("ClassIcon.LGUIPrefabSequenceComponent", new FSlateImageBrush(RootToContentDir(TEXT("LGUIPrefabSequence_16x.png")), Icon16x16));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	static FLGUIPrefabSequenceEditorStyle& Get()
	{
		static FLGUIPrefabSequenceEditorStyle Inst;
		return Inst;
	}
	
	~FLGUIPrefabSequenceEditorStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};
