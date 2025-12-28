#include "Inventory/InventoryComponent.h"

#include "DataAssets/InventoryBalanceDataAsset.h"
#include "DataAssets/ItemDefinitionDataAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		InitializeToolSlots();
		RecalculateBagWeight();
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, ToolSlots); // Replicated tool slot state
	DOREPLIFETIME(UInventoryComponent, BagEntries); // Replicated cleanup bag contents
	DOREPLIFETIME(UInventoryComponent, BagTotalWeight); // Replicated bag weight
	DOREPLIFETIME(UInventoryComponent, TrackedTools); // Replicated dropped tool tracking
}

const TArray<FToolSlotEntry>& UInventoryComponent::GetToolSlots() const
{
	return ToolSlots;
}

const TArray<FBagItemEntry>& UInventoryComponent::GetBagEntries() const
{
	return BagEntries;
}

float UInventoryComponent::GetBagTotalWeight() const
{
	return BagTotalWeight;
}

float UInventoryComponent::GetMovementSpeedMultiplier() const
{
	return GetCurveValueSafe(BalanceData ? BalanceData->MovementSpeedByWeight : nullptr);
}

float UInventoryComponent::GetStaminaDrainMultiplier() const
{
	return GetCurveValueSafe(BalanceData ? BalanceData->StaminaDrainMultiplierByWeight : nullptr);
}

void UInventoryComponent::RequestAddToolToSlot(UItemDefinitionDataAsset* ItemDefinition, const FGuid& ToolId, int32 SlotIndex)
{
	if (!ItemDefinition)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		ServerAddToolToSlot(ItemDefinition, ToolId, SlotIndex);
		return;
	}

	ServerAddToolToSlot(ItemDefinition, ToolId, SlotIndex);
}

void UInventoryComponent::RequestRemoveToolFromSlot(int32 SlotIndex)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRemoveToolFromSlot(SlotIndex);
		return;
	}

	ServerRemoveToolFromSlot(SlotIndex);
}

void UInventoryComponent::RequestAddBagItem(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity)
{
	if (!ItemDefinition || Quantity <= 0)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		ServerAddBagItem(ItemDefinition, Quantity);
		return;
	}

	ServerAddBagItem(ItemDefinition, Quantity);
}

void UInventoryComponent::RequestRemoveBagItem(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity)
{
	if (!ItemDefinition || Quantity <= 0)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		ServerRemoveBagItem(ItemDefinition, Quantity);
		return;
	}

	ServerRemoveBagItem(ItemDefinition, Quantity);
}

void UInventoryComponent::RequestLocateTool(const FGuid& ToolId)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRequestLocateTool(ToolId);
		return;
	}

	ServerRequestLocateTool(ToolId);
}

void UInventoryComponent::RegisterDroppedTool(const FGuid& ToolId, int32 OwnerPlayerId, const FVector& WorldLocation)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRegisterDroppedTool(ToolId, OwnerPlayerId, WorldLocation);
		return;
	}

	ServerRegisterDroppedTool(ToolId, OwnerPlayerId, WorldLocation);
}

void UInventoryComponent::UpdateDroppedToolLocation(const FGuid& ToolId, const FVector& WorldLocation)
{
	if (GetOwner()->HasAuthority())
	{
		ServerUpdateDroppedToolLocation(ToolId, WorldLocation);
		return;
	}

	ServerUpdateDroppedToolLocation(ToolId, WorldLocation);
}

void UInventoryComponent::RemoveDroppedTool(const FGuid& ToolId)
{
	if (GetOwner()->HasAuthority())
	{
		ServerRemoveDroppedTool(ToolId);
		return;
	}

	ServerRemoveDroppedTool(ToolId);
}

void UInventoryComponent::ServerAddToolToSlot_Implementation(UItemDefinitionDataAsset* ItemDefinition, const FGuid& ToolId, int32 SlotIndex)
{
	if (!CanModifyInventory() || !ItemDefinition)
	{
		return;
	}

	if (SlotIndex < 0 || SlotIndex >= ToolSlots.Num())
	{
		return;
	}

	FToolSlotEntry& Slot = ToolSlots[SlotIndex];
	Slot.ItemDefinition = ItemDefinition;
	Slot.ToolId = ToolId;
	Slot.bOccupied = true;

	OnToolSlotsChanged.Broadcast();
}

void UInventoryComponent::ServerRemoveToolFromSlot_Implementation(int32 SlotIndex)
{
	if (!CanModifyInventory())
	{
		return;
	}

	if (SlotIndex < 0 || SlotIndex >= ToolSlots.Num())
	{
		return;
	}

	FToolSlotEntry& Slot = ToolSlots[SlotIndex];
	Slot.ItemDefinition = nullptr;
	Slot.ToolId.Invalidate();
	Slot.bOccupied = false;

	OnToolSlotsChanged.Broadcast();
}

