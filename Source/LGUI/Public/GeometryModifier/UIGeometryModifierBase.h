// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Core/UIGeometry.h"
#include "Core/ActorComponent/UIRenderable.h"
#include "Components/ActorComponent.h"
#include "UIGeometryModifierBase.generated.h"


//For modify ui geometry, act like a filter
UCLASS(Abstract)
class LGUI_API UUIGeometryModifierBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUIGeometryModifierBase();

protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	//Execute order of this effect in actor. The greater executeOrder is the later this effect execute
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int executeOrder = 0;

	FORCEINLINE class UUIRenderable* GetRenderableUIItem();
private:
	UPROPERTY(Transient) class UUIRenderable* renderableUIItem = nullptr;
	
public:
	FORCEINLINE int GetExecuteOrder()const { return executeOrder; }
	/*
	Add or modify vertex/triangle
	InOutOriginVerticesCount:orign vertex count; after modify, new vertex count must be set to this
	InOutOriginTriangleIndicesCount:orign triangle indices count; after modify, new triangle indices count must be set to this
	OutTriangleChanged: if this modifier affect triangle, then set this to true
	*/
	virtual void ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged) PURE_VIRTUAL(UUIGeometryModifierBase::ModifyUIGeometry,);
};
