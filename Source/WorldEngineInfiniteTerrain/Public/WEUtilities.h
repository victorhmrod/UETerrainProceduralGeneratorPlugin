// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WEUtilities.generated.h"

class USceneComponent;

UCLASS()
class WORLDENGINEINFINITETERRAIN_API UWEUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="WorldEngine")
	static void NavUpdate(USceneComponent* SceneComponent);
};
