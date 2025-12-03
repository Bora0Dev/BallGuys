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
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"

ABallPawn::ABallPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // ----------------- Components -----------------

    // Create the mesh component and make it the root
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
    RootComponent = MeshComp;

    // Tell the mesh to behave like a physics ball
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetLinearDamping(0.6f); //slows how far it coasts
    MeshComp->SetAngularDamping(0.8f); //slow spin
    MeshComp->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);

    // Make sure hit events are generated (for NotifyHit)
    MeshComp->SetNotifyRigidBodyCollision(true);
    MeshComp->BodyInstance.bNotifyRigidBodyCollision = true;
    MeshComp->SetGenerateOverlapEvents(true);

    // Create a spring arm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(MeshComp);
    SpringArm->TargetArmLength = 600.f;
    SpringArm->bUsePawnControlRotation = true; // Camera rotates with controller yaw/pitch
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 15.f;

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

    TorqueStrength        = 40.f;  // "How hard we roll" 
    JumpImpulse           = 1000.f;    // "How hard we jump"
    GroundCheckDistance   = -45.f;        // Distance below the ball to look for ground DO NOT GO PAST -49.f
    KnockImpulseStrength  = 200000.f;    // Impulse to shove other balls

    CachedForwardInput = 0.f;
    CachedRightInput   = 0.f;

    // Saving base values so boost can scale them
    BaseTorqueStrength = TorqueStrength;
    BaseKnockImpulseStrength = KnockImpulseStrength;
}

void ABallPawn::BeginPlay()
{
    Super::BeginPlay();
    
    // Caching base values once so boosts have something to multiply
    BaseTorqueStrength = TorqueStrength;
    BaseKnockImpulseStrength = KnockImpulseStrength;
    
   
}

void ABallPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
 
    // Tick, tick....BOOST!
    // Only the server updates the boost timers / state
    if (HasAuthority())
    {
        // Handle active boost
        if (bIsBoosting)
        {
            BoostTimeRemaining -= DeltaSeconds;
            if (BoostTimeRemaining <= 0.f)
            {
                bIsBoosting        = false;
                BoostTimeRemaining = 0.f;

                // Restore normal strengths
                TorqueStrength       = BaseTorqueStrength;
                KnockImpulseStrength = BaseKnockImpulseStrength;
            }
        }

        // Handle cooldown ticking down
        if (CooldownTimeRemaining > 0.f)
        {
            CooldownTimeRemaining -= DeltaSeconds;
            if (CooldownTimeRemaining < 0.f)
            {
                CooldownTimeRemaining = 0.f;
            }
        }
    }
}
//---------------Boost Replication------------------------------
void ABallPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)  const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABallPawn, bIsBoosting);
    DOREPLIFETIME(ABallPawn, BoostTimeRemaining);
    DOREPLIFETIME(ABallPawn, CooldownTimeRemaining);
}

