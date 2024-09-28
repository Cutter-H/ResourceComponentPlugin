// Copyright LyCH. 2024


#include "Components/ResourceComponentBase.h"
#include "Net/UnrealNetwork.h"

// MP Reqs
#include "GameFramework/Actor.h"

void UResourceComponentBase::K2_AddResource_Implementation(float addAmount) {
	AddResource(addAmount);
}
void UResourceComponentBase::K2_DrainResource_Implementation(float drainAmount) {
	DrainResource(drainAmount);
}
void UResourceComponentBase::K2_AddResourceByPercent_Implementation(float addPercent, EResourcePercentType percentType) {
	AddResourceByPercent(addPercent, percentType);
}
void UResourceComponentBase::K2_DrainResourceByPercent_Implementation(float drainPercent, EResourcePercentType percentType) {
	DrainResourceByPercent(drainPercent, percentType);
}
void UResourceComponentBase::K2_SetRegenAmount_Implementation(float newRegenAmount) {
	SetRegenAmount(newRegenAmount);
}
void UResourceComponentBase::K2_SetRegenRate_Implementation(float newRegenRate) {
	SetRegenRate(newRegenRate);
}
void UResourceComponentBase::K2_SetRegenDelay_Implementation(float newDelayAmount) {
	SetRegenDelay(newDelayAmount);
}
float UResourceComponentBase::K2_GetMaxAmount_Implementation() const {
	return GetMaxAmount();
}
void UResourceComponentBase::K2_SetCanBeDrained_Implementation(bool bCanBeDrained) {
	SetCanBeDrained(bCanBeDrained);
}
bool UResourceComponentBase::K2_GetCanBeDrained_Implementation() const {
	return GetCanBeDrained();
}
float UResourceComponentBase::GetTimeSinceLastDrain_Implementation() const {
	return GetWorld()->GetTimeSeconds() - TimeAtLastDrain;
}

UResourceComponentBase::UResourceComponentBase() {
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

}
void UResourceComponentBase::BeginPlay() {
	Super::BeginPlay();
	if(GetOwner()->HasAuthority()) {
		CurrentAmount = bRegenBeginsEmpty ? 0.f : K2_GetMaxAmount();
	}
}
void UResourceComponentBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UResourceComponentBase, CurrentAmount);
	DOREPLIFETIME(UResourceComponentBase, TimeAtLastDrain);
}

void UResourceComponentBase::AddResource(float addAmount) {
	if (addAmount < 0) {
		K2_DrainResource(addAmount * -1);
		return;
	}
	if (CurrentAmount >= K2_GetMaxAmount()) {
		return;
	}
	float initialAmount = CurrentAmount;
	float predictedAmount = FMath::Min(CurrentAmount + addAmount, K2_GetMaxAmount());
	
	CurrentAmount = FMath::Min(K2_GetMaxAmount(), CurrentAmount + addAmount);
	BroadcastResourceChange_Net(initialAmount, predictedAmount);
	
	bool timerActive = !GetWorld()->GetTimerManager().IsTimerPending(RegenTimer);
	if (timerActive) {
		BroadcastRegenEvent_Net(EHealthRegenEventType::Tick);
		if (bFirstRegenTick) {
			bFirstRegenTick = false;
			BroadcastRegenEvent_Net(EHealthRegenEventType::Start);
		}
		if (GetCurrentPercent() >= 1) {
			StopRegenTimer();
			BroadcastRegenEvent_Net(EHealthRegenEventType::End);
		}
	}
}
void UResourceComponentBase::DrainResource(float drainAmount) {
	if (drainAmount < 0) {
		K2_AddResource(drainAmount * -1);
		return;
	}
	if (bDrainDisabled) {
		return;
	}
	float initialAmount = CurrentAmount;
	float predictedAmount = FMath::Max(0.f, CurrentAmount - drainAmount);
	
	CurrentAmount = FMath::Max(predictedAmount, 0);
	BroadcastResourceChange_Net(initialAmount, predictedAmount);

	/* Drain time registration */ {
		float gameTime = GetWorld()->GetTimeSeconds();
		TimeAtLastDrain = gameTime;
	}

	/* Regen timer */ {
		bool timerActive = !GetWorld()->GetTimerManager().IsTimerPending(RegenTimer);
		if (timerActive) {
			BroadcastRegenEvent_Net(EHealthRegenEventType::End);
		}
		SetRegenTimer();
	}
}
void UResourceComponentBase::AddResourceByPercent(float addPercent, EResourcePercentType percentType) {
	float fillValue = 0.f;
	if (percentType == EResourcePercentType::Current) {
		fillValue = CurrentAmount * addPercent;
	}
	else {
		fillValue = K2_GetMaxAmount() * addPercent;
	}
	K2_AddResource(fillValue);
}
void UResourceComponentBase::DrainResourceByPercent(float drainPercent, EResourcePercentType percentType) {
	float drainValue = 0.f;
	if (percentType == EResourcePercentType::Current) {
		drainValue = CurrentAmount * drainPercent;
	}
	else {
		drainValue = K2_GetMaxAmount() * drainPercent;
	}
	K2_DrainResource(drainValue);
}

