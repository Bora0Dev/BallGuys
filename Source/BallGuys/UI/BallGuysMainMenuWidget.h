// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/BallGuysGameInstance.h"
#include "BallGuysMainMenuWidget.generated.h"

class UButton;
class UCheckBox;
class UWidgetSwitcher;
class UWidget;
class UScrollBox;
class UTextBlock;
class UBallGuysSettingsWidget;

/**
 * Main Menu widget displayed at the start of the game.
 * Supports:
 *  - Hosting (Host button + Public/Private checkbox toggle)
 *  - Joining (Quick Join button + Server Browser list)
 *  - Settings (switches to or spawns UBallGuysSettingsWidget)
 *  - Quit Game
 * All UMG control bindings use BindWidgetOptional so Widget Blueprints compile even while layout is in progress.
 */
UCLASS()
class BALLGUYS_API UBallGuysMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Main Menu Actions (BlueprintCallable & Auto-Bound) ---

	/** Hosts a 4-player lobby using PublicLobbyCheckBox state (or Public if no checkbox is bound) */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Host")
	void HostLobby();

	/** Hosts a 4-player lobby with explicit Public/Private setting */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Host")
	void HostLobbyWithPrivacy(bool bIsPublic);

	/** Searches for public lobbies and automatically joins the first one with an open slot */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Join")
	void QuickJoinLobby();

	/** Opens the Server Browser panel and triggers a public lobby search */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Join")
	void OpenServerBrowser();

	/** Refreshes the public lobby list in the Server Browser */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Join")
	void RefreshServerList();

	/** Joins a specific public lobby from the Server Browser list by its SearchResultIndex */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Join")
	void JoinLobbyByIndex(int32 SearchResultIndex);

	/** Opens the Settings menu (switches to SettingsPanel/SettingsWidget or spawns SettingsWidgetClass) */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Settings")
	void OpenSettings();

	/** Returns to the primary Main Menu panel from Server Browser or Settings */
	UFUNCTION(BlueprintCallable, Category = "MainMenu|Navigation")
	void ShowMainMenuPanel();

	/** Quits the game */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void QuitGame();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Called in Blueprint when a lobby search finishes so custom server browser row widgets can be populated */
	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu|Join")
	void OnServerListUpdated(const TArray<FBallGuysLobbyInfo>& Lobbies, bool bWasSuccessful);

	/** Optional Settings Widget class to spawn if SettingsWidget is not embedded directly in the layout */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MainMenu|Settings")
	TSubclassOf<UBallGuysSettingsWidget> SettingsWidgetClass;

	// --- Optional UMG Bindings ---

	/** Optional switcher between MainMenuPanel (index 0), ServerBrowserPanel (index 1), and SettingsWidget (index 2) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UWidgetSwitcher> MenuWidgetSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UWidget> MainMenuPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UWidget> ServerBrowserPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UBallGuysSettingsWidget> SettingsWidget;

	/** Checked = Public lobby (visible in server browser), Unchecked = Private lobby (invite-only) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UCheckBox> PublicLobbyCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UButton> HostButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UButton> QuickJoinButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UButton> ServerBrowserButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UButton> RefreshServersButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UButton> ServerBrowserBackButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UScrollBox> ServerListScrollBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "MainMenu|UI")
	TObjectPtr<UTextBlock> StatusTextBlock;

private:
	UPROPERTY()
	TObjectPtr<UBallGuysSettingsWidget> SpawnedSettingsWidget;

	UFUNCTION()
	void HandleCreateLobbyComplete(bool bWasSuccessful);

	UFUNCTION()
	void HandleFindLobbiesComplete(const TArray<FBallGuysLobbyInfo>& Lobbies, bool bWasSuccessful);

	UFUNCTION()
	void HandleJoinLobbyComplete(bool bWasSuccessful);

	UFUNCTION()
	void HandleSettingsClosed();

	void SetStatusMessage(const FString& Message);
	void SetMenuButtonsEnabled(bool bEnabled);
};
