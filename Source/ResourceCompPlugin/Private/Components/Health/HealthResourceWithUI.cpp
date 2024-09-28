// Copyright LyCH. 2024


#include "Components/Health/HealthResourceWithUI.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"

#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

//MP Reqs
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInterface.h"

void UHealthResourceWithUI::ChangeWidgetSettings(bool bUseOnscreen, EOverheadWidgetVisibility useOverhead) {
    if (!IsValid(GetOwner())) {
        return;
    }
    if (GetOwner()->HasAuthority()) {
        bEnableOnscreen = bUseOnscreen;
        UpdateOnscreenWidgetVisibilityFromServer();
        OverheadWidgetSettings = useOverhead;
        UpdateOverheadWidgetVisibilityFromServer();
    }
    else {
        ChangeWidgetSettingsOnServer(bUseOnscreen, useOverhead);
    }
}

UHealthResourceWithUI::UHealthResourceWithUI() {
    //FString pluginContentDir = IPluginManager::Get().FindPlugin(TEXT("ResourceCompPlugin"))->GetContentDir();
    
    /* Setup default OnScreen Widget Class*/ {
        const FString defaultOnScreenDirectory = ("/Script/UMGEditor.WidgetBlueprint'/ResourceCompPlugin/UI/WBP_Screen_Widget.WBP_Screen_Widget_C'");
        static ConstructorHelpers::FClassFinder<UUserWidget> defaultOnScreen(*defaultOnScreenDirectory);
        if (defaultOnScreen.Succeeded()) {
            OnScreenWidgetClass = defaultOnScreen.Class;
        }
    }
    
    /* Setup default World Widget Class*/ {
        const FString defaultOverheadDirectory = ("/Script/UMGEditor.WidgetBlueprint'/ResourceCompPlugin/UI/WBP_World_Widget.WBP_World_Widget_C'");
        static ConstructorHelpers::FClassFinder<UUserWidget> defaultWorldWidget(*defaultOverheadDirectory);
        if (defaultWorldWidget.Succeeded()) {
            OverheadWidgetClass = defaultWorldWidget.Class;
        }
    }

    /* Setup default Overhead Widget Material*/ {
        const FString defaultOverheadMatDirectory = ("/Script/Engine.Material'/ResourceCompPlugin/Materials/M_OverheadWidgetMaterial.M_OverheadWidgetMaterial'");
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> defaultOverheadMaterial(*defaultOverheadMatDirectory);
        if (defaultOverheadMaterial.Succeeded()) {
            OverheadWidgetMaterial = defaultOverheadMaterial.Object;
        }
    }
    ///
    
}

void UHealthResourceWithUI::BeginPlay() {
    Super::BeginPlay();
    
    TryCreateOverheadWidgetComponent();

    if (IsValid(GetOwner())) {
        if (GetOwner()->HasAuthority()) {
            UpdateOverheadWidgetVisibilityFromServer();
        }
        SetOverheadVisibility(OverheadWidgetSettings);
    }

    if (APawn* owningPawn = GetOwner<APawn>()) {
        owningPawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UHealthResourceWithUI::OnOwnerControllerChanged);
        if (APlayerController* pCon = owningPawn->GetController<APlayerController>()) {
            if (pCon->IsLocalPlayerController()) {
                TryCreateOnScreenWidget(pCon);
                if (IsValid(OnScreenWidget) && bEnableOnscreen) {
                    OnScreenWidget->AddToViewport(ZOrder);
                }
                ActivePlayerController = pCon;
                if (IsValid(ActivePlayerController)) {
                    ActivePlayerController->OnPossessedPawnChanged.AddDynamic(this, &UHealthResourceWithUI::OnControllerPossessChange);
                }
            }
        }
    }    
}

void UHealthResourceWithUI::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UHealthResourceWithUI, bEnableOnscreen);
    DOREPLIFETIME(UHealthResourceWithUI, OverheadWidgetSettings);

}

