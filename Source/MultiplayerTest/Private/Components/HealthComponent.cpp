// Written by Jose Leon on March 2021.


#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	StarterHealth = 100;

	SetIsReplicated(true);
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority) {
		AActor* TheOwner = GetOwner();
		if (TheOwner)
		{
			TheOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeDamageHandle);
		}
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

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
}