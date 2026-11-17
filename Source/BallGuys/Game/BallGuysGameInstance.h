// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BallGuysGameInstance.generated.h"

/**
 * GameInstance handling input preferences and Online Subsystem (Steam/EOS) multiplayer lobbies.
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	bool bInvertXPref = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	bool bInvertYPref = false;

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
	 * Join a specific session search result directly from C++.
	 */
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	/**
	 * Join a found session by index from the most recent FindSessions query (Blueprint helper).
	 */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer|Lobby")
	void JoinSessionByIndex(int32 SessionIndex);

	/** Access the latest search results from C++ */
	const TArray<FOnlineSessionSearchResult>& GetSearchResults() const;

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
};
