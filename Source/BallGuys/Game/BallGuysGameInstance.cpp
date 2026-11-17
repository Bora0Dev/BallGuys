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
		OnCreateLobbyComplete.Broadcast(false);
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
		OnCreateLobbyComplete.Broadcast(false);
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

	OnCreateLobbyComplete.Broadcast(bWasSuccessful);

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
		DiscoveredLobbies.Empty();
		OnFindLobbiesComplete.Broadcast(DiscoveredLobbies, false);
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
		bQuickJoinPending = false;
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		DiscoveredLobbies.Empty();
		OnFindLobbiesComplete.Broadcast(DiscoveredLobbies, false);
	}
}

void UBallGuysGameInstance::QuickJoinSession()
{
	bQuickJoinPending = true;
	FindSessions(100);
}

void UBallGuysGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	DiscoveredLobbies.Empty();

	if (bWasSuccessful && LastSessionSearch.IsValid())
	{
		for (int32 Index = 0; Index < LastSessionSearch->SearchResults.Num(); ++Index)
		{
			const FOnlineSessionSearchResult& Result = LastSessionSearch->SearchResults[Index];
			if (!Result.IsValid())
			{
				continue;
			}

			const int32 MaxSlots = Result.Session.SessionSettings.NumPublicConnections;
			const int32 OpenSlots = Result.Session.NumOpenPublicConnections;
			const int32 FilledSlots = FMath::Clamp(MaxSlots - OpenSlots, 1, MAX_LOBBY_PLAYERS);

			FBallGuysLobbyInfo Info;
			Info.SearchResultIndex = Index;
			Info.HostName = Result.Session.OwningUserName.IsEmpty() ? TEXT("Lobby") : Result.Session.OwningUserName;
			Info.MaxPlayers = MaxSlots > 0 ? MaxSlots : MAX_LOBBY_PLAYERS;
			Info.CurrentPlayers = FilledSlots;
			Info.PingMs = Result.PingInMs;

			DiscoveredLobbies.Add(Info);
		}
	}

	OnFindLobbiesComplete.Broadcast(DiscoveredLobbies, bWasSuccessful);

	if (bQuickJoinPending)
	{
		bQuickJoinPending = false;

		// Join the first available public lobby with an open slot
		for (const FBallGuysLobbyInfo& Lobby : DiscoveredLobbies)
		{
			if (Lobby.CurrentPlayers < Lobby.MaxPlayers)
			{
				JoinSessionByIndex(Lobby.SearchResultIndex);
				return;
			}
		}

		OnJoinLobbyComplete.Broadcast(false);
	}
}

void UBallGuysGameInstance::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid() || !SessionResult.IsValid())
	{
		OnJoinLobbyComplete.Broadcast(false);
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
		OnJoinLobbyComplete.Broadcast(false);
	}
}

void UBallGuysGameInstance::JoinSessionByIndex(int32 SessionIndex)
{
	if (!LastSessionSearch.IsValid() || !LastSessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		OnJoinLobbyComplete.Broadcast(false);
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

	const bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);
	OnJoinLobbyComplete.Broadcast(bSuccess);

	if (!bSuccess || !SessionInterface.IsValid())
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
