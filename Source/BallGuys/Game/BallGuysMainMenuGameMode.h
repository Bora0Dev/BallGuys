// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BallGuysMainMenuGameMode.generated.h"

class UBallGuysMainMenuWidget;

/**
 * GameMode for the Main Menu map.
 * Does not spawn a BallPawn or in-game HUD; automatically creates and displays the Main Menu widget.
 */
UCLASS()
class BALLGUYS_API ABallGuysMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABallGuysMainMenuGameMode();

protected:
	virtual void BeginPlay() override;

	/** Assign your WBP_MainMenu Widget Blueprint class here in Blueprint defaults */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UBallGuysMainMenuWidget> MainMenuWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UBallGuysMainMenuWidget> MainMenuWidget;
};
