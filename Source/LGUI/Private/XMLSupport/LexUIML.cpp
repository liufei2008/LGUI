// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "XMLSupport/LexUIML.h"
#include "Core/Components/LexWidget.h"
#include "Core/Components/LexImage.h"
#include "Core/Components/LexText.h"
#include "Core/Components/LexTexture.h"
#include "Core/Components/LexSprite.h"
#include "Core/LexUIImageBrush.h"
#include "XmlFile.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIFontData_BaseObject.h"
#include "Core/LexUIManager.h"
#include "Core/LexUISpriteData.h"
#include "Core/LexUIAnchorData.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "Misc/FileHelper.h"
#include "PrefabSystem/WidgetSerializer.h"
#include "UObject/UObjectGlobals.h"
#include "XMLSupport/LexUIMLBehaviour.h"
#include "Core/Components/LexLayoutContainerFlexBox.h"
#include "Core/Components/LexLayoutContainerGrid.h"
#include "Core/Components/LexLayoutSelfFlexBox.h"
#include "Core/Components/LexLayoutSelfGrid.h"
#include "Core/Components/LexLayoutSelfAspectRatio.h"
#include "Core/Components/LexLayout.h"

// ============================================================================
// ULexUIXAMLResource
// ============================================================================

UTexture* ULexUIMLResource::GetTexture(const FString& Key) const
{
	if (const TObjectPtr<UTexture>* Found = Textures.Find(Key))
	{
		return Found->Get();
	}
	return nullptr;
}

ULexUISpriteData_BaseObject* ULexUIMLResource::GetSprite(const FString& Key) const
{
	if (const TObjectPtr<ULexUISpriteData_BaseObject>* Found = Sprites.Find(Key))
	{
		return Found->Get();
	}
	return ULexUISpriteData::GetDefaultWhiteSolid();
}

UMaterialInterface* ULexUIMLResource::GetMaterial(const FString& Key) const
{
	if (const TObjectPtr<UMaterialInterface>* Found = Materials.Find(Key))
	{
		return Found->Get();
	}
	return nullptr;
}

bool ULexUIMLResource::GetImageBrush(const FString& Key, FLexUIImageBrush& OutResult) const
{
	if (const FLexUIImageBrush* Found = ImageBrushes.Find(Key))
	{
		OutResult = *Found;
		return true;
	}
	return false;
}

ULexUIFontData_BaseObject* ULexUIMLResource::GetFont(const FString& Key) const
{
	if (const TObjectPtr<ULexUIFontData_BaseObject>* Found = Fonts.Find(Key))
	{
		return Found->Get();
	}
	return ULexUIFontData_BaseObject::GetDefaultFont();
}

ULexUIPrefab* ULexUIMLResource::GetPrefab(const FString& Key) const
{
	if (const TObjectPtr<ULexUIPrefab>* Found = Prefabs.Find(Key))
	{
		return Found->Get();
	}
	return nullptr;
}

TSubclassOf<ULexUIMLBehaviour> ULexUIMLResource::GetTemplate(const FString& Key) const
{
	if (const TSubclassOf<ULexUIMLBehaviour>* Found = Templates.Find(Key))
	{
		return *Found;
	}
	return nullptr;
}

// ============================================================================
// ULexUIMLEventBinding
// ============================================================================

void ULexUIMLEventBinding::Execute()
{
	UObject* Context = Target.Get();
	if (!Context) return;

	UFunction* Func = Context->FindFunction(FunctionName);
	if (!Func) return;

	if (ParamString.IsEmpty())
	{
		Context->ProcessEvent(Func, nullptr);
		return;
	}

	// Lazy-cache parameter buffer (rebuild if Behaviour map may have changed)
	if (!bParamsCached)
	{
		CachedParams.SetNumZeroed(Func->ParmsSize);

		TArray<FProperty*> ParamProps;
		for (TFieldIterator<FProperty> It(Func); It; ++It)
		{
			if (It->HasAnyPropertyFlags(CPF_Parm) && !It->HasAnyPropertyFlags(CPF_ReturnParm))
				ParamProps.Add(*It);
		}

		TArray<FString> Parts;
		ParamString.ParseIntoArray(Parts, TEXT(","));
		for (int32 i = 0; i < Parts.Num() && i < ParamProps.Num(); ++i)
		{
			FString Part = Parts[i].TrimStartAndEnd();

			// Resolve IdName:xxx at runtime using Behaviour's map
			if (Part.StartsWith(TEXT("IdName:")))
			{
				if (auto* Found = DataContainer->MapIdNameToObject.Find(Part.Mid(7)))
				{
					if (UObject* Obj = Found->Get())
					{
						Part = Obj->GetPathName();
					}
				}
			}

			FLexUIMLUtils::SetPropertyValueFromString(
				ParamProps[i],
				ParamProps[i]->ContainerPtrToValuePtr<void>(CachedParams.GetData()),
				Part, nullptr);
		}
		bParamsCached = true;
	}

	Context->ProcessEvent(Func, CachedParams.GetData());
}

// ============================================================================
// FLexUIXAML
// ============================================================================


static bool IsWidgetElement(const FString& Tag, UClass*& Class)
{
	if (Tag == TEXT("Widget"))
	{
		Class = nullptr;
		return true;
	}
	if (Tag == TEXT("Image"))
	{
		Class = ULexImage::StaticClass();
		return true;
	}
	if (Tag == TEXT("Text"))
	{
		Class = ULexText::StaticClass();
		return true;
	}
	if (Tag == TEXT("Texture"))
	{
		Class = ULexTexture::StaticClass();
		return true;
	}
	if (Tag == TEXT("Sprite"))
	{
		Class = ULexSprite::StaticClass();
		return true;
	}
	return false;
}

/** Check if the tag is a known Prefab name in Resources. */
static bool IsPrefabElement(const FString& Tag)
{
	return Tag == TEXT("Prefab");
}

/** Check if the tag is a known Template name. */
static bool IsTemplateElement(const FString& Tag)
{
	return Tag == TEXT("Template");
}

/** Check if the tag is a Slot element. */
static bool IsSlotElement(const FString& Tag)
{
	return Tag == TEXT("Slot");
}

/** Get the Src attribute value from a Prefab/Template XML node. */
static FString GetElementSrc(const FXmlNode* Node)
{
	return Node->GetAttribute(TEXT("Src")).TrimStartAndEnd();
}

/** Get the Name attribute value from a Slot XML node (empty = default slot). Falls back to Src. */
static FString GetSlotName(const FXmlNode* Node)
{
	FString Name = Node->GetAttribute(TEXT("Name")).TrimStartAndEnd();
	if (Name.IsEmpty()) Name = Node->GetAttribute(TEXT("Src")).TrimStartAndEnd();
	return Name;
}

/** Resolve a LayoutContainer class name to its UClass. */
static UClass* ResolveLayoutContainerClass(const FString& Name)
{
	if (Name == TEXT("FlexBox"))        return ULexLayoutContainerFlexBox::StaticClass();
	if (Name == TEXT("Grid"))           return ULexLayoutContainerGrid::StaticClass();
	return nullptr;
}