void UInventoryComponent::ServerAddBagItem_Implementation(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity)
{
	if (!CanModifyInventory() || !ItemDefinition || Quantity <= 0)
	{
		return;
	}

	const float MaxBagWeight = GetMaxBagWeight();
	const float IncomingWeight = ItemDefinition->ItemWeight * Quantity;
	if (MaxBagWeight > 0.0f && (BagTotalWeight + IncomingWeight) > MaxBagWeight)
	{
		return;
	}

	for (FBagItemEntry& Entry : BagEntries)
	{
		if (Entry.ItemDefinition == ItemDefinition)
		{
			Entry.Quantity += Quantity;
			RecalculateBagWeight();
			OnBagEntriesChanged.Broadcast();
			return;
		}
	}

	FBagItemEntry NewEntry;
	NewEntry.ItemDefinition = ItemDefinition;
	NewEntry.Quantity = Quantity;
	BagEntries.Add(NewEntry);

	RecalculateBagWeight();
	OnBagEntriesChanged.Broadcast();
}

void UInventoryComponent::ServerRemoveBagItem_Implementation(UItemDefinitionDataAsset* ItemDefinition, int32 Quantity)
{
	if (!CanModifyInventory() || !ItemDefinition || Quantity <= 0)
	{
		return;
	}

	for (int32 Index = 0; Index < BagEntries.Num(); ++Index)
	{
		FBagItemEntry& Entry = BagEntries[Index];
		if (Entry.ItemDefinition == ItemDefinition)
		{
			Entry.Quantity = FMath::Max(0, Entry.Quantity - Quantity);
			if (Entry.Quantity == 0)
			{
				BagEntries.RemoveAt(Index);
			}
			RecalculateBagWeight();
			OnBagEntriesChanged.Broadcast();
			return;
		}
	}
}

void UInventoryComponent::ServerRegisterDroppedTool_Implementation(const FGuid& ToolId, int32 OwnerPlayerId, const FVector& WorldLocation)
{
	if (!CanModifyInventory() || !ToolId.IsValid())
	{
		return;
	}

	for (FTrackedTool& TrackedTool : TrackedTools)
	{
		if (TrackedTool.ToolId == ToolId)
		{
			TrackedTool.OwnerPlayerId = OwnerPlayerId;
			TrackedTool.WorldLocation = WorldLocation;
			TrackedTool.bIsDropped = true;
			return;
		}
	}

	FTrackedTool NewTrackedTool;
	NewTrackedTool.ToolId = ToolId;
	NewTrackedTool.OwnerPlayerId = OwnerPlayerId;
	NewTrackedTool.WorldLocation = WorldLocation;
	NewTrackedTool.bIsDropped = true;
	TrackedTools.Add(NewTrackedTool);
}

void UInventoryComponent::ServerUpdateDroppedToolLocation_Implementation(const FGuid& ToolId, const FVector& WorldLocation)
{
	if (!CanModifyInventory() || !ToolId.IsValid())
	{
		return;
	}

	for (FTrackedTool& TrackedTool : TrackedTools)
	{
		if (TrackedTool.ToolId == ToolId)
		{
			TrackedTool.WorldLocation = WorldLocation;
			TrackedTool.bIsDropped = true;
			return;
		}
	}
}

void UInventoryComponent::ServerRemoveDroppedTool_Implementation(const FGuid& ToolId)
{
	if (!CanModifyInventory() || !ToolId.IsValid())
	{
		return;
	}

	for (int32 Index = 0; Index < TrackedTools.Num(); ++Index)
	{
		if (TrackedTools[Index].ToolId == ToolId)
		{
			TrackedTools.RemoveAt(Index);
			return;
		}
	}
}

void UInventoryComponent::ServerRequestLocateTool_Implementation(const FGuid& ToolId)
{
	if (!CanModifyInventory() || !ToolId.IsValid())
	{
		return;
	}

	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (IsLocateOnCooldown(CurrentTimeSeconds))
	{
		return;
	}

	const int32 OwnerPlayerId = GetOwnerPlayerId();
	for (const FTrackedTool& TrackedTool : TrackedTools)
	{
		if (TrackedTool.ToolId == ToolId && TrackedTool.bIsDropped && TrackedTool.OwnerPlayerId == OwnerPlayerId)
		{
			const FVector OwnerLocation = GetOwnerLocation();
			const FVector Offset = TrackedTool.WorldLocation - OwnerLocation;
			const float Distance = Offset.Size();
			const FVector Direction = Offset.IsNearlyZero() ? FVector::ZeroVector : Offset.GetSafeNormal();
			const ELocatorDistanceBand DistanceBand = ResolveDistanceBand(Distance);

			UpdateLocateCooldown(CurrentTimeSeconds);
			ClientReceiveToolLocation(ToolId, Direction, DistanceBand, Distance);
			OnToolLocatorResult.Broadcast(ToolId, DistanceBand, Distance);
			return;
		}
	}
}

