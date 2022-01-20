// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabHelperActor.generated.h"

class ULGUIPrefabHelperObject;
struct FLGUIPrefabOverrideParameterData;

UCLASS(NotBlueprintable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabHelperActor : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	ALGUIPrefabHelperActor();

	virtual void BeginPlay()override;
	virtual void PostInitProperties()override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Destroyed()override;
	virtual void BeginDestroy() override;

public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefabHelperObject* PrefabHelperObject = nullptr;
	void RevertPrefab();

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TArray<FLGUIPrefabOverrideParameterData> ObjectOverrideParameterArray;
#endif
#if WITH_EDITOR
	void AddMemberProperty(UObject* InObject, FName InPropertyName);
	void RemoveMemberProperty(UObject* InObject, FName InPropertyName);
	void RemoveMemberProperty(UObject* InObject);
	bool CheckParameters();
	void LoadPrefab(USceneComponent* InParent);
	void SavePrefab();
	//delete this prefab actor
	void DeleteThisInstance();
	void MoveActorToPrefabFolder();
	void CheckPrefabVersion();

	void CopyRootObjectParentAnchorData(UObject* InObject, UObject* OriginObject);

	void RevertPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet);
	void RevertPrefabOverride(UObject* InObject, FName InPropertyName);
	void RevertAllPrefabOverride();
	void ApplyPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet);
	void ApplyPrefabOverride(UObject* InObject, FName InPropertyName);
	void ApplyAllOverrideToPrefab();
#endif

#if WITH_EDITORONLY_DATA
public:
	FColor IdentityColor = FColor::Black;
	bool IsRandomColor = true;
	bool AutoDestroyLoadedActors = true;
	static TArray<FColor> AllColors;
private:
	static FName PrefabFolderName;
	TWeakPtr<SNotificationItem> NewVersionPrefabNotification;
	void OnNewVersionRevertPrefabClicked();
	void OnNewVersionDismissClicked();

	bool bAnythingDirty = false;
	bool bCanCollectProperty = true;
	bool bCanNotifyDetachment = false;

	void OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent);
	void OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain);
	void TryCollectPropertyToOverride(UObject* InObject, FProperty* InMemberProperty);

	void OnLevelActorAttached(AActor* Actor, const AActor* AttachTo);
	void OnLevelActorDetached(AActor* Actor, const AActor* DetachFrom);

	struct FAttachmentActorStruct
	{
		AActor* Actor = nullptr;
		AActor* DetachFrom = nullptr;
		AActor* AttachTo = nullptr;
	};
	FAttachmentActorStruct AttachmentActor;
	void CheckAttachment();
#endif
};
