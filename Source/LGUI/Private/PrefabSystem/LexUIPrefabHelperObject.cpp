// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LexUIPrefabHelperObject.h"
#include "LGUI.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexWidget.h"
#include "Core/Components/LexWidgetSubObjectBehaviour.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "Utils/LexUIUtils.h"
#include "PrefabSystem/LexUIObjectReaderAndWriter.h"

#include LEXUIPREFAB_SERIALIZER_NEWEST_INCLUDE

#define LOCTEXT_NAMESPACE "LGUIPrefabManager"


ULexUIPrefabHelperObject::ULexUIPrefabHelperObject()
{
	
}

#if WITH_EDITOR
void ULexUIPrefabHelperObject::BeginDestroy()
{
	ClearLoadedPrefab();
	Super::BeginDestroy();
}

void ULexUIPrefabHelperObject::Init(ULexUIPrefab* InPrefab, FLexUIPrefabInstanceScene* InPrefabInstanceScene)
{
	PrefabAsset = InPrefab;
	PrefabInstanceWorld = InPrefabInstanceScene->GetWorld();
	if (!IsValid(LoadedRootWidget))
	{
		auto Parent = InPrefabInstanceScene->GetParentForLoadPrefab(PrefabAsset);
		LoadedRootWidget = PrefabAsset->LoadPrefabWithExistingObjects(PrefabInstanceWorld.Get()
			, Parent != nullptr ? Parent->GetOuter() : PrefabInstanceWorld.Get()
			, Parent
			, MapGuidToObject, SubPrefabMap
		);
	}
	if (LoadedRootWidget == nullptr)return;
	
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &ULexUIPrefabHelperObject::OnObjectPropertyChanged);
	FCoreUObjectDelegates::OnPreObjectPropertyChanged.AddUObject(this, &ULexUIPrefabHelperObject::OnPreObjectPropertyChanged);

	ULexUIManagerWorldSubsystem::RefreshAllUI();
}
#endif

#if WITH_EDITOR

void ULexUIPrefabHelperObject::ClearLoadedPrefab()
{
	if (IsValid(LoadedRootWidget))
	{
		LoadedRootWidget->DestroyWidget();
		LoadedRootWidget = nullptr;;
	}
	MapGuidToObject.Empty();
	SubPrefabMap.Empty();
}

bool ULexUIPrefabHelperObject::IsWidgetBelongsToSubPrefab(const ULexWidget* InWidget)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InWidget))return false;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		auto& SubMapGuidToObject = SubPrefabKeyValue.Value.MapGuidToObject;
		for (auto& SubMapGuidToObjectKeyValue : SubMapGuidToObject)
		{
			if (SubMapGuidToObjectKeyValue.Value == InWidget)
			{
				return true;
			}
		}
	}
	return false;
}
bool ULexUIPrefabHelperObject::IsWidgetBelongsToMissingSubPrefab(const ULexWidget* InWidget)
{
	if (!IsValid(InWidget))return false;
#if WITH_EDITOR
	for (auto& Item : MissingPrefab)
	{
		if (InWidget == Item || InWidget->IsChildOf(Item))
		{
			return true;
		}
	}
#endif
	return false;
}

bool ULexUIPrefabHelperObject::IsSubPrefabRootWidget(const ULexWidget* InWidget)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InWidget))return false;
	return SubPrefabMap.Contains(InWidget);
}

bool ULexUIPrefabHelperObject::IsWidgetBelongsToThis(const ULexWidget* InWidget)
{
	if (IsValid(this->LoadedRootWidget))
	{
		if (InWidget->IsChildOf(LoadedRootWidget) || InWidget == LoadedRootWidget)
		{
			return true;
		}
	}
	return false;
}

bool ULexUIPrefabHelperObject::ClearInvalidObjectAndGuid()
{
	TSet<FGuid> GuidsToRemove;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (!IsValid(KeyValue.Value))
		{
			GuidsToRemove.Add(KeyValue.Key);
		}
	}
	for (auto& Item : GuidsToRemove)
	{
		MapGuidToObject.Remove(Item);

		for (auto& SubPrefabKeyValue : SubPrefabMap)
		{
			if (auto GuidInSubPrefabPtr = SubPrefabKeyValue.Value.MapObjectGuidFromParentPrefabToSubPrefab.Find(Item))
			{
				auto GuidInParentPrefab = Item;
				auto GuidInSubPrefab = *GuidInSubPrefabPtr;
				SubPrefabKeyValue.Value.MapGuidToObject.Remove(GuidInSubPrefab);
				SubPrefabKeyValue.Value.MapObjectGuidFromParentPrefabToSubPrefab.Remove(GuidInParentPrefab);
				break;
			}
		}
	}
	return GuidsToRemove.Num() > 0;
}

void ULexUIPrefabHelperObject::AddMemberPropertyToSubPrefab(ULexWidget* InSubPrefabWidget, UObject* InObject, FName InPropertyName)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabWidget))return;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabWidget == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.AddMemberProperty(InObject, InPropertyName);
			}
		}
	}
}

void ULexUIPrefabHelperObject::RemoveMemberPropertyFromSubPrefab(ULexWidget* InSubPrefabWidget, UObject* InObject, FName InPropertyName)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabWidget))return;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabWidget == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.RemoveMemberProperty(InObject, InPropertyName);
				break;
			}
		}
	}
}

void ULexUIPrefabHelperObject::RemoveAllMemberPropertyFromSubPrefab(ULexWidget* InSubPrefabRootWidget, bool InIncludeRootTransform)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabRootWidget))return;
	for (auto& KeyValue : SubPrefabMap)
	{
		auto SubPrefabRootWidget = KeyValue.Key;
		FLexUISubPrefabData& SubPrefabData = KeyValue.Value;
		SubPrefabData.CheckParameters();
		if (InSubPrefabRootWidget == SubPrefabRootWidget)
		{
			for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
			{
				auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
				TSet<FName> FilterNameSet;
				if (InSubPrefabRootWidget == DataItem.Object)//if prefab's root widget, then skip it's transform
				{
					if (!InIncludeRootTransform)
					{
						FilterNameSet.Add(ULexWidget::GetPropertyName_RelativeLocation());
						FilterNameSet.Add(ULexWidget::GetPropertyName_RelativeRotation());
						FilterNameSet.Add(ULexWidget::GetPropertyName_RelativeScale());
					}
				}

				TSet<FName> NamesToClear;
				for (auto& PropertyName : DataItem.MemberPropertyNames)
				{
					if (FilterNameSet.Contains(PropertyName))continue;
					NamesToClear.Add(PropertyName);
				}
				for (auto& PropertyName : NamesToClear)
				{
					DataItem.MemberPropertyNames.RemoveSwap(PropertyName);
				}
				if (DataItem.MemberPropertyNames.Num() == 0)
				{
					SubPrefabData.ObjectOverrideParameterArray.RemoveAt(i);
					i--;
				}
			}
			return;
		}
	}
}

