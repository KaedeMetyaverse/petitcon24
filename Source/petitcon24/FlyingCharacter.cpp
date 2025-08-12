#include "FlyingCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

AFlyingCharacter::AFlyingCharacter()
{
    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    TObjectPtr<UCharacterMovementComponent> Movement = GetCharacterMovement();
    check(Movement != nullptr);

    Movement->DefaultLandMovementMode = EMovementMode::MOVE_Flying;
    Movement->DefaultWaterMovementMode = EMovementMode::MOVE_Flying;
    Movement->NavAgentProps.bCanFly = true;
}
