#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryTypes.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;

UCLASS()
class COWFIELDCLEANUP_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateFromInventory(UInventoryComponent* InventoryComponent);

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FToolSlotEntry> ToolSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FBagItemEntry> BagEntries;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float BagWeight = 0.0f;
};
