// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#include "UTestBlueprintFunctionLibrary.h"
#include "AI/NavigationSystemBase.h"

void UUTestBlueprintFunctionLibrary::TestNavUpdate(USceneComponent* SceneComponent)
{
	if (SceneComponent->IsRegistered())
	{
		if (SceneComponent->GetWorld())
		{
			FNavigationSystem::UpdateComponentData(*SceneComponent);
		}
	}
}
