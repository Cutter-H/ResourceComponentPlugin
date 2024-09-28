# ResourceComponentPlugin
 Actor Component that can be used for various resources. Includes a modifiable Health component and a ready-made Health component with UI elements.

---

**Support: **lych.assets@gmail.com
If you need support for previous engine versions let me know!

---

**Installing the Plugin:**
[Unreal Marketplace Page](https://www.unrealengine.com/marketplace/en-US/product/849a9f0fec7040b1b4cca2b5d26a2755)

Plugins purchased through the Unreal Engine Marketplace can be installed using the process on the following page: [https://docs.unrealengine.com/5.0/en-US/working-with-plugins-in-unreal-engine/#installingpluginsfromtheunrealenginemarketplace](https://docs.unrealengine.com/5.0/en-US/working-with-plugins-in-unreal-engine/#installingpluginsfromtheunrealenginemarketplace)

Enabling the plugin can be done through the process outlined at the top of the above page.


---

**Plugin Information: **

Plugin Modules (Type):
* ResourceCompPlugin(Runtime)

Plugin Classes (Parent Class):
* ResourceComponentBase (Actor Component)
* HealthResource (ResourceComponentBase)
* HealthResourceWithUI (HealthResource)
* DamageModificationData (Primary Data Asset)
* ResourceFunctionLibrary (Blueprint Function Library)
* DamageTypeModificationInterface (Interface)

Plugin Enums and Structs



* EResourcePercentType (enum)
* EHealthRegenEventType (enum | Not available to Blueprints)
* EIncomingDamageChannel (enum)
* EIncomingDamageModificationType(enum)
* EOverheadWidgetVisibility(enum)
* FIncomingDamageModification (struct)

Supported Platforms:



* Win64

This document shows Blueprint exposed variables and functions. Classes have alternative virtual functions for C++ derivatives and the header files have comments. Blueprint friendly names are used.



---

# Resource Component Base (Actor Component)

#include "Components/ResourceComponentBase.h"




**Dispatchers**



* OnCurrentAmountChange(**Float** NewValue)
    * Called when any change to Current Amount occurs.
* OnDrain(**Float** NewValue)
    * Called when any Drain occurs.
* OnAdd(**Float** NewValue)
    * Called when any Add occurs.
* OnFill
    * Called when Add results in CurrentAmount meets MaximumAmount.
* OnEmpty
    * Called when Drain results in CurrentAmount meets 0.
* OnRegenStart
    * Called when Regeneration begins. (Immediately before the first RegenTick occurs.)
* OnRegenEnd
    * Called when Regeneration ends, whether through fill or a new drain occurring.
* OnRegenTick(**Float** NewValue)
    * Called on every tick Regen occurs.




**Variables**



* Resource Name **Name** 
    * The name used to identify this resource.
* Max Amount **Float**
    * The base maximum amount of this resource.
* Regen Amount **Float**
    * How much resource is filled per regen tick.
* Regen Rate **Float**
    * How many regen ticks occur per second.
* Regen Delay **Float**
    * The delay after draining the resource when regen begins.
* Additional Exhausted Delay **Float**
    * This is added to RegenDelay if regen begins when CurrentAmount is 0.
* Regen After Depletion **Bool**
    * If true the resource will generate even after depletion. Useful for renewable resources such as Stamina.




**Functions**



* AddResource (**Float** AddAmount)
    * Increases the resource by a certain amount.
* DrainResource (**Float** DrainAmount)
    * Reduces the resource by a certain amount.
* AddResourceByPercent (**Float** AddPercent, **EResourcePercentType** PercentType)
    * Increases the resource by a percent relative to the current value or max value.
* DrainResourceByPercent (**Float** DrainPercent, **EResourcePercentType** PercentType)
    * Reduces the resource by a percent relative to the current value or max value.
* **Float** GetCurrentAmount()
    * Gets the value of the current amount of resource.
* **Float** GetMaxAmount()
    * Gets the value of the max amount of resource
* **Float** GetCurrentPercent()
    * Returns a percent (0 - 1.0) available of the resource.
* **Float** GetTimeSinceLastDrain()
    * Gets time in seconds since the last drain attempt.
* SetRegenAmount (**Float** NewRegenAmount)
    * Modifies how much resource is filled per regen tick.
* SetRegenRate (**Float** NewRegenRate)
    * Modifies the speed of regeneration.
* SetRegenDelay(**Float** NewRegenDelay)
    * Modifies how long until regen begins after draining.
* SetAdditionalExhaustedDelay(**Float** NewValue)
    * Modifies how long until regen begins after reaching 0.
* SetRegenAfterDepletion(**Bool** NewValue)
    * Sets whether the resource regenerates at 0 resource.
* **Bool** GetCanBeDrained()
    * Checks if the resource can be drained.
* SetCanBeDrained(**Bool** NewParam)
    * Sets whether the resource can be drained.




**Notes**



* Setting any of the Regen variables to an invalid amount, for example: RegenAmount, Rate, or Delay being negative will disable regeneration.
* Many variables have Get and Set functions that can be overridden. This is to allow for modifications to the resource, such as an increase upon leveling where you may override GetMaxAmount() as MaxAmount * Level. 
* Dispatchers are called on all instances, but may be called prior to replication. Related functions have a float that provides the soon-to-be replicated value. It is highly recommended to use this output instead of GetCurrentAmount.
* Setting the Regen variables during runtime should not affect delays or ticks unless modified as such, and should continue regenerating with the modification.

---

# Health Resource (Resource Component Base)

#include "Components/Health/HealthResource.h"




**Dispatchers**



* OnGenericDamageTaken
    * Called on ApplyDamage when Point or Radial is not used.
* OnPointDamageTaken
    * Called on Point damage taken.
    * HitLocation and BoneName were removed due to the max number of variables, but these are in the FHitResult.
* OnRadialDamageTaken
    * Called on Radial damage taken.
* OnModificationDataAdded
    * Called when a modification data is added.
* OnModificationAdded
    * Called when a modification is added
* OnModificationRemoved
    * Called when a modification is removed




**Variables**



* Default Modification Data **DamageModificationData**
    * A data asset that will be added on BeginPlay.
    * If any were added to the initial ModificationRules array, these will be added afterwards.
* Modification Rules **FIncomingDamageModification&lt;array>**
    * Modifications that will be considered when receiving damage.
    * These modifications will be executed in array order.
    * If the method Override_Health is used, then all other modifications are skipped.
* Last Damage Causer **Actor**
    * Last Actor that damaged the owning actor.
* Last Location Hit From **Vector**
    * Location where the last damage was taken from.




**Functions**



* **FIncomingDamageModifications <StructArray>** GetCurrentModifications()
    * Returns all current modifiers.
* **Bool** HasModifications()
    * Returns true if this has any of the listed modifications.
* **float** GetDirection (**Vector** Direction,**Rotator** BaseRotation)
    * Returns a float that dictates direction. This functionality was ripped from the Get Direction node seen in Animation Blueprints.
* **float** GetDirectionToLocation(**Vector** Location, **Rotator** BaseRotation)
    * Alternative to GetDirection if you want to check against a location rather than a direction.
* **float** ModifyDamage(**float** damageReceived, **EIncomingDamageChannel** damageChannel, **DamageType** DamageType, **Name** boneName, **Vector** damageOrigin)
    * Modifies the incoming damage based on current modifiers. If overridden without a parent node then functionality must be manually implemented.
* **bool** ModificationAcceptsDamageType(**FIncomingDamageModification** modification, **DamageType** damageType)
    * Returns true if the damage type is utilized by the modifier. If no damage types are whitelisted, the modifier accepts all.
* GiveModifier(**FIncomingDamageModification** newModifier,**int** insertAt)
    * Adds a modifier at the given index. If the index is -1 the modifier is added at the end.
* GiveModificationData(**DamageModificationData** modificationData, **int** beginInsertAt)
    * Adds all modifiers from the data at the given index. If the index is -1 the modifiers are added at the end.
* RemoveModifier(**Name** modifierName)
    * Removes all modifiers with this name
* BindDamageDelegates
    * Used to override Bindings for OnGenericDamageTaken, OnPointDamageTaken, and OnRadialDamageTaken. Called at Beginplay.



**Notes**



* If BindDamageDelegates is overridden without binding the 3 core dispatchers, the original functionality with modifications may be broken. OnGenericDamageTaken, OnPointDamageTaken, and OnRadialDamageTaken each are bound to a function that calculates damage based on the modifications.

        
---

# Health Resource With UI (Health Resource)


#include "Components/Health/HealthResourceWithUI.h"



**Variables**



* Enable Onscreen **Bool**
    * Setting for how and when to show the on-screen widget and the overhead widget.
* Overhead Widget Settings **EOverheadWidgetVisibility<enum>**
    * Setting for how and when to show the overhead widget.
* On Screen Widget Class **User Widget<class>**
    * This class is used when creating the widget for the possessing player's UI.
* Overhead Widget Class **User Widget<class>**
    * This class is used for the character's overhead widget used in a WidgetComponent.
* Overhead Widget Material **Material Interface**
    * The material that will be applied to the overhead widget.
* Use World Space **Bool**
    * The material that will be applied to the overhead widget.
* Draw Size **Vector2D**
    * The draw size used for the overhead widget.
    * See similar settings in the WidgetComponent class.
* Draw At Desired Size **Bool**
    * If true, the Overhead Widget's draw size uses the widget’s desired size. 
    * See similar settings in the WidgetComponent class.
* Overhead Widget Offset **Transform**
    * The offset that the Vector will be from the owning actor.
* Overhead Widget Component **WidgetComponent**
    * The Widget Component that is attached to the owner.
* Overhead Widget **User Widget**
    * The User Widget used in the Overhead Widget Component.
* On Screen Widget **User Widget**
    * The User Widget that is placed on the owning player’s screen.


**Functions**



* ChangeWidgetSettings(**Bool** UseOnscreen,**OverheadWidgetVisibility** UseOverhead)
    * Returns all current modifiers.


**Notes**



* Options explaining for Widget Settings:
    * **Overhead Disabled:** The Overhead widget is never shown, but is created.
    * **Show Only On Possessed Player:** The Overhead widget is only shown to the owning player.
    * **Show Only On Other:** The Overhead widget is not shown to the owning player.
    * **Show On All:** The Overhead widget is used on both the owning player and others.


---


# Damage Modification Data (Primary Data Asset)

#include "Components/Health/HealthResourceWithUI.h"




**Variables**



* Modifications **FIncomingDamageModification&lt;array>**
    * An array of the modifications housed in this Data Asset.




**Notes**



* This data asset is used to keep collections of modifications or a single modification used in multiple instances. For example: Creating a new data asset that doubles damage from a bullet damage type can be created and easily applied to various actors.

---

# Resource Function Library(Blueprint Function Library)

#include "ResourceFunctionLibrary.h"



**Functions**

* **ResourceComponentBase** GetResourceFromActor(**Actor** actor,**Name** resourceName)
    * Returns the first resource with the given name on the actor.
* **ResourceComponentBase<array>** GetAllResourcesFromActor(**Actor** actor)
    * Returns all resources with the given name on the actor.
* **ResourceComponentBase<array>** GetResourceFromActors(**Actor<array>** actors, **Name **resourceName)
    * Returns the first resource with the given name on each actor.
* **ResourceComponentBase<array>** GetAllResourcesFromActors(**Actor<array>** actors)
    * Returns all resources with the given name on each actor.
* AddResourceToActor(**Actor** actor, **Name** resourceName, **Float** addAmount)
    * Adds to the first resource with the given name on the actor.
* DrainResourceFromActor(**Actor** actor, **Name** resourceName, **Float** drainAmount)
    * Drains from the first resource with the given name on the actor.
* AddResourcePercentToActor(**Actor** actor, **Name** resourceName, **Float** addPercent, **EResourcePercentType** percentType)
    * Adds a percent to the first resource with the given name on the actor.
* DrainResourcePercentFromActor(**Actor** actor, **Name** resourceName, **Float** drainPercent, **EResourcePercentType** percentType)
    * Drains a percent from the first resource with the given name on the actor.
* AddResourceToActors(**Actor<array>** actors, **Name** resourceName, **Float** amount)
    * Adds to the first resource with the given name on each actor.
* DrainResourceFromActors(**Actor<array>** actors,**Name** resourceName,**Float** amount)
    * Drains from the first resource with the given name on each actor.
* AddResourcePercentToActors(**Actor<array>** actors, **Name** resourceName, **Float** addPercent, **EResourcePercentType** percentType)
    * Adds a percent to the first resource with the given name on each actor.
* DrainResourcePercentFromActors(**Actor<array>** actors, **Name** resourceName, **Float** drainPercent, **EResourcePercentType** percentType)
    * Drains a percent from the first resource with the given name on each actor.
* **HealthResource<array>** GetActorHealthResources(**Actor** actor, **Name<array>** healthNameFilter)
    * Returns all health resources with the filtered names.
* **Bool** ActorImplementsAnyModification(**Actor** actor, **Name<array>** modificationNames,**Name<array>** healthNameFilter)
    * Checks if the actor implements the modification on the listed Health Resources.
* GiveModificationDataToActor(**Actor<array>** actors,**Name** resourceName,**Float** amount)
    * Drains from the first resource with the given name on each actor.
* RemoveModificationFromActor(**Actor** actor,**Name<array>** modificationNames,**Name<array>** healthResourceNameFilter)
    * Removes modification data to the filtered HealthResource components on the actor.




**Notes**



* This was created for easy access to resources on various actors by only needing the resource name and the actor.
* Most of the functions in this library should only be called on the server. This does not include the getter functions.

---

# Damage Type Modification Interface (Interface)

#include "Interfaces/DamageTypeModificationInterface.h"




**Functions**



* **Float** ModifyDamage (**Float** incomingDamage, **Actor** damagedActor)
    * Used on Damage Type classes to modify damage directly on the damage type. 
    * If the modification attempts to use this, but the damage time does not implement the interface, it will not apply the modification.




**Notes**



* This functionality is constant and can really only change by creating a new DamageType that implements the interface. 

---

# Enums and Structs



* EResourcePercentType (**enum**)
    * Current
    * Maximum
* EHealthRegenEventType (**enum | Not available to Blueprints**)
    * Tick
    * Start
    * End
* EIncomingDamageChannel (**enum**)
    * AllChannels
    * PointDamage
    * RadialDamage
    * GenericDamage
* EIncomingDamageModificationType(**enum**)
    * Add_Damage
    * Multiply_Damage
    * Override_Damage
    * Modify_From_DamageType
* EOverheadWidgetVisibility(**enum**)
    * Overhead Disabled
    * Show Only On Possessed Player
    * Show Only On Others
    * Show On All
* FIncomingDamageModification (**struct**)
    * ModificationName **Name**
        * Name used to identify this rule for GiveModification and RemoveModification.
    * DamageChannel **EIncomingDamageChannel**
        * What damage type prompts this.
    * ModificationType **EIncomingDamageModificationType**
        * How this damage modification is handled.
    * Magnitude **Float**
        * How much this modifies the damage.
    * WhitelistedBoneNames **Name<array>**
        * The bone that received the damage must be included in this array if it is not empty.
    * MaximumRange **Float**
        * The maximum distance the damage can be taken from for it to be modified. If this is less than or equal to 0 it is ignored.
        * The distance is measured from the owning actor and the HitResult trace start(Point), radial damage origin(Radial), or damage causer location(Generic).


---

**Final Notes**



* When creating a new widget for the HealthResourceWithUI component, you can create a function with the name SetResourceComponent using a ResourceComponentBase input and the function will automatically be called when using it as an Onscreen Widget or Overhead Widget.
* The HealthResourceWithUI component is not as malleable as the Health or base Resource component. It is primarily made to be a quick “drag and drop” on an actor blueprint.
* If these components are not functioning correctly in multiplayer, make sure the owning actor has Replicates enabled. If you’re using the components on a mesh you simply placed in the world it will not immediately work as that is not Replicated.
* The components above will work on actors and their subclasses. Keep in mind certain functionality such as onscreen functions will not work unless they are placed on a controllable Pawn or Character.
* All variables in the Details panel will be in either of these three categories: **Resource**, **Health**, **UI**. The functions in the function library are in: **Resource Function Library.**
* Some variables cannot be set with a simple **Set Variable **node, but must use the related setter function. This is due to other functionality being required when setting these variables to make sure the entire component updates correctly.
* Currently all modifications are stackable. Adding 2 “weak to fire (x2 damage)” modifications will result in x4 damage. Meaning there are NO precautions when accidentally adding duplicates. Additionally, removing modifications by that name will remove ALL modifications with that name, so you are not able to only remove 1. While this can be worked around, explicit functionality to better support this will come at a later date.
* Using a Modification with the Type of **Override Damage **will stop any further modifications and revert any previous modifications making the incoming damage be the magnitude value.