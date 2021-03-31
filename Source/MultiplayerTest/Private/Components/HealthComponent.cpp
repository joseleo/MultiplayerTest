// Written by Jose Leon on March 2021.


#include "Components/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	StarterHealth = 100;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* TheOwner = GetOwner();
	if (TheOwner)
	{
		TheOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeDamageHandle);
	}

	Health = StarterHealth;
}

void UHealthComponent::TakeDamageHandle(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f) return;

	// Clamps "health after damage" between 0 and the max health
	Health = FMath::Clamp(Health - Damage, 0.0f, StarterHealth);

	UE_LOG(LogTemp, Log, TEXT("Health: %s"), *FString::SanitizeFloat(Health));

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

