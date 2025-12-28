#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

class UItemDefinitionDataAsset;

UENUM(BlueprintType)
enum class EInventoryItemCategory : uint8
{
	Tool,
	Cleanup
};

UENUM(BlueprintType)
enum class ELocatorDistanceBand : uint8
{
	Near,
	Medium,
	Far,
	Unknown
};

USTRUCT(BlueprintType)
struct FToolSlotEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemDefinitionDataAsset> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid ToolId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOccupied = false;
};

USTRUCT(BlueprintType)
struct FBagItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemDefinitionDataAsset> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct FTrackedTool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid ToolId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OwnerPlayerId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsDropped = false;
};
