#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryBalanceDataAsset.generated.h"

UCLASS(BlueprintType)
class COWFIELDCLEANUP_API UInventoryBalanceDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bag|Capacity")
	int32 MaxToolSlots = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bag|Capacity")
	float MaxBagWeight = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bag|Curves")
	TObjectPtr<UCurveFloat> MovementSpeedByWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bag|Curves")
	TObjectPtr<UCurveFloat> StaminaDrainMultiplierByWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locator")
	float LocateCooldownSeconds = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locator")
	float NearDistance = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locator")
	float MediumDistance = 0.0f;
};
