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
#include "BallPawn.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;

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

    // ----------------- Input handlers (client side) -----------------

    /** Called by input system on the owning client. */
    void MoveForward(float Value);

    /** Called by input system on the owning client. */
    void MoveRight(float Value);

    /** Called by input system when Jump is pressed on the owning client. */
    void JumpPressed();

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