FLexUISubPrefabData ULexUIPrefabHelperObject::GetSubPrefabData(ULexWidget* InSubPrefabWidget)
{
	CleanupInvalidSubPrefab();
	check(IsValid(InSubPrefabWidget));
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		auto& SubMapGuidToObject = SubPrefabKeyValue.Value.MapGuidToObject;
		for (auto& KeyValue : SubMapGuidToObject)
		{
			if (InSubPrefabWidget == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.CheckParameters();
				return SubPrefabKeyValue.Value;
			}
		}
	}
	return FLexUISubPrefabData();
}

ULexWidget* ULexUIPrefabHelperObject::GetSubPrefabRootWidget(ULexWidget* InSubPrefabWidget)
{
	CleanupInvalidSubPrefab();
	check(IsValid(InSubPrefabWidget));
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabWidget == KeyValue.Value)
			{
				return SubPrefabKeyValue.Key;
			}
		}
	}
	return nullptr;
}

void ULexUIPrefabHelperObject::SavePrefab()
{
	CleanupInvalidSubPrefab();
	if (IsValid(PrefabAsset))
	{
		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto& KeyValue : MapGuidToObject)
		{
			if (IsValid(KeyValue.Value))
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
		}
		PrefabAsset->SavePrefab(LoadedRootWidget
			, MapObjectToGuid, SubPrefabMap
		);
		MapGuidToObject.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
		}
		bAnythingDirty = false;
		PrefabAsset->EnsureInstanceObjects();
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

ULexUIPrefab* ULexUIPrefabHelperObject::GetSubPrefabAsset(ULexWidget* InSubPrefabWidget)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabWidget))return nullptr;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabWidget == KeyValue.Value)
			{
				return SubPrefabKeyValue.Value.PrefabAsset;
			}
		}
	}
	return nullptr;
}

void ULexUIPrefabHelperObject::MarkOverrideParameterFromParentPrefab(UObject* InObject, const TArray<FName>& InPropertyNames)
{
	auto Widget = Cast<ULexWidget>(InObject);
	if (!Widget)
	{
		Widget = InObject->GetTypedOuter<ULexWidget>();
	}

	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (Widget == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.AddMemberProperty(InObject, InPropertyNames);
			}
		}
	}
}
void ULexUIPrefabHelperObject::MarkOverrideParameterFromParentPrefab(UObject* InObject, FName InPropertyName)
{
	auto Widget = Cast<ULexWidget>(InObject);
	if (!Widget)
	{
		Widget = InObject->GetTypedOuter<ULexWidget>();
	}

	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (Widget == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.AddMemberProperty(InObject, InPropertyName);
				break;
			}
		}
	}
}




bool ULexUIPrefabHelperObject::RefreshOnSubPrefabDirty(ULexUIPrefab* InSubPrefab, ULexWidget* InSubPrefabRootWidget)
{
	CleanupInvalidSubPrefab();

	bCanCollectProperty = false;
	bCanNotifyComponentCreateDelete = false;

	bool AnythingChange = false;

	for (auto& SubPrefabKeyValue : this->SubPrefabMap)
	{
		auto SubPrefabRootWidget = SubPrefabKeyValue.Key;
		auto& SubPrefabData = SubPrefabKeyValue.Value;
		SubPrefabData.CheckParameters();
		if (SubPrefabData.PrefabAsset == InSubPrefab
			&& (InSubPrefabRootWidget != nullptr ? SubPrefabRootWidget == InSubPrefabRootWidget : true)
			)
		{
			//store override parameter to data
			LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::WidgetSerializer serializer;
			serializer.bOverrideVersions = false;
			auto OverrideData = serializer.SaveOverrideParameterToData(SubPrefabData.ObjectOverrideParameterArray);

			auto& SubPrefabMapGuidToObject = SubPrefabData.MapGuidToObject;

			TSet<FGuid> ExtraObjectsGuidsToRemove;
			TSet<UObject*> ExtraObjectsToDelete;
			//check objects to delete: compare guid in sub-prefab's assets and this parent stored guid
			auto& MapGuidToObjectInSubPrefab = SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject;
			for (auto& KeyValue : SubPrefabMapGuidToObject)
			{
				if (!MapGuidToObjectInSubPrefab.Contains(KeyValue.Key))
				{
					ExtraObjectsGuidsToRemove.Add(KeyValue.Key);
					ExtraObjectsToDelete.Add(KeyValue.Value);
					AnythingChange = true;
				}
			}
			for (auto& Item : ExtraObjectsGuidsToRemove)
			{
				SubPrefabMapGuidToObject.Remove(Item);

				FGuid FoundGuid;
				for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
				{
					if (KeyValue.Value == Item)
					{
						FoundGuid = KeyValue.Key;
						break;
					}
				}
				if (FoundGuid.IsValid())
				{
					SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Remove(FoundGuid);
				}
				AnythingChange = true;
			}

			//refresh sub-prefab's object
			TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData> TempSubSubPrefabMap;
			auto ParentWidget = SubPrefabRootWidget->GetParent();
			InSubPrefab->LoadPrefabWithExistingObjects(GetPrefabWorld()
				, ParentWidget != nullptr ? ParentWidget->GetOuter() : GetPrefabWorld()
				, ParentWidget
				, SubPrefabMapGuidToObject, TempSubSubPrefabMap
			);

			//collect newly added object and guid
			auto ObjectExist = [&](UObject* InObject) {
				for (auto& KeyValue : this->MapGuidToObject)
				{
					if (KeyValue.Value == InObject)
					{
						return true;
					}
				}
				return false;
			};
			for (auto& KeyValue : SubPrefabMapGuidToObject)
			{
				if (!ObjectExist(KeyValue.Value))
				{
					auto NewGuid = FGuid::NewGuid();
					this->MapGuidToObject.Add(NewGuid, KeyValue.Value);
					SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(NewGuid, KeyValue.Key);
					AnythingChange = true;
				}

				//this object could be the same one, but with different guid (because it's guid not stored in parent prefab, and generated every time)
				if (ExtraObjectsToDelete.Contains(KeyValue.Value))
				{
					ExtraObjectsToDelete.Remove(KeyValue.Value);
				}
			}

			//delete extra objects
			for (auto& Obj : ExtraObjectsToDelete)
			{
				if (!IsValid(Obj))continue;
				if (auto Widget = Cast<ULexWidget>(Obj))
				{
					Widget->DestroyWidget();
				}
				else if (auto WidgetComponent = Cast<ULexUIBehaviour>(Obj))
				{
					WidgetComponent->DestroyComponent();
				}
				else
				{
					Obj->ConditionalBeginDestroy();
				}
			}

			//apply override parameter.
			{
				//clear not valid objects first
				for (int i = SubPrefabData.ObjectOverrideParameterArray.Num() - 1; i >= 0; i--)
				{
					auto& Item = SubPrefabData.ObjectOverrideParameterArray[i];
					if (!Item.Object.IsValid())
					{
						SubPrefabData.ObjectOverrideParameterArray.RemoveAt(i);
						AnythingChange = true;
					}
				}

				for (auto& ObjectOverrideItem : SubPrefabData.ObjectOverrideParameterArray)
				{
					for (auto& PropName : ObjectOverrideItem.MemberPropertyNames)
					{
						FLexUIUtils::NotifyPropertyPreChange(ObjectOverrideItem.Object.Get(), PropName);
					}
				}
				serializer.RestoreOverrideParameterFromData(OverrideData, SubPrefabData.ObjectOverrideParameterArray);
				for (auto& ObjectOverrideItem : SubPrefabData.ObjectOverrideParameterArray)
				{
					for (auto& PropName : ObjectOverrideItem.MemberPropertyNames)
					{
						FLexUIUtils::NotifyPropertyChanged(ObjectOverrideItem.Object.Get(), PropName);
					}
				}
			}

			SubPrefabRootWidget->UpdateObjectToWorldTransform();//root comp may stay prev position if not do this

			if (SubPrefabData.CheckParameters())
			{
				AnythingChange = true;
			}
		}
	}

	if (AnythingChange)
	{
		this->SavePrefab();
		if (this->PrefabAsset != nullptr)//could be null in level editor
		{
#if WITH_EDITOR
			this->PrefabAsset->bThumbnailDirty = true;
#endif
			this->PrefabAsset->MarkPackageDirty();
		}
		ClearInvalidObjectAndGuid();//incase LevelPrefab reference invalid object, eg: delete object in sub-prefab's sub-prefab, and update the prefab in level
	}
	RefreshSubPrefabVersion(InSubPrefabRootWidget);
	bCanCollectProperty = true;
	bCanNotifyComponentCreateDelete = true;
	ULexUIManagerWorldSubsystem::RefreshAllUI();
	return AnythingChange;
}

