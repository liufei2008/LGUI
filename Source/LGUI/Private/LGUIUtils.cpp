// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIDrawcall.h"
#include "Sound/SoundBase.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UIBatchMeshRenderable.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

void LGUIUtils::DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy)
{
	if (!Target->IsValidLowLevelFast())
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIUtils::DestroyActorWithHierarchy]Try to delete not valid actor"));
		return;
	}
	if (WithHierarchy)
	{
		TArray<AActor*> AllChildrenActors;
		CollectChildrenActors(Target, AllChildrenActors);//collect all actor
		for (auto item : AllChildrenActors)
		{
#if WITH_EDITOR
			if (auto world = item->GetWorld())
			{
				if (world->WorldType == EWorldType::Editor || world->WorldType == EWorldType::EditorPreview)
				{
					world->EditorDestroyActor(item, true);
				}
				else
				{
					item->Destroy();
				}
			}
#else
			item->Destroy();
#endif
		}
	}
	else
	{
#if WITH_EDITOR
		if (auto world = Target->GetWorld())
		{
			if (world->WorldType == EWorldType::Editor || world->WorldType == EWorldType::EditorPreview)
			{
				world->EditorDestroyActor(Target, true);
			}
			else
			{
				Target->Destroy();
			}
		}
#else
		Target->Destroy();
#endif
	}
}
void LGUIUtils::CollectChildrenActors(AActor* Target, TArray<AActor*>& AllChildrenActors, bool IncludeTarget)
{
	if (IncludeTarget)
	{
		AllChildrenActors.Add(Target);
	}
	TArray<AActor*> actorList;
	Target->GetAttachedActors(actorList);
	for (auto item : actorList)
	{
		CollectChildrenActors(item, AllChildrenActors, true);
	}
}
UTexture2D* LGUIUtils::CreateTexture(int32 InSize, FColor InDefaultColor, UObject* InOuter, FName InDefaultName)
{
	auto ResultTexture = NewObject<UTexture2D>(
		InOuter,
		InDefaultName,
		RF_Transient
		);
	auto PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = InSize;
	PlatformData->SizeY = InSize;
	PlatformData->PixelFormat = PF_B8G8R8A8;
	// Allocate first mipmap.
	int32 NumBlocksX = InSize / GPixelFormats[PF_B8G8R8A8].BlockSizeX;
	int32 NumBlocksY = InSize / GPixelFormats[PF_B8G8R8A8].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	PlatformData->Mips.Add(Mip);
	Mip->SizeX = InSize;
	Mip->SizeY = InSize;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* dataPtr = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[PF_B8G8R8A8].BlockBytes);
	FColor* pixelPtr = static_cast<FColor*>(dataPtr);
	for (int i = 0, count = InSize * InSize; i < count; i++)
	{
		pixelPtr[i] = InDefaultColor;
	}
	Mip->BulkData.Unlock();
	ResultTexture->SetPlatformData(PlatformData);
	return ResultTexture;
}

TArray<uint8> LGUIUtils::GetMD5(const FString& InString)
{
	FMD5 Md5Gen;
	Md5Gen.Update((unsigned char*)TCHAR_TO_ANSI(*InString), FCString::Strlen(*InString));
	TArray<uint8> Digest;
	Digest.SetNumZeroed(16);
	Md5Gen.Final(Digest.GetData());
	return Digest;
}
FString LGUIUtils::GetMD5String(const FString& InString)
{
	TArray<uint8> MD5Digest = GetMD5(InString);
	FString Md5String;
	for (TArray<uint8>::TConstIterator it(MD5Digest); it; ++it)
	{
		Md5String += FString::Printf(TEXT("%02x"), *it);
	}
	return Md5String;
}

#if WITH_EDITOR
void LGUIUtils::NotifyPropertyChanged(UObject* Object, FProperty* Property)
{
	if (!IsValid(Object))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InValid object!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
	if (Property == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InValid property!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}

	FEditPropertyChain PropertyChain;
	PropertyChain.AddHead(Property);//@todo: how to build property chain?
	TArray<UObject*> ModifiedObjects;
	ModifiedObjects.Add(Object);
	FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet, MakeArrayView(ModifiedObjects));
	FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);
	Object->PostEditChangeChainProperty(PropertyChangedChainEvent);
}
void LGUIUtils::NotifyPropertyChanged(UObject* Object, FName PropertyName)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	NotifyPropertyChanged(Object, Property);
}
void LGUIUtils::NotifyPropertyPreChange(UObject* Object, FProperty* Property)
{
	if (!IsValid(Object))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InValid object!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
	if (Property == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InValid property!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}

	FEditPropertyChain PropertyChain;
	PropertyChain.AddHead(Property);//@todo: how to build property chain?
	Object->PreEditChange(PropertyChain);
}
void LGUIUtils::NotifyPropertyPreChange(UObject* Object, FName PropertyName)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	NotifyPropertyPreChange(Object, Property);
}
#endif

