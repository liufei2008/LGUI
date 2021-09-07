// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/ActorComponent/UITextureBase.h"
#include "Core/ActorComponent/UITexture.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/UIInteractionGroup.h"
#include "Core/LGUIBehaviour.h"
#include "Core/ActorComponent/UIBackgroundBlur.h"
#include "Core/ActorComponent/UIBackgroundPixelate.h"

#include "Core/LGUIFontData.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "Core/LGUIAtlasData.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUISpriteData_BaseObject.h"

#include "Core/Actor/UIBaseActor.h"
#include "Core/Actor/UIContainerActor.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/Actor/UITextActor.h"
#include "Core/Actor/UITextureActor.h"
#include "Core/Actor/UIBackgroundBlurActor.h"
#include "Core/Actor/UIBackgroundPixelateActor.h"

#include "Event/LGUIEventSystem.h"
#include "Event/Interface/LGUIPointerClickInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Event/Interface/LGUIPointerDragEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDragDropInterface.h"
#include "Event/Interface/LGUIPointerSelectDeselectInterface.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIPointerEventData.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDrawableEvent_PresetParameter.h"
#include "Event/LGUIScreenSpaceInteraction.h"
#include "Event/LGUIScreenSpaceInteractionForNoneUI.h"
#include "Event/LGUIWorldSpaceInteraction.h"
#include "Event/LGUIWorldSpaceInteractionForNoneUI.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "Event/InputModule/LGUI_StandaloneInputModule.h"
#include "Event/InputModule/LGUI_TouchInputModule.h"

#include "Extensions/ViewportUITexture.h"
#include "Extensions/UIRing.h"
#include "Extensions/UIPolygon.h"
#include "Extensions/UIPolygonLine.h"
#include "Extensions/UIStaticMesh.h"
#include "Extensions/2DLineRenderer/UI2DLineChildrenAsPoints.h"
#include "Extensions/2DLineRenderer/UI2DLineRaw.h"
#include "Extensions/2DLineRenderer/UI2DLineRendererBase.h"
#include "Extensions/UISpriteSequencePlayer.h"
#include "Extensions/UISpriteSheetTexturePlayer.h"

#include "GeometryModifier/UIEffectGradientColor.h"
#include "GeometryModifier/UIEffectLongShadow.h"
#include "GeometryModifier/UIEffectOutline.h"
#include "GeometryModifier/UIEffectPositionAsUV.h"
#include "GeometryModifier/UIEffectShadow.h"
#include "GeometryModifier/UIEffectTextAnimation.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_PropertyWithEase.h"
#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_PropertyWithWave.h"
#include "GeometryModifier/TextAnimation/UIEffectTextAnimation_Selector.h"

#include "Interaction/UIButtonComponent.h"
#include "Interaction/UIEventBlockerComponent.h"
#include "Interaction/UIEventTriggerComponent.h"
#include "Interaction/UIScrollbarComponent.h"
#include "Interaction/UIScrollViewComponent.h"
#include "Interaction/UIScrollViewWithScrollbarComponent.h"
#include "Interaction/UISelectableComponent.h"
#include "Interaction/UISelectableTransitionComponent.h"
#include "Interaction/UISliderComponent.h"
#include "Interaction/UITextInputComponent.h"
#include "Interaction/UIToggleComponent.h"
#include "Interaction/UIToggleGroupComponent.h"

#include "Layout/UILayoutBase.h"
#include "Layout/UIGridLayout.h"
#include "Layout/UIHorizontalLayout.h"
#include "Layout/UIVerticalLayout.h"
#include "Layout/UIRoundedLayout.h"
#include "Layout/UILayoutElement.h"
#include "Layout/UISizeControlByAspectRatio.h"
#include "Layout/UISizeControlByOther.h"
#include "Layout/LGUICanvasScaler.h"

#include "PrefabSystem/ActorCopier.h"
#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/ActorReplaceTool.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabActor.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"

#include "Utils/BitConverter.h"
#include "Utils/LGUIUtils.h"

#include "LGUIBPLibrary.h"
#include "LGUIComponentReference.h"