/** Resolve a LayoutSelf class name to its UClass. */
static UClass* ResolveLayoutSelfClass(const FString& Name)
{
	if (Name == TEXT("IgnoreLayoutContainer")) return ULexLayoutSelf::StaticClass();
	if (Name == TEXT("FlexBox"))               return ULexLayoutSelfFlexBox::StaticClass();
	if (Name == TEXT("Grid"))                  return ULexLayoutSelfGrid::StaticClass();
	if (Name == TEXT("AspectRatio"))           return ULexLayoutSelfAspectRatio::StaticClass();
	return nullptr;
}

/** Handle LayoutContainer/LayoutSelf attributes. Returns true if matched (caller should continue). */
static bool TryApplyLayoutAttribute(const FString& AttrName, const FString& AttrValue, ULexWidget* Widget,
	TArray<TPair<FString, FString>>& OutDeferredContainer, TArray<TPair<FString, FString>>& OutDeferredSelf)
{
	if (AttrName == TEXT("LayoutContainer"))
	{
		if (UClass* LayoutClass = ResolveLayoutContainerClass(AttrValue))
			Widget->CreateNewLayoutContainer(LayoutClass);
		return true;
	}
	if (AttrName.StartsWith(TEXT("LayoutContainer.")))
	{
		OutDeferredContainer.Add(TPair<FString, FString>(AttrName.Mid(16), AttrValue));
		return true;
	}
	if (AttrName == TEXT("LayoutSelf"))
	{
		if (AttrValue == TEXT("IgnoreLayoutContainer"))
		{
			Widget->SetIgnoreLayout(true);
		}
		else if (UClass* LayoutClass = ResolveLayoutSelfClass(AttrValue))
			Widget->CreateNewLayoutSelf(LayoutClass);
		return true;
	}
	if (AttrName.StartsWith(TEXT("LayoutSelf.")))
	{
		OutDeferredSelf.Add(TPair<FString, FString>(AttrName.Mid(11), AttrValue));
		return true;
	}
	return false;
}

void FLexUIMLUtils::BindVarName(ULexUIMLBehaviour* EventContext, const FString& VarName, ULexWidget* Widget, ULexVisual* Visual) const
{
	if (!EventContext || VarName.IsEmpty()) return;

	FProperty* Prop = FindFProperty<FProperty>(EventContext->GetClass(), *VarName);
	if (!Prop)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s].%d - VarName '%s': property not found on %s"),
			ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *VarName, *EventContext->GetClass()->GetName());
		return;
	}

	FObjectPropertyBase* ObjProp = CastField<FObjectPropertyBase>(Prop);
	if (!ObjProp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s].%d - VarName '%s': property '%s' is not an object reference (type: %s)"),
			ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *VarName, *VarName, *Prop->GetClass()->GetName());
		return;
	}

	UClass* PropClass = ObjProp->PropertyClass;
	UObject* ValueToSet = nullptr;

	// Try matching the widget first
	if (Widget->IsA(PropClass))
	{
		ValueToSet = Widget;
	}
	// Then try the visual
	else if (Visual && Visual->IsA(PropClass))
	{
		ValueToSet = Visual;
	}

	if (ValueToSet)
	{
		ObjProp->SetObjectPropertyValue_InContainer(EventContext, ValueToSet);
		UE_LOG(LogTemp, Log, TEXT("[%s].%d - VarName '%s' → %s (%s)"),
			ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *VarName, *ValueToSet->GetName(), *PropClass->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s].%d - VarName '%s': type mismatch — property expects '%s', but widget is '%s' and visual is '%s'"),
			ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *VarName,
			*PropClass->GetName(),
			*Widget->GetClass()->GetName(),
			Visual ? *Visual->GetClass()->GetName() : TEXT("none"));
	}
}

FLexUIMLUtils::FLexUIMLUtils(bool InIsSubTemplate, TFunction<void(const TArray<ULexWidget*>&)> InAllWidgetsCreated)
{
	bIsSubTemplate = InIsSubTemplate;
	OnAllWidgetsCreated = InAllWidgetsCreated;
}

ULexUIMLBehaviour* FLexUIMLUtils::LoadFromFile(UWorld* InWorld, ULexWidget* Parent, TSubclassOf<ULexUIMLBehaviour> Class, ULexUIMLResource* InResources, const FString& FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - XML file not found: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *FilePath);
		return nullptr;
	}

	FString XmlString;
	if (!FFileHelper::LoadFileToString(XmlString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Failed to read XML file: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *FilePath);
		return nullptr;
	}

	return LoadFromString(InWorld, Parent, Class, InResources, XmlString);
}

ULexUIMLBehaviour* FLexUIMLUtils::LoadFromString(UWorld* InWorld, ULexWidget* Parent, TSubclassOf<ULexUIMLBehaviour> Class, ULexUIMLResource* InResources, const FString& XmlString)
{
	if (XmlString.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - XML string is empty."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}

	FXmlFile XmlFile;
	if (!XmlFile.LoadFile(XmlString, EConstructMethod::ConstructFromBuffer))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - XML parse error: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *XmlFile.GetLastError());
		return nullptr;
	}

	const FXmlNode* RootNode = XmlFile.GetRootNode();
	if (!RootNode)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - XML has no root node."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}

	this->World = InWorld;
	this->Resources = InResources;
	this->DataContainer = MakeShared<FLexUIML_DataContainer>();

	UClass* ScriptClass = Class.Get();

	// --- Support <LexUIML> root wrapper ---
	const FXmlNode* ContentRoot = RootNode;
	if (RootNode->GetTag() == TEXT("LexUIML"))
	{
		ParsePropertyGroups(RootNode->GetChildrenNodes());
		// Find the first non-PropertyGroup child as the content root
		for (const FXmlNode* Child : RootNode->GetChildrenNodes())
		{
			if (Child->GetTag() != TEXT("PropertyGroup")
				&& Child->GetTag() != TEXT("Include")
				)
			{
				ContentRoot = Child;
				break;
			}
		}
		if (ContentRoot == RootNode)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s].%d - <LexUIML> has no content root"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
	}

	ULexUIMLBehaviour* RootBehaviour = nullptr;
	UClass* VisualClass = nullptr;
	if (IsPrefabElement(ContentRoot->GetTag()))
	{
		if (!Resources)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s].%d - Resource is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		const FString PrefabName = GetElementSrc(ContentRoot);
		if (auto Prefab = Resources->GetPrefab(PrefabName))
		{
			RootBehaviour = ParsePrefabElement(ContentRoot, Prefab, Parent, nullptr, ScriptClass);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s].%d - Prefab '%s' not found"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *PrefabName);
			return nullptr;
		}
	}
	else if (IsTemplateElement(ContentRoot->GetTag()))
	{
		if (!Resources)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s].%d - Resource is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		const FString TemplateName = GetElementSrc(ContentRoot);
		if (auto TemplateClass = Resources->GetTemplate(TemplateName))
		{
			RootBehaviour = ParseTemplateElement(ContentRoot, TemplateClass, Parent, nullptr, ScriptClass);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s].%d - Template '%s' not found"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *TemplateName);
			return nullptr;
		}
	}
	else if (IsWidgetElement(ContentRoot->GetTag(), VisualClass))
	{
		RootBehaviour = ParseWidgetElement(ContentRoot, VisualClass, Parent, nullptr, ScriptClass);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Expected root widget element, got <%s>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *ContentRoot->GetTag());
	}

	if (!RootBehaviour)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Failed to parse root widget element."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}

	for (auto& EventBinding : EventBindings)
	{
		EventBinding->DataContainer = this->DataContainer;
	}

	if (!bIsSubTemplate)
	{
		for (int i = 0; i < AllWidgets.Num(); i++)
		{
			auto& Widget = AllWidgets[i];
			if (!Widget->HasRegistered())
			{
				Widget->OnRegister();
			}
		}
		if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(InWorld))
		{
			if (LexUIManager->HasBegunPlay())
			{
				for (int i = 0; i < AllWidgets.Num(); i++)
				{
					auto& Widget = AllWidgets[i];
					if (!Widget->HasBegunPlay())
					{
						Widget->BeginPlay();
					}
				}
			}
		}
	}
	return RootBehaviour;
}

