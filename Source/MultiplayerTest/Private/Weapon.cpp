// Written by Jose Leon on March 2021.


#include "Weapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

static int32 DebugLineTrace = 0;
FAutoConsoleVariableRef CVARDebugLineDrawing(
	TEXT("MULTI.DebugLineTrace"), DebugLineTrace,
	TEXT("Draw Debug Lines"), ECVF_Cheat);

AWeapon::AWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";

	StandardDamage = 20.0f;
	FireRate = 300.0f;		// Amount of bullets per min

	SetReplicates(true);

	NetUpdateFrequency = 60.0f;
	MinNetUpdateFrequency = NetUpdateFrequency / 2;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBtwShots = 60 / FireRate;
}

void AWeapon::Fire()
{
	if (!HasAuthority()) ServerFire();

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

			UGameplayStatics::ApplyPointDamage(HitActor, StandardDamage, ShotDirection, Hit, TheOwner->GetInstigatorController(), this, DamageType);

			PlayImpactEffects(Hit.ImpactPoint);
		}

		if (DebugLineTrace) DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 0.1f, 0, 0.2f);

		PlayFireEffects(Hit.ImpactPoint);

		if (HasAuthority())
		{
			HitScanTrace.TraceTo = Hit.ImpactPoint;
		}

		// Keep track of the time in the world when fire
		LastShotTime = GetWorld()->TimeSeconds;
	}
}

void AWeapon::ServerFire_Implementation()
{
	Fire();
}

bool AWeapon::ServerFire_Validate()
{
	return true;
}

void AWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.TraceTo);
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

void AWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleFX) UGameplayStatics::SpawnEmitterAttached(MuzzleFX, MeshComp, MuzzleSocketName);

	APawn* ThePawn = Cast<APawn>(GetOwner());
	if (ThePawn)
	{
		APlayerController* PC = Cast<APlayerController>(ThePawn->GetController());
		if (PC) PC->ClientStartCameraShake(CamShake);
	}
}

void AWeapon::PlayImpactEffects(FVector ImpactPoint)
{
	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	FVector ShotDirection = ImpactPoint - MuzzleLocation;
	ShotDirection.Normalize();

	if (ImpactFX) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, ImpactPoint, ShotDirection.Rotation());
}

// In this case we don't need  to specify this is the header file
// it's already done by UE header tool
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, HitScanTrace, COND_SkipOwner);
}