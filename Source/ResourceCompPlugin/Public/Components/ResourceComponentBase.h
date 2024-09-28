// Copyright LyCH. 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ResourceComponentBase.generated.h"

UENUM(BlueprintType)
enum EResourcePercentType {
	Current,
	Maximum
};
UENUM()
enum EHealthRegenEventType {
	Tick,
	Start,
	End
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGenericResourceEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValueResourceEvent, float, value);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Resource), meta=(BlueprintSpawnableComponent) )
class RESOURCECOMPPLUGIN_API UResourceComponentBase : public UActorComponent
{
	GENERATED_BODY()


public:
	/*
	 * Called when any change to Current Amount occurs.
	 * Also called on Add, Drain, and Regen Tick.
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnValueResourceEvent OnCurrentAmountChange;
	/*
	 * Called when any Drain occurs.
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnValueResourceEvent OnDrain;
	/*
	 * Called when any Add occurs.
	 * Also called on Regen Tick
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnValueResourceEvent OnAdd;
	/*
	 * Called when Add results in CurrentAmount meets MaximumAmount.
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnGenericResourceEvent OnFill;
	/*
	 * Called when Drain results in CurrentAmount meets 0.
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnGenericResourceEvent OnEmpty;
	/*
	 * Called when Regeneration begins. (Immediately before the first RegenTick occurs.)
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnGenericResourceEvent OnRegenStart;
	/*
	 * Called when Regeneration ends, whether through fill or a new drain occurring.
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnGenericResourceEvent OnRegenEnd;
	/*
	 * Called on every tick Regen occurs.
	 * Also called immediately after RegenStart.
	 */UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnValueResourceEvent OnRegenTick;