/**
 * Bind XML event attributes (e.g. OnClick="FuncName,Param1,Param2") to the widget's components.
 * Validates function existence, parameter count, and parameter type before binding.
 */
void FLexUIMLUtils::BindXMLEvents(ULexWidget* Widget, const FXmlNode* XmlNode, UObject* EventContext)
{
	if (!Widget || !EventContext) return;

	const TArray<ULexUIBehaviour*>& Components = Widget->GetAllComponents();

	for (const auto& Attr : XmlNode->GetAttributes())
	{
		const FString& AttrName = Attr.GetTag();
		if (!AttrName.StartsWith(TEXT("Event:"))) continue;

		// Strip "Event:" prefix to get event name (e.g. "Event:OnClick" → "OnClick")
		const FString EventName = AttrName.Mid(6);

		// Parse "FuncName,Param1,Param2,..."
		FString FuncName;
		FString ParamString;
		{
			const FString& Raw = Attr.GetValue();
			int32 CommaIdx;
			if (Raw.FindChar(TEXT(','), CommaIdx))
			{
				FuncName = Raw.Left(CommaIdx).TrimStartAndEnd();
				ParamString = Raw.Mid(CommaIdx + 1).TrimStartAndEnd();
			}
			else
			{
				FuncName = Raw.TrimStartAndEnd();
			}
		}

		UFunction* TargetFunc = EventContext->FindFunction(*FuncName);
		if (!TargetFunc)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s].%d - %s='%s' but function '%s' not found on %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__,
				*AttrName, *FuncName, *FuncName, *EventContext->GetName());
			continue;
		}

		// Collect target function param properties
		TArray<FProperty*> TargetParams;
		for (TFieldIterator<FProperty> Pit(TargetFunc); Pit; ++Pit)
		{
			if (Pit->HasAnyPropertyFlags(CPF_Parm) && !Pit->HasAnyPropertyFlags(CPF_ReturnParm))
				TargetParams.Add(*Pit);
		}

		for (ULexUIBehaviour* Comp : Components)
		{
			for (TFieldIterator<FMulticastDelegateProperty> It(Comp->GetClass()); It; ++It)
			{
				const FString DelegateName = It->GetName();
				const bool bMatch = (DelegateName == EventName)
					|| (DelegateName.StartsWith(EventName) && DelegateName.EndsWith(TEXT("BP")));

				if (!bMatch) continue;

				// Collect delegate signature param properties
				TArray<FProperty*> DelegateParams;
				UFunction* DelegateSigFunc = It->SignatureFunction;
				if (DelegateSigFunc)
				{
					for (TFieldIterator<FProperty> Pit(DelegateSigFunc); Pit; ++Pit)
					{
						if (Pit->HasAnyPropertyFlags(CPF_Parm) && !Pit->HasAnyPropertyFlags(CPF_ReturnParm))
							DelegateParams.Add(*Pit);
					}
				}

				// Check type compatibility for directly matching param count
				bool bTypesCompatible = (TargetParams.Num() == DelegateParams.Num());
				if (bTypesCompatible && TargetParams.Num() > 0)
				{
					for (int32 i = 0; i < TargetParams.Num(); ++i)
					{
						if (!TargetParams[i]->SameType(DelegateParams[i]))
						{
							bTypesCompatible = false;
							break;
						}
					}
				}

				const bool bNeedWrapper = (!ParamString.IsEmpty())
					|| (TargetParams.Num() > DelegateParams.Num());

				if (bNeedWrapper)
				{
					auto Binding = NewObject<ULexUIMLEventBinding>(Widget);
					Binding->Target = EventContext;
					Binding->FunctionName = *FuncName;
					Binding->ParamString = ParamString;
					EventBindings.Add(Binding);

					FScriptDelegate Delegate;
					Delegate.BindUFunction(Binding, GET_FUNCTION_NAME_CHECKED(ULexUIMLEventBinding, Execute));
					It->AddDelegate(Delegate, Comp);

					UE_LOG(LogTemp, Log, TEXT("[%s].%d - Bound %s.%s → %s::%s (wrapper, param='%s', dlgParams=%d, tgtParams=%d)"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__,
						*Comp->GetName(), *DelegateName,
						*EventContext->GetName(), *FuncName,
						*ParamString, DelegateParams.Num(), TargetParams.Num());
				}
				else if (!bTypesCompatible)
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s].%d - %s.%s ↔ %s::%s param types mismatch, binding skipped."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__,
						*Comp->GetName(), *DelegateName,
						*EventContext->GetName(), *FuncName);
				}
				else
				{
					FScriptDelegate Delegate;
					Delegate.BindUFunction(EventContext, *FuncName);
					It->AddDelegate(Delegate, Comp);

					UE_LOG(LogTemp, Log, TEXT("[%s].%d - Bound %s.%s → %s::%s (direct, params=%d)"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__,
						*Comp->GetName(), *DelegateName,
						*EventContext->GetName(), *FuncName,
						TargetParams.Num());
				}
				break;
			}
		}
	}
}

/** Check if AttrName is an AnchorData field and apply to the local buffer. Returns true if matched. */
static bool TryApplyAnchorDataField(const FString& AttrName, const FString& AttrValue, FLexUIAnchorData& AnchorData, bool& bChanged, UScriptStruct* AnchorDataStruct, UObject* Owner)
{
	static const TCHAR* Fields[] = {
		TEXT("Pivot"), TEXT("AnchorMin"), TEXT("AnchorMax"),
		TEXT("AnchoredPosition"), TEXT("SizeDelta"),
	};
	for (const TCHAR* Field : Fields)
	{
		if (AttrName == Field)
		{
			FProperty* SubProp = FindFProperty<FProperty>(AnchorDataStruct, Field);
			if (SubProp)
			{
				void* SubValuePtr = SubProp->ContainerPtrToValuePtr<void>(&AnchorData);
				FLexUIMLUtils::SetPropertyValueFromString(SubProp, SubValuePtr, AttrValue, Owner);
				bChanged = true;
			}
			return true;
		}
	}
	return false;
}

