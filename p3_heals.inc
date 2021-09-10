SUB p3_heals_init {
	/declare healCheckTimer timer outer 0
	/declare teamIndex int outer 1
	/declare enableteamIndex bool outer TRUE
	/declare checkTeamIndexTimer timer outer 0

	/declare castTeamNetbots string outer NONE
	/declare castTeamNetbotsList list outer

	/varset castTeamNetbots ${getBotIniOption[CastTeam.Members,${castTeamNetbots}]}
	/invoke ${castTeamNetbotsList.Append[${castTeamNetbots.Upper}]}
	/invoke ${castTeamNetbotsList.Remove[${Me.CleanName.Upper}]}

	/call validateSpellBasedArray iniHeals
	/call processArrayTargets iniHeals

	/RETURN
}

SUB healsMaintenance() {
	/if (${checkTeamIndexTimer}==0) {
		/call determineTeamIndex
	}	

	/if (${healCheckTimer}==0) {
		/if (${Me.Casting.ID}!=NULL) /return
		/if (${Me.Invis}==TRUE) /RETURN
		/if (${Me.Silenced.ID}!=NULL) /RETURN
		/if (${Me.Moving}==TRUE && ${canCastWhileMoving}==FALSE) /return

		/varset healCheckTimer 3
		/call checkHeals
	}

	/RETURN
}


