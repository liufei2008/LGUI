// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Utils/LexUIUtils.h"
#include "Sound/SoundBase.h"
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "LGUI.h"
#include "Core/LexUIMeshVertex.h"
#if WITH_EDITOR
#include "Editor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif


#define LOCTEXT_NAMESPACE "LexUIUtils"

UTexture2D* FLexUIUtils::CreateTexture(int32 InSize, FColor InDefaultColor, UObject* InOuter, FName InDefaultName)
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

TArray<uint8> FLexUIUtils::GetMD5(const FString& InString)
{
	return GetMD5((unsigned char*)TCHAR_TO_ANSI(*InString), FCString::Strlen(*InString));
}

TArray<uint8> FLexUIUtils::GetMD5(uint8* InData, uint64 InSize)
{
	FMD5 Md5Gen;
	Md5Gen.Update(InData, InSize);
	TArray<uint8> Digest;
	Digest.SetNumZeroed(16);
	Md5Gen.Final(Digest.GetData());
	return Digest;
}

FString FLexUIUtils::GetMD5String(const TArray<uint8>& InMD5Digits)
{
	FString Md5String;
	for (TArray<uint8>::TConstIterator it(InMD5Digits); it; ++it)
	{
		Md5String += FString::Printf(TEXT("%02x"), *it);
	}
	return Md5String;
}

#if WITH_EDITOR
void FLexUIUtils::NotifyPropertyChanged(UObject* Object, FProperty* Property)
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
void FLexUIUtils::NotifyPropertyChanged(UObject* Object, FName PropertyName)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	NotifyPropertyChanged(Object, Property);
}

void FLexUIUtils::ChangePropertyWithNotify(UObject* Object, FProperty* Property,
	TFunctionRef<void()> ChangePropertyFunction)
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
	PropertyChain.AddHead(Property);
	Object->PreEditChange(PropertyChain);
	ChangePropertyFunction();
	TArray<UObject*> ModifiedObjects;
	ModifiedObjects.Add(Object);
	FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet, MakeArrayView(ModifiedObjects));
	FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);
	Object->PostEditChangeChainProperty(PropertyChangedChainEvent);
}

void FLexUIUtils::ChangePropertyWithNotify(UObject* Object, FName PropertyName, TFunctionRef<void()> ChangePropertyFunction)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	ChangePropertyWithNotify(Object, Property, ChangePropertyFunction);
}

void FLexUIUtils::NotifyPropertyPreChange(UObject* Object, FProperty* Property)
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
void FLexUIUtils::NotifyPropertyPreChange(UObject* Object, FName PropertyName)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	NotifyPropertyPreChange(Object, Property);
}
#endif

FColor FLexUIUtils::ColorHSVDataToColorRGB(const FVector& InHSVColor)
{
	FLinearColor colorHSV(InHSVColor);
	return colorHSV.HSVToLinearRGB().ToFColor(false);
}
FVector FLexUIUtils::ColorRGBToColorHSVData(const FColor& InRGBColor)
{
	auto linearColorRGB = FLinearColor(ByteToFloat01(InRGBColor.R), ByteToFloat01(InRGBColor.G), ByteToFloat01(InRGBColor.B), 1.0f);
	auto linearColorHSV = linearColorRGB.LinearRGBToHSV();
	return FVector(linearColorHSV);
}

FColor FLexUIUtils::MultiplyColor(FColor A, FColor B)
{
	FColor result;
	result.R = (uint8)(A.R * ByteToFloat01(B.R));
	result.G = (uint8)(A.G * ByteToFloat01(B.G));
	result.B = (uint8)(A.B * ByteToFloat01(B.B));
	result.A = (uint8)(A.A * ByteToFloat01(B.A));
	return result;
}

UTexture2D* FLexUIUtils::GetDefaultWhiteTexture()
{
	auto defaultWhiteSolid = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/LexUIPreset_WhiteSolid"));
	if (!IsValid(defaultWhiteSolid))
	{
		auto errMsg = FText::Format(LOCTEXT("MissingDefaultContent", "{0} Load default texture error! Missing some content of LexUI plugin, reinstall this plugin may fix the issue.")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
		UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(errMsg, false, 10);
#endif
	}
	return defaultWhiteSolid;
}

#if WITH_EDITOR
//notify some information in editor
void FLexUIUtils::EditorNotification(const FText& NotifyText, bool bSuccessOrFailureSound, float ExpireDuration)
{
	if (!IsValid(GEditor))return;
	FNotificationInfo Info(NotifyText);
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = ExpireDuration;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();

	auto CompileFailSound = LoadObject<USoundBase>(NULL, bSuccessOrFailureSound ? TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileSuccess_Cue" : TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue")));
	GEditor->PlayEditorSound(CompileFailSound);
}
#endif