void ULexUIPrefabHelperObject::OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (!IsValid(InObject))return;
	if (InPropertyChangedEvent.MemberProperty == nullptr || InPropertyChangedEvent.Property == nullptr)return;
	if (LexUIPrefabSystem::LexUIPrefab_ShouldSkipProperty(InPropertyChangedEvent.MemberProperty))return;
	if (LexUIPrefabSystem::LexUIPrefab_ShouldSkipProperty(InPropertyChangedEvent.Property))return;

	TryCollectPropertyToOverride(InObject, InPropertyChangedEvent.MemberProperty);
}
void ULexUIPrefabHelperObject::OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain)
{
	if (!IsValid(InObject))return;
	auto ActiveMemberNode = InEditPropertyChain.GetActiveMemberNode();
	if (ActiveMemberNode == nullptr)return;
	auto MemberProperty = ActiveMemberNode->GetValue();
	if (MemberProperty == nullptr)return;
	if (LexUIPrefabSystem::LexUIPrefab_ShouldSkipProperty(MemberProperty))return;
	auto ActiveNode = InEditPropertyChain.GetActiveNode();
	if (ActiveNode != ActiveMemberNode)
	{
		auto Property = ActiveNode->GetValue();
		if (Property == nullptr)return;
		if (Property->HasAnyPropertyFlags(CPF_Transient))return;
	}

	TryCollectPropertyToOverride(InObject, MemberProperty);
}

void ULexUIPrefabHelperObject::TryCollectPropertyToOverride(UObject* InObject, FProperty* InMemberProperty)
{
	if (!bCanCollectProperty)return;
	if (InObject->GetWorld() == this->GetPrefabWorld())
	{
		auto PropertyName = InMemberProperty->GetFName();
		ULexWidget* PropertyWidgetInSubPrefab = nullptr;
		if (auto Widget = Cast<ULexWidget>(InObject))
		{
			if (auto ObjectProperty = CastField<FObjectPropertyBase>(InMemberProperty))
			{
				if (ObjectProperty->PropertyClass->IsChildOf(ULexUIBehaviour::StaticClass()))
				{
					return;//property change is propagated from Component to Widget, ignore it
				}
			}
			if (IsWidgetBelongsToSubPrefab(Widget))
			{
				PropertyWidgetInSubPrefab = Widget;
			}

			if (PropertyWidgetInSubPrefab != nullptr//if drag in level editor, then property change event will notify actor, so we need to collect property on actor's root component
				&& (PropertyName == ULexWidget::GetPropertyName_RelativeLocation()
					|| PropertyName == ULexWidget::GetPropertyName_RelativeRotation()
					|| PropertyName == ULexWidget::GetPropertyName_RelativeScale()
					)
				)
			{
				InObject = PropertyWidgetInSubPrefab;
			}
		}
		if (auto OuterWidget = InObject->GetTypedOuter<ULexWidget>())
		{
			if (IsWidgetBelongsToSubPrefab(OuterWidget))
			{
				bool bFindObjectInGuidMap = false;
				for (auto& KeyValue : this->MapGuidToObject)
				{
					if (KeyValue.Value == InObject)
					{
						bFindObjectInGuidMap = true;
						break;
					}
				}
				if (bFindObjectInGuidMap)
				{
					PropertyWidgetInSubPrefab = OuterWidget;
				}
			}
		}

		if (PropertyWidgetInSubPrefab)//object's member property
		{
			auto Property = FindFProperty<FProperty>(InObject->GetClass(), PropertyName);
			if (Property != nullptr)
			{
				SetAnythingDirty();
				AddMemberPropertyToSubPrefab(PropertyWidgetInSubPrefab, InObject, PropertyName);
				if (auto Widget = Cast<ULexWidget>(InObject))
				{
					if (PropertyName == ULexWidget::GetPropertyName_RelativeLocation())//if UI's relative location change, then record anchor data too
					{
						this->AddMemberPropertyToSubPrefab(Widget, InObject, ULexWidget::GetPropertyName_AnchorData());
					}
					else if (PropertyName == ULexWidget::GetPropertyName_AnchorData())//if UI's anchor data change, then record relative location too
					{
						this->AddMemberPropertyToSubPrefab(Widget, InObject, ULexWidget::GetPropertyName_RelativeLocation());
					}
				}
				//refresh override parameter
			}
		}
		else
		{
			SetAnythingDirty();
		}
	}
}

UWorld* ULexUIPrefabHelperObject::GetPrefabWorld() const
{
	return PrefabInstanceWorld.Get();
}

