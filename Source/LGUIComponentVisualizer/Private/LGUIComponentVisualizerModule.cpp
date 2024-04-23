// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIComponentVisualizerModule.h"

#include "LGUIHeaders.h"
#include "ComponentVisualizer/UIFlexibleGridLayoutComponentVisualizer.h"
#include "ComponentVisualizer/UIItemComponentVisualizer.h"
#include "ComponentVisualizer/UIPanelLayoutFlexibleGridComponentVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#define LOCTEXT_NAMESPACE "FLGUIComponentVisualizerModule"
DEFINE_LOG_CATEGORY(LGUIComponentVisualizer);

void FLGUIComponentVisualizerModule::StartupModule()
{
	//component visualizer
	{
		if (GUnrealEd)
		{
			TSharedPtr<FUIFlexibleGridLayoutComponentVisualizer> UIFlexibleGridLayoutVisualizer = MakeShareable(new FUIFlexibleGridLayoutComponentVisualizer);
			TSharedPtr<FUIItemComponentVisualizer> UIItemVisualizer = MakeShareable(new FUIItemComponentVisualizer);
			TSharedPtr<FUIPanelLayoutFlexibleGridComponentVisualizer> UIPanelLayoutFlexibleGridVisualizer = MakeShareable(new FUIPanelLayoutFlexibleGridComponentVisualizer);
			GUnrealEd->RegisterComponentVisualizer(UUIFlexibleGridLayout::StaticClass()->GetFName(), UIFlexibleGridLayoutVisualizer);
			GUnrealEd->RegisterComponentVisualizer(UUIItem::StaticClass()->GetFName(), UIItemVisualizer);
			GUnrealEd->RegisterComponentVisualizer(UUIPanelLayout_FlexibleGrid::StaticClass()->GetFName(), UIPanelLayoutFlexibleGridVisualizer);
			UIFlexibleGridLayoutVisualizer->OnRegister();
			UIItemVisualizer->OnRegister();
			UIPanelLayoutFlexibleGridVisualizer->OnRegister();
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
			GUnrealEd->UnregisterComponentVisualizer(UUIPanelLayout_FlexibleGrid::StaticClass()->GetFName());
		}
	}
}

IMPLEMENT_MODULE(FLGUIComponentVisualizerModule, LGUIComponentVisualizer)

#undef LOCTEXT_NAMESPACE