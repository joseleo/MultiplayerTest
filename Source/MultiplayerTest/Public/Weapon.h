// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UCLASS()
class MULTIPLAYERTEST_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	AWeapon();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

	// Only need to edit in the editor, not runtime, 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* ImpactFX;

	/* IMPORTANT: if using 4.26 DO NOT FORGET to correct TSubclassOf<UCameraShake>
	with TSubclassOf<UMatineeCameraShake> */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UMatineeCameraShake> CamShake;

	// Handling time between consecutive shots
	FTimerHandle TimeBtwShotsTimer;
	float LastShotTime;
	float TimeBtwShots;		// Derives from FireRate

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireRate;

public:
	void StartFire();
	void StopFire();
};
