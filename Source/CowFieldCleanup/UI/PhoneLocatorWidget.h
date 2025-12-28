#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryTypes.h"
#include "PhoneLocatorWidget.generated.h"

UCLASS()
class COWFIELDCLEANUP_API UPhoneLocatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Locator")
	void ShowLocatorResult(ELocatorDistanceBand DistanceBand, float Distance);

	UPROPERTY(BlueprintReadOnly, Category = "Locator")
	ELocatorDistanceBand CurrentDistanceBand = ELocatorDistanceBand::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Locator")
	float CurrentDistance = 0.0f;
};