FColor LGUIUtils::ColorHSVDataToColorRGB(const FVector& InHSVColor)
{
	FLinearColor colorHSV(InHSVColor);
	return colorHSV.HSVToLinearRGB().ToFColor(false);
}
FVector LGUIUtils::ColorRGBToColorHSVData(const FColor& InRGBColor)
{
	auto linearColorRGB = FLinearColor(Color255To1_Table[InRGBColor.R], Color255To1_Table[InRGBColor.G], Color255To1_Table[InRGBColor.B], 1.0f);
	auto linearColorHSV = linearColorRGB.LinearRGBToHSV();
	return FVector(linearColorHSV);
}

FColor LGUIUtils::MultiplyColor(FColor A, FColor B)
{
	FColor result;
	result.R = (uint8)(A.R * Color255To1_Table[B.R]);
	result.G = (uint8)(A.G * Color255To1_Table[B.G]);
	result.B = (uint8)(A.B * Color255To1_Table[B.B]);
	result.A = (uint8)(A.A * Color255To1_Table[B.A]);
	return result;
}

#if WITH_EDITOR
//nodify some informations in editor
void LGUIUtils::EditorNotification(FText NofityText, float ExpireDuration)
{
	if (!IsValid(GEditor))return;
	FNotificationInfo Info(NofityText);
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = ExpireDuration;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();

	auto CompileFailSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	GEditor->PlayEditorSound(CompileFailSound);
}
#endif


