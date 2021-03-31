// Written by Jose Leon on March 2021.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapon.h"
#include "Components/HealthComponent.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	// Enable support for crouching
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Don't want that capsule block the weapon trace, only mesh should block
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);


	WeaponAttachSocketName = "WeaponSocket";

	ZoomFOV = 55.0f;
	InterpSpeed = 25.0f;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComp->FieldOfView;

	// always spawn even if it collides with sth
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(BeginnerWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponAttachSocketName);
	}

	HealthComp->OnHealthChanged.AddDynamic(this, &AShooterCharacter::OnHealthChanged);
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

void AShooterCharacter::BeginZoom()
{
	bZoomActivated = true;
}

void AShooterCharacter::EndZoom()
{
	bZoomActivated = false;
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Updating FOV
	float TargetFOV = bZoomActivated ? ZoomFOV : DefaultFOV;
	float CurrentFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, InterpSpeed);

	CameraComp->SetFieldOfView(CurrentFOV);
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

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AShooterCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AShooterCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::StopFire);
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

void AShooterCharacter::StartFire()
{
	if (CurrentWeapon) CurrentWeapon->StartFire();
}

void AShooterCharacter::StopFire()
{
	if (CurrentWeapon) CurrentWeapon->StopFire();
}

void AShooterCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		// Character is dead
		bDied = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		// After 5s this whole pawn will get destroyed
		SetLifeSpan(5.0f);
	}
}