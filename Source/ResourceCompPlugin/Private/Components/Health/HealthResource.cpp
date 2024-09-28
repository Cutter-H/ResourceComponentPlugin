// Copyright LyCH. 2024


#include "Components/Health/HealthResource.h"
#include "Interfaces/DamageTypeModificationInterface.h"
#include "Data/DamageModificationData.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//MP Reqs
#include "GameFramework/Pawn.h"
#include "GameFramework/DamageType.h"
#include "Engine.h"

UHealthResource::UHealthResource() {
	ResourceName = "Health";
}
void UHealthResource::BeginPlay() {
	Super::BeginPlay();
	//Self delegates
	if (IsServer()) {
		OnGenericDamageTaken.AddDynamic(this, &UHealthResource::GenericDamageTaken);
		OnPointDamageTaken.AddDynamic(this, &UHealthResource::PointDamageTaken);
		OnRadialDamageTaken.AddDynamic(this, &UHealthResource::RadialDamageTaken);
		GiveModificationData(DefaultModificationData);
	}
	//Owner Delegates
	BindDamageDelegates();
}
void UHealthResource::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthResource, ModificationRules);
	DOREPLIFETIME(UHealthResource, LastDamageCauser);
	DOREPLIFETIME(UHealthResource, LastLocationHitFrom);
}
// Getters
bool UHealthResource::IsServer() const {
	return GetOwner()->HasAuthority();
}
bool UHealthResource::IsLocallyControlled() const {
	if (Cast<APawn>(GetOwner()))
		if (Cast<APawn>(GetOwner())->IsLocallyControlled())
			return true;
	return false;
}
bool UHealthResource::IsPlayerControlled() const {
	if (Cast<APawn>(GetOwner()))
		if (Cast<APawn>(GetOwner())->IsPlayerControlled())
			return true;
	return false;
}
bool UHealthResource::HasModifications(TArray<FName> modificationNames) {
	for (FIncomingDamageModification m : ModificationRules) {
		if (modificationNames.Contains(m.ModificationName)) {
			return true;
		}
	}
	return false;
}
float UHealthResource::GetDirection(const FVector& Direction, const FRotator& BaseRotation) const {
	if (Direction.IsNearlyZero()) {
		return 0.f;
	}
		FMatrix RotMatrix = FRotationMatrix(BaseRotation);
		FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
		FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
		FVector NormalizedVel = Direction.GetSafeNormal2D();

		// get a cos(alpha) of forward vector vs velocity
		float ForwardCosAngle = FVector::DotProduct(ForwardVector, NormalizedVel);
		// now get the alpha and convert to degree
		float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

		// depending on where right vector is, flip it
		float RightCosAngle = FVector::DotProduct(RightVector, NormalizedVel);
		if (RightCosAngle < 0) {
			ForwardDeltaDegree *= -1;
		}
		return ForwardDeltaDegree;
}
float UHealthResource::GetDirectionToLocation(const FVector& Location, const FRotator& BaseRotation) const {
	if (!GetOwner())
		return 0.f;

	FRotator DirectionRot = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), Location);

	if (DirectionRot.Vector().IsNearlyZero())
		return 0.f;

	FMatrix RotMatrix = FRotationMatrix(BaseRotation);
	FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
	FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
	FVector NormalizedVel = DirectionRot.Vector().GetSafeNormal2D();

	// get a cos(alpha) of forward vector vs velocity
	float ForwardCosAngle = FVector::DotProduct(ForwardVector, NormalizedVel);
	// now get the alpha and convert to degree
	float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

	// depending on where right vector is, flip it
	float RightCosAngle = FVector::DotProduct(RightVector, NormalizedVel);
	if (RightCosAngle < 0) {
		ForwardDeltaDegree *= -1;
	}
	return ForwardDeltaDegree;
}
// Modify Damage
float UHealthResource::K2_ModifyDamage_Implementation(float damageReceived, EIncomingDamageChannel damageChannel, const class UDamageType* DamageType, FName boneName, FVector damageOrigin) const {
	return ModifyDamage(damageReceived, damageChannel, DamageType, boneName, damageOrigin);
}
float UHealthResource::ModifyDamage(float damageReceived, EIncomingDamageChannel damageChannel, const class UDamageType* DamageType, FName boneName, FVector damageOrigin) const {
	float modifiedDamage = damageReceived;
	bool bWhiteListedDamageType = true;
	bool bCorrectBone = false;
	bool bWithinRange = false;
	bool bCorrectDamageChannel = false;
	for (FIncomingDamageModification modification : ModificationRules) {
		bWhiteListedDamageType = ModificationAcceptsDamageType(modification, DamageType);
		bCorrectBone = (damageChannel != EIncomingDamageChannel::PointDamage || modification.WhitelistedBoneNames.Num() == 0 || modification.WhitelistedBoneNames.Contains(boneName));
		bWithinRange = (modification.MinimumRange <= 0 || (damageOrigin - GetOwner()->GetActorLocation()).Length() >= modification.MinimumRange)
			&& (modification.MaximumRange <= 0 || (damageOrigin - GetOwner()->GetActorLocation()).Length() <= modification.MaximumRange);
		bCorrectDamageChannel = (damageChannel == modification.DamageChannel || modification.DamageChannel == AllChannels);
		if (bWhiteListedDamageType && bWithinRange && bCorrectBone && bCorrectDamageChannel) {

			if (modification.ModificationType == EIncomingDamageModificationType::Override_Damage) {
				return modification.Magnitude;
			}
			if (modification.ModificationType == EIncomingDamageModificationType::Modify_From_DamageType) {
				if (UKismetSystemLibrary::DoesImplementInterface(DamageType->GetClass()->GetDefaultObject(), UDamageTypeModificationInterface::StaticClass())) {
					modifiedDamage = IDamageTypeModificationInterface::Execute_ModifyDamage(DamageType->GetClass()->GetDefaultObject(), damageReceived, GetOwner());
				}
			}
			if (modification.ModificationType == EIncomingDamageModificationType::Add_Damage) {
				modifiedDamage = modifiedDamage + modification.Magnitude;
			}
			if (modification.ModificationType == EIncomingDamageModificationType::Multiply_Damage) {
				modifiedDamage = modifiedDamage * modification.Magnitude;
			}
		}
	}
	return modifiedDamage;
}
bool UHealthResource::ModificationAcceptsDamageType(FIncomingDamageModification modification, const UDamageType* damageType) const {
	if (modification.WhitelistedDamageTypes.Num() <= 0 || modification.WhitelistedDamageTypes.Contains(damageType->GetClass())) { return true; }
	
	if (!modification.bWhitelistChildDamageTypes) { return false; }

	for (TSubclassOf<UDamageType> damageClass : modification.WhitelistedDamageTypes) {
		if (damageType->GetClass()->IsChildOf(damageClass))
			return true;
	}

	return false;
}
void UHealthResource::GiveModifier(FIncomingDamageModification newModifier, int insertAt) {
	if (insertAt >= 0) {
		ModificationRules.Insert(newModifier, insertAt);
	}
	else {
		ModificationRules.Add(newModifier);
	}
	ModificationChanged(newModifier, true);
	
}
void UHealthResource::GiveModificationData(UDamageModificationData* modificationData, int beginInsertAt) {
	if (!IsValid(modificationData)) { return; }

	for (int i = 0; i < modificationData->Modifications.Num(); i++) {
		if (beginInsertAt < 0) {
			GiveModifier(modificationData->Modifications[i]);
		}
		else {
			GiveModifier(modificationData->Modifications[i], beginInsertAt + i);
		}
	}
	ModificationDataAdded(modificationData);
}
void UHealthResource::RemoveModifier(FName modifierName) {
	for (int index = 0; index < ModificationRules.Num(); index++) {
		if (ModificationRules[index].ModificationName == modifierName) {
			const FIncomingDamageModification mod = ModificationRules[index];
			ModificationRules.RemoveAt(index);
			ModificationChanged(mod, false);
		}
	}
}
void UHealthResource::ModificationDataAdded_Implementation(const UDamageModificationData* modificationData) {
	OnModificationDataAdded.Broadcast(modificationData);
}
void UHealthResource::ModificationChanged_Implementation(FIncomingDamageModification modification, bool bAdded) {
	bAdded ? OnModificationAdded.Broadcast(modification) : OnModificationRemoved.Broadcast(modification);
}
// Damage Binders
void UHealthResource::OnAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser) {
	LastDamageCauser = DamageCauser;
	if (bBlockDamage) {
		bBlockDamage = false;
		return;
	}
	float modifiedDamage = ModifyDamage(Damage, EIncomingDamageChannel::GenericDamage, DamageType, FName(), DamageCauser->GetActorLocation());
	DrainResource(modifiedDamage);
	LastLocationHitFrom = DamageCauser->GetActorLocation();
	OnGenericDamageTaken.Broadcast(GetOwner(), modifiedDamage, DamageType, InstigatedBy, DamageCauser);
	if (bDebug) {
		FString debugString = FString(GetNameSafe(this)).Append(": Damage received in Any Damage: ").Append(FString::SanitizeFloat(Damage));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Green, *debugString);
	}

}
void UHealthResource::OnPointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser) {
	bBlockDamage = true;
	float modifiedDamage = ModifyDamage(Damage, EIncomingDamageChannel::PointDamage, DamageType, BoneName, DamageCauser->GetActorLocation());
	DrainResource(modifiedDamage);
	LastLocationHitFrom = DamageCauser->GetActorLocation();
	OnPointDamageTaken.Broadcast(DamagedActor, modifiedDamage, InstigatedBy, HitLocation, HitComponent, BoneName, ShotFromDirection, DamageType, DamageCauser);
	if (bDebug) {
		FString debugString = FString(GetNameSafe(this)).Append(": Point  Damage: ").Append(FString::SanitizeFloat(modifiedDamage));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Green, *debugString);
	}

}
void UHealthResource::OnRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser) {
	bBlockDamage = true;
	float modifiedDamage = ModifyDamage(Damage, EIncomingDamageChannel::RadialDamage, DamageType, FName(), Origin);
	DrainResource(modifiedDamage);
	LastLocationHitFrom = Origin;
	OnRadialDamageTaken.Broadcast(DamagedActor, modifiedDamage, DamageType, Origin, HitInfo, InstigatedBy, DamageCauser);
	if (bDebug) {
		FString debugString = FString(GetNameSafe(this)).Append(": Radial Damage: ").Append(FString::SanitizeFloat(modifiedDamage));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Green, *debugString);
	}
}

void UHealthResource::K2_BindDamageDelegates_Implementation(){
	BindDamageDelegates();
}
void UHealthResource::BindDamageDelegates() {
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthResource::OnAnyDamage);
	GetOwner()->OnTakePointDamage.AddDynamic(this, &UHealthResource::OnPointDamage);
	GetOwner()->OnTakeRadialDamage.AddDynamic(this, &UHealthResource::OnRadialDamage);
}

void UHealthResource::GenericDamageTaken_Implementation(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser) {
	if (!IsServer()) {
		OnGenericDamageTaken.Broadcast(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);
	}
}
void UHealthResource::PointDamageTaken_Implementation(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser) {
	if (!IsServer()) {
		OnPointDamageTaken.Broadcast(DamagedActor, Damage, InstigatedBy, HitLocation, HitComponent, BoneName, ShotFromDirection, DamageType, DamageCauser);
	}
}
void UHealthResource::RadialDamageTaken_Implementation(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser) {
	if (!IsServer()) {
		OnRadialDamageTaken.Broadcast(DamagedActor, Damage, DamageType, Origin, HitInfo, InstigatedBy, DamageCauser);
	}
}

