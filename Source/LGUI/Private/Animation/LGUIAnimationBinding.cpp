// 

#include "Animation/LGUIAnimationBinding.h"
#include "Animation/LGUIAnimation.h"
#include "UObject/Object.h"
#include "Core/Actor/UIBaseActor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Core/ActorComponent/UIAnimationComp.h"
#include "Utils/LGUIUtils.h"
#include "LGUI.h"

/* FLGUIAnimationBinding interface
 *****************************************************************************/

PRAGMA_DISABLE_OPTIMIZATION

UObject* FLGUIAnimationBinding::FindRuntimeObject(UUIAnimationComp& AnimComp, const ULGUIAnimation* Animation) const
{
	if (bIsRootActor)
	{
		return AnimComp.GetOwner();
	}
	
	// Find in child actor
	AActor* FoundObject = LGUIUtils::FindChildActorByUIActorID(AnimComp.GetOwner(), UIActorID, true);
	if (FoundObject == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("FindRuntimeObject failed! WidgetName=%s ActorUniqueId=%d not exist in Animation=%s, UIContainerActor=%s"), 
			*ActorName.ToString(), UIActorID, *GetNameSafe(Animation), *GetNameSafe(AnimComp.GetOwner()));
	}
	UE_LOG(LGUI, Log ,TEXT("LGUIAnimationBinding::FindRuntimeObject， WidgetName=%s, FoundObject=%s"), *ActorName.ToString(), *GetNameSafe(FoundObject));
	return FoundObject;
}

PRAGMA_ENABLE_OPTIMIZATION