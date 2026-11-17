#include "World/BallGuysKillVolume.h"
#include "Components/BoxComponent.h"
#include "Game/BallGuysGameMode.h"
#include "Game/BallGuysLobbyGameMode.h"
#include "Player/BallPawn.h"
#include "Kismet/GameplayStatics.h"

ABallGuysKillVolume::ABallGuysKillVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	KillBox = CreateDefaultSubobject<UBoxComponent>(TEXT("KillBox"));
	RootComponent = KillBox;

	KillBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	KillBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void ABallGuysKillVolume::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		KillBox->OnComponentBeginOverlap.AddDynamic(this, &ABallGuysKillVolume::OnOverlapBegin);
	}
}

void ABallGuysKillVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this))
	{
		UE_LOG(LogTemp, Warning, TEXT("KillVolume Overlap with: %s"), *OtherActor->GetName());

		ABallPawn* PlayerPawn = Cast<ABallPawn>(OtherActor);
		if (PlayerPawn)
		{
			AGameModeBase* AuthGM = UGameplayStatics::GetGameMode(GetWorld());
			if (ABallGuysGameMode* GameMode = Cast<ABallGuysGameMode>(AuthGM))
			{
				UE_LOG(LogTemp, Warning, TEXT("KillVolume killing pawn: %s"), *PlayerPawn->GetName());
				GameMode->PlayerDied(PlayerPawn->GetController());
			}
			else if (ABallGuysLobbyGameMode* LobbyGameMode = Cast<ABallGuysLobbyGameMode>(AuthGM))
			{
				LobbyGameMode->RespawnLobbyPlayer(PlayerPawn->GetController());
			}
		}
	}
}
