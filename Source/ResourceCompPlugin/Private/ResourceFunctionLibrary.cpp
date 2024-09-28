// Copyright LyCH. 2024


#include "ResourceFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/ResourceComponentBase.h"
#include "Components/Health/HealthResource.h"

UResourceComponentBase* UResourceFunctionLibrary::GetResourceFromActor(AActor* actor, FName resourceName) {
    if (!IsValid(actor)) {
        return nullptr;
    }
    const TArray<UActorComponent*> components = actor->GetComponents().Array();

    for (int i = 0; i < components.Num(); i++) {
        if (UResourceComponentBase* resource = Cast<UResourceComponentBase>(components[i])) {
            if (resource->GetResourceName() == resourceName) {
                return resource;
            }
        }
    }
    return nullptr;
}
TArray<UResourceComponentBase*> UResourceFunctionLibrary::GetAllResourcesFromActor(AActor* actor) {
    if (!IsValid(actor)) {
        return TArray<UResourceComponentBase*>();
    }
    const TArray<UActorComponent*> components = actor->GetComponents().Array();
    TArray<UResourceComponentBase*> retVal;
    for (int i = 0; i < components.Num(); i++) {
        if (UResourceComponentBase* resource = Cast<UResourceComponentBase>(components[i])) {
            retVal.Add(resource);
        }
    }
    return retVal;
}
TArray<UResourceComponentBase*> UResourceFunctionLibrary::GetResourceFromActors(TArray<AActor*> actors, FName resourceName) {
    TArray<UResourceComponentBase*> retVal;
    for (int i = 0; i < actors.Num(); i++) {
        if (UResourceComponentBase* resource = GetResourceFromActor(actors[i], resourceName)) {
            retVal.Add(resource);
        }
    }
    return retVal;
}
TArray<UResourceComponentBase*> UResourceFunctionLibrary::GetAllResourcesFromActors(TArray<AActor*> actors) {
    TArray<UResourceComponentBase*> retVal;
    for (int i = 0; i < actors.Num(); i++) {
        retVal.Append(GetAllResourcesFromActor(actors[i]));
    }
    return retVal;
}

void UResourceFunctionLibrary::AddResourceToActor(AActor* actor, FName resourceName, float addAmount) {
    if (!IsValid(actor)) { return; }
    if (UResourceComponentBase* resource = GetResourceFromActor(actor)) {
        resource->K2_AddResource(addAmount);
    }
}
void UResourceFunctionLibrary::DrainResourceFromActor(AActor* actor, FName resourceName, float drainAmount) {
    if (!IsValid(actor)) { return; }
    if (UResourceComponentBase* resource = GetResourceFromActor(actor)) {
        resource->K2_DrainResource(drainAmount);
    }
}
void UResourceFunctionLibrary::AddResourcePercentToActor(AActor* actor, FName resourceName, float addPercent, EResourcePercentType percentType) {
    if (!IsValid(actor)) { return; }
    if (UResourceComponentBase* resource = GetResourceFromActor(actor)) {
        resource->K2_AddResourceByPercent(addPercent, percentType);
    }
}
void UResourceFunctionLibrary::DrainResourcePercentFromActor(AActor* actor, FName resourceName, float drainPercent, EResourcePercentType percentType) {
    if (!IsValid(actor)) { return; }
    if (UResourceComponentBase* resource = GetResourceFromActor(actor)) {
        resource->K2_DrainResourceByPercent(drainPercent, percentType);
    }
}

void UResourceFunctionLibrary::AddResourceToActors(TArray<AActor*> actors, FName resourceName, float addAmount) {
    TArray<UResourceComponentBase*> resources = GetResourceFromActors(actors, resourceName);
    for (int r = 0; r < resources.Num(); r++) {
        resources[r]->K2_AddResource(addAmount);
    }
}
void UResourceFunctionLibrary::DrainResourceFromActors(TArray<AActor*> actors, FName resourceName, float drainAmount) {
    TArray<UResourceComponentBase*> resources = GetResourceFromActors(actors, resourceName);
    for (int r = 0; r < resources.Num(); r++) {
        resources[r]->K2_DrainResource(drainAmount);
    }
}
void UResourceFunctionLibrary::AddResourcePercentToActors(TArray<AActor*> actors, FName resourceName, float addPercent, EResourcePercentType percentType) {
    TArray<UResourceComponentBase*> resources = GetResourceFromActors(actors, resourceName);
    for (int r = 0; r < resources.Num(); r++) {
        resources[r]->K2_AddResourceByPercent(addPercent, percentType);
    }
}
void UResourceFunctionLibrary::DrainResourcePercentFromActors(TArray<AActor*> actors, FName resourceName, float drainPercent, EResourcePercentType percentType) {
    TArray<UResourceComponentBase*> resources = GetResourceFromActors(actors, resourceName);
    for (int r = 0; r < resources.Num(); r++) {
        resources[r]->K2_DrainResourceByPercent(drainPercent, percentType);
    }
}

TArray<UHealthResource*> UResourceFunctionLibrary::GetActorHealthResources(AActor* actor, TArray<FName> healthResourceNameFilter) {
    if (!IsValid(actor)) { return TArray<UHealthResource*>(); }

    TArray<UHealthResource*> retVal;
    actor->GetComponents<UHealthResource>(retVal);
    if (healthResourceNameFilter.Num() > 0) {
        for (int h = retVal.Num()-1; h >= 0; h--) {
            if (!healthResourceNameFilter.Contains(retVal[h]->GetResourceName())) {
                retVal.RemoveAt(h);
            }
        }
    }
    return retVal;
}

bool UResourceFunctionLibrary::ActorImplementsAnyModification(AActor* actor, TArray<FName> modificationNames, TArray<FName> healthResourceNameFilter) {
    if (!IsValid(actor)) { return false; }
    bool retVal = false;
    TArray<UHealthResource*> healthComps = GetActorHealthResources(actor, healthResourceNameFilter);
    for (UHealthResource* h : healthComps) {
        if (h->HasModifications(modificationNames)) { return true; }
    }
    return false;
}

void UResourceFunctionLibrary::GiveModificationDataToActor(AActor* actor, TArray<UDamageModificationData*> modificationData, TArray<FName> healthResourceNameFilter) {
    if (!IsValid(actor)) { return; }

    TArray<UHealthResource*> healthComps = GetActorHealthResources(actor, healthResourceNameFilter);
    for (int h = 0; h < healthComps.Num(); h++) {
        for (UDamageModificationData* mD : modificationData) {
            healthComps[h]->GiveModificationData(mD);
        }
    }
}

void UResourceFunctionLibrary::RemoveModificationFromActor(AActor* actor, TArray<FName> modificationNames, TArray<FName> healthResourceNameFilter) {
    if (!IsValid(actor)) { return; }

    TArray<UHealthResource*> healthComps = GetActorHealthResources(actor, healthResourceNameFilter);
    for (int h = 0; h < healthComps.Num(); h++) {
        for (FName mN : modificationNames) {
            healthComps[h]->RemoveModifier(mN);
        }
    }
}

