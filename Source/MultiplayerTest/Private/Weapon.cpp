// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "TimerManager.h"

static int32 DebugLineTrace = 0;
FAutoConsoleVariableRef CVARDebugLineDrawing (
	TEXT("MULTI.DebugLineTrace"), DebugLineTrace, 
	TEXT("Draw Debug Lines"), ECVF_Cheat);

AWeapon::AWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";

	FireRate = 300;		// Amount of bullets per min
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBtwShots = 60 / FireRate;
}

void AWeapon::Fire()
{
	// Ray from pawn eyes to crosshair
	AActor* TheOwner = GetOwner();

	if (TheOwner)
	{
		// Setting up a basic line trace
		FVector EyeLocation;
		FRotator EyeRotation;

		TheOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + EyeRotation.Vector() * 10000;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(TheOwner);
		QueryParams.AddIgnoredActor(this);		// Ignore the weapon
		QueryParams.bTraceComplex = true;		// It will trace against each individual triangle of the mesh we're hitting for an exact result

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_GameTraceChannel1, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, TheOwner->GetInstigatorController(), this, DamageType);

			if (ImpactFX) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}

		if (DebugLineTrace) DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 0.1f, 0, 0.2f);

		if (MuzzleFX) UGameplayStatics::SpawnEmitterAttached(MuzzleFX, MeshComp, MuzzleSocketName);

		APawn* ThePawn = Cast<APawn>(GetOwner());
		if (ThePawn)
		{
			APlayerController* PC = Cast<APlayerController>(ThePawn->GetController());
			if (PC) PC->ClientStartCameraShake(CamShake);
		}
		// Keep track of the time in the world when fire
		LastShotTime = GetWorld()->TimeSeconds;
	}
}

void AWeapon::StartFire()
{
	// Any negative value for FirstDelay would be replace by zero
	float FirstDelay = FMath::Max(LastShotTime + TimeBtwShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimeBtwShotsTimer, this, &AWeapon::Fire, TimeBtwShots, true, FirstDelay);
}

void AWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimeBtwShotsTimer);
}
