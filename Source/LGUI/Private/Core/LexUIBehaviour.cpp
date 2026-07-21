// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIBehaviour.h"

#include "LGUI.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexWidget.h"

ULexUIBehaviour::ULexUIBehaviour()
{
	bCanExecuteBlueprintEvent = GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native);
	CallbacksBeforeAwake.SetNumZeroed((int)ECallbackFunctionType::COUNT);
}

void ULexUIBehaviour::BeginPlay()
{
	auto Widget = this->GetWidget();
	check(Widget);
	check (!this->bIsAwakeCalled);
	this->Call_Awake();
	if (Widget->GetWidgetActiveInHierarchy())
	{
		check (!this->bIsEnableCalled);
		bCanExecuteTick = bStartWithTickEnabled;
		this->Call_OnEnable();
	}
}
void ULexUIBehaviour::EndPlay()
{
	if (bIsEnableCalled)
	{
		Call_OnDisable();
	}
	if (bIsAwakeCalled)
	{
		Call_OnDestroy();
	}
}
void ULexUIBehaviour::OnRegister()
{
	if (auto Widget = GetWidget())
	{
		Widget->GetWidgetActiveChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnWidgetActiveChanged);
		Widget->GetTransformChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnTransformChanged);
		Widget->GetDimensionChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnDimensionsChanged);
		Widget->GetChildDimensionChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnChildDimensionsChanged);
		Widget->GetAttachmentChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnAttachmentChanged);
		Widget->GetSiblingIndexChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnSiblingIndexChanged);
		Widget->GetInteractableChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnInteractableChanged);
		Widget->GetRaycastableChangedEvent().AddUObject(this, &ULexUIBehaviour::Call_OnRaycastableChanged);
	}
}
void ULexUIBehaviour::OnUnregister()
{
	if (IsValid(CacheWidget))
	{
		CacheWidget->GetWidgetActiveChangedEvent().RemoveAll(this);
		CacheWidget->GetTransformChangedEvent().RemoveAll(this);
		CacheWidget->GetDimensionChangedEvent().RemoveAll(this);
		CacheWidget->GetChildDimensionChangedEvent().RemoveAll(this);
		CacheWidget->GetAttachmentChangedEvent().RemoveAll(this);
		CacheWidget->GetSiblingIndexChangedEvent().RemoveAll(this);
		CacheWidget->GetInteractableChangedEvent().RemoveAll(this);
		CacheWidget->GetRaycastableChangedEvent().RemoveAll(this);
	}
}

#if WITH_EDITOR
void ULexUIBehaviour::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
	}
}
#endif

UWorld* ULexUIBehaviour::GetWorld() const
{
	auto Widget = GetWidget();
	if (!Widget)return nullptr;
	return Widget->GetWorld();
}

int32 ULexUIBehaviour::GetComponentIndexInWidget() const
{
	if (auto Widget = GetWidget())
	{
		return Widget->GetAllComponents().IndexOfByKey(this);
	}
	return INDEX_NONE;
}

void ULexUIBehaviour::SetCanExecuteTick(bool Value)
{
	if (bCanExecuteTick != Value)
	{
		bCanExecuteTick = Value;
		if (bIsStartCalled)
		{
			if (bCanExecuteTick)
			{
				ULexUIManagerWorldSubsystem::AddLexUIBehavioursForTick(this);
			}
			else
			{
				ULexUIManagerWorldSubsystem::RemoveLexUIBehavioursFromTick(this);
			}
		}
	}
}

void ULexUIBehaviour::OnEnable()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnEnable();
	}
}
void ULexUIBehaviour::OnDisable()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnDisable();
	}
}

void ULexUIBehaviour::OnDestroy()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnDestroy();
	}
}

void ULexUIBehaviour::Awake()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveAwake();
	}
}
void ULexUIBehaviour::Start()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveStart();
	}
}
void ULexUIBehaviour::Tick(float DeltaTime)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveTick(DeltaTime);
	}
}

void ULexUIBehaviour::Call_Awake()
{
	for (auto& CallbackFunc : CallbacksBeforeAwake)
	{
		if (CallbackFunc != nullptr)
		{
			CallbackFunc();
		}
	}
	CallbacksBeforeAwake.Empty();
	
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (bIsAwakeCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Awake already executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsAwakeCalled = true;
	Awake();
}

void ULexUIBehaviour::Call_OnEnable()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (bIsEnableCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d OnEnable already executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsEnableCalled = true;

	OnEnable();
	if (!bIsStartCalled)
	{
		ULexUIManagerWorldSubsystem::AddLexUIBehavioursForStart(this);
	}
	else
	{
		if (bCanExecuteTick)
		{
			ULexUIManagerWorldSubsystem::AddLexUIBehavioursForTick(this);
		}
	}
}

void ULexUIBehaviour::Call_OnDisable()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (!bIsEnableCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d OnEnable not executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsEnableCalled = false;

	OnDisable();
	if (!bIsStartCalled)
	{
		ULexUIManagerWorldSubsystem::RemoveLexUIBehavioursFromStart(this);
	}
	else
	{
		if (bCanExecuteTick)
		{
			ULexUIManagerWorldSubsystem::RemoveLexUIBehavioursFromTick(this);
		}
	}
}

void ULexUIBehaviour::Call_OnDestroy()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (!bIsAwakeCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Awake already executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsAwakeCalled = false;
	OnDestroy();
}

void ULexUIBehaviour::Call_Start()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (bIsStartCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Start already executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsStartCalled = true;
	Start();
}

ULexWidget* ULexUIBehaviour::GetWidget() const
{
	if (!IsValid(CacheWidget))
	{
		CacheWidget = this->GetTypedOuter<ULexWidget>();
	}
	return CacheWidget.Get();
}

FString ULexUIBehaviour::GetPathDisplayName() const
{
	return GetWidget()->GetPathDisplayName() / this->GetName();
}

void ULexUIBehaviour::DestroyComponent()
{
	GetWidget()->RemoveComponent(this);
}

void ULexUIBehaviour::OnInteractableChanged(bool Interactable) 
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnInteractableChanged(Interactable);
	}
}

