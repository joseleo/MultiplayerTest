// Written by Jose Leon on March 2021.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	void RestartDeadPlayer();

public:
	AShooterGameMode();

	virtual void Tick(float DeltaSeconds) override;
};
