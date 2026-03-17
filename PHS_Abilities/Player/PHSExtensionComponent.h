#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PHSAbilitySet.h"
#include "PHSExtensionComponent.generated.h"

class UAbilitySystemComponent;
class UPHSAbilitySet;

/**
 UPHSExtensionComponent
 
 Injected onto a character (typically via a GameFeature Add Components action)
 to grant a list of ability sets to the owner's AbilitySystemComponent.
 
 Grants on BeginPlay (server-authoritative). Stores all granted handles and
 revokes them cleanly in EndPlay — no ability leaks across PIE sessions or
 GameFeature deactivations.
 
 ASC resolution order:
 1. IAbilitySystemInterface on the owner (preferred / correct).
 2. UAbilitySystemGlobals::GetAbilitySystemComponentFromActor (fallback for
 	third-party plugins like Motion that have an ASC but don't implement the
 	interface in a way visible from C++).
 
 To port to a new project:
 - Rename PHS_API to the new module export macro.
 - If the project doesn't use Motion, the GlobalsFallback comment can be removed.
 - AbilitySetsToGrant is set in the Blueprint subclass Class Defaults.
 */
UCLASS(Blueprintable, ClassGroup = (PHS), meta = (BlueprintSpawnableComponent))
class PHS_API UPHSExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPHSExtensionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 Ability sets to grant to the owning actor's ASC on BeginPlay.
	 Set these in the Blueprint subclass Class Defaults.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "PHS|Abilities")
	TArray<TObjectPtr<UPHSAbilitySet>> AbilitySetsToGrant;

private:
	/** Resolves the ASC from the owner via interface or global fallback. */
	UAbilitySystemComponent* GetASC() const;

	/** Grants all AbilitySetsToGrant to the ASC and populates GrantedHandles. */
	void GrantAbilitySets(UAbilitySystemComponent* ASC);

	/**
	 Handles returned from each granted ability set.
	 Used to cleanly revoke everything in EndPlay.
	 One entry per entry in AbilitySetsToGrant.
	 */
	UPROPERTY()
	TArray<FPHSAbilitySet_GrantedHandles> GrantedHandles;
};