/**
 * Parse a prefab-tag node: instantiate the ULexUIPrefab,
 * then apply attributes and children from the XML node.
 */
ULexUIMLBehaviour* FLexUIMLUtils::ParsePrefabElement(const FXmlNode* PrefabNode, ULexUIPrefab* Prefab, ULexWidget* ParentWidget, ULexUIMLBehaviour* EventContext, UClass* ScriptClass)
{
	const FString& Tag = PrefabNode->GetTag();

	// Instantiate prefab
	TMap<FGuid, TObjectPtr<UObject>> SubPrefabMapGuidToObject;
	ULexWidget* NewWidget =
		LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::WidgetSerializer::LoadSubPrefab(World, World, Prefab
		, ParentWidget, SubPrefabMapGuidToObject
		, [&](ULexWidget*, const TMap<FGuid, TObjectPtr<UObject>>&, const TMap<TObjectPtr<UObject>, FGuid>&, const TArray<ULexWidget*>& InSubWidgets)
		{
			this->AllWidgets.Append(InSubWidgets);
		});
	if (!NewWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Failed to instantiate Prefab '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *Tag);
		return nullptr;
	}

	// --- Apply attributes from XML ---
	FLexUIAnchorData AnchorData;
	UScriptStruct* AnchorDataStruct = FLexUIAnchorData::StaticStruct();
	bool bAnchorDataChanged = false;

	TArray<TPair<FString, FString>> DeferredLayoutContainerProps;
	TArray<TPair<FString, FString>> DeferredLayoutSelfProps;

	// Build combined attribute map: Style first, node attrs override
	TMap<FString, FString> CombinedAttrs;
	ApplyStyleAttributes(PrefabNode->GetAttribute(TEXT("Style")), CombinedAttrs);
	for (const auto& Attr : PrefabNode->GetAttributes())
	{
		if (Attr.GetTag() == TEXT("Style")) continue;
		CombinedAttrs.Add(Attr.GetTag(), Attr.GetValue());
	}

	for (const auto& Pair : CombinedAttrs)
	{
		const FString& AttrName = Pair.Key;
		const FString& AttrValue = Pair.Value;

		if (AttrName == TEXT("DisplayName"))
		{
			NewWidget->SetDisplayName(AttrValue);
			continue;
		}
		if (AttrName == TEXT("VarName"))
		{
			// --- Bind VarName to script property ---
			BindVarName(EventContext, AttrValue, NewWidget, NewWidget->GetVisual());
			continue;
		}

		if (TryApplyLayoutAttribute(AttrName, AttrValue, NewWidget, DeferredLayoutContainerProps, DeferredLayoutSelfProps)) continue;

		if (TryApplyAnchorDataField(AttrName, AttrValue, AnchorData, bAnchorDataChanged, AnchorDataStruct, NewWidget)) continue;

		// Try widget property, then visual
		if (!ApplyPropertyValue(NewWidget, AttrName, AttrValue))
		{
			ULexVisual* Vis = NewWidget->GetVisual();
			if (!Vis || !ApplyPropertyValue(Vis, AttrName, AttrValue))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Unknown property '%s' on Prefab <%s>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__,
					*AttrName, *Tag);
			}
		}
	}

	if (bAnchorDataChanged)
	{
		NewWidget->SetAnchorData(AnchorData);
	}

	ApplyDeferredLayoutProps(NewWidget, DeferredLayoutContainerProps, DeferredLayoutSelfProps);

	// --- Script behaviour (root widget only) ---
	if (ScriptClass && !EventContext)
	{
		EventContext = Cast<ULexUIMLBehaviour>(NewWidget->AddComponent(ScriptClass));
		if (EventContext)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s].%d - Added script behaviour '%s' to root widget"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *ScriptClass->GetName());
		}
	}

	// --- Bind XML events (OnClick="FuncName", etc.) ---
	BindXMLEvents(NewWidget, PrefabNode, EventContext);

	// Prefabs do not allow child elements
	if (PrefabNode->GetChildrenNodes().Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s].%d - <Prefab:XXX> does not allow child elements"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
	return EventContext;
}

ULexUIMLBehaviour* FLexUIMLUtils::ParseTemplateElement(const FXmlNode* TemplateNode, TSubclassOf<ULexUIMLBehaviour> TemplateClass, ULexWidget* ParentWidget, ULexUIMLBehaviour* EventContext, UClass* ScriptClass)
{
	const FString& Tag = TemplateNode->GetTag();

	// Load the template's UIML — this instantiates the widget tree as children of ParentWidget
	ULexUIMLBehaviour* TemplateBehaviour = ULexUIMLBehaviour::CreateByClass(TemplateClass, World, ParentWidget, Resources, true
		, [=, this](const TArray<ULexWidget*>& SubTemplateAllWidgets)
		{
			this->AllWidgets.Append(SubTemplateAllWidgets);
		});
	if (!TemplateBehaviour)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Failed to instantiate Template '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *Tag);
		return nullptr;
	}

	ULexWidget* RootWidget = TemplateBehaviour->GetWidget();
	if (!RootWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Template '%s' has no root widget"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *Tag);
		return nullptr;
	}

	// --- Apply attributes from XML ---
	FLexUIAnchorData AnchorData;
	UScriptStruct* AnchorDataStruct = FLexUIAnchorData::StaticStruct();
	bool bAnchorDataChanged = false;

	TArray<TPair<FString, FString>> DeferredLayoutContainerProps;
	TArray<TPair<FString, FString>> DeferredLayoutSelfProps;

	// Build combined attribute map: Style first, node attrs override
	TMap<FString, FString> CombinedAttrs;
	ApplyStyleAttributes(TemplateNode->GetAttribute(TEXT("Style")), CombinedAttrs);
	for (const auto& Attr : TemplateNode->GetAttributes())
	{
		if (Attr.GetTag() == TEXT("Style")) continue;
		CombinedAttrs.Add(Attr.GetTag(), Attr.GetValue());
	}

	for (const auto& Pair : CombinedAttrs)
	{
		const FString& AttrName = Pair.Key;
		const FString& AttrValue = Pair.Value;

		if (AttrName == TEXT("DisplayName"))
		{
			RootWidget->SetDisplayName(AttrValue);
			continue;
		}
		if (AttrName == TEXT("VarName"))
		{
			// --- Bind VarName to script property ---
			BindVarName(EventContext, AttrValue, RootWidget, RootWidget->GetVisual());
			continue;
		}

		if (TryApplyLayoutAttribute(AttrName, AttrValue, RootWidget, DeferredLayoutContainerProps, DeferredLayoutSelfProps)) continue;

		if (TryApplyAnchorDataField(AttrName, AttrValue, AnchorData, bAnchorDataChanged, AnchorDataStruct, RootWidget)) continue;

		// Try widget property, then visual
		if (!ApplyPropertyValue(RootWidget, AttrName, AttrValue))
		{
			ULexVisual* Vis = RootWidget->GetVisual();
			if (!Vis || !ApplyPropertyValue(Vis, AttrName, AttrValue))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Unknown property '%s' on Template <%s>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__,
					*AttrName, *Tag);
			}
		}
	}

	if (bAnchorDataChanged)
	{
		RootWidget->SetAnchorData(AnchorData);
	}

	ApplyDeferredLayoutProps(RootWidget, DeferredLayoutContainerProps, DeferredLayoutSelfProps);
	
	// --- Bind XML events (OnClick="FuncName", etc.) ---
	BindXMLEvents(RootWidget, TemplateNode, EventContext);

	// --- Process child elements (slot-based) ---
	if (!ProcessTemplateChildElements(TemplateNode->GetChildrenNodes(), TemplateBehaviour, EventContext, ScriptClass))
	{
		return nullptr;
	}
	return EventContext;
}

