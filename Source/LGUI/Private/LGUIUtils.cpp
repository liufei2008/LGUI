// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIDrawcall.h"
#include "Sound/SoundBase.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
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


//find first Canvas in hierarchy
void LGUIUtils::FindRootCanvas(AActor* actor, ULGUICanvas*& resultCanvas)
{
	if (!IsValid(actor))return;
	auto tempComp = GetComponentInParent<ULGUICanvas>(actor, false);
	if (tempComp != nullptr && tempComp->IsRegistered())
	{
		resultCanvas = tempComp;
	}

	auto parentActor = actor->GetAttachParentActor();
	if (parentActor != nullptr)
	{
		FindRootCanvas(parentActor, resultCanvas);
	}
}

void LGUIUtils::FindParentCanvas(AActor* actor, ULGUICanvas*& resultCanvas)
{
	if (!IsValid(actor))return;
	auto parentActor = actor->GetAttachParentActor();
	if (parentActor != nullptr)
	{
		auto tempComp = GetComponentInParent<ULGUICanvas>(parentActor, false);
		if (tempComp != nullptr)
		{
			resultCanvas = tempComp;
		}
		return;
	}
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

FColor LGUIUtils::ColorHSVDataToColorRGB(FVector InHSVColor)
{
	FLinearColor colorHSV(InHSVColor);
	return colorHSV.HSVToLinearRGB().ToFColor(false);
}
FVector LGUIUtils::ColorRGBToColorHSVData(FColor InRGBColor)
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
	EObjectFlags of = obj->GetFlags();
	UE_LOG(LGUI, Log, TEXT("obj:%s\
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
\n	RF_KeepForCooker:%d\
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
, obj->HasAnyFlags(EObjectFlags::RF_KeepForCooker)
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

float LGUIUtils::Color255To1_Table[256] =
{
	0,0.003921569,0.007843138,0.01176471,0.01568628,0.01960784,0.02352941,0.02745098,0.03137255,0.03529412,0.03921569,0.04313726,0.04705882,0.05098039
	,0.05490196,0.05882353,0.0627451,0.06666667,0.07058824,0.07450981,0.07843138,0.08235294,0.08627451,0.09019608,0.09411765,0.09803922,0.1019608,0.1058824
	,0.1098039,0.1137255,0.1176471,0.1215686,0.1254902,0.1294118,0.1333333,0.1372549,0.1411765,0.145098,0.1490196,0.1529412,0.1568628,0.1607843,0.1647059,0.1686275
	,0.172549,0.1764706,0.1803922,0.1843137,0.1882353,0.1921569,0.1960784,0.2,0.2039216,0.2078431,0.2117647,0.2156863,0.2196078,0.2235294,0.227451,0.2313726,0.2352941
	,0.2392157,0.2431373,0.2470588,0.2509804,0.254902,0.2588235,0.2627451,0.2666667,0.2705882,0.2745098,0.2784314,0.282353,0.2862745,0.2901961,0.2941177,0.2980392,0.3019608
	,0.3058824,0.3098039,0.3137255,0.3176471,0.3215686,0.3254902,0.3294118,0.3333333,0.3372549,0.3411765,0.345098,0.3490196,0.3529412,0.3568628,0.3607843,0.3647059,0.3686275
	,0.372549,0.3764706,0.3803922,0.3843137,0.3882353,0.3921569,0.3960784,0.4,0.4039216,0.4078431,0.4117647,0.4156863,0.4196078,0.4235294,0.427451,0.4313726,0.4352941,0.4392157
	,0.4431373,0.4470588,0.4509804,0.454902,0.4588235,0.4627451,0.4666667,0.4705882,0.4745098,0.4784314,0.4823529,0.4862745,0.4901961,0.4941176,0.4980392,0.5019608,0.5058824,0.509804
	,0.5137255,0.5176471,0.5215687,0.5254902,0.5294118,0.5333334,0.5372549,0.5411765,0.5450981,0.5490196,0.5529412,0.5568628,0.5607843,0.5647059,0.5686275,0.572549,0.5764706,0.5803922
	,0.5843138,0.5882353,0.5921569,0.5960785,0.6,0.6039216,0.6078432,0.6117647,0.6156863,0.6196079,0.6235294,0.627451,0.6313726,0.6352941,0.6392157,0.6431373,0.6470588,0.6509804,0.654902
	,0.6588235,0.6627451,0.6666667,0.6705883,0.6745098,0.6784314,0.682353,0.6862745,0.6901961,0.6941177,0.6980392,0.7019608,0.7058824,0.7098039,0.7137255,0.7176471,0.7215686,0.7254902,0.7294118
	,0.7333333,0.7372549,0.7411765,0.7450981,0.7490196,0.7529412,0.7568628,0.7607843,0.7647059,0.7686275,0.772549,0.7764706,0.7803922,0.7843137,0.7882353,0.7921569,0.7960784,0.8,0.8039216,0.8078431
	,0.8117647,0.8156863,0.8196079,0.8235294,0.827451,0.8313726,0.8352941,0.8392157,0.8431373,0.8470588,0.8509804,0.854902,0.8588235,0.8627451,0.8666667,0.8705882,0.8745098,0.8784314,0.8823529,0.8862745
	,0.8901961,0.8941177,0.8980392,0.9019608,0.9058824,0.9098039,0.9137255,0.9176471,0.9215686,0.9254902,0.9294118,0.9333333,0.9372549,0.9411765,0.945098,0.9490196,0.9529412,0.9568627,0.9607843,0.9647059
	,0.9686275,0.972549,0.9764706,0.9803922,0.9843137,0.9882353,0.9921569,0.9960784,1
};

TAtomic<uint32> LGUIUtils::LGUITextureNameSuffix(0);

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif