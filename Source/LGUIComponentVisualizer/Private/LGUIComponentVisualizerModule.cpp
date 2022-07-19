// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIComponentVisualizerModule.h"

#include "LGUIHeaders.h"
#include "ComponentVisualizer/UIFlexibleGridLayoutComponentVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#define LOCTEXT_NAMESPACE "FLGUIComponentVisualizerModule"
DEFINE_LOG_CATEGORY(LGUIComponentVisualizer);

void FLGUIComponentVisualizerModule::StartupModule()
{
	//component visualizer
	{
		TSharedPtr<FUIFlexibleGridLayoutComponentVisualizer> Visualizer = MakeShareable(new FUIFlexibleGridLayoutComponentVisualizer);
		if (GUnrealEd)
		{
			GUnrealEd->RegisterComponentVisualizer(UUIFlexibleGridLayout::StaticClass()->GetFName(), Visualizer);
			Visualizer->OnRegister();
		}
	}
}

void FLGUIComponentVisualizerModule::ShutdownModule()
{
	//unregister component visualizer
	{
		if (GUnrealEd)
		{
			GUnrealEd->UnregisterComponentVisualizer(UUIFlexibleGridLayout::StaticClass()->GetFName());
		}
	}
}

IMPLEMENT_MODULE(FLGUIComponentVisualizerModule, LGUIComponentVisualizer)

#undef LOCTEXT_NAMESPACE