bool ULexUIPrefabHelperObject::CleanupInvalidLinkToSubPrefabObject()
{
	auto IsValidParentLinkedGuid = [&](const FGuid& InCheckGuid) {
		for (auto& SubPrefabKeyValue : SubPrefabMap)
		{
			auto& SubPrefabData = SubPrefabKeyValue.Value;
			if (SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(InCheckGuid))
			{
				return true;
			}
		}
		return false;
	};

	TSet<FGuid> GuidsToRemove;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (!IsValidParentLinkedGuid(KeyValue.Key))
		{
			GuidsToRemove.Add(KeyValue.Key);
		}
	}
	for (auto& Item : GuidsToRemove)
	{
		MapGuidToObject.Remove(Item);
	}
	return GuidsToRemove.Num() > 0;
}

#pragma region RevertAndApply
/**
 * When revert, if the parameter is RelativeLocation, then Widget's AnchorData will also be reverted. Revert parameter is just copy data from origin to dest, origin means the temporary created objects in prefab's preview world.
 * But since AnchorData is relative to parent, and parent may not have the same AnchorData (because parent is temporary created inside preview world), so we need to set parent's AnchorData to now object's parent's AnchorData.
 */
void ULexUIPrefabHelperObject::CopyRootObjectParentAnchorData(UObject* InObject, UObject* OriginObject)
{
	if (auto Widget = Cast<ULexWidget>(InObject))
	{
		if (SubPrefabMap.Contains(Widget))//if is sub prefab's root component
		{
			auto InObjectWidget = Cast<ULexWidget>(InObject);
			auto OriginObjectWidget = Cast<ULexWidget>(OriginObject);
			if (InObjectWidget != nullptr && OriginObjectWidget != nullptr)//if is Widget, we need to copy parent's property to origin object's parent property, to make anchor & location calculation right
			{
				auto InObjectParent = InObjectWidget->GetParent();
				auto OriginObjectParent = OriginObjectWidget->GetParent();
				if (InObjectParent != nullptr && OriginObjectParent != nullptr)
				{
					//copy relative location
					auto RelativeLocationProperty = FindFProperty<FProperty>(InObjectParent->GetClass(), ULexWidget::GetPropertyName_RelativeLocation());
					RelativeLocationProperty->CopyCompleteValue_InContainer(OriginObjectParent, InObjectParent);
					FLexUIUtils::NotifyPropertyChanged(OriginObjectParent, RelativeLocationProperty);
					//copy anchor data
					auto AnchorDataProperty = FindFProperty<FProperty>(InObjectParent->GetClass(), ULexWidget::GetPropertyName_AnchorData());
					AnchorDataProperty->CopyCompleteValue_InContainer(OriginObjectParent, InObjectParent);
					FLexUIUtils::NotifyPropertyChanged(OriginObjectParent, AnchorDataProperty);
				}
			}
		}
	}
}

void ULexUIPrefabHelperObject::RevertPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInPrefab, const FLexUISubPrefabData& SubPrefabData, int RawArrayIndex, bool IsInsideRawArray)
{
	if (Property->ArrayDim > 1 && !IsInsideRawArray)
	{
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			RevertPrefabPropertyValue(ContextObject, Property, ContainerPointerInSrc, ContainerPointerInPrefab, SubPrefabData, i, true);
		}
		return;
	}
	bool bPropertySupportDirectCopyValue = false;
	if (CastField<FClassProperty>(Property) != nullptr)
	{
		bPropertySupportDirectCopyValue = true;
	}
	else if (auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		if (auto ObjectInPrefab = ObjectProperty->GetObjectPropertyValue_InContainer(ContainerPointerInPrefab))
		{
			auto ObjectClass = ObjectInPrefab->GetClass();
			if (ObjectInPrefab->IsAsset())
			{
				bPropertySupportDirectCopyValue = true;
			}
			else
			{
				//search object in guid
				FGuid ObjectGuidInPrefab;
				for (auto& KeyValue : SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject)
				{
					if (KeyValue.Value == ObjectInPrefab)
					{
						ObjectGuidInPrefab = KeyValue.Key;
						break;
					}
				}
				FGuid ObjectGuidInParent;
				//find valid guid, get the guid in prarent, with the guid then get object, so that object is the real value
				if (ObjectGuidInPrefab.IsValid())
				{
					for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
					{
						if (KeyValue.Value == ObjectGuidInPrefab)
						{
							ObjectGuidInParent = KeyValue.Key;
							break;
						}
					}
				}
				if (ObjectGuidInParent.IsValid())
				{
					auto ObjectInParent = this->MapGuidToObject[ObjectGuidInParent];
					if (ObjectClass->IsChildOf(ULexWidget::StaticClass())
						|| ObjectClass->IsChildOf(ULexWidgetSubObjectBehaviour::StaticClass())
						|| ObjectClass->IsChildOf(ULexUIBehaviour::StaticClass())
						)
					{
						ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInSrc, ObjectInParent, RawArrayIndex);
					}
					else
					{
						if (ObjectClass->HasAnyClassFlags(EClassFlags::CLASS_EditInlineNew)
							//&& ObjectProperty->HasAnyPropertyFlags(EPropertyFlags::CPF_InstancedReference)//is this necessary?
							)
						{
							if (!IsValid(ObjectInParent))
							{
								//EditInlineNew object can create new one
								ObjectInParent = NewObject<UObject>(ContextObject, ObjectClass, NAME_None, RF_NoFlags, ObjectInPrefab);
								ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInSrc, ObjectInParent, RawArrayIndex);
								this->MapGuidToObject[ObjectGuidInParent] = ObjectInParent;
							}
							for (const auto PropertyItem : TFieldRange<FProperty>(ObjectClass))//check property inside object
							{
								RevertPrefabPropertyValue(ObjectInParent, PropertyItem, ObjectInParent, ObjectInPrefab, SubPrefabData);
							}
						}
						else
						{
							auto InfoText = FText::Format(LOCTEXT("RevertPrefabPropertyValue_MissingConditionWarning", "LexUI have not handle this condition:\nobject: '{0}'\nobjectClass: '{1}'")
								, FText::FromString(ObjectInPrefab->GetPathName()), FText::FromString(ObjectClass->GetPathName()));
							UE_LOG(LGUI, Log, TEXT("%s"), *InfoText.ToString());
							FLexUIUtils::EditorNotification(InfoText, false);
						}
					}
				}
			}
		}
		else
		{
			bPropertySupportDirectCopyValue = true;
		}
	}
	else if (auto ArrayProperty = CastField<FArrayProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);//just copy so we don't need to resize it
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptArrayHelper ArrayHelperForPrefab(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < ArrayHelper.Num(); i++)
		{
			RevertPrefabPropertyValue(ContextObject, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), ArrayHelperForPrefab.GetRawPtr(i), SubPrefabData);
		}
	}
	else if (auto MapProperty = CastField<FMapProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);//just copy so we don't need to resize it
		FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptMapHelper MapHelperForPrefab(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < MapHelper.Num(); i++)
		{
			RevertPrefabPropertyValue(ContextObject, MapProperty->KeyProp, MapHelper.GetKeyPtr(i), MapHelperForPrefab.GetKeyPtr(i), SubPrefabData);
			RevertPrefabPropertyValue(ContextObject, MapProperty->ValueProp, MapHelper.GetPairPtr(i), MapHelperForPrefab.GetPairPtr(i), SubPrefabData);
		}
		MapHelperForPrefab.Rehash();
	}
	else if (auto SetProperty = CastField<FSetProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);//just copy so we don't need to resize it
		FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptSetHelper SetHelperForPrefab(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < SetHelper.Num(); i++)
		{
			RevertPrefabPropertyValue(ContextObject, SetProperty->ElementProp, SetHelper.GetElementPtr(i), SetHelperForPrefab.GetElementPtr(i), SubPrefabData);
		}
		SetHelperForPrefab.Rehash();
	}
	else if (auto StructProperty = CastField<FStructProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);
		auto StructPtr = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInSrc, RawArrayIndex);
		auto StructPtrForPrefab = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInPrefab, RawArrayIndex);
		for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
		{
			RevertPrefabPropertyValue(ContextObject, *It, StructPtr, StructPtrForPrefab, SubPrefabData);
		}
	}
	else
	{
		bPropertySupportDirectCopyValue = true;
	}
	if (bPropertySupportDirectCopyValue)
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);
	}
}
void ULexUIPrefabHelperObject::RevertPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames)
{
	GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperties", "Revert Prefab Override: {0}"), FText::FromString(InObject->GetName())));
	InObject->Modify();
	this->Modify();

	auto Widget = Cast<ULexWidget>(InObject);
	if (!Widget)
	{
		Widget = InObject->GetTypedOuter<ULexWidget>();
	}
	auto SubPrefabData = GetSubPrefabData(Widget);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
			break;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto ObjectInPrefab = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
	CopyRootObjectParentAnchorData(InObject, ObjectInPrefab);

	bCanCollectProperty = false;
	{
		for (auto PropertyName : InPropertyNames)
		{
			if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
			{
				//notify
				FLexUIUtils::NotifyPropertyPreChange(InObject, Property);//need to do PreChange here, so that actor's PostContructionScript can work
				//set to default value
				RevertPrefabPropertyValue(InObject, Property, InObject, ObjectInPrefab, SubPrefabData);
				AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
				//delete item
				RemoveMemberPropertyFromSubPrefab(Widget, InObject, PropertyName);
				//notify
				FLexUIUtils::NotifyPropertyChanged(InObject, Property);
				SetAnythingDirty();

				auto RelatedPropertyName = GetExtraRelatedPropertyForApplyOrRevert(InObject, PropertyName);
				if (RelatedPropertyName != NAME_None)
				{
					if (auto RelatedProperty = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), RelatedPropertyName))
					{
						//set to default value
						RevertPrefabPropertyValue(InObject, RelatedProperty, InObject, ObjectInPrefab, SubPrefabData);
						AfterObjectPropertyApplyOrRevert(InObject, RelatedPropertyName);
						//delete item
						RemoveMemberPropertyFromSubPrefab(Widget, InObject, RelatedPropertyName);
					}
				}
			}
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULexUIManagerWorldSubsystem::RefreshAllUI();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootWidget(Widget));
}

