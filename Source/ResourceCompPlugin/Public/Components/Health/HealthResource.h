// Copyright LyCH. 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/ResourceComponentBase.h"
#include "Data/DamageModificationData.h"
#include "HealthResource.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnGenericDamageTakenSignature, AActor*, DamagedActor, float, Damage, const UDamageType*, DamageType, AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_NineParams(FOnPointDamageTakenSignature, AActor*, DamagedActor, float, Damage, AController*, InstigatedBy, FVector, HitLocation, UPrimitiveComponent*, HitComponent, FName, BoneName, FVector, ShotFromDirection, const UDamageType*, DamageType, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(FOnRadialDamageTakenSignature, AActor*, DamagedActor, float, Damage, const UDamageType*, DamageType, FVector, Origin, const FHitResult&, HitInfo, AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModificationSignature, const FIncomingDamageModification&, modification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModificationDataSignature, const UDamageModificationData*, modification);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Resource), meta = (BlueprintSpawnableComponent))
class RESOURCECOMPPLUGIN_API UHealthResource : public UResourceComponentBase
{
	GENERATED_BODY()
protected:
	/**
	 * How long debug messages last.
	 *///UPROPERTY()//BlueprintReadOnly, Category = "Health", meta = (EditCondition = "bDebug", EditConditionHides))
	float DebugShowtime = 1.f;
	/**
	 * A data asset that will be added on BeginPlay.
	 * If any were added to the initial ModificationRules array, these will be added afterwards.
	 */UPROPERTY(EditAnywhere, Category = "Health|Modifications")
	TObjectPtr<UDamageModificationData> DefaultModificationData;
	/**
	 * Modifications that will be considered when receiving damage.
	 * These modifications will be executed in array order.
	 * Override_Health skips all other modifications.
	 */UPROPERTY(Replicated, EditAnywhere, Category = "Health|Modifications", meta = (TitleProperty = "ModificationName"))
	TArray<FIncomingDamageModification> ModificationRules;
	/**
	 * Last Actor that damaged owner.
	 */UPROPERTY(Replicated, BlueprintReadWrite, Category = "Health|Damage")
	TObjectPtr<AActor> LastDamageCauser;
	/**
	 * Where the last damage taken from.
	 */UPROPERTY(Replicated, BlueprintReadWrite, Category = "Health|Damage")
	FVector LastLocationHitFrom;
private:
	/**
	 * Should debug draws be called.
	 *///UPROPERTY(EditDefaultsOnly, Category = "Health")
	bool bDebug = false;
	/**
	 * This is used so damage is not taken twice. (Such as Point damage + Any damage)
	 */
	bool bBlockDamage = false;
/////////////////////////
//////////// FUNCTIONS //
/////////////////////////

#pragma region Overrides
protected:
	UHealthResource();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
#pragma endregion
#pragma region Getters
public:
	/**
	 * True if running on the server.
	 */UFUNCTION()
	virtual bool IsServer() const;
	/**
	 * True if running on the owning player (AI is server).
	 */UFUNCTION()
	virtual bool IsLocallyControlled() const;
	/**
	 * Is the owner of this a pawn that's controlled by a player?
	 */UFUNCTION()
	virtual bool IsPlayerControlled() const;
	/**
	 * Returns all current modifiers.
	 */UFUNCTION(BlueprintCallable, Category = "Health|Modifications")
	TArray<FIncomingDamageModification> GetCurrentModifications() const {
		return ModificationRules;
	}
	/**
	 * Returns true if this has any of the listed modifications.
	 */ UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Health|Modifications")
	bool HasModifications(TArray<FName> modificationNames);
	/**
	 * Get Direction by Direction.
	 * Included so it is easy to tell what direction the actor was damaged from.
	 * * This was moved from the engine's AnimInstance. *
	 */UFUNCTION(BlueprintCallable, Category = "Health")
	virtual float GetDirection(const FVector& Direction, const FRotator& BaseRotation) const;
	/**
	 * Get Direction by Location.
	 * Included so it is easy to tell what direction the actor was damaged from.
	 * * This was moved from the engine's AnimInstance. *
	 */UFUNCTION(BlueprintCallable, Category = "Health")
	virtual float GetDirectionToLocation(const FVector& Location, const FRotator& BaseRotation) const;

#pragma endregion
#pragma region Modify Damage
public:
	/*
	 * Called when a modification Data asset has been added to the resource.
	 */UPROPERTY(BlueprintAssignable, Category = "Health|Modifications")
	FModificationDataSignature OnModificationDataAdded;
	/*
	 * Called when a modification has been added to the resource.
	 */UPROPERTY(BlueprintAssignable, Category = "Health|Modifications")
	FModificationSignature OnModificationAdded;
	/*
	 * Called when a modification has been removed from the resource.
	 */UPROPERTY(BlueprintAssignable, Category = "Health|Modifications")
	FModificationSignature OnModificationRemoved;
protected:
	/**
	 * Modifies the incoming damage.
	 */UFUNCTION(BlueprintNativeEvent, Category = "Health", meta=(DisplayName = "Modify Damage"))
	 float K2_ModifyDamage(float damageReceived, EIncomingDamageChannel damageChannel, const class UDamageType* DamageType, FName boneName, FVector damageOrigin) const;
	/**
	 * Modifies the incoming damage.
	 */UFUNCTION()
	virtual float ModifyDamage(float damageReceived, EIncomingDamageChannel damageChannel, const class UDamageType* DamageType, FName boneName, FVector damageOrigin) const;
	/**
	 * Checks if the modification allows the damage type.
	 */UFUNCTION(BlueprintCallable, Category = "Health|Modifications")
	virtual bool ModificationAcceptsDamageType(FIncomingDamageModification modification, const UDamageType* damageType) const;
public:
	/**
	 * Adds a modifier at the given index.
	 * @param insertAt Inserting at -1 adds to the end.
	 */UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Health|Modifications")
	virtual void GiveModifier(FIncomingDamageModification newModifier, int insertAt = -1);
	/*
	 * Gives all of the modifications listed in the Data Asset.
	 * @param beginInsertAt Inserting at -1 adds to the end.
	 */UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Health|Modifications")
	virtual void GiveModificationData(UDamageModificationData* modificationData, int beginInsertAt = -1);
	/**
	 * Removes all modifiers with this name.
	 */UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Health|Modifications")
	virtual void RemoveModifier(FName modifierName);
private:
	 /*
	 * Replicates the OnModificationDataAdded delegate.
	 */UFUNCTION(NetMulticast, Reliable)
	 void ModificationDataAdded(const UDamageModificationData* modificationData);
	 /*
	 * Replicates the OnModificationAdded and OnModificationRemoved delegates.
	 */UFUNCTION(NetMulticast, Reliable)
	 void ModificationChanged(FIncomingDamageModification modification, bool bAdded);
	 
#pragma endregion
#pragma region Damage Binders
protected:
	/**
	 * Called when owning actor takes any damage. (Overriding may remove OnDamageTaken call.)
	 */UFUNCTION(BlueprintCallable, Category = "Health|Damage Intake")
	virtual void OnAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	/**
	 * Called when owning actor takes Point damage. (Overriding may remove OnDamageTaken call.)
	 */UFUNCTION(BlueprintCallable, Category = "Health|Damage Intake")
	virtual void OnPointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);
	/**
	 * Called when owning actor takes radial damage. (Overriding may remove OnDamageTaken call.)
	 */UFUNCTION(BlueprintCallable, Category = "Health|Damage Intake")
	virtual void OnRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);
