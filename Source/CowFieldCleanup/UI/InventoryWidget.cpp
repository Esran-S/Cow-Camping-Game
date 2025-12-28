#include "UI/InventoryWidget.h"

#include "Inventory/InventoryComponent.h"

void UInventoryWidget::UpdateFromInventory(UInventoryComponent* InventoryComponent)
{
	if (!InventoryComponent)
	{
		return;
	}

	ToolSlots = InventoryComponent->GetToolSlots();
	BagEntries = InventoryComponent->GetBagEntries();
	BagWeight = InventoryComponent->GetBagTotalWeight();
}
