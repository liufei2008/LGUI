// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIFontData_BaseObject.h"
#include "LGUI.h"

ULGUIFontData_BaseObject* ULGUIFontData_BaseObject::GetDefaultFont()
{
	static auto defaultFont = LoadObject<ULGUIFontData_BaseObject>(NULL, TEXT("/LGUI/DefaultSDFFont"));
	if (defaultFont == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIFontData_BaseObject::GetDefaultFont]Load default font error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
		return nullptr;
	}
	return defaultFont;
}