	/*
	 * Increases the resource by a certain amount.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Add Resource", KeyWords = "Fill Increase"))
	 void K2_AddResource(float addAmount);
	/*
	 * Reduces the resource by a certain amount.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Drain Resource", KeyWords = "Decrease Remove Subtract"))
	 void K2_DrainResource(float drainAmount);
	/*
	 * Increases the resource by a percent relative to the current value or max value.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Add Resource by Percent", KeyWords = "Fill Increase"))
	 void K2_AddResourceByPercent(float addPercent, EResourcePercentType percentType);
	/*
	 * Reduces the resource by a percent relative to the current value or max value.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Drain Resource by Percent", KeyWords = "Decrease Remove Subtract"))
	 void K2_DrainResourceByPercent(float drainPercent, EResourcePercentType percentType);
	/*
	 * Modifies how much resource is filled per regen tick.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Set Regen Amount"))
	 void K2_SetRegenAmount(float newRegenAmount);
	/*
	 * Modifies the speed of regeneration.
	 * @param newRegenRate How many ticks per second regen is called.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Set Regen Rate"))
	 void K2_SetRegenRate(float newRegenRate);
	/*
	 * Modifies how long until regen begins after draining.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Set Regen Delay"))
	 void K2_SetRegenDelay(float newDelayAmount);
	 /*
	  * Gets the value of the max amount of resource.
	  */UFUNCTION(BlueprintNativeEvent, BlueprintPure, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Get Max Amount"))
	  float K2_GetMaxAmount() const;
	 /*
	 * Toggles if the resource can be drained.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Set Can Be Drained"))
	 void K2_SetCanBeDrained(bool newParam);
	 /*
	  * Checks if the resource can be drained.
	  */UFUNCTION(BlueprintAuthorityOnly, BlueprintNativeEvent, BlueprintCallable, Category = "Resource", meta = (DisplayName = "Get Can Be Drained"))
	  bool K2_GetCanBeDrained() const;
	 /*
	  * Modifies how long until regen begins after reaching 0.
	  */UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource")
	  void SetAdditionalExhaustedDelay(float newValue) {
		  AdditionalExhaustedDelay = newValue;
	  }
	 /*
	 * Sets whether the resource regenerates at 0 resource
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource")
	  void SetRegenAfterDepletion(bool newValue) {
	  bRegenAfterDepletion = newValue;
	 }
	 /*
	 * Gets time in seconds since the last drain attempt.
	 */UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resource")
	 float GetTimeSinceLastDrain() const;
	 /*
	 * Returns the name of this resource.
	 */UFUNCTION(BlueprintCallable, Category = "Resource")
	 virtual FName GetResourceName() const {
		 return ResourceName;
	 }
	 /*
	 * Gets the value of the current amount of resource.
	 */UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Get Current Amount"))
	 virtual float GetCurrentAmount() const {
		 return CurrentAmount;
	 }
	 /*
	 * Returns a percent (0 - 1.0) available of the resource.
	 * (Returns Current/Maximum)
	 */UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Get Current Percent"))
	 virtual float GetCurrentPercent() const {
		 return CurrentAmount / K2_GetMaxAmount();
	 }
protected:
	/*
	 * The name of this resource.
	 */UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	FName ResourceName = "Default";
	/*
	 * The base maximum amount of resource.
	 */UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	float MaxAmount = 100.f;
	/*
	 * If true, the resource begins at 0 instead of Get Max Amount.
	 */UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	bool bRegenBeginsEmpty = false;
	/*
	 * How much resource is filled per tick.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Regen")
	float RegenAmount = 5.f;
	/*
	 * How many ticks per second occur.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Regen")
	float RegenRate = 5.f;
	/*
	 * The delay after draining the resource when regen begins.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Regen")
	float RegenDelay = 1.f;
	/*
	 * This is added to RegenDelay if regen begins when CurrentAmount is 0.
	 */UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource|Regen")
	float AdditionalExhaustedDelay = 0.f;
	/*
	 * If true the resource will generate even after depletion. Useful for renewable resources such as Stamina.
	 */UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource|Regen")
	 bool bRegenAfterDepletion = false;

	UResourceComponentBase();
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const;
	virtual void BeginPlay() override;
	// For the functions below, see the K2_FunctionName versions for details regarding functionality.

	UFUNCTION()
	virtual void AddResource(float addAmount);
	UFUNCTION()
	virtual void DrainResource(float drainAmount);
	UFUNCTION()
	virtual void AddResourceByPercent(float addPercent, EResourcePercentType percentType);
	UFUNCTION()
	virtual void DrainResourceByPercent(float drainPercent, EResourcePercentType percentType);
	UFUNCTION()
	virtual void SetRegenAmount(float newRegenAmount);
	UFUNCTION()
	virtual void SetRegenRate(float newRegenRate);
	UFUNCTION()
	virtual void SetRegenDelay(float newRegenDelay);
	UFUNCTION()
	virtual float GetMaxAmount() const { return MaxAmount; }
	UFUNCTION()
	virtual void SetCanBeDrained(bool bCanBeDrained) { bDrainDisabled = !bCanBeDrained; }
	UFUNCTION()
	virtual bool GetCanBeDrained() const { return !bDrainDisabled; }

private:
	UPROPERTY(Replicated)
	float CurrentAmount = 100.f;

	UPROPERTY(Replicated)
	float TimeAtLastDrain = 0.f;

	// This is only used on the server. No reason for replication.
	UPROPERTY()
	FTimerHandle RegenTimer; 
	// This is only used on the server. No reason for replication.
	UPROPERTY()
	bool bFirstRegenTick = false;
	// This is only used on the server. No reason for replication.
	UPROPERTY()
	bool bDrainDisabled;

	UFUNCTION()
	bool ShouldRegen() const;

	UFUNCTION()
	void StopRegenTimer(){ GetWorld()->GetTimerManager().ClearTimer(RegenTimer); RegenTimer.Invalidate(); }

	UFUNCTION()
	float GetRegenDelay() const { return CurrentAmount <= 0 ? bRegenAfterDepletion ? RegenDelay + AdditionalExhaustedDelay : -1 : RegenDelay; }

	UFUNCTION()
	void SetRegenTimer(float initialDelay = -1);

	UFUNCTION(Server, Reliable)
	void RegisterDrainTime_Server(float time);

	UFUNCTION(Server, Reliable)
	void AddResource_Server(float additional);

	UFUNCTION(Server, Reliable)
	void DrainResource_Server(float removal);

	UFUNCTION(NetMulticast, Reliable)
	void BroadcastResourceChange_Net(float oldValue, float newValue);

	UFUNCTION(NetMulticast, Reliable)
	void BroadcastRegenEvent_Net(EHealthRegenEventType type, float newValue = 0);

};