void UResourceComponentBase::SetRegenAmount(float newRegenAmount) {
	float timerRemaining = GetRegenDelay();
	if (RegenTimer.IsValid()) {
		timerRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(RegenTimer);
	}
	RegenAmount = newRegenAmount;
	if (timerRemaining > 0) {
		SetRegenTimer(timerRemaining);
	}
	else {
		SetRegenTimer();
	}
}
void UResourceComponentBase::SetRegenRate(float newRegenRate) {
	float timerRemaining = GetRegenDelay();
	if (RegenTimer.IsValid()) {
		timerRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(RegenTimer);
	}
	RegenRate = newRegenRate;
	if (timerRemaining > 0) {
		SetRegenTimer(timerRemaining);
	}
	else {
		SetRegenTimer();
	}
}
void UResourceComponentBase::SetRegenDelay(float newRegenDelay) {
	float timerRemaining = 0.f;
	if (RegenTimer.IsValid()) {
		timerRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(RegenTimer);
	}
	RegenDelay = newRegenDelay;
	if (timerRemaining > 0) {
		SetRegenTimer(timerRemaining);
	}
	else {
		SetRegenTimer();
	}
}
bool UResourceComponentBase::ShouldRegen() const {
	if ((RegenAmount <= 0) || (RegenRate <= 0) || (GetRegenDelay() < 0)) {
		return false;
	}
	return true;
}
void UResourceComponentBase::SetRegenTimer(float initialDelay) {
	StopRegenTimer();
	if (!ShouldRegen()) {
		return;
	}

	FTimerDelegate timerDel;
	timerDel.BindUObject(this, &UResourceComponentBase::K2_AddResource, RegenAmount);

	FTimerManagerTimerParameters timerParams;
	if (initialDelay >= 0) {
		timerParams.FirstDelay = initialDelay;
	}
	else {
		bFirstRegenTick = true;
		timerParams.FirstDelay = GetRegenDelay();
	}
	timerParams.bLoop = true;

	GetWorld()->GetTimerManager().SetTimer(RegenTimer, timerDel, 1 / RegenRate, timerParams);
}

void UResourceComponentBase::RegisterDrainTime_Server_Implementation(float time) {
	TimeAtLastDrain = time;
}
void UResourceComponentBase::AddResource_Server_Implementation(float additional) {
	float initialAmount = CurrentAmount;
	CurrentAmount = FMath::Min(K2_GetMaxAmount(), CurrentAmount + additional);
	BroadcastResourceChange_Net(initialAmount, CurrentAmount);
}
void UResourceComponentBase::DrainResource_Server_Implementation(float removal) {
	float initialAmount = CurrentAmount;
	CurrentAmount = FMath::Max(CurrentAmount - removal, 0);
	BroadcastResourceChange_Net(initialAmount, CurrentAmount);
}

void UResourceComponentBase::BroadcastResourceChange_Net_Implementation(float oldValue, float newValue) {
	if (oldValue != newValue) {
		OnCurrentAmountChange.Broadcast(newValue);
	}
	if (oldValue > newValue) {
		OnDrain.Broadcast(newValue);
	}
	if (newValue > oldValue) {
		OnAdd.Broadcast(newValue);
	}
	if (newValue != oldValue && newValue == 0) {
		OnEmpty.Broadcast();
	}
	if (newValue != oldValue && newValue == GetMaxAmount()) {
		OnFill.Broadcast();
	}
}
void UResourceComponentBase::BroadcastRegenEvent_Net_Implementation(EHealthRegenEventType type, float newValue) {
	switch (type) {
	case EHealthRegenEventType::Start:
		OnRegenStart.Broadcast();
		break;
	case EHealthRegenEventType::Tick:
		OnRegenTick.Broadcast(newValue);
		break;
	case EHealthRegenEventType::End:
		OnRegenEnd.Broadcast();
		break;
	}
}
