// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIComponentVisualizerModule.h"

#include "LGUIHeaders.h"
#include "ComponentVisualizer/UIFlexibleGridLayoutComponentVisualizer.h"
#include "ComponentVisualizer/UIItemComponentVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#define LOCTEXT_NAMESPACE "FLGUIComponentVisualizerModule"
DEFINE_LOG_CATEGORY(LGUIComponentVisualizer);

void FLGUIComponentVisualizerModule::StartupModule()
{
	//component visualizer
	{
		TSharedPtr<FUIFlexibleGridLayoutComponentVisualizer> UIFlexibleGridLayoutVisualizer = MakeShareable(new FUIFlexibleGridLayoutComponentVisualizer);
		TSharedPtr<FUIItemComponentVisualizer> UIItemVisualizer = MakeShareable(new FUIItemComponentVisualizer);
		if (GUnrealEd)
		{
			GUnrealEd->RegisterComponentVisualizer(UUIFlexibleGridLayout::StaticClass()->GetFName(), UIFlexibleGridLayoutVisualizer);
			GUnrealEd->RegisterComponentVisualizer(UUIItem::StaticClass()->GetFName(), UIItemVisualizer);
			UIFlexibleGridLayoutVisualizer->OnRegister();
			UIItemVisualizer->OnRegister();
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
			GUnrealEd->UnregisterComponentVisualizer(UUIItem::StaticClass()->GetFName());
		}
	}
}

IMPLEMENT_MODULE(FLGUIComponentVisualizerModule, LGUIComponentVisualizer)

#undef LOCTEXT_NAMESPACE