void ULexUIPrefabHelperObject::RevertAllPrefabOverride(UObject* InObject)
{
	bCanCollectProperty = false;
	{
		auto Widget = Cast<ULexWidget>(InObject);
		if (!Widget)
		{
			Widget = InObject->GetTypedOuter<ULexWidget>();
		}
		auto SubPrefabData = GetSubPrefabData(Widget);
		auto SubPrefabRootWidget = GetSubPrefabRootWidget(Widget);

		GEditor->BeginTransaction(LOCTEXT("RevertPrefabOnAll_Transaction", "Revert Prefab Override"));
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		this->Modify();

		auto SubPrefabAsset = SubPrefabData.PrefabAsset;
		auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
					break;
				}
			}
			FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
			return SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
		};
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			auto SourceObject = DataItem.Object.Get();
			TSet<FName> FilterNameSet;
			auto ObjectInPrefab = FindOriginObjectInSourcePrefab(SourceObject);
			CopyRootObjectParentAnchorData(SourceObject, ObjectInPrefab);

			TSet<FName> NamesToClear;
			for (auto PropertyName : DataItem.MemberPropertyNames)
			{
				if (FilterNameSet.Contains(PropertyName))continue;
				NamesToClear.Add(PropertyName);
				if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
				{
					//notify
					FLexUIUtils::NotifyPropertyPreChange(SourceObject, Property);//need to do PreChange here, so that actor's PostContructionScript can work
					//set to default value
					RevertPrefabPropertyValue(InObject, Property, SourceObject, ObjectInPrefab, SubPrefabData);
					AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
					//notify
					FLexUIUtils::NotifyPropertyChanged(SourceObject, Property);

					auto RelatedPropertyName = GetExtraRelatedPropertyForApplyOrRevert(InObject, PropertyName);
					if (RelatedPropertyName != NAME_None)
					{
						NamesToClear.Add(RelatedPropertyName);
						if (auto RelatedProperty = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), RelatedPropertyName))
						{
							//set to default value
							RevertPrefabPropertyValue(InObject, RelatedProperty, SourceObject, ObjectInPrefab, SubPrefabData);
							AfterObjectPropertyApplyOrRevert(InObject, RelatedPropertyName);
							//delete item
							RemoveMemberPropertyFromSubPrefab(Widget, SourceObject, RelatedPropertyName);
						}
					}
				}
			}
			for (auto& PropertyName : NamesToClear)
			{
				DataItem.MemberPropertyNames.Remove(PropertyName);
			}
		}
		RemoveAllMemberPropertyFromSubPrefab(SubPrefabRootWidget, true);

		SetAnythingDirty();
		GEditor->EndTransaction();
		//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
		RefreshSubPrefabVersion(GetSubPrefabRootWidget(Widget));
	}
	bCanCollectProperty = true;
	ULexUIManagerWorldSubsystem::RefreshAllUI();
}

