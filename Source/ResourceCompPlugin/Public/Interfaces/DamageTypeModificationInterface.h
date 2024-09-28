// Copyright LyCH. 2024

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageTypeModificationInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageTypeModificationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RESOURCECOMPPLUGIN_API IDamageTypeModificationInterface
{
	GENERATED_BODY()

public:
	/*
	 * Used on Damage Type classes to modify damage directly on the damage type. 
	 * If the modification attempts to use this, but the damage time does not implement the interface, it will not apply the modification.
	 */UFUNCTION(BlueprintNativeEvent, Category = "Health System")
	float ModifyDamage(float incomingDamage, AActor* damagedActor) const;
};
