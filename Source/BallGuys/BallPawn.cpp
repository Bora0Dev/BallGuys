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

    // Create a spring arm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(MeshComp);
    SpringArm->TargetArmLength = 600.f;
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

    TorqueStrength        = 40.f;  // "How hard we roll" changed to 25 from 5,000,000
    JumpImpulse           = 1000.f;    // "How hard we jump" changed to 600 from 250,000
    GroundCheckDistance   = -45.f;        // Distance below the ball to look for ground DO NOT GO PAST -49.f
    KnockImpulseStrength  = 200000.f;    // Impulse to shove other balls

    CachedForwardInput = 0.f;
    CachedRightInput   = 0.f;
}

void ABallPawn::BeginPlay()
{
    Super::BeginPlay();
    /*
    // Add Enhanced Input mapping context on the owning client
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
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
    */
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

    /* if (HasAuthority())
    {
        const bool bNowGrounded = IsGrounded();

        // We just "landed" this frame
        if (bNowGrounded && !bWasGroundedLastFrame)
        {
            bHasJumpedSinceLastGround = false;  // allow new jump
        }
        bWasGroundedLastFrame = bNowGrounded;
    } */ // part of the how to break a jump with booleans lesson
}

void ABallPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind axes (legacy input system)
    // Make sure you have these names set in Project Settings → Input.
    /*
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABallPawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"),   this, &ABallPawn::MoveRight);
    
    // Camera (uses built-in APawn helpers to rotate the controller)
    PlayerInputComponent->BindAxis(TEXT("Turn"),   this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);

    // Bind jump action
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABallPawn::JumpPressed);
    */

    
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
        /* // OLD 2D axis camera input
        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABallPawn::HandleLook);
        }
        */
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

    if (IsLocallyControlled())
    {
        Server_AddMovementInput(ForwardValue, RightValue);
    }
}

/*
void ABallPawn::HandleLook(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();

    const float YawInput   = LookAxis.X; // Mouse/gamepad X → yaw
    const float PitchInput = LookAxis.Y; // Mouse/gamepad Y → pitch (invert via modifiers if desired)

    AddControllerYawInput(YawInput);
    AddControllerPitchInput(PitchInput);
}
*/

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

void ABallPawn::HandleJump(const FInputActionValue& Value)
{
    // Digital action; we only care that it fired
    if (IsLocallyControlled())
    {
        Server_Jump();
    }
}
// Old code using deprecated Input System--vvvv
/* void ABallPawn::MoveForward(float Value)
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
*/
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
    FRotator ControlRot = FRotator::ZeroRotator;//old code GetActorRotation();
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
    FVector TorqueAxis = FVector::CrossProduct(/* MoveDir,*/ Up, MoveDir);

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
    // Must be GROUNDED and NOT have jumped since last frame
   if (!IsGrounded() /* || bHasJumpedSinceLastGround */)
    {
        return;
    }

    const FVector Impulse = FVector::UpVector * JumpImpulse;

    // VelChange=true makes the impulse independent of mass (feels more “gamey”)
    MeshComp->AddImpulse(Impulse, NAME_None, true);

    // Lock out further jumps until we leave ground and jump again
   /* bHasJumpedSinceLastGround = true; */
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