ULexUIMLBehaviour* FLexUIMLUtils::ParseWidgetElement(const FXmlNode* WidgetNode, UClass* VisualClass, ULexWidget* ParentWidget, ULexUIMLBehaviour* EventContext, UClass* ScriptClass)
{
	const FString& Tag = WidgetNode->GetTag();

	// Create widget
	ULexWidget* NewWidget = NewObject<ULexWidget>(World, ULexWidget::StaticClass(), NAME_None, RF_Transactional);
	if (!NewWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Failed to create widget for <%s>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *Tag);
		return nullptr;
	}
	AllWidgets.Add(NewWidget);

	// Create typed Visual if needed
	ULexVisual* CreatedVisual = nullptr;
	if (VisualClass)
	{
		CreatedVisual = NewWidget->CreateNewVisual(VisualClass);
	}

	// --- Apply attributes ---
	// Collect AnchorData changes into a local buffer, apply once at the end.
	FLexUIAnchorData AnchorData;
	UScriptStruct* AnchorDataStruct = FLexUIAnchorData::StaticStruct();
	bool bAnchorDataChanged = false;

	// Deferred Layout sub-properties (may appear before LayoutContainer/LayoutSelf creation)
	TArray<TPair<FString, FString>> DeferredLayoutContainerProps;
	TArray<TPair<FString, FString>> DeferredLayoutSelfProps;

	// Build combined attribute map: Style first, node attrs override
	TMap<FString, FString> CombinedAttrs;
	ApplyStyleAttributes(WidgetNode->GetAttribute(TEXT("Style")), CombinedAttrs);
	for (const auto& Attr : WidgetNode->GetAttributes())
	{
		if (Attr.GetTag() == TEXT("Style")) continue;
		CombinedAttrs.Add(Attr.GetTag(), Attr.GetValue());
	}

	for (const auto& Pair : CombinedAttrs)
	{
		const FString& AttrName = Pair.Key;
		const FString& AttrValue = Pair.Value;

		if (AttrName == TEXT("DisplayName"))
		{
			NewWidget->SetDisplayName(AttrValue);
			continue;
		}
		if (AttrName == TEXT("IdName"))
		{
			NewWidget->SetDisplayName(AttrValue);
			DataContainer->MapIdNameToObject.Add(AttrValue, NewWidget);
			continue;
		}
		if (AttrName == TEXT("ImageBrush") || AttrName == TEXT("Font") || AttrName == TEXT("Texture") || AttrName == TEXT("Sprite") || AttrName == TEXT("VarName")) continue;

		if (TryApplyLayoutAttribute(AttrName, AttrValue, NewWidget, DeferredLayoutContainerProps, DeferredLayoutSelfProps)) continue;

		if (TryApplyAnchorDataField(AttrName, AttrValue, AnchorData, bAnchorDataChanged, AnchorDataStruct, NewWidget)) continue;

		// Try direct / dotted property (e.g. RenderOpacity, RelativeLocation.X)
		if (!ApplyPropertyValue(NewWidget, AttrName, AttrValue))
		{
			// Also try on the Visual if one was created
			if (!CreatedVisual || !ApplyPropertyValue(CreatedVisual, AttrName, AttrValue))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Unknown property '%s' on <%s>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__,
					*AttrName, *Tag);
			}
		}
	}

	// Apply accumulated AnchorData once
	if (bAnchorDataChanged)
	{
		NewWidget->SetAnchorData(AnchorData);
	}

	ApplyDeferredLayoutProps(NewWidget, DeferredLayoutContainerProps, DeferredLayoutSelfProps);

	// --- Apply resource to Visual ---
	if (CreatedVisual && Resources)
	{
		if (Tag == TEXT("Image"))
		{
			auto SrcValue = WidgetNode->GetAttribute("ImageBrush");
			ULexImage* ImageVisual = Cast<ULexImage>(CreatedVisual);
			if (ImageVisual && !SrcValue.IsEmpty())
			{
				FLexUIImageBrush Brush;
				if (Resources->GetImageBrush(SrcValue, Brush))
				{
					ImageVisual->SetBrush(Brush);
				}
				else if (auto Texture = Resources->GetTexture(SrcValue))
				{
					ImageVisual->SetBrush_Texture(Texture);
				}
				else if (auto Sprite = Resources->GetSprite(SrcValue))
				{
					ImageVisual->SetBrush_LexUISprite(Sprite);
				}
				else
				{
					ImageVisual->SetBrush_LexUISprite(ULexUISpriteData::GetDefaultWhiteSolid());
					UE_LOG(LogTemp, Warning, TEXT("[%s].%d - ImageBrush resource '%s' not found, using default"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *SrcValue);
				}
			}
		}
		else if (Tag == TEXT("Text"))
		{
			auto FontValue = WidgetNode->GetAttribute("Font");
			ULexText* TextVisual = Cast<ULexText>(CreatedVisual);
			if (TextVisual && !FontValue.IsEmpty())
			{
				if (ULexUIFontData_BaseObject* Font = Resources->GetFont(FontValue))
				{
					TextVisual->SetFont(Font);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Font resource '%s' not found, using default"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *FontValue);
				}
			}
		}
		else if (Tag == TEXT("Texture"))
		{
			auto SrcValue = WidgetNode->GetAttribute("Texture");
			ULexTexture* TextureVisual = Cast<ULexTexture>(CreatedVisual);
			if (TextureVisual && !SrcValue.IsEmpty())
			{
				if (UTexture* Tex = Resources->GetTexture(SrcValue))
				{
					TextureVisual->SetTexture(Tex);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Texture resource '%s' not found, using default"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *SrcValue);
				}
			}
		}
		else if (Tag == TEXT("Sprite"))
		{
			auto SrcValue = WidgetNode->GetAttribute("Sprite");
			ULexSprite* SpriteVisual = Cast<ULexSprite>(CreatedVisual);
			if (SpriteVisual && !SrcValue.IsEmpty())
			{
				if (ULexUISpriteData_BaseObject* Spr = Resources->GetSprite(SrcValue))
				{
					SpriteVisual->SetSprite(Spr);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Sprite resource '%s' not found, using default"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *SrcValue);
				}
			}
		}
	}

	// --- Script behaviour (root widget only) ---
	if (ScriptClass && !EventContext)
	{
		EventContext = Cast<ULexUIMLBehaviour>(NewWidget->AddComponent(ScriptClass));
		if (EventContext)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s].%d - Added script behaviour '%s' to root widget"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *ScriptClass->GetName());
		}
	}

	// --- Bind VarName to script property ---
	{
		FString VarName = WidgetNode->GetAttribute(TEXT("VarName"));
		BindVarName(EventContext, VarName, NewWidget, CreatedVisual);
	}

	// --- Bind XML events (OnClick="FuncName", etc.) ---
	BindXMLEvents(NewWidget, WidgetNode, EventContext);

	// --- Process child elements ---
	ProcessChildElements(WidgetNode->GetChildrenNodes(), NewWidget, EventContext, ScriptClass);

	NewWidget->SetParent(ParentWidget, false);

	return EventContext;
}