FName ULexUIPrefabHelperObject::GetExtraRelatedPropertyForApplyOrRevert(UObject* InObject, FName InPropertyName)
{
	if (InObject->IsA<ULexWidget>())
	{
		if (InPropertyName == ULexWidget::GetPropertyName_RelativeLocation())
		{
			InPropertyName = ULexWidget::GetPropertyName_AnchorData();
		}
	}
	return NAME_None;
}
void ULexUIPrefabHelperObject::AfterObjectPropertyApplyOrRevert(UObject* InObject, FName InPropertyName)
{
	if (auto Widget = Cast<ULexWidget>(InObject))
	{
		if (InPropertyName == ULexWidget::GetPropertyName_AnchorData())
		{
			Widget->CalculateTransformFromAnchor();//calculate transform here, because when NotifyPropertyChanged the PostActorConstruction->MoveComponent will call then anchor will calculate from transform value which is wrong
			this->RemoveMemberPropertyFromSubPrefab(Widget, InObject, ULexWidget::GetPropertyName_RelativeLocation());//remove RelativeLocation override because Widget use AnchorData to calculate RelativeLocation
		}
	}
}

void ULexUIPrefabHelperObject::ApplyPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInPrefab, const FLexUISubPrefabData& SubPrefabData, int RawArrayIndex, bool IsInsideRawArray)
{
	if (Property->ArrayDim > 1 && !IsInsideRawArray)
	{
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			ApplyPrefabPropertyValue(ContextObject, Property, ContainerPointerInSrc, ContainerPointerInPrefab, SubPrefabData, i, true);
		}
		return;
	}
	bool bPropertySupportDirectCopyValue = false;
	if (CastField<FClassProperty>(Property) != nullptr)
	{
		bPropertySupportDirectCopyValue = true;
	}
	else if (auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		if (auto ObjectInParent = ObjectProperty->GetObjectPropertyValue_InContainer(ContainerPointerInSrc))
		{
			auto ObjectClass = ObjectInParent->GetClass();
			if (ObjectInParent->IsAsset())
			{
				bPropertySupportDirectCopyValue = true;
			}
			else
			{
				//search object in guid
				FGuid ObjectGuidInParent;
				for (auto& KeyValue : this->MapGuidToObject)
				{
					if (KeyValue.Value == ObjectInParent)
					{
						ObjectGuidInParent = KeyValue.Key;
						break;
					}
				}
				FGuid ObjectGuidInPrefab;
				//find valid guid, get the guid in prefab, with the guid then get object, so that object is the real value
				if (ObjectGuidInParent.IsValid()
					&& SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(ObjectGuidInParent)//check if the guid exist in this sub-prefab, because there could be multiple sub-prefabs
					)
				{
					ObjectGuidInPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuidInParent];
				}
				if (ObjectGuidInPrefab.IsValid())
				{
					auto ObjectInPrefab = SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuidInPrefab];
					if (ObjectClass->IsChildOf(ULexWidget::StaticClass())
						|| ObjectClass->IsChildOf(ULexWidgetSubObjectBehaviour::StaticClass())
						|| ObjectClass->IsChildOf(ULexUIBehaviour::StaticClass())
						)
					{
						ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInPrefab, ObjectInPrefab, RawArrayIndex);
					}
					else
					{
						if (ObjectClass->HasAnyClassFlags(EClassFlags::CLASS_EditInlineNew)
							//&& ObjectProperty->HasAnyPropertyFlags(EPropertyFlags::CPF_InstancedReference)//is this necessary?
							)
						{
							if (!IsValid(ObjectInPrefab))
							{
								//EditInlineNew object can create new one
								ObjectInPrefab = NewObject<UObject>(ContextObject, ObjectClass, NAME_None, RF_NoFlags, ObjectInParent);
								ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInPrefab, ObjectInPrefab, RawArrayIndex);
								SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuidInPrefab] = ObjectInPrefab;
							}
							for (const auto PropertyItem : TFieldRange<FProperty>(ObjectClass))//check property inside object
							{
								ApplyPrefabPropertyValue(ObjectInPrefab, PropertyItem, ObjectInParent, ObjectInPrefab, SubPrefabData);
							}
						}
						else
						{
							auto InfoText = FText::Format(LOCTEXT("ApplyPrefabPropertyValue_MissingConditionWarning", "LexUI have not handle this condition:\nobject: '{0}'\nobjectClass: '{1}'")
								, FText::FromString(ObjectInParent->GetPathName()), FText::FromString(ObjectClass->GetPathName()));
							UE_LOG(LGUI, Warning, TEXT("%s"), *InfoText.ToString());
							FLexUIUtils::EditorNotification(InfoText, false);
						}
					}
				}
				else
				{
					auto InfoText = FText::Format(LOCTEXT("ApplyPrefabPropertyValue_ReferencingOuterObject", "This property '{0}' is referencing object which is not belongs to this prefab, will ignore it.")
						, FText::FromString(ObjectProperty->GetPathName()));
					UE_LOG(LGUI, Log, TEXT("%s"), *InfoText.ToString());
					FLexUIUtils::EditorNotification(InfoText, false);
				}
			}
		}
		else
		{
			bPropertySupportDirectCopyValue = true;
		}
	}
	else if (auto ArrayProperty = CastField<FArrayProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);//just copy so we don't need to resize it
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptArrayHelper ArrayHelperForDst(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < ArrayHelper.Num(); i++)
		{
			ApplyPrefabPropertyValue(ContextObject, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), ArrayHelperForDst.GetRawPtr(i), SubPrefabData);
		}
	}
	else if (auto MapProperty = CastField<FMapProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);//just copy so we don't need to resize it
		FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptMapHelper MapHelperForDst(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < MapHelper.Num(); i++)
		{
			ApplyPrefabPropertyValue(ContextObject, MapProperty->KeyProp, MapHelper.GetKeyPtr(i), MapHelperForDst.GetKeyPtr(i), SubPrefabData);
			ApplyPrefabPropertyValue(ContextObject, MapProperty->ValueProp, MapHelper.GetPairPtr(i), MapHelperForDst.GetPairPtr(i), SubPrefabData);
		}
		MapHelperForDst.Rehash();
	}
	else if (auto SetProperty = CastField<FSetProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);//just copy so we don't need to resize it
		FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptSetHelper SetHelperForDst(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < SetHelper.Num(); i++)
		{
			ApplyPrefabPropertyValue(ContextObject, SetProperty->ElementProp, SetHelper.GetElementPtr(i), SetHelperForDst.GetElementPtr(i), SubPrefabData);
		}
		SetHelperForDst.Rehash();
	}
	else if (auto StructProperty = CastField<FStructProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);
		auto StructPtr = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInSrc, RawArrayIndex);
		auto StructPtrForDst = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInPrefab, RawArrayIndex);
		for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
		{
			ApplyPrefabPropertyValue(ContextObject, *It, StructPtr, StructPtrForDst, SubPrefabData);
		}
	}
	else
	{
		bPropertySupportDirectCopyValue = true;
	}
	if (bPropertySupportDirectCopyValue)
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);
	}
}
void ULexUIPrefabHelperObject::ApplyPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames)
{
	GEditor->BeginTransaction(FText::Format(LOCTEXT("ApplyPrefabOnObjectProperties", "Apply Prefab Override: {0}"), FText::FromString(InObject->GetName())));
	InObject->Modify();
	this->Modify();

	auto Widget = Cast<ULexWidget>(InObject);
	if (!Widget)
	{
		Widget = InObject->GetTypedOuter<ULexWidget>();
	}
	auto SubPrefabData = GetSubPrefabData(Widget);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
			break;
		}
	}
	//object not exist
	if (!ObjectGuid.IsValid())
	{
		return;
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto ObjectInPrefab = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

	bCanCollectProperty = false;
	{
		for (auto PropertyName : InPropertyNames)
		{
			if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
			{
				//set to default value
				ApplyPrefabPropertyValue(ObjectInPrefab, Property, InObject, ObjectInPrefab, SubPrefabData);
				AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
				//delete item
				RemoveMemberPropertyFromSubPrefab(Widget, InObject, PropertyName);
				//notify
				FLexUIUtils::NotifyPropertyChanged(ObjectInPrefab, Property);

				SetAnythingDirty();
				
				auto RelatedPropertyName = GetExtraRelatedPropertyForApplyOrRevert(InObject, PropertyName);
				if (RelatedPropertyName != NAME_None)
				{
					if (auto RelatedProperty = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), RelatedPropertyName))
					{
						//set to default value
						ApplyPrefabPropertyValue(ObjectInPrefab, RelatedProperty, InObject, ObjectInPrefab, SubPrefabData);
						AfterObjectPropertyApplyOrRevert(InObject, RelatedPropertyName);
						//delete item
						RemoveMemberPropertyFromSubPrefab(Widget, InObject, RelatedPropertyName);
					}
				}
			}
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			//mark on sub prefab, because the object could belongs to subprefab's subprefab.
			SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(ObjectInPrefab, InPropertyNames);

			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULexUIManagerWorldSubsystem::RefreshAllUI();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootWidget(Widget));
}
void ULexUIPrefabHelperObject::ApplyAllOverrideToPrefab(UObject* InObject)
{
	auto Widget = Cast<ULexWidget>(InObject);
	if (!Widget)
	{
		Widget = InObject->GetTypedOuter<ULexWidget>();
	}
	auto SubPrefabData = GetSubPrefabData(Widget);
	auto SubPrefabRootWidget = GetSubPrefabRootWidget(Widget);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(LOCTEXT("ApplyPrefabOnAll_Transaction", "Apply Prefab Override"));
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		this->Modify();

		auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
					break;
				}
			}
			if (ObjectGuid.IsValid())
			{
				FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
				return SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
			}
			else
			{
				return (TObjectPtr<UObject>)nullptr;
			}
		};
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			auto SourceObject = DataItem.Object.Get();
			TSet<FName> FilterNameSet;
			if (SourceObject == SubPrefabRootWidget)//if is root widget of prefab, then skip it's transform
			{
				FilterNameSet.Add(ULexWidget::GetPropertyName_RelativeLocation());
				FilterNameSet.Add(ULexWidget::GetPropertyName_RelativeRotation());
				FilterNameSet.Add(ULexWidget::GetPropertyName_RelativeScale());
			}
			if (auto ObjectInPrefab = FindOriginObjectInSourcePrefab(SourceObject))
			{
				TSet<FName> NamesToClear;
				for (auto PropertyName : DataItem.MemberPropertyNames)
				{
					if (FilterNameSet.Contains(PropertyName))continue;
					NamesToClear.Add(PropertyName);
					if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
					{
						//set to default value
						ApplyPrefabPropertyValue(ObjectInPrefab, Property, SourceObject, ObjectInPrefab, SubPrefabData);
						AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
						//notify
						FLexUIUtils::NotifyPropertyChanged(ObjectInPrefab, Property);

						auto RelatedPropertyName = GetExtraRelatedPropertyForApplyOrRevert(InObject, PropertyName);
						if (RelatedPropertyName != NAME_None)
						{
							NamesToClear.Add(RelatedPropertyName);
							if (auto RelatedProperty = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), RelatedPropertyName))
							{
								ApplyPrefabPropertyValue(ObjectInPrefab, RelatedProperty, SourceObject, ObjectInPrefab, SubPrefabData);
								AfterObjectPropertyApplyOrRevert(InObject, RelatedPropertyName);
							}
						}
					}
				}
				//mark on sub prefab, because the object could belongs to subprefab's subprefab.
				SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(ObjectInPrefab, DataItem.MemberPropertyNames);

				for (auto& PropertyName : NamesToClear)
				{
					DataItem.MemberPropertyNames.Remove(PropertyName);
				}
			}
			else//if not find OriginObject, means the SourceObject is newly created (added new component) @todo: automatic add component to origin prefab
			{
				if (SourceObject->IsA(ULexUIBehaviour::StaticClass()))
				{
					auto InfoText = FText::Format(LOCTEXT("NewComponentInPrefabInstance", "Detect none tracked component: '{0}' in PrefabInstance. Note children of a Prefab instance cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to add component to the prefab asset itself, or unpack the prefab instance to remove its prefab connection."), FText::FromString(SourceObject->GetName()));
					FMessageDialog::Open(EAppMsgType::Ok, InfoText);
				}
			}
		}
		RemoveAllMemberPropertyFromSubPrefab(SubPrefabRootWidget, false);
		//save origin prefab
		{
			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}

		SetAnythingDirty();
		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULexUIManagerWorldSubsystem::RefreshAllUI();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootWidget(Widget));
}
#pragma endregion RevertAndApply

