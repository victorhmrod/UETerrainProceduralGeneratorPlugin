// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#include "WEUtilities.h"
#include "AI/NavigationSystemBase.h"

void UWEUtilities::NavUpdate(USceneComponent* SceneComponent)
{
	if (SceneComponent->IsRegistered())
	{
		if (SceneComponent->GetWorld())
		{
			FNavigationSystem::UpdateComponentData(*SceneComponent);
		}
	}
}
