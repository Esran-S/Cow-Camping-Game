#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryTypes.h"
#include "InventoryComponent.generated.h"

class UInventoryBalanceDataAsset;
class UItemDefinitionDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBagWeightChanged, float, NewWeight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToolSlotsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBagEntriesChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnToolLocatorResult, const FGuid&, ToolId, ELocatorDistanceBand, DistanceBand, float, Distance);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COWFIELDCLEANUP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FToolSlotEntry>& GetToolSlots() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FBagItemEntry>& GetBagEntries() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetBagTotalWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Movement")
	float GetMovementSpeedMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Movement")
	float GetStaminaDrainMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Tools")
	void RequestAddToolToSlot(UItemDefinitionDataAsset* ItemDefinition, const FGuid& ToolId, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Tools")
	void RequestRemoveToolFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	void RequestAddBagItem(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	void RequestRemoveBagItem(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Locator")
	void RequestLocateTool(const FGuid& ToolId);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Locator")
	void RegisterDroppedTool(const FGuid& ToolId, int32 OwnerPlayerId, const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Locator")
	void UpdateDroppedToolLocation(const FGuid& ToolId, const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Locator")
	void RemoveDroppedTool(const FGuid& ToolId);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnBagWeightChanged OnBagWeightChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnToolSlotsChanged OnToolSlotsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnBagEntriesChanged OnBagEntriesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnToolLocatorResult OnToolLocatorResult;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void InitializeToolSlots();
	void RecalculateBagWeight();
	float GetCurveValueSafe(const UCurveFloat* Curve) const;
	float GetMaxBagWeight() const;
	float GetLocateCooldownSeconds() const;
	float GetNearDistance() const;
	float GetMediumDistance() const;
	ELocatorDistanceBand ResolveDistanceBand(float Distance) const;
	FVector GetOwnerLocation() const;
	int32 GetOwnerPlayerId() const;
	bool IsLocateOnCooldown(float CurrentTimeSeconds) const;
	void UpdateLocateCooldown(float CurrentTimeSeconds);
	bool CanModifyInventory() const;

	UFUNCTION(Server, Reliable)
	void ServerAddToolToSlot(UItemDefinitionDataAsset* ItemDefinition, const FGuid& ToolId, int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerRemoveToolFromSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerAddBagItem(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void ServerRemoveBagItem(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void ServerRegisterDroppedTool(const FGuid& ToolId, int32 OwnerPlayerId, const FVector& WorldLocation);

	UFUNCTION(Server, Reliable)
	void ServerUpdateDroppedToolLocation(const FGuid& ToolId, const FVector& WorldLocation);

	UFUNCTION(Server, Reliable)
	void ServerRemoveDroppedTool(const FGuid& ToolId);

	UFUNCTION(Server, Reliable)
	void ServerRequestLocateTool(const FGuid& ToolId);

	UFUNCTION(Client, Reliable)
	void ClientReceiveToolLocation(const FGuid& ToolId, const FVector& Direction, ELocatorDistanceBand DistanceBand, float Distance);

	UFUNCTION()
	void OnRep_ToolSlots();

	UFUNCTION()
	void OnRep_BagEntries();

	UFUNCTION()
	void OnRep_BagTotalWeight();

	UFUNCTION()
	void OnRep_TrackedTools();

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInventoryBalanceDataAsset> BalanceData;

	UPROPERTY(ReplicatedUsing = OnRep_ToolSlots)
	TArray<FToolSlotEntry> ToolSlots;

	UPROPERTY(ReplicatedUsing = OnRep_BagEntries)
	TArray<FBagItemEntry> BagEntries;

	UPROPERTY(ReplicatedUsing = OnRep_BagTotalWeight)
	float BagTotalWeight = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_TrackedTools)
	TArray<FTrackedTool> TrackedTools;

	float LastLocateRequestTimeSeconds = -1.0f;
};
