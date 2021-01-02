## 2.10.0
#### Modify:
Rename UISector to UIPolygon, remake it so it is much handy now. (You may need UE's "redirect" if you use UISector before, but some properties may need to mannually setup).
Remake UIRing, based on UI2DLineRenderer. Now UIRing and UI2DLine is more powerfull. (You may need UE's "redirect" if you use UIMesh before).
Rename UIMesh to UIStaticMesh. (You may need UE's "redirect" if you use UIMesh before).
LGUIPrefab will not crash editor, but give an error message if something wrong happen.
#### NewFeature:
Add UIPolygonLine(a line/frame version of UIPolygon).
Material now support WorldPositionOffset.
#### Fix:
Click to select UI elements, now is much stable.
Fix drawcall collection info display error.
Lot of bugs fixed.
## 2.9.10
#### Fix:
UIBackgroundPixelate flipped.
#### Modify:
UIBackgroundBlur and UIBackgroundPixelate now become stable, but not support mobile.
Simplify post process shader.
## 2.9.9
#### Fix:
Fix a crash caused by LGUI's scene outliner extension.
Fix a crash when package android platform.
#### NewFeature:
New type of UI element: UIMesh, allow to use a UStaticMesh to render in screen space UI.
#### Modify:
Simplify post process shader.
## 2.9.8
#### Fix:
Fix a crash when click RecreateThis in prefab.
Fix a issue in UIScrollViewWithScrollbar, add more hints.
## 2.9.7
#### Fix:
Fix a crash when destroy UIBackgroundBlur and UIBackgroundPixelate.
Fix bug: UITexture not update when click snap size.
#### Change:
When select a actor and copy/paste use LGUI's method, the new actors will use selected actor as parent.
--Thanks to "四青", he helped testing this plugin.
#### NewFeature:
Support material masked blend mode for screen space UI's custom material.
## 2.9.6
#### Fix:
Fix ScrollViewWithScrollbar issue.
Fix Start call issue in LGUIBehaviour.
#### NewFeature:
Add a blueprint function for runtime create LGUISpriteData.
## 2.9.5  
#### Fix:  
Fix a crash caused by Raycaster.  
Fix problems with undo system.  
--Thanks to "Wifi"(it's acturally a name), he helped testing this plugin.  
## 2.9.4  
#### Fix:  
Duplicated screen space UI on viewport, when PIE with multiplayer(player count more than 1).  
## 2.9.3  
#### Fix:  
Avoid GWorld for LTweenActor creation, so LTween related code should provide a object to get world.  
## 2.9.2  
#### NewFeature:  
Android platform ready!  
## 2.9.1  
#### NewFeature:  
LGUIDrawableEvent now have preset parameter struct, like LGUIDrawableEvent_Float/ LGUIDrawableEvent_Int32...  
#### Fix;  
Fix UIPlayTween's loop issue.  
Fix LGUIDrawableEvent's editor issue when use Vector2/Vector3/Vector4/Color/LinearColor/Rotator as parameter.  
## 2.9  
#### NewFeature:  
TextAnimation, WOW!!! You can use UIEffectTextAnimation to customize it.  
LGUIDrawableEvent now support multiple component of same type on actor, use component name to identify.  
LGUIDrawableEvent now get a very clean and clear editor interface.  
LGUIPlayTween, for quick setup tween animation.  
LGUIComponentReference now get a specific blueprint node, that you can get component directly.  
#### Fix:  
Fix LGUICanvasScaler scale and size issue.  
Fix Layout update issue, when there is hierarchy nested LGUICanvas exist.  
## 2.8.6  
#### Fix:  
Fix ScreenSpaceUI preview in editor viewport.  
Other fix: crash caused by LGUICanvas, pixel perfect render of UIText.  
## 2.8.5  
#### Fix:  
Fix a crash when delete LGUICanvas.  
Fix pixel perfect calculation issue caused by 2.8.2 version.  
Fix: add or remove canvas not update correctly.  
#### Change:  
Change LGUICanvas's update methold. Basicall the old methold will update hierarchy multiple times in one frame, new method will update in multiple frames, this is more stable.  
## 2.8.4  
#### Fix:  
Fix alpha not update correctly issue, caused by 2.8.3 version.  
Fix a crash when unload current level after use "Copy Component Values".  
Fix drawcall count error.  
Fix a crash when deserialize actor from prefab.  
Fix hierarchyIndex sort error.  
#### NewFeature:  
Add SCOPE_CYCLE_COUNTER to LGUIBehaviour's update function.  
## 2.8.3  
#### Improve:  
Increase performance.  
#### NewFeature:  
Now we can preview screen space UI in edit mode, just choose a viewport(which you can find in "Window/Viewports/") and click the button "LGUI Tools/Active Viewport as LGUI Preview".  
#### Fix:  
Duplicate actor or generate a prefab in edit mode will also create a LGUIManagerActor, this is incorrect, fixed.  
Click and select UI element in viewport more stable now.  
## 2.8.2  
#### NewFeature:  
LGUICanvas support render to custom RenderTarget2D.(Experimental)  
#### Improve:  
Performance improve.  
#### Fix:  
UISprite's fill type in pixel perfect mode.  
## 2.8.1  
#### Change:  
Remove UIPanel.  
#### NewFeature:  
Add Canvas and drawcall count tip info in scene outliner.  
#### Fix:  
Fix basic setup not interactable issue.  
Fix UIText disappear when second time input from UITextInputComponent in some special case.  
## 2.8  
#### Change:  
LGUIEventSystem's RegisterGlobalListener now use FLGUIBaseEventDelegate/FLGUIBaseEventDynamicDelegate. After your delegate is called, you can cast ULGUIBaseEventData to ULGUIPointerEventData if you need.  
LGUI_Saved_Data.json file now save to project intermediate folder.  
Move LGUISettings to DefaultEngine.ini.  
LGUIPointerXXXInterface.h file move to Event/Interface/ folder.  
#### NewFeature:  
EventSystem now user InputModule to handle input events, this should be more extensible for us to make different input for different platform, such as mobile.  
Add dragToHold parameter to LGUIXXXRayemitter.  
LGUIXXXInputMobule and LGUIXXXRaycaster now can use ActorComponent's Activate/Deactivate.  
#### Fix:  
Fix a crash caused by UIText's OverflowType=ClampContent.  
Fix UITexture's UVRect issue.  
Fix LGUIBehaviour execute order error.  
## 2.7.2  
#### Change:  
Change FLGUIPointerEventData to ULGUIPointerEventData.  
Deprecate LGUIPointerEventData's currentComponent, use enterComponent instead.  
#### Improve:  
Auto UIItem's hierarchyIndex management.  
## 2.7.1  
#### New feature:  
UISprite add fill type: Horizontal/ Vertical/ Radial90/ Radial180/ Radial360, all these type can flip fill direction and support several origin type.  
Editor can preview canvas resolution scale of LGUICanvasScaler.  
UISizeControlByAspectRatio now support FitInParent/ EnvelopeParent.  
#### Improve:  
Improve UISelectableTransitionComponent.  
Improve layout editor.  
#### Fix:  
Fix virtual cameram draw issue of LGUICanvasScaler.  
Fix a build warning of bind none const function.  
## 2.7  
#### Modify:  
Change UIComponentBase to LGUIBehaviour. If you already use UIComponentBase, follow these steps:  
```  
a.Find config file in your (UE4Project)/Config/DefaultEngine.ini  
b.Find or add line [/Script/Engine.Engine]  
c.Add these lines under [/Script/Engine.Engine]:  
    +ActiveClassRedirects=(OldClassName="/Script/LGUI.UIComponentBase",NewClassName="/Script/LGUI.  
    LGUIBehaviour")  
```  
UIBackgroundBlur now use maskTexture to acturally mask out blur area. add strengthTexture to control blur strength.  
#### New feature:  
LGUIBehaviour is a totally remaked class, all I want is trying to make it more like Unity's MonoBehaviour. Because I think Unity's interfaces are more normative.  
LGUIBehaviour can control event execute order by changing LGUIBehaviourExecuteOrder in LGUISettings.  
#### Fix:  
UISlider's handle's position issue when DirectionType=RightToLeft/TopToBottom.  
## 2.6.3  
#### New feature:  
Now we can pass UMaterialInstanceDynamic to LGUIRenderable's custom material.  
## 2.6.2  
#### Improve:  
Improve italic style text render.  
#### New feature:  
Now we can click to select UI direct inside level editor.  
## 2.6.1  
#### Improve:  
Improve visual effect and performance of UIBackgroundBlur.  
## 2.6  
#### New feature:  
Add UIPostProcess UI element type, now we can extend our own post process on UI.  
Add UIBackgroundBlur effect, which is based on UIPostProcess.  
#### Fix:  
Fix a compile error in some specific condition(inline function xxx not defined)  
Improve pixel perfect and dynamic pixel scale of UIText render  
Fix a bug: If a UProperty is SceneComponent and AttachParent property have value, then endless loop will occur.  
#### Modify:  
Recreate UIFlyoutMenu and UIComboBox.  
Change LGUIPointerEventData's property "hitComponent" to "currentComponent".  
## 2.5  
#### New feature:  
Add rich text support in UIText. Check "Rich Text" property to see more.  
LGUICanvasScaler now provides more options for users to control UI'adaptive behaviour.  
#### Fix:  
Fix a crash on LGUIPrefab when hit revert button.  
Fix a crash when change level, caused by UIItem.  
Fix a UIText texture display error.  
## 2.4.1  
#### Modify:  
Delete actor will also check delete prefab.  
Add engine version to prefab, so prefab made by different engine version will show a warning.  
#### Fix:  
Fix a crash when use UIText in blueprint actor editor, although work in blueprint actor is not recommaded for LGUI.  
## 2.4  
#### Fix:  
Fix a crash when compile blueprint that inherit from UIItem.  
Fix a compile warning in UIItem.  
#### New feature:  
Add UISpriteBase_BP and UITextureBase_BP for blueprint extend custom UI element.  
LTween add blueprint tween for some basic types, eg. FloatTo/IntTo/Vector2To/Vector3To...  
## 2.3.4  
#### Modify:  
Increase performance.  
#### New feature:  
Add UIRenderable_BP for blueprint can extend custom UI element.  
## 2.3.3  
#### Fix:  
Fix a fatal error after shutdown, caused by LGUIFontData.  
Fix a pontential link error caused by UI2DLineRendererBase.  
## 2.3.2  
#### Fix:  
Recreate default plugin's prefab assets.  
Fix a bug for LGUIDrawableEvent when recompile blueprint.  
Add more check to LGUIDrawableEvent to prevent crash.  
#### New feature:  
Add lambda function to LTween's RegisterUpdateEvent, it's just a quick entry and not for blueprint.  
## 2.3.1  
#### Fix:  
Fix crash when redo from Prefab instance.  
#### New feature:  
Add "Browser to Prefab asset" to prefab edit.  
## 2.3:  
#### Fix:  
Fix crash when add LGUICanvas in blueprint actor. But blueprint actor for LGUI is still not recommended.  
#### Modify:  
Rename LTween's XXXGetterFunction to LTweenXXXGetterFunction, and same change to XXXSettterFunction.  
#### New feature:  
LGUIFont now can pack font texture into UISprite's atlas texture! This can be very helpful to reduce drawcall. All we need to do is to change LGUIFont's packingTag property to a UISprite's packingTag.  
Create folder for LGUIPrefab actor in SceneOutliner.  
## 2.2：  
#### Fix:  
Fix a bug of UITextInputComponent multi line edit.  
Screen space UI render issue.  
#### New feature:  
Navigation support, eg: use tab key to navigate to next UISelectableComponent. Call NavigationXXX function in LGUIEventSystem.  
UITextInputComponent support ignore keys, canbe useful for navigation.   
LGUIComponentReference now support multiple component of same type on same actor, all we need to do is just click and select a component.  
## 2.1:  
#### Fix:  
Screen space UI sort order problem.  
UITextInputComponent more stable.  
#### Update:  
Change default rotate from (-90, 0, 90) to (-90, 0, 0).  
Replace LGUIEventSystemActor with LGUIEventSystem component.  
Modify for 2DLineRendererBase, remove CurrentPointArray property, add UseGrowValue to determin whether or not use grow value.  
#### New feature:  
Add uiScale property to LGUICanvas, maybe useful in future.  
## 2.0:  
Screen space UI (Or HUD) not get a huge performance update. The 1.x version use SceneCapture2D, which is vary bad on performance. But now the new 2.x version of LGUI's HUD, is render direct on screen, no additional performance cost!  
Remove UIPanel, use LGUICanvas to render all UI elements.  
Remove UIRoot, LGUICanvas and LGUICanvasScaler will do the job.  




## 1.14:  
#### Fix:  
button "Create Prefab" not create content.  
some crash fixed.  
#### Update:  
change UIItem's base class UPrimitiveComponent to USceneComponent, this can save a lot of disk space and memory. But!!!!! You Need To Recreate Your Prefab Which Contains LGUI Elemnt.  
add RenderTargetSizeMultiply for UIRoot, increase this value can perform a supersampleing thing.  
#### New feature:  
(still under develop)Add a button to UIRoot component named "Open Screen Space UI Viewer", which can open a window to preview Screen Space UI.  
Save scene outliner's hierarchy state now support save and open map. 
## 1.13:  
#### Fix:  
material's IsFontTexture property not set correctly.  
#### New feature:  
save scene outliner's hierarchy state(actor's folder) before play, and restore it after end play.  
## 1.12:  
#### Fix:  
screen space UI alpha blend problem.  
#### Upgrade:  
prefab: when select a actor, drag to create prefab will attach to that actor.  
editor: ReplaceUIElement now can keep and replace object reference.  
#### Modify:  
Avoid using GWorld, so LoadPrefab must provide a object to get world.  
#### New feature:  
A new column on SceneOutliner, provide ability to create/modify UI direct in SceneOutliner.  
## 1.11:  
Fix some bugs.  
Fix bug: crash caused by UITextInputComponent, when select text content.  
Fix LTween's InOutBack ease type error.  
Add preset interaction components, LGUIScreenSpaceInteraction/LGUIWorldSpaceInteraction, for quick setup.  
Add "Screen Space UI" and "World Space UI" preset setup at editor "LGUI Tools/Basic Setup".  
Add "Replace UI Elements" at editor "LGUI Tools/Replace UI Elements".  
## 1.10:  
Fix some bugs.  
Modify layout and editor.  
## 1.9:  
Fix a crash caused by ActorSerializer.  
Add UENUM support for LGUIDrawableEvent.  
UI2DLine add "grow" feature, can grow line from start to end.  
Change LGUIEditor's rule of duplicate object's name.  
## 1.8:  
Fix LTween crash issue.  
ActorSerializer(PrefabSystem) and ActorCopier now support instanced object property.  
LGUIComponentReference add blueprint support.  
## 1.7:  
Fix some bugs.  
Improve performance of UITexture's tiled mode and UISprite's tiled mode.  
## 1.6:  
Fix a bug that cause an error and stop cook process.  
Add more alignment options for layout control. 
## 1.5:  
Expose more interface to blueprint(most is layout component).  
Add UVRect property to UITexture, so UITexture can choose a rect area to show texture. If any prefab contains the UITexture component, then you should recreate that prefab.  
Modify UIRoot for editor simulation mode. When you use overlay UI by SceneCapture2D, you can eject player and hide UI.  
Show drawcall count on UIPanel's details panel.  
Fix edit mode update issue.  
Fix a crash caused by UISprite tiled mode.  
## 1.4:  
Fix some bugs  
Replace editor update with FTickableObject.  
## 1.3:  
Change some editor icon.  
Add new editor enter on toolbar, with LGUI icon button.  
Add new editor helper function "Change Trace Channel", this can quickly change UI element's trace channel with hierarchy.  
Add new function "SetTraceChannel" and "GetTraceChannel" to UIItem.  
## 1.2:  
Thanks to <jeff@lockerman.org>, I modified some class names from 2DLineXXX to UI2DLineXXX, because digits as first char of class name is not good.  
If you already use some of these classes, follow these steps:  
```  
        a.Find config file in your (UE4Project)/Config/DefaultEngine.ini  
        b.Find or add line [/Script/Engine.Engine]  
        c.Add these lines under [/Script/Engine.Engine]:  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineChildrenAsPoints",NewClassName="/Script/LGUI.UI2DLineChildrenAsPoints")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineChildrenAsPointsActor",NewClassName="/Script/LGUI.UI2DLineChildrenAsPointsActor")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineRaw",NewClassName="/Script/LGUI.UI2DLineRaw")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineActor",NewClassName="/Script/LGUI.UI2DLineActor")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineRendererBase",NewClassName="/Script/LGUI.UI2DLineRendererBase")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineRing",NewClassName="/Script/LGUI.UI2DLineRing")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineRingActor",NewClassName="/Script/LGUI.UI2DLineRingActor")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineRingFrame",NewClassName="/Script/LGUI.UI2DLineRingFrame")  
            +ActiveClassRedirects=(OldClassName="/Script/LGUI.2DLineRingFrameActor",NewClassName="/Script/LGUI.UI2DLineRingFrameActor")  
```  
After these steps, the engine should should recognize these new classes.  
## 1.0:  
First submit  