void FLexUIMLUtils::ProcessChildElements(const TArray<FXmlNode*>& Children, ULexWidget* ParentWidget, ULexUIMLBehaviour* EventContext, UClass* ScriptClass)
{
	for (const FXmlNode* Child : Children)
	{
		const FString& ChildTag = Child->GetTag();
		UClass* VisualClass = nullptr;
		if (IsPrefabElement(ChildTag))
		{
			if (!Resources)
			{
				UE_LOG(LogTemp, Error, TEXT("[%s].%d - Resource is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
				continue;
			}
			const FString PrefabName = GetElementSrc(Child);
			if (auto ChildPrefab = Resources->GetPrefab(PrefabName))
			{
				ParsePrefabElement(Child, ChildPrefab, ParentWidget, EventContext, ScriptClass);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[%s].%d - Prefab '%s' not found"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *PrefabName);
				continue;
			}
		}
		else if (IsTemplateElement(ChildTag))
		{
			if (!Resources)
			{
				UE_LOG(LogTemp, Error, TEXT("[%s].%d - Resource is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
				continue;
			}
			const FString TemplateName = GetElementSrc(Child);
			if (auto ChildTemplateClass = Resources->GetTemplate(TemplateName))
			{
				ParseTemplateElement(Child, ChildTemplateClass, ParentWidget, EventContext, ScriptClass);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[%s].%d - Template '%s' not found"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *TemplateName);
				continue;
			}
		}
		else if (IsSlotElement(ChildTag))
		{
			const FString SlotName = GetSlotName(Child);
			if (bIsSubTemplate && EventContext)
			{
				ParseSlotElement(Child, SlotName, ParentWidget, EventContext);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s].%d - <Slot> is only valid inside a Template's own UIML"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			}
		}
		else if (IsWidgetElement(ChildTag, VisualClass))
		{
			ParseWidgetElement(Child, VisualClass, ParentWidget, EventContext, ScriptClass);
		}
		else
		{
			ParsePropertyElement(Child, ParentWidget);
		}
	}
}

void FLexUIMLUtils::ParseSlotElement(const FXmlNode* SlotNode, const FString& SlotName, ULexWidget* ParentWidget, ULexUIMLBehaviour* EventContext)
{
	// Create an empty placeholder widget
	ULexWidget* Placeholder = NewObject<ULexWidget>(World, ULexWidget::StaticClass(), NAME_None, RF_Transactional);
	if (!Placeholder)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s].%d - Failed to create placeholder for <Slot>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	Placeholder->SetParent(ParentWidget, false);

	if (SlotName.IsEmpty())
	{
		DefaultSlot = Placeholder;
		UE_LOG(LogTemp, Log, TEXT("[%s].%d - Bound default slot placeholder"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
	else
	{
		NamedSlots.Add(SlotName, Placeholder);
		UE_LOG(LogTemp, Log, TEXT("[%s].%d - Bound named slot '%s' placeholder"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *SlotName);
	}
}

bool FLexUIMLUtils::ProcessTemplateChildElements(const TArray<FXmlNode*>& Children, ULexUIMLBehaviour* TemplateBehaviour, ULexUIMLBehaviour* EventContext, UClass* ScriptClass)
{
	for (const FXmlNode* Child : Children)
	{
		const FString& ChildTag = Child->GetTag();

		UClass* VisualClass = nullptr;
		if (IsSlotElement(ChildTag))
		{
			const FString SlotName = GetSlotName(Child);
			// <Slot:Name> or <Slot> — create widgets inside this slot
			const TArray<FXmlNode*>& SlotChildren = Child->GetChildrenNodes();
			for (const FXmlNode* SlotChild : SlotChildren)
			{
				const FString& SlotChildTag = SlotChild->GetTag();

				ULexWidget* SlotParent = nullptr;
				if (SlotName.IsEmpty())
				{
					SlotParent = DefaultSlot.Get();
				}
				else
				{
					if (TWeakObjectPtr<ULexWidget>* Found = NamedSlots.Find(SlotName))
					{
						SlotParent = Found->Get();
					}
				}

				if (!SlotParent)
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Slot '%s' not found on template"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, SlotName.IsEmpty() ? TEXT("Default") : *SlotName);
					continue;
				}

				UClass* SlotVisualClass = nullptr;
				if (IsWidgetElement(SlotChildTag, SlotVisualClass))
				{
					ParseWidgetElement(SlotChild, SlotVisualClass, SlotParent, EventContext, ScriptClass);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Only widget elements are allowed inside <Slot>, got <%s>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *SlotChildTag);
				}
			}
		}
		else if (IsWidgetElement(ChildTag, VisualClass))
		{
			// Direct widget child → fill default slot
			ULexWidget* DefaultSlotWidget = DefaultSlot.Get();
			if (!DefaultSlotWidget)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Template has no default slot, cannot place <%s>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *ChildTag);
				continue;
			}
			// Parse the widget, parenting it to the slot placeholder instead of TemplateRoot
			ParseWidgetElement(Child, VisualClass, DefaultSlotWidget, EventContext, ScriptClass);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Unexpected child <%s> inside <Template:XXX>"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *ChildTag);
		}
	}
	return true;
}

void FLexUIMLUtils::ApplyDeferredLayoutProps(ULexWidget* Widget, const TArray<TPair<FString, FString>>& DeferredContainer, const TArray<TPair<FString, FString>>& DeferredSelf)
{
	for (const auto& Pair : DeferredContainer)
	{
		if (auto* LC = Widget->GetLayoutContainer())
			ApplyPropertyValue(LC, Pair.Key, Pair.Value);
	}
	for (const auto& Pair : DeferredSelf)
	{
		if (auto* LS = Widget->GetLayoutSelf())
			ApplyPropertyValue(LS, Pair.Key, Pair.Value);
	}
}

void FLexUIMLUtils::ParsePropertyGroups(const TArray<FXmlNode*>& Children)
{
	PropertyGroups.Empty();
	TSet<FString> VisitedIncludes;
	ParsePropertyGroups_Internal(Children, VisitedIncludes);
}

void FLexUIMLUtils::ParsePropertyGroups_Internal(const TArray<FXmlNode*>& Children, TSet<FString>& VisitedIncludes)
{
	for (const FXmlNode* Child : Children)
	{
		const FString& Tag = Child->GetTag();
		if (Tag == TEXT("PropertyGroup"))
		{
			const FString Name = Child->GetAttribute(TEXT("Name")).TrimStartAndEnd();
			if (Name.IsEmpty()) continue;

			TArray<TPair<FString, FString>>& Group = PropertyGroups.FindOrAdd(Name);
			for (const auto& Attr : Child->GetAttributes())
			{
				const FString& AttrName = Attr.GetTag();
				if (AttrName == TEXT("Name")) continue;
				Group.Add(TPair<FString, FString>(AttrName, Attr.GetValue()));
			}
		}
		else if (Tag == TEXT("Include"))
		{
			FString Src = Child->GetAttribute(TEXT("Src")).TrimStartAndEnd();
			if (Src.IsEmpty()) continue;

			const FString FullPath = FPaths::ProjectContentDir() / Src;
			if (VisitedIncludes.Contains(FullPath)) continue;
			VisitedIncludes.Add(FullPath);

			FString XmlString;
			if (!FFileHelper::LoadFileToString(XmlString, *FullPath))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s].%d - Failed to load Include: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *Src);
				continue;
			}

			FXmlFile XmlFile;
			if (!XmlFile.LoadFile(XmlString, EConstructMethod::ConstructFromBuffer)) continue;

			const FXmlNode* IncludeRoot = XmlFile.GetRootNode();
			if (!IncludeRoot || IncludeRoot->GetTag() != TEXT("LexUIML")) continue;

			ParsePropertyGroups_Internal(IncludeRoot->GetChildrenNodes(), VisitedIncludes);
		}
	}
}

