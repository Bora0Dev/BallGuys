#include "BallGuysKillVolume.h"
#include "Components/BoxComponent.h"
#include "BallGuysGameMode.h"
#include "BallPawn.h"
#include "Kismet/GameplayStatics.h"

ABallGuysKillVolume::ABallGuysKillVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	KillBox = CreateDefaultSubobject<UBoxComponent>(TEXT("KillBox"));
	RootComponent = KillBox;

	KillBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	KillBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	KillBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
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
		ABallPawn* PlayerPawn = Cast<ABallPawn>(OtherActor);
		if (PlayerPawn)
		{
			ABallGuysGameMode* GameMode = Cast<ABallGuysGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (GameMode)
			{
				GameMode->PlayerDied(PlayerPawn->GetController());
			}
		}
	}
}
