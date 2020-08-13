// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/LGUISettings.h"
#include "LGUI.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUIAtlasData.h"
#include "Core/LGUIBehaviour.h"

#if WITH_EDITOR
void ULGUISettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() != GET_MEMBER_NAME_CHECKED(ULGUISettings, defaultTraceChannel))
		{
			ULGUISpriteData::MarkAllSpritesNeedToReinitialize();
			ULGUIAtlasManager::InitCheck();
		}
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUISettings, LGUIBehaviourExecuteOrder))
		{
			//check repeat
			for (int i = 0; i < LGUIBehaviourExecuteOrder.Num() - 1; i++)
			{
				for (int j = i + 1; j < LGUIBehaviourExecuteOrder.Num(); j++)
				{
					if (LGUIBehaviourExecuteOrder[i] == LGUIBehaviourExecuteOrder[j])
					{
						LGUIBehaviourExecuteOrder.RemoveAt(j);
						i--;
						break;
					}
				}
			}
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
//ELGUIAtlasPackingType ULGUISettings::GetAtlasPackingType(const FName& InPackingTag)
//{
//	return GetAtlasSettings(InPackingTag).packingType;
//}
const TMap<FName, FLGUIAtlasSettings>& ULGUISettings::GetAllAtlasSettings()
{
	return GetDefault<ULGUISettings>()->atlasSettingForSpecificPackingTag;
}
const TArray<TSubclassOf<ULGUIBehaviour>>& ULGUISettings::GetLGUIBehaviourExecuteOrder()
{
	return GetDefault<ULGUISettings>()->LGUIBehaviourExecuteOrder;
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
#endif