void UHealthResourceWithUI::TryCreateOnScreenWidget(APlayerController* owningPlayer) {
    if (IsValid(OnScreenWidget) || !IsValid(GetOwner<APawn>()) || !IsValid(OnScreenWidgetClass) || !(GetOwner<APawn>()->IsLocallyControlled())) {
        return;
    }
    else {
        OnScreenWidget = UWidgetBlueprintLibrary::Create(GetWorld(), OnScreenWidgetClass, owningPlayer);
        if (IsValid(OnScreenWidget)) {
                        
            /* Sets a reference to this on the widget. */
            UFunction* setResourceCompFunction = OnScreenWidget->FindFunction(FName("SetResourceComponent"));
            if (IsValid(setResourceCompFunction)) {
                struct FArgStruct {
                    UResourceComponentBase* Arg;
                };
                FArgStruct argParam;
                argParam.Arg = this;
                OnScreenWidget->ProcessEvent(setResourceCompFunction, &argParam);
            }
        }
    }
}

void UHealthResourceWithUI::TryCreateOverheadWidgetComponent() {
    if (IsValid(OverheadWidgetComponent) || !IsValid(GetOwner()) || !IsValid(OverheadWidgetClass)) {
        if (IsValid(OverheadWidgetComponent)) {
            if (APawn* pawn = GetOwner<APawn>()) {
                if (APlayerController* pCon = pawn->GetController<APlayerController>()) {
                    if (pCon->IsLocalPlayerController()) {
                        OverheadWidgetComponent->SetOwnerPlayer(pCon->GetLocalPlayer());
                    }
                }
            }
        }
        return;
    }
    if (USceneComponent* overheadRoot = Cast<USceneComponent>(GetOwner()->AddComponentByClass(USceneComponent::StaticClass(), true, GetOwner()->GetActorTransform(), true))) {
        GetOwner()->FinishAddComponent(overheadRoot, false, FTransform());
        overheadRoot->RegisterComponent();
        overheadRoot->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
        overheadRoot->SetUsingAbsoluteRotation(true);
        overheadRoot->SetUsingAbsoluteScale(true);
        OverheadWidgetComponent = Cast<UWidgetComponent>(GetOwner()->AddComponentByClass(UWidgetComponent::StaticClass(), true, GetOwner()->GetActorTransform(), true));
        if (IsValid(OverheadWidgetComponent)) {
            OverheadWidgetComponent->SetIsReplicated(true);
            OverheadWidgetComponent->SetWidgetSpace(bUseWorldSpace ? EWidgetSpace::World : EWidgetSpace::Screen);
            OverheadWidgetComponent->SetDrawAtDesiredSize(bDrawAtDesiredSize);
            OverheadWidgetComponent->SetDrawSize(DrawSize);
            OverheadWidgetComponent->SetWidgetClass(OverheadWidgetClass);
            OverheadWidgetComponent->SetComponentTickEnabled(false);
            OverheadWidgetComponent->SetMaterial(0, OverheadWidgetMaterial);
            GetOwner()->FinishAddComponent(OverheadWidgetComponent, false, FTransform());
            overheadRoot->RegisterComponent();
            OverheadWidgetComponent->AttachToComponent(overheadRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            OverheadWidgetComponent->SetRelativeTransform(WidgetOffset);
            if (APawn* pawn = GetOwner<APawn>()) {
                if (APlayerController* pCon = pawn->GetController<APlayerController>()) {
                    if (pCon->IsLocalPlayerController()) {
                        OverheadWidgetComponent->SetOwnerPlayer(pCon->GetLocalPlayer());
                    }
                }
            }

            /* Sets a reference to this on the widget. */
            OverheadWidget = OverheadWidgetComponent->GetWidget();
            if (IsValid(OverheadWidget)) {
                /**/
                UFunction* setResourceCompFunction = OverheadWidget->FindFunction(FName("SetResourceComponent"));
                if (IsValid(setResourceCompFunction)) {
                    //The parameter value apparently has to be in a struct. WHYYYYY
                    struct FArgStruct {
                        UResourceComponentBase* Arg;
                    };
                    FArgStruct argParam;
                    argParam.Arg = this;
                    OverheadWidget->ProcessEvent(setResourceCompFunction, &argParam);
                }
                /**/
            }
        }
    }
}

void UHealthResourceWithUI::ChangeWidgetSettingsOnServer_Implementation(bool bUseOnscreen, EOverheadWidgetVisibility useOverhead) {
    bEnableOnscreen = bUseOnscreen;
    UpdateOnscreenWidgetVisibilityFromServer();
    OverheadWidgetSettings = useOverhead;
    UpdateOverheadWidgetVisibilityFromServer();
}

void UHealthResourceWithUI::UpdateOverheadWidgetVisibilityFromServer_Implementation() {
    TryCreateOverheadWidgetComponent();
    SetOverheadVisibility(OverheadWidgetSettings);
}

void UHealthResourceWithUI::UpdateOnscreenWidgetVisibilityFromServer_Implementation() {
    if (APlayerController* pCon = GetOwner<APawn>()->GetController<APlayerController>()) {
        if (pCon->IsLocalPlayerController()) {
            TryCreateOnScreenWidget(pCon);
            if (IsValid(OnScreenWidget) && bEnableOnscreen) {
                OnScreenWidget->AddToViewport(ZOrder);
            }
            else {
                OnScreenWidget->RemoveFromParent();
            }
        }
    }
}

void UHealthResourceWithUI::SetOverheadVisibility(EOverheadWidgetVisibility newVisibility) {
    if (!IsValid(OverheadWidgetComponent)) {
        return;
    }
    OverheadWidgetComponent->SetOwnerNoSee(false);
    OverheadWidgetComponent->SetOnlyOwnerSee(false);

    switch (newVisibility) {
    case EOverheadWidgetVisibility::OHWO_ShowOnlyOnPossessedPlayer: {
        OverheadWidgetComponent->SetOnlyOwnerSee(true);
        break;
    }
    case EOverheadWidgetVisibility::OHWO_ShowOnlyOnOther: {
        OverheadWidgetComponent->SetOwnerNoSee(true);
        break;
    }
    case EOverheadWidgetVisibility::OHWO_OverheadDisabled: {
        OverheadWidgetComponent->SetOnlyOwnerSee(true);
        OverheadWidgetComponent->SetOwnerNoSee(true);
        break;
    }
    }
}

void UHealthResourceWithUI::OnOwnerControllerChanged(APawn* pawn, AController* oldController, AController* newController) {

    // Always use world is always on so changes are not needed.
    // The world widget is created on Beginplay so only visibility changes are needed.
    if (IsValid(GetOwner())) {
        if (GetOwner()->HasAuthority()) {
            UpdateOverheadWidgetVisibilityFromServer();
        }
        else {
            SetOverheadVisibility(OverheadWidgetSettings);
        }
    }

    if (bEnableOnscreen) {
        APlayerController* pCon = Cast<APlayerController>(newController);
        if (IsValid(pCon)) {
            if (pCon->IsLocalController()) {
                ActivePlayerController = pCon;
                if (IsValid(ActivePlayerController)) {
                    ActivePlayerController->OnPossessedPawnChanged.AddDynamic(this, &UHealthResourceWithUI::OnControllerPossessChange);
                }
                TryCreateOnScreenWidget(pCon);
                if (IsValid(OnScreenWidget)) {
                    OnScreenWidget->AddToViewport(ZOrder);
                }
            }
        }
    }
}

void UHealthResourceWithUI::OnControllerPossessChange(APawn* OldPawn, APawn* NewPawn) {
    if (IsValid(OldPawn)) {
        if (OldPawn == GetOwner<APawn>()) {
            if (IsValid(OnScreenWidget)) {
                OnScreenWidget->RemoveFromParent();
            }
            if (IsValid(ActivePlayerController)) {
                ActivePlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &UHealthResourceWithUI::OnControllerPossessChange);
            }
        }
    }
    if (IsValid(GetOwner())) {
        if (GetOwner()->HasAuthority()) {
            UpdateOverheadWidgetVisibilityFromServer();
        }
        else {
            SetOverheadVisibility(OverheadWidgetSettings);
        }
    }
}