void ULexUIPrefabHelperObject::RefreshSubPrefabVersion(ULexWidget* InSubPrefabRootWidget)
{
	if (InSubPrefabRootWidget != nullptr)
	{
		auto& SubPrefabData = SubPrefabMap[InSubPrefabRootWidget];
		SubPrefabData.OverallVersionMD5 = SubPrefabData.PrefabAsset->GenerateOverallVersionMD5();
	}
	else
	{
		for (auto& KeyValue : SubPrefabMap)
		{
			KeyValue.Value.OverallVersionMD5 = KeyValue.Value.PrefabAsset->GenerateOverallVersionMD5();
		}
	}
}

void ULexUIPrefabHelperObject::MakePrefabAsSubPrefab(ULexUIPrefab* InPrefab, ULexWidget* InWidget, const TMap<FGuid, TObjectPtr<UObject>>& InSubMapGuidToObject, const TArray<FLexUIPrefabOverrideParameterData>& InObjectOverrideParameterArray)
{
	FLexUISubPrefabData SubPrefabData;
	SubPrefabData.PrefabAsset = InPrefab;
	SubPrefabData.OverallVersionMD5 = InPrefab->GenerateOverallVersionMD5();
	SubPrefabData.MapGuidToObject = InSubMapGuidToObject;
	SubPrefabData.ObjectOverrideParameterArray = InObjectOverrideParameterArray;
	
	auto FindOrAddSubPrefabObjectGuidInParentPrefab = [&](UObject* InObject) {
		for (auto& KeyValue : MapGuidToObject)
		{
			if (KeyValue.Value == InObject)
			{
				return KeyValue.Key;
			}
		}
		auto NewGuid = FGuid::NewGuid();
		MapGuidToObject.Add(NewGuid, InObject);
		return NewGuid;
	};
	for (auto& KeyValue : InSubMapGuidToObject)
	{
		auto GuidInParentPrefab = FindOrAddSubPrefabObjectGuidInParentPrefab(KeyValue.Value);
		if (!SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(GuidInParentPrefab))
		{
			SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParentPrefab, KeyValue.Key);
		}
	}
	SubPrefabMap.Add(InWidget, SubPrefabData);
	//mark SiblingIndex as default override parameter
	this->AddMemberPropertyToSubPrefab(InWidget, InWidget, ULexWidget::GetPropertyName_SiblingIndex());

	SetAnythingDirty();
}