void FLexUIUtils::LogObjectFlags(UObject* obj)
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
void FLexUIUtils::LogClassFlags(UClass* cls)
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

void FLexUIUtils::StaticMeshToLexUIMeshRenderData(const UStaticMesh* InMesh, TArray<FLexUIMeshVertex>& OutVerts, TArray<uint16>& OutIndexes)
{
	const FStaticMeshLODResources& LOD = InMesh->GetRenderData()->LODResources[0];
	const int32 NumSections = LOD.Sections.Num();
	if (NumSections > 1)
	{
		auto WarningText = FText::Format(LOCTEXT("StaticMeshHasMultipleSections", "StaticMesh {0} has {1} sections, only the first one will be used."), FText::FromString(InMesh->GetName()), NumSections);
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(WarningText, false, 10);
#endif
		UE_LOG(LGUI, Warning, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *WarningText.ToString());
	}

	// Populate Vertex Data
	{
		const uint32 NumVerts = LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		OutVerts.Empty();
		OutVerts.Reserve(NumVerts);

		static const int32 MAX_SUPPORTED_UV_SETS = 4;
		const int32 TexCoordsPerVertex = LOD.GetNumTexCoords();
		if (TexCoordsPerVertex > MAX_SUPPORTED_UV_SETS)
		{
			auto WarningText = FText::Format(LOCTEXT("StaticMeshHasTooManyUVSets", "StaticMesh {0} has {1} UV sets; LGUI vertex data supports at most {2}."), FText::FromString(InMesh->GetName()), TexCoordsPerVertex, MAX_SUPPORTED_UV_SETS);
#if WITH_EDITOR
			FLexUIUtils::EditorNotification(WarningText, false, 10);
#endif
			UE_LOG(LGUI, Warning, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *WarningText.ToString());
		}

		for (uint32 i = 0; i < NumVerts; ++i)
		{
			// Copy Position
			const FVector3f& Position = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(i);

			// Copy Color
			FColor Color = (LOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() > 0) ? LOD.VertexBuffers.ColorVertexBuffer.VertexColor(i) : FColor::White;

			// Copy all the UVs that we have, and as many as we can fit.
			const FVector2f& UV0 = (TexCoordsPerVertex > 0) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 0) : FVector2f(1, 1);

			const FVector2f& UV1 = (TexCoordsPerVertex > 1) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 1) : FVector2f(1, 1);

			const FVector2f& UV2 = (TexCoordsPerVertex > 2) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 2) : FVector2f(1, 1);

			const FVector2f& UV3 = (TexCoordsPerVertex > 3) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 3) : FVector2f(1, 1);

			const FVector3f TangentX = FVector3f(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(i));
			const FVector3f TangentZ = FVector3f(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(i));

			OutVerts.Add(FLexUIMeshVertex(
				Position,
				TangentX,
				TangentZ,
				Color,
				UV0,
				UV1,
				UV2,
				UV3
			));
		}
	}

	// Populate Index data
	{
		FIndexArrayView SourceIndexes = LOD.IndexBuffer.GetArrayView();
		const int32 NumIndexes = SourceIndexes.Num();
		OutIndexes.Empty();
		OutIndexes.Reserve(NumIndexes);
		for (int32 i = 0; i < NumIndexes; ++i)
		{
			OutIndexes.Add(SourceIndexes[i]);
		}

		// Sort the index buffer such that verts are drawn in Z-order.
		// Assume that all triangles are coplanar with Z == SomeValue.
		ensure(NumIndexes % 3 == 0);
		for (int32 a = 0; a < NumIndexes; a += 3)
		{
			for (int32 b = 0; b < NumIndexes; b += 3)
			{
				const float VertADepth = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(OutIndexes[a]).Z;
				const float VertBDepth = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(OutIndexes[b]).Z;
				if (VertADepth < VertBDepth)
				{
					// Swap the order in which triangles will be drawn
					Swap(OutIndexes[a + 0], OutIndexes[b + 0]);
					Swap(OutIndexes[a + 1], OutIndexes[b + 1]);
					Swap(OutIndexes[a + 2], OutIndexes[b + 2]);
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