void ABallPawn::OnRep_IsBoosting()
{
    if (bIsBoosting)
    {
        TorqueStrength = BaseTorqueStrength * BoostMultiplier;
        KnockImpulseStrength = BaseKnockImpulseStrength * BoostMultiplier;
    }
    else
    {
        TorqueStrength = BaseTorqueStrength;
        KnockImpulseStrength = BaseKnockImpulseStrength;
    }
}
//---------Setting up Player Input Component--------------------
void ABallPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Use EnhancedInputComponent instead of plain UInputComponent
    // 1) Add the mapping context for the LOCAL player that owns this pawn
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->IsLocalController())
        {
            if (ULocalPlayer* LP = PC->GetLocalPlayer())
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsys =
                    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
                {
                    if (DefaultMappingContext)
                    {
                        Subsys->AddMappingContext(DefaultMappingContext, 0);
                    }
                }
            }
        }
    }
    // Bind actions using EnhancedInputComponent
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABallPawn::HandleMove);
        }
        
        if (TurnAction)
        {
            EnhancedInput->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ABallPawn::TurnCamera);
        }

        if (LookUpAction)
        {
            EnhancedInput->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &ABallPawn::LookUpCamera);
        }
        
        if (JumpAction)
        {
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ABallPawn::HandleJump);
        }

        //---------BOOST------------
        if (BoostAction)
        {
            EnhancedInput->BindAction(BoostAction, ETriggerEvent::Started, this, &ABallPawn::HandleBoost);
        }
        // Testing HandleBoost with JumpAction binding
        /* if (JumpAction)
        {
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ABallPawn::/* HandleJump  HandleBoost);
        } */
        // X and Y invert axes toggles
        if (InvertXAction)
        {
            EnhancedInput->BindAction(InvertXAction, ETriggerEvent::Started, this, &ABallPawn::HandleInvertX);
        }

        if (InvertYAction)
        {
            EnhancedInput->BindAction(InvertYAction, ETriggerEvent::Started, this, &ABallPawn::HandleInvertY);
        }
    }
}

// ----------------- Client-side input handlers -----------------

void ABallPawn::HandleMove(const FInputActionValue& Value)
{
    // Expecting a 2D axis (X = Right, Y = Forward)
    const FVector2D MoveAxis = Value.Get<FVector2D>();

    const float ForwardValue = MoveAxis.Y;
    const float RightValue   = MoveAxis.X;

    // Cache if you still want, but main thing: send to server
    CachedForwardInput = ForwardValue;
    CachedRightInput   = RightValue;

    // Client-side prediction: Apply force immediately
    ApplyMovementInput(ForwardValue, RightValue);

    if (IsLocallyControlled())
    {
        Server_AddMovementInput(ForwardValue, RightValue);
    }
}

// Turn camera
void ABallPawn::TurnCamera(const FInputActionValue& Value)
{
    float Axis = Value.Get<float>();

    if (bInvertTurnAxis)
    {
        Axis *= -1.f;
    }
    
    AddControllerYawInput(Axis);
}
// LookUp camera
void ABallPawn::LookUpCamera(const FInputActionValue& Value)
{
    float Axis = Value.Get<float>();

    if (bInvertLookUpAxis)
    {
        Axis *= -1.f;
    }
    
    AddControllerPitchInput(Axis);
}
// Invert X-Axis Turn camera
void ABallPawn::HandleInvertX(const FInputActionValue& Value)
{
    // Digital action; only toggle on press NOT release
    const bool bPressed = Value.Get<bool>();
    if (!bPressed || !IsLocallyControlled())
    {
        return;
    }

    bInvertTurnAxis = !bInvertTurnAxis;
    // (Optional; notify UI here)
}
// Invert Y-Axis LookUp camera
void ABallPawn::HandleInvertY(const FInputActionValue& Value)
{
    const bool bPressed = Value.Get<bool>();
    if (!bPressed || !IsLocallyControlled())
    {
        return;
    }

    bInvertLookUpAxis = !bInvertLookUpAxis;
    // (Optional: notify UI here)
}
//------------- Jump input -----------------------
void ABallPawn::HandleJump(const FInputActionValue& Value)
{
    // Digital action; we only care that it fired
    // Client-side prediction
    ApplyJump();

    if (IsLocallyControlled())
    {
        Server_Jump();
    }
}

//--------------Boost Client-side input handler------------------
void  ABallPawn::HandleBoost(const FInputActionValue& Value)
{
    const bool bPressed = Value.Get<bool>();
    // Only the locally controlled pawn should send the RPC
    if (!IsLocallyControlled())
    {
        return;
    }
    
    if (!bPressed)
    {
        return;  // We only care about the press not the release
    }

    Server_TryBoost();
}

// ----------------- Shared Movement Logic -----------------