void LGUIUtils::LogObjectFlags(UObject* obj)
{
	UE_LOG(LGUI, Log, TEXT("object:%s\
\n	flagValue:%d\
\n	RF_Public:%d\
\n	RF_Standalone:%d\
\n	RF_MarkAsNative:%d\
\n	RF_Transactional:%d\
\n	RF_ClassDefaultObject:%d\
\n	RF_ArchetypeObject:%d\
\n	RF_Transient:%d\
\n	RF_MarkAsRootSet:%d\
\n	RF_TagGarbageTemp:%d\
\n	RF_NeedInitialization:%d\
\n	RF_NeedLoad:%d\
\n	RF_NeedPostLoad:%d\
\n	RF_NeedPostLoadSubobjects:%d\
\n	RF_NewerVersionExists:%d\
\n	RF_BeginDestroyed:%d\
\n	RF_FinishDestroyed:%d\
\n	RF_BeingRegenerated:%d\
\n	RF_DefaultSubObject:%d\
\n	RF_WasLoaded:%d\
\n	RF_TextExportTransient:%d\
\n	RF_LoadCompleted:%d\
\n	RF_InheritableComponentTemplate:%d\
\n	RF_DuplicateTransient:%d\
\n	RF_StrongRefOnFrame:%d\
\n	RF_NonPIEDuplicateTransient:%d\
\n	RF_WillBeLoaded:%d\
")
, *obj->GetPathName()
, obj->GetFlags()
, obj->HasAnyFlags(EObjectFlags::RF_Public)
, obj->HasAnyFlags(EObjectFlags::RF_Standalone)
, obj->HasAnyFlags(EObjectFlags::RF_MarkAsNative)
, obj->HasAnyFlags(EObjectFlags::RF_Transactional)
, obj->HasAnyFlags(EObjectFlags::RF_ClassDefaultObject)
, obj->HasAnyFlags(EObjectFlags::RF_ArchetypeObject)
, obj->HasAnyFlags(EObjectFlags::RF_Transient)
, obj->HasAnyFlags(EObjectFlags::RF_MarkAsRootSet)
, obj->HasAnyFlags(EObjectFlags::RF_TagGarbageTemp)
, obj->HasAnyFlags(EObjectFlags::RF_NeedInitialization)
, obj->HasAnyFlags(EObjectFlags::RF_NeedLoad)
, obj->HasAnyFlags(EObjectFlags::RF_NeedPostLoad)
, obj->HasAnyFlags(EObjectFlags::RF_NeedPostLoadSubobjects)
, obj->HasAnyFlags(EObjectFlags::RF_NewerVersionExists)
, obj->HasAnyFlags(EObjectFlags::RF_BeginDestroyed)
, obj->HasAnyFlags(EObjectFlags::RF_FinishDestroyed)
, obj->HasAnyFlags(EObjectFlags::RF_BeingRegenerated)
, obj->HasAnyFlags(EObjectFlags::RF_DefaultSubObject)
, obj->HasAnyFlags(EObjectFlags::RF_WasLoaded)
, obj->HasAnyFlags(EObjectFlags::RF_TextExportTransient)
, obj->HasAnyFlags(EObjectFlags::RF_LoadCompleted)
, obj->HasAnyFlags(EObjectFlags::RF_InheritableComponentTemplate)
, obj->HasAnyFlags(EObjectFlags::RF_DuplicateTransient)
, obj->HasAnyFlags(EObjectFlags::RF_StrongRefOnFrame)
, obj->HasAnyFlags(EObjectFlags::RF_NonPIEDuplicateTransient)
, obj->HasAnyFlags(EObjectFlags::RF_WillBeLoaded)
);
}
void LGUIUtils::LogClassFlags(UClass* cls)
{
	UE_LOG(LGUI, Log, TEXT("class:%s\
\n	flagValue:%d\
\n	CLASS_Abstract:%d\
\n	CLASS_DefaultConfig:%d\
\n	CLASS_Config:%d\
\n	CLASS_Transient:%d\
\n	CLASS_Optional:%d\
\n	CLASS_MatchedSerializers:%d\
\n	CLASS_ProjectUserConfig:%d\
\n	CLASS_Native:%d\
\n	CLASS_NotPlaceable:%d\
\n	CLASS_PerObjectConfig:%d\
\n	CLASS_ReplicationDataIsSetUp:%d\
\n	CLASS_EditInlineNew:%d\
\n	CLASS_CollapseCategories:%d\
\n	CLASS_Interface:%d\
\n	CLASS_Const:%d\
\n	CLASS_NeedsDeferredDependencyLoading:%d\
\n	CLASS_CompiledFromBlueprint:%d\
\n	CLASS_MinimalAPI:%d\
\n	CLASS_RequiredAPI:%d\
\n	CLASS_DefaultToInstanced:%d\
\n	CLASS_TokenStreamAssembled:%d\
\n	CLASS_HasInstancedReference:%d\
\n	CLASS_Hidden:%d\
\n	CLASS_Deprecated:%d\
\n	CLASS_HideDropDown:%d\
\n	CLASS_GlobalUserConfig:%d\
\n	CLASS_Intrinsic:%d\
\n	CLASS_Constructed:%d\
\n	CLASS_ConfigDoNotCheckDefaults:%d\
\n	CLASS_NewerVersionExists:%d\
")
, *cls->GetPathName()
, cls->GetClassFlags()
, cls->HasAnyClassFlags(EClassFlags::CLASS_Abstract)
, cls->HasAnyClassFlags(EClassFlags::CLASS_DefaultConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Config)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Transient)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Optional)
, cls->HasAnyClassFlags(EClassFlags::CLASS_MatchedSerializers)
, cls->HasAnyClassFlags(EClassFlags::CLASS_ProjectUserConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Native)
, cls->HasAnyClassFlags(EClassFlags::CLASS_NotPlaceable)
, cls->HasAnyClassFlags(EClassFlags::CLASS_PerObjectConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_ReplicationDataIsSetUp)
, cls->HasAnyClassFlags(EClassFlags::CLASS_EditInlineNew)
, cls->HasAnyClassFlags(EClassFlags::CLASS_CollapseCategories)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Interface)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Const)
, cls->HasAnyClassFlags(EClassFlags::CLASS_NeedsDeferredDependencyLoading)
, cls->HasAnyClassFlags(EClassFlags::CLASS_CompiledFromBlueprint)
, cls->HasAnyClassFlags(EClassFlags::CLASS_MinimalAPI)
, cls->HasAnyClassFlags(EClassFlags::CLASS_RequiredAPI)
, cls->HasAnyClassFlags(EClassFlags::CLASS_DefaultToInstanced)
, cls->HasAnyClassFlags(EClassFlags::CLASS_TokenStreamAssembled)
, cls->HasAnyClassFlags(EClassFlags::CLASS_HasInstancedReference)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Hidden)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Deprecated)
, cls->HasAnyClassFlags(EClassFlags::CLASS_HideDropDown)
, cls->HasAnyClassFlags(EClassFlags::CLASS_GlobalUserConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Intrinsic)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Constructed)
, cls->HasAnyClassFlags(EClassFlags::CLASS_ConfigDoNotCheckDefaults)
, cls->HasAnyClassFlags(EClassFlags::CLASS_NewerVersionExists)
);
}

