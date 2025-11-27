// Fill out your copyright notice in the Description page of Project Settings.

// BallPawn.cpp

#include "BallPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ABallPawn::ABallPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // ----------------- Components -----------------

    // Create the mesh component and make it the root
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    RootComponent = MeshComp;

    // Tell the mesh to behave like a physics ball
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);

    // Make sure hit events are generated (for NotifyHit)
    MeshComp->SetNotifyRigidBodyCollision(true);
    MeshComp->BodyInstance.bNotifyRigidBodyCollision = true;

    // Create a spring arm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(MeshComp);
    SpringArm->TargetArmLength = 400.f;
    SpringArm->bUsePawnControlRotation = true; // Camera rotates with controller yaw/pitch

    // Create a camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false; // We already rotate the spring arm

    // ----------------- Replication -----------------

    // This pawn should exist on server and all clients
    SetReplicates(true);
    // Let the engine replicate physics movement for us
    SetReplicateMovement(true);
    // Optional: tweak net update rate
    SetNetUpdateFrequency(100.f);
    SetMinNetUpdateFrequency(30.f);

    // ----------------- Movement tuning defaults -----------------

    TorqueStrength        = 50000000.f;  // "How hard we roll"
    JumpImpulse           = 200000.f;    // "How hard we jump"
    GroundCheckDistance   = 60.f;        // Distance below the ball to look for ground
    KnockImpulseStrength  = 200000.f;    // Impulse to shove other balls

    CachedForwardInput = 0.f;
    CachedRightInput   = 0.f;
}

void ABallPawn::BeginPlay()
{
    Super::BeginPlay();

    // At runtime, you'll usually have a Blueprint subclass of ABallPawn
    // where you assign:
    //  - the Static Mesh asset
    //  - maybe Material, etc.
    // This C++ class just defines behaviour + basic setup.
}

void ABallPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // We don't need to do anything each tick here yet.
    // All movement is driven by input → RPC → physics on server.
}

void ABallPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind axes (legacy input system)
    // Make sure you have these names set in Project Settings → Input.
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABallPawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"),   this, &ABallPawn::MoveRight);
    
    // Camera (uses built-in APawn helpers to rotate the controller)
    PlayerInputComponent->BindAxis(TEXT("Turn"),   this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);

    // Bind jump action
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABallPawn::JumpPressed);
}

// ----------------- Client-side input handlers -----------------

void ABallPawn::MoveForward(float Value)
{
    CachedForwardInput = Value;

    // Only the owning client should send input to the server
    if (IsLocallyControlled())
    {
        // Send both axes so the server has the full input vector
        Server_AddMovementInput(CachedForwardInput, CachedRightInput);
    }
}

void ABallPawn::MoveRight(float Value)
{
    CachedRightInput = Value;

    if (IsLocallyControlled())
    {
        Server_AddMovementInput(CachedForwardInput, CachedRightInput);
    }
}

void ABallPawn::JumpPressed()
{
    if (IsLocallyControlled())
    {
        Server_Jump();
    }
}

// ----------------- Server RPC implementations -----------------

void ABallPawn::Server_AddMovementInput_Implementation(float ForwardValue, float RightValue)
{
    // This runs on the SERVER.
    // We apply torque to the physics body, and the resulting movement
    // replicates to all clients via bReplicateMovement.

    if (!MeshComp || !MeshComp->IsSimulatingPhysics())
    {
        return;
    }

    // If the player isn't actually pressing anything, we can early-out
    if (FMath::IsNearlyZero(ForwardValue) && FMath::IsNearlyZero(RightValue))
    {
        return;
    }

    // Use the controller's yaw as our input space, like a typical third-person game
    FRotator ControlRot = GetActorRotation();
    if (AController* PC = Controller)
    {
        ControlRot = PC->GetControlRotation();
    }

    // We only care about yaw (XZ plane)
    FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

    // Forward/right in world space based on camera/controller yaw
    const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
    const FVector RightDir   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    // Desired move direction in the world (where we want the ball to roll)
    FVector MoveDir = ForwardDir * ForwardValue + RightDir * RightValue;

    if (MoveDir.IsNearlyZero())
    {
        return;
    }

    MoveDir.Normalize();

    // To make a sphere roll in MoveDir, we want a torque axis perpendicular
    // to MoveDir and Up. Using the right-hand rule:
    //   TorqueAxis = MoveDir x Up
    const FVector Up = FVector::UpVector;
    FVector TorqueAxis = FVector::CrossProduct(MoveDir, Up);

    if (TorqueAxis.IsNearlyZero())
    {
        return;
    }

    TorqueAxis.Normalize();

    const FVector Torque = TorqueAxis * TorqueStrength;

    // Add torque in radians (physics-space)
    MeshComp->AddTorqueInRadians(Torque, NAME_None, true);
}

void ABallPawn::Server_Jump_Implementation()
{
    // Only jump on server (authoritative)
    if (!MeshComp || !MeshComp->IsSimulatingPhysics())
    {
        return;
    }

    // Simple grounded check so we can't spam jump in midair
    if (!IsGrounded())
    {
        return;
    }

    const FVector Impulse = FVector::UpVector * JumpImpulse;

    // VelChange=true makes the impulse independent of mass (feels more “gamey”)
    MeshComp->AddImpulse(Impulse, NAME_None, true);
}

// ----------------- Ground check -----------------

bool ABallPawn::IsGrounded() const
{
    if (!MeshComp)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector Start = MeshComp->GetComponentLocation();
    const FVector End   = Start - FVector(0.f, 0.f, GroundCheckDistance);

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BallGroundCheck), false, this);

    // Trace downwards – you might want ECC_WorldStatic / ECC_WorldDynamic
    const bool bHit = World->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    // Debug line (optional – comment out for production)
    // DrawDebugLine(World, Start, End, bHit ? FColor::Green : FColor::Red, false, 0.f, 0, 1.f);

    return bHit;
}

// ----------------- Collision: pushing other balls -----------------

void ABallPawn::NotifyHit(
    UPrimitiveComponent* MyComp,
    AActor* Other,
    UPrimitiveComponent* OtherComp,
    bool bSelfMoved,
    FVector HitLocation,
    FVector HitNormal,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    // Only the SERVER should apply shoving impulses, because it's authoritative.
    if (!HasAuthority())
    {
        return;
    }

    if (!Other || !OtherComp)
    {
        return;
    }

    // Only shove other ABallPawn instances (other player balls)
    ABallPawn* OtherBall = Cast<ABallPawn>(Other);
    if (!OtherBall)
    {
        return;
    }

    if (!OtherComp->IsSimulatingPhysics())
    {
        return;
    }

    // Direction from us → them, so they get pushed away from us
    FVector Dir = Other->GetActorLocation() - GetActorLocation();
    if (Dir.IsNearlyZero())
    {
        return;
    }

    Dir.Normalize();

    const FVector Impulse = Dir * KnockImpulseStrength;

    // Apply impulse at the hit location for a more physical feel
    OtherComp->AddImpulseAtLocation(Impulse, HitLocation);
}



