// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUIFontData_BaseObject.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"

#define LOCTEXT_NAMESPACE "LGUIFontData_BaseObject"

ULGUIFontData_BaseObject* ULGUIFontData_BaseObject::GetDefaultFont()
{
	static auto defaultFont = LoadObject<ULGUIFontData_BaseObject>(NULL, TEXT("/LGUI/DefaultSDFFont"));
	if (defaultFont == nullptr)
	{
		auto errMsg = LOCTEXT("MissingDefaultContent", "[ULGUIFontData_BaseObject::GetDefaultFont] Load default font error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.");
		UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errMsg, 10);
#endif
		return nullptr;
	}
	return defaultFont;
}

#undef LOCTEXT_NAMESPACE