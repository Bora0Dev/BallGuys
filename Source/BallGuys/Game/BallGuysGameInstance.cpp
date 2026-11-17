// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BallGuysGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

UBallGuysGameInstance::UBallGuysGameInstance()
	: CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
	, DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
	, SessionUserInviteAcceptedDelegate(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionUserInviteAccepted))
{
}

void UBallGuysGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// Listen for Steam/EOS overlay invites (used to join Private invite-only lobbies)
			SessionUserInviteAcceptedDelegateHandle =
				SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegate);
		}
	}
}

void UBallGuysGameInstance::Shutdown()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
	}

	Super::Shutdown();
}

void UBallGuysGameInstance::CreateSession(bool bIsPublic)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// If a session already exists, destroy it first and recreate on completion
	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		bCreateSessionOnDestroy = true;
		bPendingPublicSession = bIsPublic;

		DestroySessionCompleteDelegateHandle =
			SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	CreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());

	const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	const bool bIsLAN = (Subsystem && Subsystem->GetSubsystemName() == "NULL");

	LastSessionSettings->bIsLANMatch = bIsLAN;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->BuildUniqueId = 1;

	if (bIsPublic)
	{
		// Public lobby: 4 public slots, visible in server browser
		LastSessionSettings->NumPublicConnections = MAX_LOBBY_PLAYERS;
		LastSessionSettings->NumPrivateConnections = 0;
		LastSessionSettings->bShouldAdvertise = true;
		LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	}
	else
	{
		// Private lobby: 4 private slots, hidden from server browser (invite-only)
		LastSessionSettings->NumPublicConnections = 0;
		LastSessionSettings->NumPrivateConnections = MAX_LOBBY_PLAYERS;
		LastSessionSettings->bShouldAdvertise = false;
		LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	}

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const bool bStarted = (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
		? SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings)
		: SessionInterface->CreateSession(0, NAME_GameSession, *LastSessionSettings);

	if (!bStarted)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
}

void UBallGuysGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(bPendingPublicSession);
	}
}

void UBallGuysGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (!bWasSuccessful)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("LobbyMap?listen"));
	}
}

void UBallGuysGameInstance::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FindSessionsCompleteDelegateHandle =
		SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;

	const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	LastSessionSearch->bIsLanQuery = (Subsystem && Subsystem->GetSubsystemName() == "NULL");
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const bool bStarted = (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
		? SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef())
		: SessionInterface->FindSessions(0, LastSessionSearch.ToSharedRef());

	if (!bStarted)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
}

void UBallGuysGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (!bWasSuccessful || !LastSessionSearch.IsValid())
	{
		return;
	}

	// LastSessionSearch->SearchResults now holds the discovered public lobbies
}

void UBallGuysGameInstance::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid() || !SessionResult.IsValid())
	{
		return;
	}

	JoinSessionCompleteDelegateHandle =
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	const bool bStarted = (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
		? SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult)
		: SessionInterface->JoinSession(0, NAME_GameSession, SessionResult);

	if (!bStarted)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
}

void UBallGuysGameInstance::JoinSessionByIndex(int32 SessionIndex)
{
	if (!LastSessionSearch.IsValid() || !LastSessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		return;
	}

	JoinSession(LastSessionSearch->SearchResults[SessionIndex]);
}

void UBallGuysGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	if (Result != EOnJoinSessionCompleteResult::Success || !SessionInterface.IsValid())
	{
		return;
	}

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		if (APlayerController* PlayerController = GetFirstLocalPlayerController())
		{
			PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UBallGuysGameInstance::OnSessionUserInviteAccepted(
	const bool bWasSuccessful,
	const int32 ControllerId,
	FUniqueNetIdPtr UserId,
	const FOnlineSessionSearchResult& InviteResult)
{
	if (bWasSuccessful && InviteResult.IsValid())
	{
		JoinSession(InviteResult);
	}
}

const TArray<FOnlineSessionSearchResult>& UBallGuysGameInstance::GetSearchResults() const
{
	static const TArray<FOnlineSessionSearchResult> EmptyResults;
	return LastSessionSearch.IsValid() ? LastSessionSearch->SearchResults : EmptyResults;
}
