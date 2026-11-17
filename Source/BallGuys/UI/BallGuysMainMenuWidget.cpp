// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BallGuysMainMenuWidget.h"
#include "UI/BallGuysSettingsWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UBallGuysMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Configure UI input mode and show mouse cursor for the Main Menu
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}

	// Default the Public/Private toggle to Public (checked)
	if (PublicLobbyCheckBox)
	{
		PublicLobbyCheckBox->SetIsChecked(true);
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostLobby);
	}
	if (QuickJoinButton)
	{
		QuickJoinButton->OnClicked.AddDynamic(this, &ThisClass::QuickJoinLobby);
	}
	if (ServerBrowserButton)
	{
		ServerBrowserButton->OnClicked.AddDynamic(this, &ThisClass::OpenServerBrowser);
	}
	if (RefreshServersButton)
	{
		RefreshServersButton->OnClicked.AddDynamic(this, &ThisClass::RefreshServerList);
	}
	if (ServerBrowserBackButton)
	{
		ServerBrowserBackButton->OnClicked.AddDynamic(this, &ThisClass::ShowMainMenuPanel);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &ThisClass::OpenSettings);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &ThisClass::QuitGame);
	}
	if (SettingsWidget)
	{
		SettingsWidget->OnSettingsClosed.AddDynamic(this, &ThisClass::HandleSettingsClosed);
	}

	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->OnCreateLobbyComplete.AddDynamic(this, &ThisClass::HandleCreateLobbyComplete);
		GI->OnFindLobbiesComplete.AddDynamic(this, &ThisClass::HandleFindLobbiesComplete);
		GI->OnJoinLobbyComplete.AddDynamic(this, &ThisClass::HandleJoinLobbyComplete);
	}
}

void UBallGuysMainMenuWidget::NativeDestruct()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.RemoveDynamic(this, &ThisClass::HostLobby);
	}
	if (QuickJoinButton)
	{
		QuickJoinButton->OnClicked.RemoveDynamic(this, &ThisClass::QuickJoinLobby);
	}
	if (ServerBrowserButton)
	{
		ServerBrowserButton->OnClicked.RemoveDynamic(this, &ThisClass::OpenServerBrowser);
	}
	if (RefreshServersButton)
	{
		RefreshServersButton->OnClicked.RemoveDynamic(this, &ThisClass::RefreshServerList);
	}
	if (ServerBrowserBackButton)
	{
		ServerBrowserBackButton->OnClicked.RemoveDynamic(this, &ThisClass::ShowMainMenuPanel);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked.RemoveDynamic(this, &ThisClass::OpenSettings);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.RemoveDynamic(this, &ThisClass::QuitGame);
	}
	if (SettingsWidget)
	{
		SettingsWidget->OnSettingsClosed.RemoveDynamic(this, &ThisClass::HandleSettingsClosed);
	}

	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->OnCreateLobbyComplete.RemoveDynamic(this, &ThisClass::HandleCreateLobbyComplete);
		GI->OnFindLobbiesComplete.RemoveDynamic(this, &ThisClass::HandleFindLobbiesComplete);
		GI->OnJoinLobbyComplete.RemoveDynamic(this, &ThisClass::HandleJoinLobbyComplete);
	}

	Super::NativeDestruct();
}

void UBallGuysMainMenuWidget::HostLobby()
{
	const bool bIsPublic = PublicLobbyCheckBox ? PublicLobbyCheckBox->IsChecked() : true;
	HostLobbyWithPrivacy(bIsPublic);
}

void UBallGuysMainMenuWidget::HostLobbyWithPrivacy(bool bIsPublic)
{
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		SetMenuButtonsEnabled(false);
		SetStatusMessage(bIsPublic ? TEXT("Hosting public lobby...") : TEXT("Hosting private lobby..."));
		GI->CreateSession(bIsPublic);
	}
}

void UBallGuysMainMenuWidget::QuickJoinLobby()
{
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		SetMenuButtonsEnabled(false);
		SetStatusMessage(TEXT("Searching for an available public lobby..."));
		GI->QuickJoinSession();
	}
}

void UBallGuysMainMenuWidget::OpenServerBrowser()
{
	if (MenuWidgetSwitcher && ServerBrowserPanel)
	{
		MenuWidgetSwitcher->SetActiveWidget(ServerBrowserPanel);
	}

	RefreshServerList();
}