void ABallPawn::ApplyMovementInput(float ForwardValue, float RightValue)
{
    if (!MeshComp || !MeshComp->IsSimulatingPhysics())
    {
        return;
    }

    // If the player isn't actually pressing anything, we can early-out
    if (FMath::IsNearlyZero(ForwardValue) && FMath::IsNearlyZero(RightValue))
    {
        return;
    }

    // Use the controller's yaw as our input space
    FRotator ControlRot = FRotator::ZeroRotator;
    if (AController* PC = GetController())
    {
        ControlRot = PC->GetControlRotation();
    }

    // We only care about yaw (XZ plane)
    FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

    // Forward/right in world space based on camera/controller yaw
    const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
    const FVector RightDir   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    // Desired move direction in the world
    FVector MoveDir = ForwardDir * ForwardValue + RightDir * RightValue;

    if (MoveDir.IsNearlyZero())
    {
        return;
    }

    MoveDir.Normalize();

    // TorqueAxis = MoveDir x Up
    const FVector Up = FVector::UpVector;
    FVector TorqueAxis = FVector::CrossProduct(Up, MoveDir);

    if (TorqueAxis.IsNearlyZero())
    {
        return;
    }

    TorqueAxis.Normalize();

    const FVector Torque = TorqueAxis * TorqueStrength;

    // Add torque in radians (physics-space)
    MeshComp->AddTorqueInRadians(Torque, NAME_None, true);
}

void ABallPawn::ApplyJump()
{
    if (!MeshComp || !MeshComp->IsSimulatingPhysics())
    {
        return;
    }

    // Simple grounded check
    if (!IsGrounded())
    {
        return;
    }

    const FVector Impulse = FVector::UpVector * JumpImpulse;
    MeshComp->AddImpulse(Impulse, NAME_None, true);
}

//-----------Sever-side Boost logic----------------
void ABallPawn::Server_TryBoost_Implementation()
{
    // DEBUG
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1, 2.f, FColor::Yellow,
            FString::Printf(TEXT("Server_TryBoost: Cooldown set to %.2f"), BoostCooldown)
            );
    }
    
    // Only the server controls the boost state
 if (bIsBoosting)
 {
     return;  // already boosting
 }
    if (CooldownTimeRemaining> 0.f)
    {
        return; // still on cooldown
    }

    // Start Boost
    bIsBoosting           = true;
    BoostTimeRemaining    = BoostDuration;
    CooldownTimeRemaining = BoostCooldown;

    // Apply boosted strengths
    OnRep_IsBoosting();
    
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

    //Centre of the ball and its radius
    const FVector Center = MeshComp->GetComponentLocation();
    const float Radius   = MeshComp->Bounds.SphereRadius;
    
    const FVector Start = Center;
    const FVector End   = Center - FVector(0.f, 0.f, Radius + GroundCheckDistance);

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BallGroundCheck), false, this);

    // Trace downwards – you might want ECC_WorldStatic / ECC_WorldDynamic
    const bool bHit = World->SweepSingleByChannel(
        Hit,
        Start,
        End,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius * 0.9f),
        Params
    );

    // Debug line (optional – comment out for production)
    
   /* const FColor Color = bHit ? FColor::Green : FColor::Red;
    DrawDebugLine(World, Start, End, Color, false, 0.f, 0, 1.f);
    DrawDebugSphere(World, End, Radius * 0.9f, 16, Color, false, 0.f); */
     
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

// ----------------- Server RPC implementations -----------------

void ABallPawn::Server_AddMovementInput_Implementation(float ForwardValue, float RightValue)
{
    // Server also applies the force (authoritative simulation)
    // Avoid double application on Listen Server host
    if (!IsLocallyControlled()) 
    {
        ApplyMovementInput(ForwardValue, RightValue);
    }
}

void ABallPawn::Server_Jump_Implementation()
{
    // Only jump on server (authoritative)
    // Avoid double application on Listen Server host
    if (!IsLocallyControlled())
    {
        ApplyJump();
    }
}