float LGUIUtils::Color255To1_Table[256] =
{
	0,0.003921569f,0.007843138f,0.01176471f,0.01568628f,0.01960784f,0.02352941f,0.02745098f,0.03137255f,0.03529412f,0.03921569f,0.04313726f,0.04705882f,0.05098039f
	,0.05490196f,0.05882353f,0.0627451f,0.06666667f,0.07058824f,0.07450981f,0.07843138f,0.08235294f,0.08627451f,0.09019608f,0.09411765f,0.09803922f,0.1019608f,0.1058824f
	,0.1098039f,0.1137255f,0.1176471f,0.1215686f,0.1254902f,0.1294118f,0.1333333f,0.1372549f,0.1411765f,0.145098f,0.1490196f,0.1529412f,0.1568628f,0.1607843f,0.1647059f,0.1686275f
	,0.172549f,0.1764706f,0.1803922f,0.1843137f,0.1882353f,0.1921569f,0.1960784f,0.2f,0.2039216f,0.2078431f,0.2117647f,0.2156863f,0.2196078f,0.2235294f,0.227451f,0.2313726f,0.2352941f
	,0.2392157f,0.2431373f,0.2470588f,0.2509804f,0.254902f,0.2588235f,0.2627451f,0.2666667f,0.2705882f,0.2745098f,0.2784314f,0.282353f,0.2862745f,0.2901961f,0.2941177f,0.2980392f,0.3019608f
	,0.3058824f,0.3098039f,0.3137255f,0.3176471f,0.3215686f,0.3254902f,0.3294118f,0.3333333f,0.3372549f,0.3411765f,0.345098f,0.3490196f,0.3529412f,0.3568628f,0.3607843f,0.3647059f,0.3686275f
	,0.372549f,0.3764706f,0.3803922f,0.3843137f,0.3882353f,0.3921569f,0.3960784f,0.4f,0.4039216f,0.4078431f,0.4117647f,0.4156863f,0.4196078f,0.4235294f,0.427451f,0.4313726f,0.4352941f,0.4392157f
	,0.4431373f,0.4470588f,0.4509804f,0.454902f,0.4588235f,0.4627451f,0.4666667f,0.4705882f,0.4745098f,0.4784314f,0.4823529f,0.4862745f,0.4901961f,0.4941176f,0.4980392f,0.5019608f,0.5058824f,0.509804f
	,0.5137255f,0.5176471f,0.5215687f,0.5254902f,0.5294118f,0.5333334f,0.5372549f,0.5411765f,0.5450981f,0.5490196f,0.5529412f,0.5568628f,0.5607843f,0.5647059f,0.5686275f,0.572549f,0.5764706f,0.5803922f
	,0.5843138f,0.5882353f,0.5921569f,0.5960785f,0.6f,0.6039216f,0.6078432f,0.6117647f,0.6156863f,0.6196079f,0.6235294f,0.627451f,0.6313726f,0.6352941f,0.6392157f,0.6431373f,0.6470588f,0.6509804f,0.654902f
	,0.6588235f,0.6627451f,0.6666667f,0.6705883f,0.6745098f,0.6784314f,0.682353f,0.6862745f,0.6901961f,0.6941177f,0.6980392f,0.7019608f,0.7058824f,0.7098039f,0.7137255f,0.7176471f,0.7215686f,0.7254902f,0.7294118f
	,0.7333333f,0.7372549f,0.7411765f,0.7450981f,0.7490196f,0.7529412f,0.7568628f,0.7607843f,0.7647059f,0.7686275f,0.772549f,0.7764706f,0.7803922f,0.7843137f,0.7882353f,0.7921569f,0.7960784f,0.8f,0.8039216f,0.8078431f
	,0.8117647f,0.8156863f,0.8196079f,0.8235294f,0.827451f,0.8313726f,0.8352941f,0.8392157f,0.8431373f,0.8470588f,0.8509804f,0.854902f,0.8588235f,0.8627451f,0.8666667f,0.8705882f,0.8745098f,0.8784314f,0.8823529f,0.8862745f
	,0.8901961f,0.8941177f,0.8980392f,0.9019608f,0.9058824f,0.9098039f,0.9137255f,0.9176471f,0.9215686f,0.9254902f,0.9294118f,0.9333333f,0.9372549f,0.9411765f,0.945098f,0.9490196f,0.9529412f,0.9568627f,0.9607843f,0.9647059f
	,0.9686275f,0.972549f,0.9764706f,0.9803922f,0.9843137f,0.9882353f,0.9921569f,0.9960784f,1
};

TAtomic<uint32> LGUIUtils::LGUITextureNameSuffix(0);

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif