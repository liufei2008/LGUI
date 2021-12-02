// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/LGUISettings.h"
#include "LGUI.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUIAtlasData.h"

#if WITH_EDITOR
#ifdef LGUI_DRAWCALLMODE_AUTO
float ULGUISettings::cacheAutoManageDepthThreshold = -1;
#endif
void ULGUISettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISettings, defaultAtlasSetting)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISettings, atlasSettingForSpecificPackingTag)
			)
		{
			ULGUISpriteData::MarkAllSpritesNeedToReinitialize();
			ULGUIAtlasManager::InitCheck();
		}
#ifdef LGUI_DRAWCALLMODE_AUTO
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISettings, autoManageDepthThreshold))
		{
			cacheAutoManageDepthThreshold = autoManageDepthThreshold;
		}
#endif
	}
}
#endif
const FLGUIAtlasSettings& ULGUISettings::GetAtlasSettings(const FName& InPackingTag)
{
	auto lguiSettings = GetDefault<ULGUISettings>();
	if (auto atlasSettings = lguiSettings->atlasSettingForSpecificPackingTag.Find(InPackingTag))
	{
		return *atlasSettings;
	}
	else
	{
		return lguiSettings->defaultAtlasSetting;
	}
}
int32 ULGUISettings::GetAtlasTextureInitialSize(const FName& InPackingTag)
{
	return ConvertAtlasTextureSizeTypeToSize(GetAtlasSettings(InPackingTag).atlasTextureInitialSize);
}
bool ULGUISettings::GetAtlasTextureSRGB(const FName& InPackingTag)
{
	return GetAtlasSettings(InPackingTag).atlasTextureUseSRGB;
}
int32 ULGUISettings::GetAtlasTexturePadding(const FName& InPackingTag)
{
	return GetAtlasSettings(InPackingTag).spaceBetweenSprites;
}
TextureFilter ULGUISettings::GetAtlasTextureFilter(const FName& InPackingTag)
{
	return GetAtlasSettings(InPackingTag).atlasTextureFilter;
}
const TMap<FName, FLGUIAtlasSettings>& ULGUISettings::GetAllAtlasSettings()
{
	return GetDefault<ULGUISettings>()->atlasSettingForSpecificPackingTag;
}
ELGUIScreenSpaceUIAntiAliasing ULGUISettings::GetAntiAliasingSampleCount()
{
	return GetDefault<ULGUISettings>()->antiAliasing;
}
#ifdef LGUI_DRAWCALLMODE_AUTO
float ULGUISettings::GetAutoManageDepthThreshold()
{
#if WITH_EDITOR
	if (cacheAutoManageDepthThreshold <= -0.5f)
	{
		cacheAutoManageDepthThreshold = GetDefault<ULGUISettings>()->autoManageDepthThreshold;
	}
	return cacheAutoManageDepthThreshold;
#else
	return GetDefault<ULGUISettings>()->autoManageDepthThreshold;
#endif
}
#endif
int32 ULGUISettings::GetPriorityInSceneViewExtension()
{
	return GetDefault<ULGUISettings>()->PriorityInSceneViewExtension;
}


#if WITH_EDITOR
FSimpleMulticastDelegate ULGUIEditorSettings::LGUIPreviewSetting_EditorPreviewViewportIndexChange;
void ULGUIEditorSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUIEditorSettings, LGUIPreview_EditorViewIndex))
		{
			if (LGUIPreviewSetting_EditorPreviewViewportIndexChange.IsBound())
			{
				LGUIPreviewSetting_EditorPreviewViewportIndexChange.Broadcast();
			}
		}
	}
}
int32 ULGUIEditorSettings::GetLGUIPreview_EditorViewIndex()
{
	return GetDefault<ULGUIEditorSettings>()->LGUIPreview_EditorViewIndex;
}
void ULGUIEditorSettings::SetLGUIPreview_EditorViewIndex(int32 value)
{
	GetMutableDefault<ULGUIEditorSettings>()->LGUIPreview_EditorViewIndex = value;
	if (LGUIPreviewSetting_EditorPreviewViewportIndexChange.IsBound())
	{
		LGUIPreviewSetting_EditorPreviewViewportIndexChange.Broadcast();
	}
}
bool ULGUIEditorSettings::GetPreserveHierarchyState()
{
	return GetDefault<ULGUIEditorSettings>()->PreserveHierarchyState;
}
float ULGUIEditorSettings::GetDelayRestoreHierarchyTime()
{
	return GetDefault<ULGUIEditorSettings>()->DelayRestoreHierarchyTime;
}
#endif