SUB checkHeals {
	/declare i int local
	/declare cancelConditions string local
	/declare healTargetName string local
	/declare healSpawn spawn local
	/declare healSpellName string local
	/declare healSpell spell local
	/declare healPct int local 90
	/declare useTeamIndex bool local FALSE
	/declare checkSpellID int local
	/declare healLockoutTimerName string local
	/declare triggerIdleCasting bool local TRUE
	/declare cancelHealHP int local 
	/declare botBuffIDs list local 
	/declare netbotsReadyForHeal list local

	/invoke ${botBuffIDs.Delimiter[ ]}

	/for i 1 to ${iniHeals.Size}
		/varset triggerIdleCasting TRUE
		/invoke ${botBuffIDs.Clear}
		/invoke ${netbotsReadyForHeal.Clear}

		/if (${iniHeals[${i},1].Equal[-]}) /break
		/if (${iniHeals[${i},${bEnabled}].Equal[NO]}) /continue

		/varset healSpellName ${iniHeals[${i},1]}

		/if (${iniHeals[${i},${bRequireCastIdle}].NotEqual[-]}) {
			/if (${idleCastingTimer}>0) /continue
			/varset triggerIdleCasting FALSE
		}

		/if (${iniHeals[${i},${bNotIfAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}>0 || ${persistentAssistTags.Count}>0) {
				/call validateAssistTags ${iniHeals[${i},${bNotIfAssistTag}]} TRUE
				/if (${Macro.Return}==TRUE) /continue
			}
		}
		/if (${iniHeals[${i},${bRequireAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}==0 && ${persistentAssistTags.Count}==0) /continue
			/call validateAssistTags ${iniHeals[${i},${bRequireAssistTag}]}
			/if (${Macro.Return}==FALSE) /continue
		}

		/if (${iniHeals[${i},${bNotIfMounted}].NotEqual[-]}) {
			/if (${Me.Mount.ID}!=NULL) /continue
		}

		/if (${iniHeals[${i},${bCastableOnlyResting}].NotEqual[-]}) {
			/if (${Me.CombatState.Equal[COMBAT]}) /continue
		}

		/if (${iniHeals[${i},${bSelfOverHP}].NotEqual[-]}) {
			/if (${Me.PctHPs} < ${iniHeals[${i},${bSelfOverHP}]}) /continue
		}

		/if (${iniHeals[${i},${bSelfUnderHP}].NotEqual[-]}) {
			/if (${Me.PctHPs} >= ${iniHeals[${i},${bSelfUnderHP}]}) /continue
		}

		/if (${iniHeals[${i},${bSelfOverMana}].NotEqual[-]}) {
			/if (${Me.PctMana} < ${iniHeals[${i},${bSelfOverMana}]}) /continue
		}

		/if (${iniHeals[${i},${bSelfUnderMana}].NotEqual[-]}) {
			/if (${Me.PctMana} >= ${iniHeals[${i},${bSelfUnderMana}]}) /continue
		}

		/if (${iniHeals[${i},${bRequireMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 40]} < ${iniHeals[${i},${bRequireMobsInProximity}]}) /continue
		}

		/if (${iniHeals[${i},${bRequireMaxMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 75]} > ${iniHeals[${i},${bRequireMaxMobsInProximity}]}) /continue
		}

		/if (${iniHeals[${i},${bRequireHurtPartyMembers}].NotEqual[-]}) {
			/if (${checkHurtPartyMembers[${iniHeals[${i},${bRequireHurtPartyMembers}]},45]}==FALSE) /continue
		}

		/if (${iniHeals[${i},${bUseTeamIndex}].NotEqual[-]}) {
			/varset useTeamIndex TRUE
		} else {
			/varset useTeamIndex FALSE
		}

		/if (${iniHeals[${i},${bHealPct}].NotEqual[-]}) /varset healPct ${iniHeals[${i},${bHealPct}]}

		/call getSpellFromCastRequest "${healSpellName}"
		/vardata healSpell ${Macro.Return}

		/if (${healSpell.TargetType.Equal[Pet]}) {
			/if (${Me.Pet.ID}==NULL) /continue
			/if (${Me.Pet.PctHPs}>${healPct}) /continue
			/vardata healSpawn Spawn[id ${Me.Pet.ID}]
		} else /if (${healSpell.TargetType.Equal[Self]}) {
			/if (${Me.PctHPs}>${healPct}) /continue
			/vardata healSpawn Spawn[id ${Me.ID}]
		} else {
			/call getNetBotReadyForHeal ${healPct} ${i}
			/if (${Macro.Return.Equal[NONE]}) /continue	
			/invoke ${netbotsReadyForHeal.Append[${Macro.Return}]}

			/if (${useTeamIndex}==TRUE && ${castTeamNetbots.NotEqual[NONE]}) {
				/if (${Bool[${netbotsReadyForHeal.Item[${teamIndex}]}]}==FALSE) /continue
				/vardata healSpawn Spawn[=${netbotsReadyForHeal.Item[${teamIndex}]}]
			} else {
				/vardata healSpawn Spawn[=${netbotsReadyForHeal.Item[0]}]
			}
		}

		/varset healLockoutTimerName heal_${healSpawn.ID}_${healSpellName}
		/if (${isLockoutExpired[heal_${healSpawn.ID}_${healSpell.ID}]}==FALSE) /continue

		/if (${healSpawn.Distance3D} > ${healSpell.MyRange} && ${healSpawn.Distance3D} > ${healSpell.AERange}) /continue

		/if (${iniHeals[${i},${bNotIfShortBuff}].NotEqual[-]}) {
			/varset checkSpellID ${Spell[${iniHeals[${i},${bNotIfShortBuff}]}].ID}
			/if (${Select[${checkSpellID},${NetBots[${healSpawn.Name}].ShortBuff}]}>0) /continue
		}

		/if (${healSpell.Duration}>0) {
			/invoke ${botBuffIDs.Append[${NetBots[${healSpawn.Name}].ShortBuff}
			/invoke ${botBuffIDs.Append[${NetBots[${healSpawn.Name}].Buff}
			/if (${botBuffIDs.Contains[${healSpell.ID}]}==TRUE) /continue
			/varset cancelConditions TARGETHASBUFF:${healSpell.Name}|
		}

		/if (${iniHeals[${i},${bNoCancelChecks}].Equal[-]}) {
			/varset cancelHealHP ${iniHeals[${i},${bCancelAtHP}]}
			/if (${cancelHealHP}==0) /varset cancelHealHP ${healPct}
			/varset cancelConditions TARGETOVERHP:${cancelHealHP}|
		}

		/if (${iniHeals[${i},${bMemInGem}].NotEqual[-]} && ${Me.Gem[${healSpellName}]}==NULL) {
			/call memorizeSpellInGem "${healSpellName}" ${iniHeals[${i},${bMemInGem}]}
		}

		/if (${isCastReady[${healSpellName}]}==FALSE) {
			/continue
		}

		/if (${iniHeals[${i},${bDoCommand}].NotEqual[-]}) /docommand ${iniHeals[${i},${bDoCommand}]}

		/invoke (${cq.Append[CAST;SPELL=${healSpellName};HEALSPELLID=${healSpell.ID};TARGETID=${healSpawn.ID};CANCELCONDITIONS=${cancelConditions.Left[-1]};SRC=iniHeals;SRCIDX=${i}]})
		/break

	/next i

	/RETURN
}

SUB onHealCastComplete(string result, string opts) {
	/declare i int local ${getStackCommandOpt[${opts},SRCIDX]}
	/declare targetID int local ${getStackCommandOpt[${opts},TARGETID]}
	/declare healSpellID ${getStackCommandOpt[${buffSpellName},HEALSPELLID]}

	/if (${iniHeals[${i},${bRecastDelay}].NotEqual[-]}) {
		/call lockout heal_${targetID}_${healSpellID} ${iniHeals[${i},${bRecastDelay}]}
	}

	/RETURN
}

SUB checkHurtPartyMembers(string params, int distanceLimit)
	/declare requireHurtChars int local ${params.Token[1,@]}
	/declare requireHealthPct int local ${params.Token[2,@]}
	/declare hurtChars int local 0

	/if (${Me.Grouped}==FALSE) /return FALSE

	/declare i int local 0
	/for i 0 to ${Group.Members}
		/if (${Group.Member[${i}].OtherZone}==TRUE) /continue
		/if (${Group.Member[${i}].Dead}==TRUE) /continue
		/if (${distanceLimit} > 0 && ${Group.Member[${i}].Distance} > ${distanceLimit}) /continue
		/if (${Group.Member[${i}].PctHPs} > ${requireHealthPct}) /continue

		/varset hurtChars ${Math.Calc[${hurtChars}+1]}
	/next i

	/if (${hurtChars}>= ${requireHurtChars}) /return TRUE

	/return FALSE
/RETURN

SUB getNetBotReadyForHeal(int healPct, int iniIndex)
	/declare i int local 0
	/declare netbotName string local
	/declare netbotCount int local ${NetBots.Counts}
	/declare lowestPct int local 100
	/declare healNames list local
	/declare li listiterator local
	/declare b string local
	
	/if (${iniHeals[${iniIndex},${bXTargetHeal}].NotEqual[-]} || ${iniHeals[${iniIndex},${bXTargetOnlyHeal}].NotEqual[-]})  {
		/for i 1 to 12
			/if (${Me.XTarget[${i}].ID}==NULL) /continue
			/if (${Me.XTarget[${i}].ID}==0) /continue
			/if (${Me.XTarget[${i}].Type.NotEqual[PC]} && ${Me.XTarget[${i}].Type.NotEqual[Pet]}) /continue
			/if (${Me.XTarget[${i}].Type.Equal[Pet]} && ${Me.XTarget[${i}].Master.Type.NotEqual[PC]}) /continue
			/if (${Me.XTarget[${i}].PctHPs} <= ${healPct}) {
				/if (${healNames.Index[${Me.XTarget[${i}].CleanName}]}!=-1) /continue
				/if (${isLockoutExpired[heal_${Me.XTarget[${i}].ID}_${iniHeals[${iniIndex},1]}]}==FALSE) /continue

				/if (${Me.XTarget[${i}].PctHPs}<${lowestPct}) {
					/varset lowestPct ${Me.XTarget[${i}].PctHPs}
					/invoke ${healNames.Insert[0,${Me.XTarget[${i}].CleanName}]}
				} else {
					/invoke ${healNames.Append[${Me.XTarget[${i}].CleanName}]}
				}
			}
		/next i
	}

	/if (${iniHeals[${iniIndex},${bXTargetOnlyHeal}].NotEqual[-]}) /goto :doneWithNames

	/varset lowestPct 100
	/if (${Me.Grouped}==TRUE && ${iniHeals[${iniIndex},${bIncludeGroup}].NotEqual[-]}) {
		/for i 1 to 5
			/if (${Group.Member[${i}].ID}==NULL) /continue
			/if (${Group.Member[${i}].OtherZone}==TRUE) /continue
			/if (${Group.Member[${i}].OtherZone}==TRUE) /continue
			/if (${Group.Member[${i}].PctHPs} > ${healPct}) /continue
			/if (${healNames.Contains[${Group.Member[${i}]}==FALSE) /continue				
			/if (${isLockoutExpired[heal_${Group.Member[${i}].ID}_${iniHeals[${iniIndex},1]}]}==FALSE) /continue

			/if (${Group.Member[${i}].PctHPs}<=${lowestPct}) {
				/varset lowestPct ${Group.Member[${i}].PctHPs}
				/invoke ${healNames.Insert[0,${Group.Member[${i}]}]}
			} else {
				/invoke ${healNames.Append[${Group.Member[${i}]}]}
			}
		/next i
	}

	/varset lowestPct 100
	/for i 1 to ${netbotCount}
		/varset netbotName ${NetBots.Client[${i}]}

		/if (${netbotName.Equal[NULL]}) /break

		/if (${NetBots[${netbotName}].InZone} == FALSE) /continue

		/if (${Spawn[${netbotName}].Dead}==TRUE) /continue

		/if (${NetBots[${netbotName}].State.Equal[DEAD]}==TRUE || ${NetBots[${netbotName}].State.Equal[STUN]} && ${NetBots[${netbotName}].CurrentHPs} < -1) /continue

		/if (${NetBots[${netbotName}].Invulnerable}==1) /continue

		/if (${iniHeals[${iniIndex},${bTargets}].NotEqual[-]}) {
			/if (${Select[${netbotName},${iniHeals[${iniIndex},${bTargets}]}]}==0) /continue
		}

		/if (${iniHeals[${iniIndex},${bCheckShortBuffs}].NotEqual[-]}) {
			/call getSpellFromCastRequest "${iniHeals[${iniIndex},1]}"
			/if (${Macro.Return.Equal[UNKNOWN]}) /continue
			/if (${NetBots[${netbotName}].ShortBuff.Find[${${Macro.Return}.ID}]}!=0) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequireGroup}].NotEqual[-]}) {
			/if (${Group.Member[${netbotName}].ID}==NULL) /continue
		}

		/if (${iniHeals[${iniIndex},${bTargetUnderLevel}].NotEqual[-]}) {
			/if (${NetBots[${netbotName}].Level} >= ${iniHeals[${iniIndex},${bTargetUnderLevel}]}) /continue
		}
		/if (${iniHeals[${iniIndex},${bTargetOverLevel}].NotEqual[-]}) {
			/if (${NetBots[${netbotName}].Level} <= ${iniHeals[${iniIndex},${bTargetOverLevel}]}) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequireDiseased}].NotEqual[-]}) {
			/if (${NetBots[${netbotName}].Diseased} == 0) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequirePoisoned}].NotEqual[-]}) {
			/if (${NetBots[${netbotName}].Poisoned} == 0) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequireCursed}].NotEqual[-]}) {
			/if (${NetBots[${netbotName}].Cursed} == 0) /continue
		}

		| bRequireHurtPartyMembers implies group memeber, so verify
		/if (${iniHeals[${iniIndex},${bRequireHurtPartyMembers}].NotEqual[-]}) {
			/if (${Group.Member[${netbotName}].ID}==NULL) /continue
		}

		/if (${NetBots[${netbotName}].PctHPs} <= ${healPct}) {
			/if (${healNames.Contains[${netbotName}]}==TRUE) /continue
			/if (${NetBots[${netbotName}].PctHPs} < ${lowestPct}) {
				/varset lowestPct ${NetBots[${netbotName}].PctHPs}
				/invoke ${healNames.Insert[0,${netbotName}]}
			} else {
				/invoke ${healNames.Append[${netbotName}]}
			}
		}
	/next i

	:doneWithNames

	/if (${healNames.Count}==0) /return NONE

	/vardata li healNames.First
	/while (${li.IsEnd}==FALSE) {
		/varset b ${b}${li.Value}
		/invoke ${li.Advance}
		/if (${li.IsEnd}==TRUE) /break
		/varset b ${b},
	}

	/return ${b}
