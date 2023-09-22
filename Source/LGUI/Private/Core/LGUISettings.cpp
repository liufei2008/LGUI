// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUISettings.h"
#include "LGUI.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUIDynamicSpriteAtlasData.h"

#if WITH_EDITOR
float ULGUISettings::CacheAutoBatchThreshold = -1;
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
			ULGUIDynamicSpriteAtlasManager::InitCheck();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUISettings, AutoBatchThreshold))
		{
			CacheAutoBatchThreshold = AutoBatchThreshold;
		}
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
float ULGUISettings::GetAutoBatchThreshold()
{
#if WITH_EDITOR
	if (CacheAutoBatchThreshold <= -0.5f)
	{
		CacheAutoBatchThreshold = GetDefault<ULGUISettings>()->AutoBatchThreshold;
	}
	return CacheAutoBatchThreshold;
#else
	return GetDefault<ULGUISettings>()->AutoBatchThreshold;
#endif
}
bool ULGUISettings::GetLogPrefabLoadTime()
{
	return GetDefault<ULGUISettings>()->bLogPrefabLoadTime;
}
int32 ULGUISettings::ConvertAtlasTextureSizeTypeToSize(const ELGUIAtlasTextureSizeType& InType)
{
	return ((int32)FMath::Pow(2.0, (double)InType)) * 256;
}
int32 ULGUISettings::GetPriorityInSceneViewExtension()
{
	return GetDefault<ULGUISettings>()->PriorityInSceneViewExtension;
}


#if WITH_EDITOR
FSimpleMulticastDelegate ULGUIEditorSettings::LGUIPreviewSetting_EditorPreviewViewportIndexChange;
FSimpleMulticastDelegate ULGUIEditorSettings::LGUIEditorSetting_PreserveHierarchyStateChange;
void ULGUIEditorSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	auto MemberProperty = PropertyChangedEvent.MemberProperty;
	auto Property = PropertyChangedEvent.Property;
	if (MemberProperty && Property)
	{
		if (MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUIEditorSettings, LGUIPreview_EditorViewIndex))
		{
			if (LGUIPreviewSetting_EditorPreviewViewportIndexChange.IsBound())
			{
				LGUIPreviewSetting_EditorPreviewViewportIndexChange.Broadcast();
			}
		}
		else if (MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUIEditorSettings, ExtraPrefabFolders)
			|| (
				Property->GetFName() == GET_MEMBER_NAME_CHECKED(FDirectoryPath, Path)
				&& MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUIEditorSettings, ExtraPrefabFolders)
				)
			)
		{
			for (FDirectoryPath& PathToFix : ExtraPrefabFolders)
			{
				if (!PathToFix.Path.IsEmpty() && !PathToFix.Path.StartsWith(TEXT("/"), ESearchCase::CaseSensitive))
				{
					PathToFix.Path = FString::Printf(TEXT("/Game/%s"), *PathToFix.Path);
				}
			}
			if (IsValid(GEditor))
			{
				GEditor->BroadcastLevelActorListChanged();//refresh Outliner menu
			}
		}
		else if (MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUIEditorSettings, bPreserveHierarchyState))
		{
			LGUIEditorSetting_PreserveHierarchyStateChange.Broadcast();
		}
	}
}
void ULGUIEditorSettings::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
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
	return GetDefault<ULGUIEditorSettings>()->bPreserveHierarchyState;
}
float ULGUIEditorSettings::GetDelayRestoreHierarchyTime()
{
	return GetDefault<ULGUIEditorSettings>()->DelayRestoreHierarchyTime;
}
#endif
