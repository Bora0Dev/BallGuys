// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BallGuysMainMenuGameMode.h"
#include "UI/BallGuysMainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ABallGuysMainMenuGameMode::ABallGuysMainMenuGameMode()
{
	// No playable pawn or in-game HUD on the Main Menu map
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}

void ABallGuysMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuWidgetClass)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			MainMenuWidget = CreateWidget<UBallGuysMainMenuWidget>(PC, MainMenuWidgetClass);
			if (MainMenuWidget)
			{
				MainMenuWidget->AddToViewport();
			}
		}
	}
}