void FLexUIMLUtils::ApplyStyleAttributes(const FString& Style, TMap<FString, FString>& OutAttrs) const
{
	TArray<FString> StyleNames;
	Style.ParseIntoArray(StyleNames, TEXT(","));
	for (const FString& StyleName : StyleNames)
	{
		const FString Trimmed = StyleName.TrimStartAndEnd();
		if (const TArray<TPair<FString, FString>>* Group = PropertyGroups.Find(Trimmed))
		{
			for (const auto& Pair : *Group)
			{
				OutAttrs.Add(Pair.Key, Pair.Value); // later styles override earlier ones
			}
		}
	}
}

void FLexUIMLUtils::ParsePropertyElement(const FXmlNode* PropNode, ULexWidget* TargetWidget)
{
	const FString& Tag = PropNode->GetTag();

	// 1. Try text content as the property value: <RenderOpacity>0.8</RenderOpacity>
	FString ValueStr = PropNode->GetContent().TrimStartAndEnd();

	// 2. Try "Value" attribute: <RenderOpacity Value="0.8"/>
	if (ValueStr.IsEmpty())
	{
		ValueStr = PropNode->GetAttribute(TEXT("Value"));
	}

	// If we have a simple value, apply directly
	if (!ValueStr.IsEmpty())
	{
		ApplyPropertyValue(TargetWidget, Tag, ValueStr);
		return;
	}

	// 3. No single value — treat attributes as struct sub-properties
	//    e.g. <RelativeLocation X="100" Y="50" Z="0"/>
	//    Each attr is applied as "Tag.AttrName" via dotted path.
	const TArray<FXmlAttribute>& Attrs = PropNode->GetAttributes();
	for (const auto& Attr : Attrs)
	{
		const FString& AttrName = Attr.GetTag();
		const FString& AttrValue = Attr.GetValue();
		if (AttrName != TEXT("Value"))
		{
			FString FullPropName = FString::Printf(TEXT("%s.%s"), *Tag, *AttrName);
			ApplyPropertyValue(TargetWidget, FullPropName, AttrValue);
		}
	}
}

/** Convert comma-separated values to ImportText format for common UE structs. */
static FString ConvertStructValueForImportText(const UScriptStruct* ScriptStruct, const FString& InValue)
{
	// Already in (Key=Value,...) format? Return as-is
	if (InValue.StartsWith(TEXT("("))) return InValue;

	const FName StructName = ScriptStruct->GetFName();
	const bool bIsColorType = (StructName == NAME_Color || StructName == NAME_LinearColor);

	// Hex color: #RGB / #RGBA / #RRGGBB / #RRGGBBAA
	if (bIsColorType && InValue.StartsWith(TEXT("#")))
	{
		FString Hex = InValue.Mid(1);
		const int32 Len = Hex.Len();
		if (Len == 3 || Len == 4 || Len == 6 || Len == 8)
		{
			// Normalize to 6 or 8 hex digits
			if (Len == 3) { Hex = FString::Printf(TEXT("%c%c%c%c%c%c"), Hex[0],Hex[0],Hex[1],Hex[1],Hex[2],Hex[2]); }
			if (Len == 4) { Hex = FString::Printf(TEXT("%c%c%c%c%c%c%c%c"), Hex[0],Hex[0],Hex[1],Hex[1],Hex[2],Hex[2],Hex[3],Hex[3]); }
			if (Hex.Len() == 6) { Hex += TEXT("FF"); }
			// Now 8 hex digits: RRGGBBAA
			const uint32 R = FParse::HexDigit(Hex[0]) * 16 + FParse::HexDigit(Hex[1]);
			const uint32 G = FParse::HexDigit(Hex[2]) * 16 + FParse::HexDigit(Hex[3]);
			const uint32 B = FParse::HexDigit(Hex[4]) * 16 + FParse::HexDigit(Hex[5]);
			const uint32 A = FParse::HexDigit(Hex[6]) * 16 + FParse::HexDigit(Hex[7]);
			return FString::Printf(TEXT("(R=%u,G=%u,B=%u,A=%u)"), R, G, B, A);
		}
	}

	TArray<FString> Parts;
	InValue.ParseIntoArray(Parts, TEXT(","));
	if (Parts.Num() == 0) return InValue;

	if (StructName == NAME_Vector2D && Parts.Num() >= 2)
	{
		return FString::Printf(TEXT("(X=%s,Y=%s)"), *Parts[0].TrimStartAndEnd(), *Parts[1].TrimStartAndEnd());
	}
	if (StructName == NAME_Vector && Parts.Num() >= 3)
	{
		return FString::Printf(TEXT("(X=%s,Y=%s,Z=%s)"), *Parts[0].TrimStartAndEnd(), *Parts[1].TrimStartAndEnd(), *Parts[2].TrimStartAndEnd());
	}
	if (StructName == NAME_Vector4 && Parts.Num() >= 4)
	{
		return FString::Printf(TEXT("(X=%s,Y=%s,Z=%s,W=%s)"), *Parts[0].TrimStartAndEnd(), *Parts[1].TrimStartAndEnd(), *Parts[2].TrimStartAndEnd(), *Parts[3].TrimStartAndEnd());
	}
	if (StructName == NAME_Color && Parts.Num() >= 4)
	{
		return FString::Printf(TEXT("(R=%s,G=%s,B=%s,A=%s)"), *Parts[0].TrimStartAndEnd(), *Parts[1].TrimStartAndEnd(), *Parts[2].TrimStartAndEnd(), *Parts[3].TrimStartAndEnd());
	}
	if (StructName == NAME_LinearColor && Parts.Num() >= 4)
	{
		return FString::Printf(TEXT("(R=%s,G=%s,B=%s,A=%s)"), *Parts[0].TrimStartAndEnd(), *Parts[1].TrimStartAndEnd(), *Parts[2].TrimStartAndEnd(), *Parts[3].TrimStartAndEnd());
	}
	if (StructName == NAME_Rotator && Parts.Num() >= 3)
	{
		return FString::Printf(TEXT("(P=%s,Y=%s,R=%s)"), *Parts[0].TrimStartAndEnd(), *Parts[1].TrimStartAndEnd(), *Parts[2].TrimStartAndEnd());
	}
	if (StructName == NAME_Quat && Parts.Num() >= 4)
	{
		return FString::Printf(TEXT("(X=%s,Y=%s,Z=%s,W=%s)"), *Parts[0].TrimStartAndEnd(), *Parts[1].TrimStartAndEnd(), *Parts[2].TrimStartAndEnd(), *Parts[3].TrimStartAndEnd());
	}

	return InValue;
}

