#include "UI/PhoneLocatorWidget.h"

void UPhoneLocatorWidget::ShowLocatorResult(ELocatorDistanceBand DistanceBand, float Distance)
{
	CurrentDistanceBand = DistanceBand;
	CurrentDistance = Distance;
}
