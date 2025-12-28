#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "Inventory/InventoryTypes.h"
#include "ItemDefinitionDataAsset.generated.h"

UCLASS(BlueprintType)
class COWFIELDCLEANUP_API UItemDefinitionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EInventoryItemCategory Category = EInventoryItemCategory::Cleanup;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	float ItemWeight = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bStackable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "bStackable"))
	int32 MaxStackSize = 0;
};