void UInventoryComponent::ClientReceiveToolLocation_Implementation(const FGuid& ToolId, const FVector& Direction, ELocatorDistanceBand DistanceBand, float Distance)
{
	OnToolLocatorResult.Broadcast(ToolId, DistanceBand, Distance);
}

void UInventoryComponent::OnRep_ToolSlots()
{
	OnToolSlotsChanged.Broadcast();
}

void UInventoryComponent::OnRep_BagEntries()
{
	OnBagEntriesChanged.Broadcast();
}

void UInventoryComponent::OnRep_BagTotalWeight()
{
	OnBagWeightChanged.Broadcast(BagTotalWeight);
}

void UInventoryComponent::OnRep_TrackedTools()
{
}

void UInventoryComponent::InitializeToolSlots()
{
	const int32 DesiredSlots = BalanceData ? BalanceData->MaxToolSlots : 3;
	ToolSlots.SetNum(DesiredSlots);
	for (FToolSlotEntry& Slot : ToolSlots)
	{
		Slot.bOccupied = false;
		Slot.ItemDefinition = nullptr;
		Slot.ToolId.Invalidate();
	}
}

void UInventoryComponent::RecalculateBagWeight()
{
	float NewWeight = 0.0f;
	for (const FBagItemEntry& Entry : BagEntries)
	{
		if (Entry.ItemDefinition)
		{
			NewWeight += Entry.ItemDefinition->ItemWeight * Entry.Quantity;
		}
	}

	BagTotalWeight = NewWeight;
	OnBagWeightChanged.Broadcast(BagTotalWeight);
}

float UInventoryComponent::GetCurveValueSafe(const UCurveFloat* Curve) const
{
	if (!Curve)
	{
		return 1.0f;
	}

	return Curve->GetFloatValue(BagTotalWeight);
}

float UInventoryComponent::GetMaxBagWeight() const
{
	return BalanceData ? BalanceData->MaxBagWeight : 0.0f;
}

float UInventoryComponent::GetLocateCooldownSeconds() const
{
	return BalanceData ? BalanceData->LocateCooldownSeconds : 0.0f;
}

float UInventoryComponent::GetNearDistance() const
{
	return BalanceData ? BalanceData->NearDistance : 0.0f;
}

float UInventoryComponent::GetMediumDistance() const
{
	return BalanceData ? BalanceData->MediumDistance : 0.0f;
}

ELocatorDistanceBand UInventoryComponent::ResolveDistanceBand(float Distance) const
{
	const float NearDistance = GetNearDistance();
	const float MediumDistance = GetMediumDistance();

	if (NearDistance <= 0.0f || MediumDistance <= 0.0f)
	{
		return ELocatorDistanceBand::Unknown;
	}

	if (Distance <= NearDistance)
	{
		return ELocatorDistanceBand::Near;
	}

	if (Distance <= MediumDistance)
	{
		return ELocatorDistanceBand::Medium;
	}

	return ELocatorDistanceBand::Far;
}

FVector UInventoryComponent::GetOwnerLocation() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
}

int32 UInventoryComponent::GetOwnerPlayerId() const
{
	const AActor* OwnerActor = GetOwner();
	const APlayerState* PlayerState = OwnerActor ? OwnerActor->GetPlayerState() : nullptr;
	return PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE;
}

bool UInventoryComponent::IsLocateOnCooldown(float CurrentTimeSeconds) const
{
	const float Cooldown = GetLocateCooldownSeconds();
	if (Cooldown <= 0.0f)
	{
		return false;
	}

	return LastLocateRequestTimeSeconds >= 0.0f && (CurrentTimeSeconds - LastLocateRequestTimeSeconds) < Cooldown;
}

void UInventoryComponent::UpdateLocateCooldown(float CurrentTimeSeconds)
{
	LastLocateRequestTimeSeconds = CurrentTimeSeconds;
}

bool UInventoryComponent::CanModifyInventory() const
{
	return GetOwner() && GetOwner()->HasAuthority();
}