/RETURN

SUB determineTeamIndex
	/if (${castTeamNetbots.Equal[NONE]}) /return

	/if (${checkTeamIndexTimer}>0) /return
	/varset checkTeamIndexTimer 2s

	/declare i int local
	/declare j int local
	/declare entryCount int local ${castTeamNetbotsList.Count}
	/declare entryName string local
	/declare myManaAmount int local
	/declare entryManaAmount int local

	/declare highMana int local 101

	| RESET TO 1 FOR THIS SCAN
	/varset teamIndex ${Math.Calc[${entryCount}]} 

	|/if (${Me.Stunned}==TRUE || ${Me.SpellInCooldown}==TRUE) {
	|   /varset teamIndex 100
	|   /return
	|}

	/for i 0 to ${Math.Calc[${entryCount}-1]}
		/varset entryName ${castTeamNetbotsList.Item[${i}]}
		/varset entryName ${entryName.Left[1]}${entryName.Right[-1].Lower}

		/varset myManaAmount ${Me.PctMana}
		/varset entryManaAmount ${NetBots[${entryName}].PctMana}

		| ARE THEY CASTING SOMETHING? BUMP ME UP
		|/if (${Bool[${NetBots[${entryName}].Casting}]}==TRUE) {
		|   /varset teamIndex ${Math.Calc[${teamIndex}-1]}
		|   /continue
		|}

		/if (${NetBots[${entryName}].InZone}==FALSE) {
			/varset teamIndex ${Math.Calc[${teamIndex}-1]}
			/continue
		}

		| IS ANOTHER NETBOT STUNNED OR DEAD? INCREASE MY PRIORITY
		/if (${Select[${NetBots[${entryName}].State},DEAD,STUN,HOVER]}>0) {
			/varset teamIndex ${Math.Calc[${teamIndex}-1]}
			/continue
		}

		| DOES MY MANA MATCH ANOTHER EXACTLY? ONE OF US NEEDS TO DEFER - USE ID AS A SEQUENTIAL DIFFERENTIATOR
		/if (${entryManaAmount}==${myManaAmount}) {
			| IF MY ID IS HIGHER THAN THEIR ID, INCREASE MY PRI
			/if (${Me.ID}>${Spawn[PC ${entryName}].ID}) {
				/varset teamIndex ${Math.Calc[${teamIndex}-1]}
			}
		} else /if (${myManaAmount} > ${entryManaAmount}) {
			/varset teamIndex ${Math.Calc[${teamIndex}-1]}
		}
	/next i

/RETURN
