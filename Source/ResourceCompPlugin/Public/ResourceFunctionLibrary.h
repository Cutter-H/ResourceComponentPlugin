// Copyright LyCH. 2024

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ResourceFunctionLibrary.generated.h"

class UResourceComponentBase;
class UHealthResource;
class UDamageModificationData;
enum EResourcePercentType;

/**
 * 
 */
UCLASS()
class RESOURCECOMPPLUGIN_API UResourceFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/*
	* Returns the first resource with the given name on the actor.
	*/UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Resource Function Library")
	static UResourceComponentBase* GetResourceFromActor(AActor* actor, FName resourceName = "Default");
	/*
	* Returns all resources with the given name on the actor.
	*/UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Resource Function Library")
	static TArray<UResourceComponentBase*> GetAllResourcesFromActor(AActor* actor);
	/*
	* Returns the first resource with the given name on each actor.
	*/UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Resource Function Library")
	static TArray<UResourceComponentBase*> GetResourceFromActors(TArray<AActor*> actors, FName resourceName = "Default");
	/*
	* Returns all resources with the given name on each actor.
	*/UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Resource Function Library")
	static TArray<UResourceComponentBase*> GetAllResourcesFromActors(TArray<AActor*> actors);

	/*
	* Adds to the first resource with the given name on the actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Fill Increase"))
	static void AddResourceToActor(AActor* actor, FName resourceName, float addAmount);
	/*
	* Drains from the first resource with the given name on the actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Decrease Remove Subtract"))
	static void DrainResourceFromActor(AActor* actor, FName resourceName, float drainAmount);
	/*
	* Adds a percent to the first resource with the given name on the actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Fill Increase"))
	static void AddResourcePercentToActor(AActor* actor, FName resourceName, float addPercent, EResourcePercentType percentType);
	/*
	* Drains a percent from the first resource with the given name on the actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Decrease Remove Subtract"))
	static void DrainResourcePercentFromActor(AActor* actor, FName resourceName, float drainPercent, EResourcePercentType percentType);

	/*
	* Adds to the first resource with the given name on each actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Fill Increase"))
	static void AddResourceToActors(TArray<AActor*> actors, FName resourceName, float addAmount);
	/*
	* Drains from the first resource with the given name on each actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Decrease Remove Subtract"))
	static void DrainResourceFromActors(TArray<AActor*> actors, FName resourceName, float drainAmount);
	/*
	* Adds a percent to the first resource with the given name on each actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Fill Increase"))
	static void AddResourcePercentToActors(TArray<AActor*> actors, FName resourceName, float addPercent, EResourcePercentType percentType);
	/*
	* Drains a percent from the first resource with the given name on each actor.
	*/UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library", meta = (KeyWords = "Decrease Remove Subtract"))
	static void DrainResourcePercentFromActors(TArray<AActor*> actors, FName resourceName, float drainPercent, EResourcePercentType percentType);

	/*
	 * Returns all health resources with the filtered names.
	 * If the resource name filter is empty, returns all available HealthResources.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, BlueprintPure, Category = "Resource Function Library|Health", meta = (AutoCreateRefTerm = "healthResourceNameFilter"))
	 static TArray<UHealthResource*> GetActorHealthResources(AActor* actor, TArray<FName> healthResourceNameFilter);
	/*
	 * Checks if the actor implements the modification on the listed Health Resources.
	 * If the resource name filter is empty, checks all available HealthResources.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, BlueprintPure, Category = "Resource Function Library|Health", meta = (AutoCreateRefTerm = "healthResourceNameFilter"))
	static bool ActorImplementsAnyModification(AActor* actor, TArray<FName> modificationName, TArray<FName> healthResourceNameFilter);
	/*
	 * Adds modification data to the filtered HealthResource components on the actor.
	 * If resource names are not given, then the modification data is applied to all HealthResources.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library|Health", meta = (AutoCreateRefTerm = "healthResourceNameFilter"))
	static void GiveModificationDataToActor(AActor* actor, TArray<UDamageModificationData*> modificationData, TArray<FName> healthResourceNameFilter);
	/*
	 * Removes modification data to the filtered HealthResource components on the actor.
	 * If resource names are not given, then the modification data is removed from all HealthResources.
	 */UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Resource Function Library|Health", meta = (AutoCreateRefTerm = "healthResourceNameFilter"))
	 static void RemoveModificationFromActor(AActor* actor, TArray<FName> modificationNames, TArray<FName> healthResourceNameFilter);
	
};
