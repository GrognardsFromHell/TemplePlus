
#pragma once

struct AnimSlot;

int GoalIsAlive(AnimSlot& ars);
int GoalActionPerform2(AnimSlot& slot); // used in ag_attempt_attack
int GoalAttackEndTurnIfUnreachable(AnimSlot& slot);
int GoalIsProne(AnimSlot& slot);
int GoalIsConcealed(AnimSlot& slot);
int GoalIsRotatedTowardNextPathNode(AnimSlot &slot);
int GoalIsSlotFlag10NotSet(AnimSlot& slot);
int GoalSlotFlagSet8If4AndNotSetYet(AnimSlot& slot);
int GoalPlayGetHitAnim(AnimSlot& slot); // belongs in ag_hit_by_weapon
int GoalIsCurrentPathValid(AnimSlot& slot);
int GoalReturnFalse(AnimSlot &slot); // Assumes that data1 is a spell number
int GoalUnconcealAnimate(AnimSlot& slot);
int GoalStateFunc130(AnimSlot& slot);
int GoalPickpocketPerform(AnimSlot &slot);
int GoalIsAnimatingConjuration(AnimSlot &slot);
int GoalAttemptSpell(AnimSlot &slot);
