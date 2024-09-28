// Copyright LyCH. 2024


#include "Data/DamageModificationData.h"

TArray<FName> UDamageModificationData::GetAllModificationNames() const {
    TArray<FName> retVal;

    for (FIncomingDamageModification mod : Modifications) {
        retVal.Add(mod.ModificationName);
    }
    return retVal;
}
