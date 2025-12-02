#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BallGuysHUD.generated.h"

/**
 * 
 */
UCLASS()
class BALLGUYS_API ABallGuysHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ABallGuysHUD();

protected:
	virtual void BeginPlay() override;

public:
	// Assign WBP_HUD here in the Blueprint subclass
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> MainHUDWidgetClass;

	UPROPERTY()
	class UUserWidget* MainHUDWidget;
};
