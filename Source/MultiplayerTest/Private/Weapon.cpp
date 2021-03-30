// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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
		QueryParams.AddIgnoredActor(this);		// ignore the weapon
		QueryParams.bTraceComplex = true;		// it will trace against each individual triangle of the mesh we're hitting for an exact result

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, TheOwner->GetInstigatorController(), this, DamageType);

			if (ImpactFX)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
		}

		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 0.1f, 0, 0.2f);

		if (MuzzleFX)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleFX, MeshComp, MuzzleSocketName);
		}
	}

}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

