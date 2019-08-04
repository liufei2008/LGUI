// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/LGUISettings.h"
#include "LGUI.h"
#include "Core/LGUISpriteData.h"

#if WITH_EDITOR
void ULGUISettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() != TEXT("defaultTraceChannel")
			&& Property->GetFName() != TEXT("maxPanelUpdateTimeInOneFrame"))
		{
			ULGUISpriteData::MarkAllSpritesNeedToReinitialize();
		}
	}
}
#endif
int32 ULGUISettings::GetAtlasTextureInitialSize(const FName& InPackingTag)
{
	auto lguiSettings = GetDefault<ULGUISettings>();
	if (auto atlasSettings = lguiSettings->atlasSettingForSpecificPackingTag.Find(InPackingTag))
	{
		return ConvertAtlasTextureSizeTypeToSize(atlasSettings->atlasTextureInitialSize);
	}
	else
	{
		return ConvertAtlasTextureSizeTypeToSize(lguiSettings->defaultAtlasSetting.atlasTextureInitialSize);
	}
}
bool ULGUISettings::GetAtlasTextureSRGB(const FName& InPackingTag)
{
	auto lguiSettings = GetDefault<ULGUISettings>();
	if (auto atlasSettings = lguiSettings->atlasSettingForSpecificPackingTag.Find(InPackingTag))
	{
		return atlasSettings->atlasTextureUseSRGB;
	}
	else
	{
		return lguiSettings->defaultAtlasSetting.atlasTextureUseSRGB;
	}
}
int32 ULGUISettings::GetAtlasTexturePadding(const FName& InPackingTag)
{
	auto lguiSettings = GetDefault<ULGUISettings>();
	if (auto atlasSettings = lguiSettings->atlasSettingForSpecificPackingTag.Find(InPackingTag))
	{
		return atlasSettings->spaceBetweenSprites;
	}
	else
	{
		return lguiSettings->defaultAtlasSetting.spaceBetweenSprites;
	}
}
TextureFilter ULGUISettings::GetAtlasTextureFilter(const FName& InPackingTag)
{
	auto lguiSettings = GetDefault<ULGUISettings>();
	if (auto atlasSettings = lguiSettings->atlasSettingForSpecificPackingTag.Find(InPackingTag))
	{
		return atlasSettings->atlasTextureFilter;
	}
	else
	{
		return lguiSettings->defaultAtlasSetting.atlasTextureFilter;
	}
}