void UBallGuysMainMenuWidget::RefreshServerList()
{
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		if (RefreshServersButton)
		{
			RefreshServersButton->SetIsEnabled(false);
		}
		SetStatusMessage(TEXT("Refreshing public lobbies..."));
		GI->FindSessions(100);
	}
}

void UBallGuysMainMenuWidget::JoinLobbyByIndex(int32 SearchResultIndex)
{
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		SetMenuButtonsEnabled(false);
		SetStatusMessage(TEXT("Joining lobby..."));
		GI->JoinSessionByIndex(SearchResultIndex);
	}
}

void UBallGuysMainMenuWidget::OpenSettings()
{
	if (MenuWidgetSwitcher && SettingsWidget)
	{
		SettingsWidget->LoadCurrentSettingsIntoUI();
		MenuWidgetSwitcher->SetActiveWidget(SettingsWidget);
		return;
	}

	if (SettingsWidgetClass && !SpawnedSettingsWidget)
	{
		SpawnedSettingsWidget = CreateWidget<UBallGuysSettingsWidget>(GetOwningPlayer(), SettingsWidgetClass);
		if (SpawnedSettingsWidget)
		{
			SpawnedSettingsWidget->OnSettingsClosed.AddDynamic(this, &ThisClass::HandleSettingsClosed);
			SpawnedSettingsWidget->AddToViewport(10);
		}
	}
}

void UBallGuysMainMenuWidget::ShowMainMenuPanel()
{
	if (SpawnedSettingsWidget)
	{
		SpawnedSettingsWidget->OnSettingsClosed.RemoveDynamic(this, &ThisClass::HandleSettingsClosed);
		SpawnedSettingsWidget->RemoveFromParent();
		SpawnedSettingsWidget = nullptr;
	}

	if (MenuWidgetSwitcher)
	{
		if (MainMenuPanel)
		{
			MenuWidgetSwitcher->SetActiveWidget(MainMenuPanel);
		}
		else
		{
			MenuWidgetSwitcher->SetActiveWidgetIndex(0);
		}
	}
}

void UBallGuysMainMenuWidget::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UBallGuysMainMenuWidget::HandleCreateLobbyComplete(bool bWasSuccessful)
{
	SetMenuButtonsEnabled(true);
	if (!bWasSuccessful)
	{
		SetStatusMessage(TEXT("Failed to create lobby."));
	}
}

void UBallGuysMainMenuWidget::HandleFindLobbiesComplete(const TArray<FBallGuysLobbyInfo>& Lobbies, bool bWasSuccessful)
{
	if (RefreshServersButton)
	{
		RefreshServersButton->SetIsEnabled(true);
	}
	SetMenuButtonsEnabled(true);

	if (!bWasSuccessful)
	{
		SetStatusMessage(TEXT("Failed to search for lobbies."));
	}
	else
	{
		SetStatusMessage(FString::Printf(TEXT("Found %d public %s."), Lobbies.Num(), Lobbies.Num() == 1 ? TEXT("lobby") : TEXT("lobbies")));
	}

	// Notify the Widget Blueprint so it can build/populate Server Browser rows
	OnServerListUpdated(Lobbies, bWasSuccessful);
}

void UBallGuysMainMenuWidget::HandleJoinLobbyComplete(bool bWasSuccessful)
{
	SetMenuButtonsEnabled(true);
	if (!bWasSuccessful)
	{
		SetStatusMessage(TEXT("Could not join lobby."));
	}
}

void UBallGuysMainMenuWidget::HandleSettingsClosed()
{
	ShowMainMenuPanel();
}

void UBallGuysMainMenuWidget::SetStatusMessage(const FString& Message)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(Message));
	}
}

void UBallGuysMainMenuWidget::SetMenuButtonsEnabled(bool bEnabled)
{
	if (HostButton)
	{
		HostButton->SetIsEnabled(bEnabled);
	}
	if (QuickJoinButton)
	{
		QuickJoinButton->SetIsEnabled(bEnabled);
	}
	if (ServerBrowserButton)
	{
		ServerBrowserButton->SetIsEnabled(bEnabled);
	}
}
