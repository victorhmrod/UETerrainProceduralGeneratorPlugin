// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UTestBlueprintFunctionLibrary.generated.h"

class USceneComponent;

UCLASS()
class WORLDENGINEINFINITETERRAIN_API UUTestBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="WorldEngine")
	static void TestNavUpdate(USceneComponent* SceneComponent);
	
	
};