void ULexUIPrefabHelperObject::BreakPrefabVariant()
{
	SubPrefabMap.Remove(LoadedRootWidget);
	PrefabAsset->bIsPrefabVariant = false;
	ClearInvalidObjectAndGuid();
}

void ULexUIPrefabHelperObject::RemoveSubPrefabByRootWidget(ULexWidget* InPrefabRootWidget)
{
	if (SubPrefabMap.Contains(InPrefabRootWidget))
	{
		SubPrefabMap.Remove(InPrefabRootWidget);
	}
#if WITH_EDITOR
	else if (MissingPrefab.Contains(InPrefabRootWidget))
	{
		MissingPrefab.Remove(InPrefabRootWidget);
	}
#endif
	ClearInvalidObjectAndGuid();
}

void ULexUIPrefabHelperObject::RemoveSubPrefabByAnyWidgetOfSubPrefab(ULexWidget* InPrefabWidget)
{
	if (auto RootWidget = GetSubPrefabRootWidget(InPrefabWidget))
	{
		RemoveSubPrefabByRootWidget(RootWidget);
	}
}

ULexUIPrefab* ULexUIPrefabHelperObject::GetPrefabAssetBySubPrefabObject(UObject* InObject)
{
	auto Widget = Cast<ULexWidget>(InObject);
	if (!Widget)
	{
		Widget = InObject->GetTypedOuter<ULexWidget>();
	}
	return GetSubPrefabData(Widget).PrefabAsset;
}

bool ULexUIPrefabHelperObject::CleanupInvalidSubPrefab()
{
	bool bAnythingChanged = false;

	{
		SubPrefabMap.Remove(nullptr);
		//invalid sub prefab
		TSet<ULexWidget*> SubPrefabKeysToRemove;
		for (auto& KeyValue : SubPrefabMap)
		{
			if (IsValid(KeyValue.Key) && !IsValid(KeyValue.Value.PrefabAsset))
			{
				SubPrefabKeysToRemove.Add(KeyValue.Key);
#if WITH_EDITOR
				MissingPrefab.Add(KeyValue.Key);
#endif
			}
		}
		//invalid guid mapped object
		TSet<FGuid> GuidKeysToRemove;
		for (auto& Item : SubPrefabKeysToRemove)
		{
			SubPrefabMap.Remove(Item);
			if (IsValid(Item))
			{
				//cleanup MapGuidToObject, because these object could belongs to sub prefab that is about to remove
				for (auto& GuidToObjectKeyValue : MapGuidToObject)
				{
					if (IsValid(GuidToObjectKeyValue.Value))
					{
						if (GuidToObjectKeyValue.Value->IsInOuter(Item) || GuidToObjectKeyValue.Value == Item)
						{
							if (!GuidKeysToRemove.Contains(GuidToObjectKeyValue.Key))
							{
								GuidKeysToRemove.Add(GuidToObjectKeyValue.Key);
							}
						}
					}
					else
					{
						if (!GuidKeysToRemove.Contains(GuidToObjectKeyValue.Key))
						{
							GuidKeysToRemove.Add(GuidToObjectKeyValue.Key);
						}
					}
				}
			}
		}
		if (SubPrefabKeysToRemove.Num() > 0)
		{
			if (OnSubPrefabNewVersionUpdated.IsBound())
			{
				OnSubPrefabNewVersionUpdated.Broadcast();
			}
		}
		for (auto& Item : GuidKeysToRemove)
		{
			MapGuidToObject.Remove(Item);
		}
		bAnythingChanged = SubPrefabKeysToRemove.Num() > 0 || GuidKeysToRemove.Num() > 0;
		if (bAnythingChanged)
		{
			SetAnythingDirty();
		}
#if WITH_EDITOR
		MissingPrefab.Remove(nullptr);
#endif
	}
	return bAnythingChanged;
}
bool ULexUIPrefabHelperObject::GetAnythingDirty()const
{
	return bAnythingDirty; 
}
void ULexUIPrefabHelperObject::SetAnythingDirty()
{
	bAnythingDirty = true;
	PrefabAsset->MarkPackageDirty();
}

#if WITH_EDITOR
#include "Editor.h"
#endif
void ULexUIPrefabHelperObject::CheckPrefabVersion()
{
	bool bAnythingChanged = CleanupInvalidSubPrefab();
	for (auto& KeyValue : SubPrefabMap)
	{
		auto& SubPrefabData = KeyValue.Value;
		if (SubPrefabData.OverallVersionMD5 != SubPrefabData.PrefabAsset->GenerateOverallVersionMD5())
		{
			this->RefreshOnSubPrefabDirty(SubPrefabData.PrefabAsset, KeyValue.Key);
			bAnythingChanged = true;
		}
	}

	if (this->ClearInvalidObjectAndGuid())
	{
		bAnythingChanged = true;
	}
	if (bAnythingChanged)
	{
		this->PrefabAsset->MarkPackageDirty();
	}
}

ULexUIPrefabHelperObject* ULexUIPrefabHelperObject::GetPrefabHelperObject_WhichManageThisWidget(ULexWidget* InWidget)
{
	if (!IsValid(InWidget))return nullptr;
	for (TObjectIterator<ULexUIPrefabHelperObject> Itr; Itr; ++Itr)
	{
		if (Itr->IsWidgetBelongsToThis(InWidget))
		{
			return *Itr;
		}
	}
	return nullptr;
}

#endif


#undef LOCTEXT_NAMESPACE