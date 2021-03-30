// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	// enable support for crouching
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShooterCharacter::MoveForward(float Amount)
{
	AddMovementInput(GetActorForwardVector() * Amount);
}

void AShooterCharacter::MoveRight(float Amount)
{
	AddMovementInput(GetActorRightVector() * Amount);

}

void AShooterCharacter::BeginCrouch()
{
	Crouch();
}

void AShooterCharacter::EndCrouch()
{
	UnCrouch();
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AShooterCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
}

FVector AShooterCharacter::GetPawnViewLocation() const
{
	// If the camera comp is valid we return its location instead of the default location (see GetPawnViewLocation)
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	// ...if fails call the original implementation
	return Super::GetPawnViewLocation();
}