#pragma endregion
#pragma region Damage Replication
public:
	/**
	 * Called on ApplyDamage when Point or Radial is not used.
	 */UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category = "Health")
	FOnGenericDamageTakenSignature OnGenericDamageTaken;
	/**
	 * Called on Point damage taken.
	 * HitLocation and BoneName were removed due to max number of var, but these are in the FHitResult.
	 */UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category = "Health")
	FOnPointDamageTakenSignature OnPointDamageTaken;
	/**
	 * Called on Radial damage taken.
	 */UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category = "Health")
	FOnRadialDamageTakenSignature OnRadialDamageTaken;
	/*
	* Override to binds different damage functions.
	* The original function bindings are: OnGenericDamageTaken, OnPointDamageTaken, OnRadialDamageTaken.
	* 
	* If you override this without using these delegates then the resource will not drain on damage 
	* and it will need to be manually implemented.
	*/UFUNCTION(BlueprintNativeEvent, Category = "Health", meta=(DisplayName = "Bind Damage Delegates"))
	void K2_BindDamageDelegates();
	/*
	* Override to binds different damage functions.
	* The original function bindings are: OnGenericDamageTaken, OnPointDamageTaken, OnRadialDamageTaken.
	*
	* If you override this without using these delegates then the resource will not drain on damage
	* and it will need to be manually implemented.
	*/UFUNCTION()
	virtual void BindDamageDelegates();
private:
	/**
	 * Bound to Delegate FOnGenericDamageTakenSignature
	 */UFUNCTION(NetMulticast, Reliable)
	virtual void GenericDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	/**
	 * Bound to Delegate FOnPointDamageTakenSignature
	 */UFUNCTION(NetMulticast, Reliable)
	virtual void PointDamageTaken(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);
	/**
	 * Bound to Delegate FOnRadialDamageTakenSignature
	 */UFUNCTION(NetMulticast, Reliable)
	virtual void RadialDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser);
#pragma endregion
};
