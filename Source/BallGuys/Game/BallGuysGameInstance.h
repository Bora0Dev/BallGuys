// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BallGuysGameInstance.generated.h"

/**
 * Blueprint-friendly summary of a discovered public lobby for the Server Browser UI.
 */
USTRUCT(BlueprintType)
struct FBallGuysLobbyInfo
{
	GENERATED_BODY()

	/** Index into the latest FindSessions search results */
	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer|Lobby")
	int32 SearchResultIndex = -1;

	/** Owning player's display name */
	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer|Lobby")
	FString HostName;

	/** Current number of connected players (e.g. 2) */
	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer|Lobby")
	int32 CurrentPlayers = 0;

	/** Maximum players allowed in the lobby (hardcapped at 4) */
	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer|Lobby")
	int32 MaxPlayers = 4;

	/** Ping to the lobby in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer|Lobby")
	int32 PingMs = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateLobbyComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFindLobbiesComplete, const TArray<FBallGuysLobbyInfo>&, Lobbies, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinLobbyComplete, bool, bWasSuccessful);

/**
 * GameInstance handling player preferences (Input/Audio) and Online Subsystem (Steam/EOS) multiplayer lobbies.
 */
UCLASS()
class BALLGUYS_API UBallGuysGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UBallGuysGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	/** Hardcap for lobby player count */
	static constexpr int32 MAX_LOBBY_PLAYERS = 4;

	// --- Input Preferences ---

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Input")
	bool bInvertXPref = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Input")
	bool bInvertYPref = false;

	// --- Audio Preferences (0.0 - 1.0) ---

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Audio")
	float MusicVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Audio")
	float SFXVolume = 1.0f;

	// --- Multiplayer Lobby API ---

	/**
	 * Host a new 4-player lobby session.
	 * @param bIsPublic If true, creates a public lobby visible in the server browser.
	 *                  If false, creates a private invite-only lobby.
	 */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer|Lobby")
	void CreateSession(bool bIsPublic = true);

	/**
	 * Search for public lobbies advertised on the Online Subsystem.
	 * @param MaxSearchResults Maximum number of public sessions to query.
	 */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer|Lobby")
	void FindSessions(int32 MaxSearchResults = 100);

	/**
	 * Search for public lobbies and automatically join the first available one with an open slot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer|Lobby")
	void QuickJoinSession();

	/**
	 * Join a specific session search result directly from C++.
	 */
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	/**
	 * Join a found session by index from the most recent FindSessions query.
	 */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer|Lobby")
	void JoinSessionByIndex(int32 SessionIndex);

	/** Access the latest raw search results from C++ */
	const TArray<FOnlineSessionSearchResult>& GetSearchResults() const;

	/** Access the latest Blueprint-friendly lobby list */
	UFUNCTION(BlueprintPure, Category = "Multiplayer|Lobby")
	const TArray<FBallGuysLobbyInfo>& GetDiscoveredLobbies() const { return DiscoveredLobbies; }

	// --- UI Events ---

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer|Lobby")
	FOnCreateLobbyComplete OnCreateLobbyComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer|Lobby")
	FOnFindLobbiesComplete OnFindLobbiesComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer|Lobby")
	FOnJoinLobbyComplete OnJoinLobbyComplete;

protected:
	/** Online Session callbacks */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	UPROPERTY()
	TArray<FBallGuysLobbyInfo> DiscoveredLobbies;

	/** Delegates and handles */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnSessionUserInviteAcceptedDelegate SessionUserInviteAcceptedDelegate;
	FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

	/** Recreate session state if a previous session existed when hosting */
	bool bCreateSessionOnDestroy = false;
	bool bPendingPublicSession = true;

	/** Whether the current FindSessions query was triggered by QuickJoinSession */
	bool bQuickJoinPending = false;
};
