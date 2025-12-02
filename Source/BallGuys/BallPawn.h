// Fill out your copyright notice in the Description page of Project Settings.
// BallPawn.h
// Simple rolling ball pawn for a multiplayer game.
// - Physics-simulated sphere
// - Server-authoritative movement (RPCs)
// - Jump
// - Push other balls on collision

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Components/InputComponent.h"
#include "Net/UnrealNetwork.h"
#include "BallPawn.generated.h" // "...generated.h ALWAYS LAST in #include(s)


// Forward declarations
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class BALLGUYS_API ABallPawn : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ABallPawn();

    // Called every frame
    virtual void Tick(float DeltaSeconds) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    /** Root + visual + physics body.
     *  Intentionally a StaticMeshComponent so it can simulate physics and collide.
     *  You will assign the actual mesh asset in a Blueprint subclass.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;

    /** Spring arm to hold the camera at a distance behind the ball. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArm;

    /** Third-person camera. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* Camera;

    //---- Enhanced Input-------

    /** Mapping context for this pawn (move, look, jump). Assign IMC_BallPawn in BP. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputMappingContext* DefaultMappingContext;

    /** Move action (Axis2D). Assign IA_Move in BP. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* MoveAction;

    /** Look action (Axis2D). Assign IA_Look in BP. */
   /* UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* LookAction; */

    /** Turn action (Axis1D). Assign IA_Turn in BP. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* TurnAction;

    /** LookUp action (Axis1D). Assign IA_LookUp in BP. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* LookUpAction;
    
    /** Jump-action (Digital). Assign IA_Jump in BP. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* JumpAction;

    // Invert X+Y AXIS toggle actions (digital)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
    UInputAction* InvertXAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* InvertYAction;

    // Per-player X+Y AXIS settings (local only, no replication needed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
    bool bInvertTurnAxis = false;  // X (left/right)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
    bool bInvertLookUpAxis = false; // Y (up/down)

    //-------------Boost input--------------
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* BoostAction;

    //-------------Boost Settings------------
    /** How much to multiply torque/knock strength while boosting. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boost")
    float BoostMultiplier = 2.f;

    /** How long the boost lasts (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boost")
    float BoostDuration = 1.f;

    /** How long before we can boost again (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boost")
    float BoostCooldown = 5.f;

    /** True while the boost is active. Replicated for UI / FX. */
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Boost")
    bool bIsBoosting = false;

    /** Time left on the current boost (seconds). */
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Boost")
    float BoostTimeRemaining = 0.f;

    /** Time left on cooldown before we can boost again (seconds). */
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Boost")
    float CooldownTimeRemaining = 0.f;

    /** Base (non-boosted) values so we can restore after boost. */
    float BaseTorqueStrength = 0.f;
    float BaseKnockImpulseStrength = 0.f;

    
    //---- Input handlers (client-side)------
    void HandleMove(const FInputActionValue& Value);
    /* void HandleLook(const FInputActionValue& Value); *///Old look Input Axis2D
    void HandleJump(const FInputActionValue& Value);
    void TurnCamera(const FInputActionValue& Value);//X-Axis
    void LookUpCamera(const FInputActionValue& Value);//Y-Axis

    //----invert X+Y AXIS toggle handlers-----------
    void HandleInvertX(const FInputActionValue& Value);
    void HandleInvertY(const FInputActionValue& Value);

    //------Boost input handler---------------------
    void HandleBoost(const FInputActionValue& Value);
    
    // ----------------- Movement tuning -----------------

    /** How strong the torque is when trying to roll the ball. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Movement")
    float TorqueStrength;

    /** Upward impulse for jump. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Movement")
    float JumpImpulse;

    /** How far down we trace to check if the ball is on the ground. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Movement")
    float GroundCheckDistance;

    /** Impulse strength applied to other balls when we hit them. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Movement")
    float KnockImpulseStrength;

    // ----------------- Client-side input cache -----------------

    /** Last frame's forward input (from -1 to 1). Only meaningful on the owning client. */
    float CachedForwardInput;

    /** Last frame's right input (from -1 to 1). Only meaningful on the owning client. */
    float CachedRightInput;

    // ----------------- OLD Input handlers (client side) -----------------

    /** Called by input system on the owning client. */
    //void MoveForward(float Value);

    /** Called by input system on the owning client. */
    //void MoveRight(float Value);

    /** Called by input system when Jump is pressed on the owning client. */
    //void JumpPressed();

    // ----------------- Server RPCs -----------------

    /** Server-side movement handler.
     *  Called from the owning client with the latest axis values.
     */
    UFUNCTION(Server, Reliable)
    void Server_AddMovementInput(float ForwardValue, float RightValue);

    /** Server-side jump handler.
     *  Called from the owning client when jump input is pressed.
     */
    UFUNCTION(Server, Reliable)
    void Server_Jump();

    /** Server-side Boost Handler to start boost (authority decides) */
    UFUNCTION(Server, Reliable)
    void Server_TryBoost();

    // ----------------- Helpers -----------------

    /** Simple ground check via line trace straight down from the ball. */
    bool IsGrounded() const;
    // added to stop spam jumping
    /* bool bWasGroundedLastFrame = false;
    bool bHasJumpedSinceLastGround = false; */ // possible to break the jump with these

    /** Called whenever this actor's primitive component hits something.
     *  We override this so we can knock other balls away on the server.
     */
    virtual void NotifyHit(
        UPrimitiveComponent* MyComp,
        AActor* Other,
        UPrimitiveComponent* OtherComp,
        bool bSelfMoved,
        FVector HitLocation,
        FVector HitNormal,
        FVector NormalImpulse,
        const FHitResult& Hit
    ) override;
};


