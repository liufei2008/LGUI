// Copyright 2019-2022 LexLiu. All Rights Reserved.
#include "K2Node_LGUICompRef.h"
#include "LGUIHeaders.h"
#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"
#include "KismetCompilerMisc.h"
#include "KismetCompiledFunctionContext.h"
#include "EdGraphUtilities.h"
#include "Textures/SlateIcon.h"
#include "BlueprintNodeSignature.h"
#include "Engine/Blueprint.h"
#include "SPinTypeSelector.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_Variable.h"
#include "K2Node_CallFunction.h"

#define LOCTEXT_NAMESPACE "UK2Node_LGUICompRef_GetComponent"

void UK2Node_LGUICompRef_GetComponent::AllocateDefaultPins()
{
	//input pin
	UScriptStruct* compRefScriptStruct = FLGUIComponentReference::StaticStruct();
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, compRefScriptStruct, TEXT("LGUI Component Reference"));
	//output pin
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, TEXT("Output"));
}
void UK2Node_LGUICompRef_GetComponent::ReconstructNode()
{
	//SetOutputPinType();
	Super::ReconstructNode();
}
void UK2Node_LGUICompRef_GetComponent::PostReconstructNode()
{
	SetOutputPinType();
	Super::PostReconstructNode();
}
FText UK2Node_LGUICompRef_GetComponent::GetNodeTitle(ENodeTitleType::Type TitleType)const
{
	//if (TitleType == ENodeTitleType::FullTitle)
	//{
	//	return autoOutputTypeSuccess ? LOCTEXT("UK2Node_LGUI_GetComponentTitle", ".") : LOCTEXT("UK2Node_LGUI_GetComponentTitle", "!Get");
	//}
	return LOCTEXT("GetComponentTitle_Full", "Get Component for LGUIComponentReference");
}
FText UK2Node_LGUICompRef_GetComponent::GetCompactNodeTitle()const
{
	return autoOutputTypeSuccess ? LOCTEXT("UK2Node_LGUI_GetCompactNodeTitle", "\x2022") : LOCTEXT("UK2Node_LGUI_GetCompactNodeTitle", "!Get");
}
FText UK2Node_LGUICompRef_GetComponent::GetKeywords() const
{
	return FText(LOCTEXT("UK2Node_LGUI_GetKeywords", "ComponentRef"));
}
FText UK2Node_LGUICompRef_GetComponent::GetTooltipText()const
{
	return LOCTEXT("GetComponent_Tooltip", "Get component from LGUIComponentReference.\
\nIf the node is \"!Get\", that means auto cast failed, so you need to cast the result ActorComponent to your desired type.");
}
//TSharedPtr<SWidget> UK2Node_LGUICompRef_GetComponent::CreateNodeImage() const
//{
//	return SPinTypeSelector::ConstructPinTypeImage(Pins[0]);
//}
//FSlateIcon UK2Node_LGUICompRef_GetComponent::GetIconAndTint(FLinearColor& OutColor)const
//{
//	return Super::GetIconAndTint(OutColor);
//}
void UK2Node_LGUICompRef_GetComponent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar)const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* RetRefNodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(RetRefNodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, RetRefNodeSpawner);
	}
}
FBlueprintNodeSignature UK2Node_LGUICompRef_GetComponent::GetSignature()const
{
	FBlueprintNodeSignature nodeSignature = Super::GetSignature();
	return nodeSignature;
}
FText UK2Node_LGUICompRef_GetComponent::GetMenuCategory()const
{
	return LOCTEXT("UK2Node_LGUI_GetMenuCategory", "LGUI");
}

void UK2Node_LGUICompRef_GetComponent::SetOutputPinType()
{
	if (Pins.Num() >= 2)
	{
		if (Pins[0]->LinkedTo.Num() > 0)
		{
			if (auto variablePin = Pins[0]->LinkedTo[0])
			{
				if (UK2Node_Variable* variableNode = Cast<UK2Node_Variable>(variablePin->GetOwningNode()))
				{
					auto propertyName = variableNode->GetVarName();
					if (auto blueprint = variableNode->GetBlueprint())
					{
						if (auto generatedClass = blueprint->GeneratedClass)
						{
							if (auto objectInstance = generatedClass->GetDefaultObject())
							{
								auto propertyField = TFieldRange<FProperty>(generatedClass);
								for (const auto propertyItem : propertyField)
								{
									if (auto structProperty = CastField<FStructProperty>(propertyItem))
									{
										if (structProperty->Struct == FLGUIComponentReference::StaticStruct())
										{
											if (structProperty->GetFName() == propertyName)
											{
												FLGUIComponentReference* structPtr = structProperty->ContainerPtrToValuePtr<FLGUIComponentReference>(objectInstance);
												if (structPtr->GetComponentClass() != nullptr)
												{
													UEdGraphPin* outputPin = Pins[1];
													outputPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
													outputPin->PinType.PinSubCategoryObject = structPtr->GetComponentClass().Get();
													autoOutputTypeSuccess = true;
													return;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		UEdGraphPin* outputPin = Pins[1];
		outputPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		outputPin->PinType.PinSubCategoryObject = UActorComponent::StaticClass();
		//outputPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
		//outputPin->PinType.PinSubCategoryObject = nullptr;
		autoOutputTypeSuccess = false;
	}
}
void UK2Node_LGUICompRef_GetComponent::ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog)const
{
	Super::ValidateNodeDuringCompilation(MessageLog);
	//@todo: give some hint when class change.
	//if (!autoOutputTypeSuccess)
	//{
	//	auto msg = FString(TEXT("Auto cast fail! You need to cast the result ActorComponent to your desired type."));
	//	MessageLog.Note(*msg);
	//}
}
void UK2Node_LGUICompRef_GetComponent::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);
	SetOutputPinType();
}
void UK2Node_LGUICompRef_GetComponent::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
	SetOutputPinType();
}
void UK2Node_LGUICompRef_GetComponent::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);
	SetOutputPinType();
}
void UK2Node_LGUICompRef_GetComponent::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UFunction* BlueprintFunction = ULGUIBPLibrary::StaticClass()->FindFunctionByName("K2_LGUICompRef_GetComponent");
	if (!BlueprintFunction)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InvalidFunctionName", "The function has not been found.").ToString(), this);
		return;
	}

	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);

	CallFunction->SetFromFunction(BlueprintFunction);
	CallFunction->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunction, this);

	CompilerContext.MovePinLinksToIntermediate(*Pins[0], *CallFunction->FindPinChecked(TEXT("InLGUICompRef")));
	auto FunctionResultPin = CallFunction->FindPinChecked(TEXT("OutResult"));
	FunctionResultPin->PinType.PinCategory = Pins[1]->PinType.PinCategory;
	FunctionResultPin->PinType.PinSubCategoryObject = Pins[1]->PinType.PinSubCategoryObject;
	CompilerContext.MovePinLinksToIntermediate(*Pins[1], *FunctionResultPin);

	//After we are done we break all links to this node (not the internally created one)
	BreakAllNodeLinks();
}
bool UK2Node_LGUICompRef_GetComponent::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
	return false;
}
bool UK2Node_LGUICompRef_GetComponent::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason)const
{
	return false;
}
bool UK2Node_LGUICompRef_GetComponent::ReferencesVariable(const FName& InVarName, const UStruct* InScope)const
{
	GetSchema()->ReconstructNode(*(UK2Node_LGUICompRef_GetComponent*)this);
	return false;
}

#undef LOCTEXT_NAMESPACE