/** Set a property value from string using typed setters (avoids ImportText format issues). */
void FLexUIMLUtils::SetPropertyValueFromString(FProperty* Property, void* ValuePtr, const FString& ValueStr, UObject* Owner)
{
	if (FNumericProperty* P = CastField<FNumericProperty>(Property)) { P->SetNumericPropertyValueFromString(ValuePtr, *ValueStr); return; }
	if (FBoolProperty*    P = CastField<FBoolProperty>(Property))    { P->SetPropertyValue(ValuePtr, ValueStr.ToBool()); return; }
	if (FStrProperty*     P = CastField<FStrProperty>(Property))     { P->SetPropertyValue(ValuePtr, ValueStr); return; }
	if (FNameProperty*    P = CastField<FNameProperty>(Property))    { P->SetPropertyValue(ValuePtr, FName(*ValueStr)); return; }
	if (FTextProperty*    P = CastField<FTextProperty>(Property))    { P->SetPropertyValue(ValuePtr, FText::FromString(ValueStr)); return; }
	if (FEnumProperty*    P = CastField<FEnumProperty>(Property))
	{
		FNumericProperty* Underlying = P->GetUnderlyingProperty();
		if (Underlying && P->GetEnum())
		{
			int64 V = P->GetEnum()->GetValueByNameString(ValueStr);
			if (V == INDEX_NONE) V = FCString::Atoi64(*ValueStr);
			Underlying->SetIntPropertyValue(ValuePtr, V);
		}
		return;
	}
	if (FStructProperty*  P = CastField<FStructProperty>(Property))
	{
		UScriptStruct* ScriptStruct = P->Struct;
		if (ScriptStruct)
		{
			const FString Formatted = ConvertStructValueForImportText(ScriptStruct, ValueStr);
			ScriptStruct->ImportText(*Formatted, ValuePtr, Owner, PPF_None, nullptr, TEXT("FLexUIXAML"));
		}
		return;
	}

	// Last resort: ImportText
	Property->ImportText_Direct(*ValueStr, ValuePtr, Owner, PPF_None);
}

bool FLexUIMLUtils::ApplyPropertyValue(UObject* Target, const FString& PropertyName, const FString& ValueStr)
{
	if (!Target || PropertyName.IsEmpty())
	{
		return false;
	}

	// Handle nested path: "AnchorData.Pivot" or "RelativeLocation.X"
	FString TopProp, SubProp;
	if (PropertyName.Split(TEXT("."), &TopProp, &SubProp))
	{
		FProperty* TopProperty = FindFProperty<FProperty>(Target->GetClass(), *TopProp);
		if (!TopProperty)
		{
			return false;
		}

		if (FStructProperty* StructProp = CastField<FStructProperty>(TopProperty))
		{
			void* StructPtr = StructProp->ContainerPtrToValuePtr<void>(Target);
			UScriptStruct* ScriptStruct = StructProp->Struct;

			// Find the sub-property within the struct
			FProperty* SubProperty = FindFProperty<FProperty>(ScriptStruct, *SubProp);
			if (!SubProperty)
			{
				return false;
			}

			// Set sub-property via typed setter
			void* SubValuePtr = SubProperty->ContainerPtrToValuePtr<void>(StructPtr);
			SetPropertyValueFromString(SubProperty, SubValuePtr, ValueStr, Target);
			return true;
		}
		return false;
	}

	// Direct property
	FProperty* Property = FindFProperty<FProperty>(Target->GetClass(), *PropertyName);
	if (!Property)
	{
		return false;
	}

	// Numeric types (float, double, int, int64, uint32, byte, etc.)
	if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Property))
	{
		NumericProp->SetNumericPropertyValueFromString_InContainer(Target, *ValueStr);
		return true;
	}
	if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
	{
		BoolProp->SetPropertyValue_InContainer(Target, ValueStr.ToBool());
		return true;
	}

	// String types
	if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
	{
		StrProp->SetPropertyValue_InContainer(Target, ValueStr);
		return true;
	}
	if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
	{
		NameProp->SetPropertyValue_InContainer(Target, FName(*ValueStr));
		return true;
	}
	if (FTextProperty* TextProp = CastField<FTextProperty>(Property))
	{
		TextProp->SetPropertyValue_InContainer(Target, FText::FromString(ValueStr));
		return true;
	}

	// Struct types (FVector, FVector2D, FColor, FLinearColor, FRotator, FQuat, etc.)
	if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
	{
		void* StructPtr = StructProp->ContainerPtrToValuePtr<void>(Target);
		UScriptStruct* ScriptStruct = StructProp->Struct;
		if (StructPtr && ScriptStruct)
		{
			const FString Formatted = ConvertStructValueForImportText(ScriptStruct, ValueStr);
			ScriptStruct->ImportText(*Formatted, StructPtr, Target, PPF_None, nullptr, TEXT("FLexUIXAML"));
			return true;
		}
		return false;
	}

	// Enum types
	if (FByteProperty* ByteProp = CastField<FByteProperty>(Property))
	{
		if (ByteProp->Enum)
		{
			int64 EnumVal = ByteProp->Enum->GetValueByNameString(ValueStr);
			if (EnumVal == INDEX_NONE)
			{
				EnumVal = FCString::Atoi64(*ValueStr);
			}
			ByteProp->SetPropertyValue_InContainer(Target, static_cast<uint8>(EnumVal));
		}
		else
		{
			ByteProp->SetPropertyValue_InContainer(Target, static_cast<uint8>(FCString::Atoi(*ValueStr)));
		}
		return true;
	}
	if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
	{
		FNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
		if (UnderlyingProp && EnumProp->GetEnum())
		{
			int64 EnumVal = EnumProp->GetEnum()->GetValueByNameString(ValueStr);
			if (EnumVal == INDEX_NONE)
			{
				EnumVal = FCString::Atoi64(*ValueStr);
			}
			void* ValuePtr = EnumProp->ContainerPtrToValuePtr<void>(Target);
			UnderlyingProp->SetIntPropertyValue(ValuePtr, EnumVal);
		}
		return true;
	}

	return false;
}