void ULexUIBehaviour::OnTransformChanged()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnTransformChanged();
	}
}

void ULexUIBehaviour::OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
	}
}

void ULexUIBehaviour::OnChildDimensionsChanged(ULexWidget* Child, bool PivotChanged, bool WidthChanged,
	bool HeightChanged)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnChildDimensionsChanged(Child, PivotChanged, WidthChanged, HeightChanged);
	}
}

void ULexUIBehaviour::OnAttachmentChanged()
{ 
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnAttachmentChanged();
	}
}

void ULexUIBehaviour::OnSiblingIndexChanged()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnSiblingIndexChanged();
	}
}

void ULexUIBehaviour::OnRaycastableChanged(bool Raycastable)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnRaycastableChanged(Raycastable);
	}
}

void ULexUIBehaviour::Call_OnInteractableChanged(bool Interactable)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnInteractableChanged(Interactable);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnInteractableChanged(Interactable);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake[(int)ECallbackFunctionType::OnInteractableChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnInteractableChanged(Interactable);
				}};
		}
	}
}

void ULexUIBehaviour::Call_OnTransformChanged()
{
#if WITH_EDITOR
	if (!GetWorld())return;
	if (!GetWorld()->IsGameWorld())//edit mode
	{
		OnTransformChanged();
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnTransformChanged();
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake[(int)ECallbackFunctionType::OnTransformChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnTransformChanged();
				}};
		}
	}
}

void ULexUIBehaviour::Call_OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
#if WITH_EDITOR
	if (!GetWorld())return;
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake[(int)ECallbackFunctionType::OnDimensionsChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
				}};
		}
	}
}

void ULexUIBehaviour::Call_OnChildDimensionsChanged(ULexWidget* Child, bool PivotChanged, bool WidthChanged,
	bool HeightChanged)
{
#if WITH_EDITOR
	if (!GetWorld())return;
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnChildDimensionsChanged(Child, PivotChanged, WidthChanged, HeightChanged);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnChildDimensionsChanged(Child, PivotChanged, WidthChanged, HeightChanged);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake[(int)ECallbackFunctionType::OnChildDimensionsChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnChildDimensionsChanged(Child, PivotChanged, WidthChanged, HeightChanged);
				}};
		}
	}
}

void ULexUIBehaviour::Call_OnAttachmentChanged()
{
#if WITH_EDITOR
	if (!GetWorld())return;
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnAttachmentChanged();
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnAttachmentChanged();
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake[(int)ECallbackFunctionType::OnAttachmentChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnAttachmentChanged();
				}};
		}
	}
}

void ULexUIBehaviour::Call_OnSiblingIndexChanged()
{
#if WITH_EDITOR
	if (!GetWorld())return;
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnSiblingIndexChanged();
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnSiblingIndexChanged();
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake[(int)ECallbackFunctionType::OnSiblingIndexChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnSiblingIndexChanged();
				}};
		}
	}
}

void ULexUIBehaviour::Call_OnWidgetActiveChanged(bool WidgetActive)
{
#if WITH_EDITOR
	if (!GetWorld())return;
	if (!this->GetWorld()->IsGameWorld())return;//edit mode
#endif
	if (bIsAwakeCalled)
	{
		if (WidgetActive)
		{
			if (!bIsEnableCalled)
			{
#if WITH_EDITOR
				if (GetWorld() && !GetWorld()->IsGameWorld())//edit mode
				{

				}
				else
#endif
				{
					Call_OnEnable();
				}
			}
		}
		else
		{
			if (bIsEnableCalled)
			{
#if WITH_EDITOR
				if (GetWorld() && !this->GetWorld()->IsGameWorld())//edit mode
				{

				}
				else
#endif
				{
					Call_OnDisable();
				}
			}
		}
	}
	else//awake not called, should be the first time that get WidgetActive
	{
		if (!this->GetWidget()->HasBegunPlay())
		{
			if (WidgetActive)
			{
				Call_Awake();
				if (!bIsEnableCalled)
				{
					Call_OnEnable();
				}
			}
		}
	}
}

void ULexUIBehaviour::Call_OnRaycastableChanged(bool Raycastable)
{
#if WITH_EDITOR
	if (!GetWorld())return;
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnRaycastableChanged(Raycastable);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnRaycastableChanged(Raycastable);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake[(int)ECallbackFunctionType::OnRaycastableChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnRaycastableChanged(Raycastable);
				}};
		}
	}
}
