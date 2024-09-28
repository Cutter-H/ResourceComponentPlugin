// Copyright LyCH. 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/Health/HealthResource.h"
#include "Blueprint/UserWidget.h"
#include "HealthResourceWithUI.generated.h"

class UWidgetComponent;

UENUM(BlueprintType)
enum EOverheadWidgetVisibility : uint8{
	OHWO_OverheadDisabled UMETA(Tooltip = "The Overhead widget is never shown, but is created.", DisplayName = "Overhead Disabled"),
	OHWO_ShowOnlyOnPossessedPlayer UMETA(Tooltip = "The Overhead widget is only shown to the owning player.", DisplayName="Show Only On Possessed Player"),
	OHWO_ShowOnlyOnOther UMETA(Tooltip = "The Overhead widget is not shown to the owning player.", DisplayName="Show Only On Others"),
	OHWO_ShowOnAll UMETA(Tooltip = "The Overhead widget is used on both the owning player and others.", DisplayName="Show On All")
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Resource), meta = (BlueprintSpawnableComponent))
class RESOURCECOMPPLUGIN_API UHealthResourceWithUI : public UHealthResource
{
	GENERATED_BODY()
public:
	/*
	 * This controls if the OnScreenWidget is used at all. If true, it will be added to the player's screen when possessed.
	 */UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "UI")
	bool bEnableOnscreen= true;
	/**
	 * Setting for how and when to show the overhead widget.
	 */UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "UI")
	TEnumAsByte<EOverheadWidgetVisibility> OverheadWidgetSettings = EOverheadWidgetVisibility::OHWO_ShowOnlyOnOther;
	/*
	 * This class is used when creating the widget for the possessing player's UI.
	 */UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> OnScreenWidgetClass;
	/*
	 * This class is used for the character's overhead widget used in a WidgetComponent.
	 */UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> OverheadWidgetClass;

	/*
	 * The Z Order for the OnscreenWidget. Higher values are rendered on top.
	 */UPROPERTY(EditAnywhere, Category = "UI|Onscreen")
	 int ZOrder = 0;
	/*
	 * The material that will be applied to the overhead widget.
	 */UPROPERTY(EditAnywhere, Category = "UI|Overhead")
	TObjectPtr<UMaterialInterface> OverheadWidgetMaterial;
	/*
	 * The space where the widget will appear. If false, the widget will be in screen space.
	 * See similar settings in the WidgetComponent class.
	 */UPROPERTY(EditAnywhere, Category = "UI|Overhead")
	bool bUseWorldSpace = true;
	/*
	 * The draw size used for the overhead widget.
	 * See similar settings in the WidgetComponent class.
	 */UPROPERTY(EditAnywhere, Category = "UI|Overhead")
	 FVector2D DrawSize = FVector2D(126.f, 16.f);
	/*
	 * If true, the World Widget's draw size is overridden. 
	 * See similar settings in the WidgetComponent class.
	 */UPROPERTY(EditAnywhere, Category = "UI|Overhead")
	bool bDrawAtDesiredSize = false;
	/*
	 * The offset the Vector will be from the owning actor. 
	 */UPROPERTY(EditAnywhere, Category = "UI|Overhead")
	FTransform WidgetOffset = FTransform(FVector(0,0,100.f));
	/*
	 * The User Widget that is placed on the owning player’s screen.
	 */UPROPERTY(BlueprintReadOnly, Category = "UI|Widgets")
	 TObjectPtr<UWidgetComponent> OverheadWidgetComponent;
	/*
	 * The Widget Component that is attached to the owner.
	 */UPROPERTY(BlueprintReadOnly, Category = "UI|Widgets")
	TObjectPtr<UUserWidget> OverheadWidget;
	/*
	 * The User Widget used in the World Widget Component.
	 */UPROPERTY(BlueprintReadOnly, Category = "UI|Widgets")
	TObjectPtr<UUserWidget> OnScreenWidget;
	/*
	 * Changes how the widgets are viewed.
	 */UFUNCTION(BlueprintCallable, Category = "UI")
	void ChangeWidgetSettings(bool bUseOnscreen, EOverheadWidgetVisibility useOverhead);

protected:
	UHealthResourceWithUI();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
	virtual void TryCreateOnScreenWidget(APlayerController* owningPlayer);
	virtual void TryCreateOverheadWidgetComponent();
	
	TObjectPtr<APlayerController> ActivePlayerController;

private:
	UFUNCTION(Server, Reliable)
	void ChangeWidgetSettingsOnServer(bool bUseOnscreen, EOverheadWidgetVisibility useOverhead);
	UFUNCTION(NetMulticast, Reliable)
	void UpdateOverheadWidgetVisibilityFromServer();
	UFUNCTION(Client, Reliable)
	void UpdateOnscreenWidgetVisibilityFromServer();
	UFUNCTION()
	void SetOverheadVisibility(EOverheadWidgetVisibility newVisibility);
	UFUNCTION()
	void OnOwnerControllerChanged(APawn* pawn, AController* oldController, AController* newController);
	UFUNCTION()
	void OnControllerPossessChange(APawn* OldPawn, APawn* NewPawn);
};
