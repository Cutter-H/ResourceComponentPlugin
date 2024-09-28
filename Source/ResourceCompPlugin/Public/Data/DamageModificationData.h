// Copyright LyCH. 2024

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/DamageType.h" //MP Req
#include "DamageModificationData.generated.h"

UENUM(BlueprintType)
enum EIncomingDamageChannel {
	AllChannels,
	PointDamage,
	RadialDamage,
	GenericDamage
};
UENUM(BlueprintType)
enum EIncomingDamageModificationType {
	Add_Damage,
	Multiply_Damage,
	Override_Damage,
	Modify_From_DamageType
};
USTRUCT(BlueprintType)
struct FIncomingDamageModification {
	GENERATED_BODY()
	/**
	 * String used to identify this rule for GiveModification and RemoveModification
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification")
	 FName ModificationName = FName();
	/**
	 * What damage type prompts this
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification")
	TEnumAsByte<EIncomingDamageChannel> DamageChannel;
	/**
	 * How this damage modification is handled.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification")
	TEnumAsByte< EIncomingDamageModificationType> ModificationType;
	/**
	 * How much this modifies the damage.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification",
		meta = (EditCondition = "ModificationType != EIncomingDamageModificationType::Modify_From_DamageType", EditConditionHides))
	float Magnitude = 0.f;
	/**
	 * The bone that received the damage must be included in this array if it is not empty.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification",
		meta = (EditCondition = "DamageChannel == EIncomingDamageChannel::PointDamage || DamageChannel == EIncomingDamageChannel::AllChannels", EditConditionHides))
	TArray<FName> WhitelistedBoneNames;

	/*
	 * The minimum distance the damage can be taken from for it to be modified. If this is less than or equal to 0 it is ignored.
	 * The distance is measured from the owning actor and the HitResult trace start(Point), radial damage origin(Radial), or damage causer location(Generic).
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification")
	 float MinimumRange = 0.f;
	/*
	 * The maximum distance the damage can be taken from for it to be modified. If this is less than or equal to 0 it is ignored.
	 * The distance is measured from the owning actor and the HitResult trace start(Point), radial damage origin(Radial), or damage causer location(Generic).
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification")
	float MaximumRange = 0.f;

	/**
	 * The damage received must be from these damage types if the array is not empty.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification")
	TArray<TSubclassOf<UDamageType>> WhitelistedDamageTypes;
	/**
	 * If true it will whitelist all children of the whitelisted damage types.
	 */UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable|Damage Modification",
		meta = (EditCondition = "IsValid(WhitelistedDamageTypes[0])", EditConditionHides))
	bool bWhitelistChildDamageTypes = false;

	FIncomingDamageModification()
		: ModificationName(FName())
		, DamageChannel(EIncomingDamageChannel::AllChannels)
		, ModificationType(EIncomingDamageModificationType::Multiply_Damage)
		, Magnitude(1.f)
		, WhitelistedBoneNames(TArray<FName>())
		, MinimumRange(0.f)
		, MaximumRange(0.f)
		, WhitelistedDamageTypes(TArray<TSubclassOf<UDamageType>>())
		, bWhitelistChildDamageTypes(false) {
	}

};

/**
 * 
 */
UCLASS()
class RESOURCECOMPPLUGIN_API UDamageModificationData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Damage Modifications")
	FText Description = FText::FromString("This variable is never used and is for the editor only. It is for a quick note of the Data Asset when viewing it in the editor.");
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Damage Modifications", meta = (TitleProperty = "ModificationName"))
	TArray< FIncomingDamageModification> Modifications;

	UFUNCTION(BlueprintCallable, Category = "Damage Modifications")
	TArray<FName> GetAllModificationNames() const;
	
};
