SUB p3_heals_init {
	/call p3RegisterCommand TAGSCHANGED healsTagsChanged 50
	

	/declare healCheckTimer timer outer 0
	/declare teamIndex int outer 0
	/declare enableteamIndex bool outer TRUE
	/declare checkTeamIndexTimer timer outer 0
	/declare refreshHealTableTimer timer outer 0

	/declare HT_NETBOT int outer 1
	/declare HT_GROUP int outer 2
	/declare HT_XTAR int outer 4
	/declare HT_NETPET int outer 8
	|/declare HT_MYPET int outer 16

	/declare healthTable list outer

	/declare castTeamNetbots string outer NONE
	/declare castTeamNetbotsList list outer

	/declare autoRezEnabled bool outer FALSE
	/declare autoRezSpells string outer NONE
	/declare autoRezRequireHealIndex int outer 0

	/varset autoRezEnabled ${getBotIniOption[Misc.AutoRezEnabled,${autoRezEnabled}]}
	/varset autoRezSpells ${getBotIniOption[Misc.AutoRezSpells,${autoRezSpells}]}
	/varset autoRezRequireHealIndex ${getBotIniOption[Misc.AutoRezTeamIndex,${autoRezRequireHealIndex}]}

	/varset castTeamNetbots ${getBotIniOption[CastTeam.Members,${castTeamNetbots}]}
	/invoke ${castTeamNetbotsList.Append[${castTeamNetbots.Upper}]}
	/invoke ${castTeamNetbotsList.Remove[${Me.CleanName.Upper}]}

	/call validateSpellBasedArray iniHeals
	/call processArrayTargets iniHeals
	/call validateTagsFromArray iniHeals

	/RETURN
}

SUB healsTagsChanged() {
	/call validateTagsFromArray iniHeals
	/RETURN
}

SUB healsMaintenance() {

	/if (${healCheckTimer}==0) {
		/varset healCheckTimer 5

		/call determineTeamIndex
		
		/if (${Me.Casting.ID}!=NULL) /return
		/if (${Me.Invis}==TRUE) /RETURN
		/if (${Me.Silenced.ID}!=NULL) /RETURN
		/if (${Me.Moving}==TRUE && ${canCastWhileMoving}==FALSE) /return

		/call checkHeals

		/if (${Macro.Return}==TRUE) /RETURN RESET

		/if (${autoRezEnabled}==TRUE) {
			/call checkAutoRez
		}
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
	/declare healMask int local ${Math.Calc[${HT_NETBOT}|${HT_GROUP}]}
	/declare getHealthTableIndex int local 1
	/declare actionTaken bool local FALSE

	/invoke ${botBuffIDs.Delimiter[ ]}

	/echo my team index: ${teamIndex}


	/call refreshHealTables

	/for i 1 to ${iniHeals.Size}
		/varset healMask 1
		/varset triggerIdleCasting TRUE
		/varset cancelConditions
		/varset getHealthTableIndex 0
		/invoke ${botBuffIDs.Clear}
		/invoke ${netbotsReadyForHeal.Clear}

		/if (${iniHeals[${i},1].Equal[-]}) /break
		/if (${iniHeals[${i},${bEnabled}].Equal[NO]}) /continue
		/if (${iniHeals[${i},${bTagsInvalidated}].Equal[YES]}) /continue

		/varset healSpellName ${iniHeals[${i},1]}

		/if (${iniHeals[${i},${bRequireCastIdle}].NotEqual[-]}) {
			/if (${idleCastingTimer}>0) /continue
			/varset triggerIdleCasting FALSE
		}

		|/if (${iniHeals[${i},${bNotIfAssistTag}].NotEqual[-]}) {
		|	/if (${assistTags.Count}>0 || ${persistentAssistTags.Count}>0) {
		|		/call validateAssistTags ${iniHeals[${i},${bNotIfAssistTag}]} TRUE
		|		/if (${Macro.Return}==TRUE) /continue
		|	}
		|}
		|/if (${iniHeals[${i},${bRequireAssistTag}].NotEqual[-]}) {
		|	/if (${assistTags.Count}==0 && ${persistentAssistTags.Count}==0) /continue
		|	/call validateAssistTags ${iniHeals[${i},${bRequireAssistTag}]}
		|	/if (${Macro.Return}==FALSE) /continue
		|}


		/if (${iniHeals[${i},${bRequireTeamIndex}].NotEqual[-]}) {
			/if (${teamIndex}!=${iniHeals[${i},${bRequireTeamIndex}]}) /continue
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

		/if (${healSpell.TargetType.Equal[Self]}) {
			/if (${Me.PctHPs}>${healPct}) /continue
			/vardata healSpawn Spawn[id ${Me.ID}]
		} else {
			/if (${iniHeals[${i},${bUseTeamIndex}].NotEqual[-]}) /varset getHealthTableIndex ${teamIndex}

			/if (${iniHeals[${i},${bIncludeNetBotPets}].NotEqual[-]}) /varset healMask ${Math.Calc[${healMask}|${HT_NETPET}].Int}
			/if (${iniHeals[${i},${bIncludeXTargets}].NotEqual[-]}) /varset healMask ${Math.Calc[${healMask}|${HT_XTAR}].Int}
			/if (${iniHeals[${i},${bOnlyXTargets}].NotEqual[-]}) /varset healMask ${HT_XTAR}
			/if (${iniHeals[${i},${bOnlyNetBotPets}].NotEqual[-]}) /varset healMask ${HT_NETPET}
			
			/call getHealthTableEntry ${getHealthTableIndex} ${healMask} ${healPct} ${i}
			/if (${Macro.Return.Equal[-]}) /continue

			/echo healspawn: ${Macro.Return}
			/vardata healSpawn Spawn[=${Macro.Return}]

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

		/if (${iniHeals[${i},${bCancelAtHP}].NotEqual[-]}) {
			/varset cancelHealHP ${iniHeals[${i},${bCancelAtHP}]}
			/if (${cancelHealHP}==0) /varset cancelHealHP ${healPct}
			/varset cancelConditions TARGETOVERHP:${cancelHealHP}|${cancelConditions}
		}

		/if (${iniHeals[${i},${bNoCancelChecks}].NotEqual[-]}) {
			/varset cancelConditions
		}

		/if (${iniHeals[${i},${bMemInGem}].NotEqual[-]} && ${Me.Gem[${healSpellName}]}==NULL) {
			/call memorizeSpellInGem "${healSpellName}" ${iniHeals[${i},${bMemInGem}]}
		}

		/if (${isCastReady[${healSpellName}]}==FALSE) {
			/continue
		}

		/if (${iniHeals[${i},${bDoCommand}].NotEqual[-]}) /docommand ${iniHeals[${i},${bDoCommand}]}

		/bc ${healSpellName} => ${healSpawn}  (MTI: ${teamIndex})  RTI? ${iniHeals[${i},${bRequireTeamIndex}]} (Pct: ${iniHeals[${i},${bHealPct}]})
		/invoke (${cq.Append[CAST;SPELL=${healSpellName};HEALSPELLID=${healSpell.ID};TARGETID=${healSpawn.ID};CANCELCONDITIONS=${cancelConditions.Left[-1]};SRC=iniHeals;SRCIDX=${i}]})
		/RETURN TRUE

	/next i

	/RETURN FALSE
}


SUB refreshHealTables {
	|/if (${refreshHealTableTimer}>0) /return
	|/varset refreshHealTableTimer 3

	/declare i int local
	/declare m int local
	/declare netbotName string local
	/declare netbotCount ${NetBots.Counts}
	/declare trackIDs list local
	/declare spawnID int local
	/declare searchIndex int local
	/declare mask int local

	/invoke ${healthTable.Clear}

	/for i 1 to ${netbotCount}
		/varset netbotName ${NetBots.Client[${i}]}
		/if (${NetBots[${netbotName}].InZone}==FALSE) /continue
		/if (${netbotName.Equal[NULL]}) /break
		/if (${Spawn[${netbotName}].Dead}==TRUE) /continue
		/if (${NetBots[${netbotName}].State.Equal[DEAD]}==TRUE || ${NetBots[${netbotName}].State.Equal[STUN]} && ${NetBots[${netbotName}].CurrentHPs} < -1) /continue
		/if (${NetBots[${netbotName}].Invulnerable}==1) /continue

		/invoke ${healthTable.Append[${strPad[${NetBots[${netbotName}].PctHPs}]}:${netbotName}:1]}
		/invoke ${trackIDs.Append[${NetBots[${netbotName}].ID}]}

		/if (${NetBots[${netbotName}].PetID}!=NULL) {
			/invoke ${healthTable.Append[${strPad[${NetBots[${netbotName}].PetHP}]}:${Spawn[id ${NetBots[${netbotName}].PetID}].CleanName}:8]}
			/invoke ${trackIDs.Append[${NetBots[${netbotName}].PetID}]}
		}
	/next i

	/if (${Me.Grouped}==TRUE) {
		/for i 0 to ${Group.Members}
			/if (${Group.Member[${i}].OtherZone}==TRUE) /continue
			/if (${Group.Member[${i}].Spawn.Dead}==TRUE) /continue

			/varset spawnID ${Group.Member[${i}].ID}
			/varset searchIndex ${trackIDs.Index[${spawnID}]}
			/if (${searchIndex}==-1) {
				/invoke ${healthTable.Append[${strPad[${Group.Member[${i}].PctHPs}]}:${Group.Member[${i}].CleanName}:2]}
				/invoke ${trackIDs.Append[${spawnID}]}
			}
		/next i
	}

	/for i 1 to 12
		/if (${Me.XTarget[${i}].ID}==NULL) /continue
		/if (${Me.XTarget[${i}].ID}==0) /continue
		/if (${Me.XTarget[${i}].Type.NotEqual[PC]} && ${Me.XTarget[${i}].Type.NotEqual[Pet]}) /continue
		/if (${Me.XTarget[${i}].Type.Equal[Pet]} && ${Me.XTarget[${i}].Master.Type.NotEqual[PC]}) /continue

		/varset spawnID ${Me.XTarget[${i}].ID}
		/varset searchIndex ${trackIDs.Index[${spawnID}]}
		/if (${searchIndex}==-1) {
			/invoke ${healthTable.Append[${strPad[${Me.XTarget[${i}].PctHPs}]}:${Me.XTarget[${i}].CleanName}:4]}
			/invoke ${trackIDs.Append[${spawnID}]}
		}
	/next i

	/invoke ${healthTable.Sort}

	/return

	/echo =====
	/echo ${healthTable.Item[0]}
	/echo ${healthTable.Item[1]}
	/echo ${healthTable.Item[2]}
	/echo ${healthTable.Item[3]}
	/echo ${healthTable.Item[4]}
	/echo ${healthTable.Item[5]}
	/echo ${healthTable.Item[6]}
	/echo ${healthTable.Item[7]}
	/echo ${healthTable.Item[8]}
	/echo ${healthTable.Item[9]}
	/echo ${healthTable.Item[10]}
	/echo ${healthTable.Item[11]}
	/echo ${healthTable.Item[12]}
	/echo ${healthTable.Item[13]}
	/echo ${healthTable.Item[14]}
	/echo ${healthTable.Item[15]}
	/echo ${healthTable.Item[16]}
	/echo ${healthTable.Item[17]}

	/RETURN
}

SUB strPad(string i) {
	/if (${i}>99) /return ${i}
	/if (${i}>9 && ${i}<100) /return 0${i}
	/if (${i}<9 && ${i}>0) /return 00${i}
	/if (${i}<0) /return 000

	/echo OOPS: ${i}
	|/beep
	/RETURN 000
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

SUB getHealthTableEntry(int teamIndexOffset, string mask, string pctHP, int iniIndex) {
	/declare i int local 
	/declare hp string local
	/declare name string local
	/declare m string local
	/declare e string local
	/declare matchCount int local 

	/varset teamIndexOffset ${Math.Calc[${teamIndexOffset}+1]}

	/for i 0 to ${Math.Calc[${healthTable.Count}-1]}
		/varset e ${healthTable.Item[${i}]}

		/varset name ${e.Token[2,:]}
		/varset m ${e.Token[3,:]}
		/varset hp ${e.Token[1,:]}

		|/echo eval: ${name} ${hp} ${mask} & ${m} 

		/if (${hp}>${pctHP}) /continue

		|/if (${Math.Calc[${mask}&${m}]}==0) /continue

		/if (${m} == 1 && ${iniHeals[${iniIndex},${bTargets}].NotEqual[-]}) {
			/if (${Select[${name},${iniHeals[${iniIndex},${bTargets}]}]}==0) /continue
		}

		/if (${iniHeals[${iniIndex},${bCheckShortBuffs}].NotEqual[-]}) {
			/if (${mask}!=1) /continue
			/call getSpellFromCastRequest "${iniHeals[${iniIndex},1]}"
			/if (${Macro.Return.Equal[UNKNOWN]}) /continue
			/if (${NetBots[${name}].ShortBuff.Find[${${Macro.Return}.ID}]}!=0) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequireDiseased}].NotEqual[-]}) {
			/if (${mask}!=1) /continue
			/if (${NetBots[${name}].Diseased} == 0) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequirePoisoned}].NotEqual[-]}) {
			/if (${mask}!=1) /continue
			/if (${NetBots[${name}].Poisoned} == 0) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequireCursed}].NotEqual[-]}) {
			/if (${mask}!=1) /continue
			/if (${NetBots[${name}].Cursed} == 0) /continue
		}

		| bRequireHurtPartyMembers implies group memeber, so verify
		/if (${iniHeals[${iniIndex},${bRequireHurtPartyMembers}].NotEqual[-]}) {
			/if (${Group.Member[${name}].ID}==NULL) /continue
		}

		/if (${iniHeals[${iniIndex},${bRequireGroup}].NotEqual[-]}) {
			/if (${Group.Member[${name}].ID}==NULL) /continue
		}

		/if (${iniHeals[${iniIndex},${bTargetUnderLevel}].NotEqual[-]}) {
			/if (${Spawn[=${name}].Level} >= ${iniHeals[${iniIndex},${bTargetUnderLevel}]}) /continue
		}

		/if (${iniHeals[${iniIndex},${bTargetOverLevel}].NotEqual[-]}) {
			/if (${Spawn[=${name}].Level} <= ${iniHeals[${iniIndex},${bTargetOverLevel}]}) /continue
		}

		/varset matchCount ${Math.Calc[${matchCount}+1]}
		/if (${matchCount}==${teamIndexOffset}) /return ${name}
	/next i

	/RETURN -
}

SUB getNetBotReadyForHeal(int healPct, int iniIndex)
	/declare i int local 0
	/declare netbotName string local
	/declare netbotCount int local ${NetBots.Counts}
	/declare lowestPct int local 100
	/declare healNames list local
	/declare li listiterator local
	/declare b string local
	
	/if (${iniHeals[${iniIndex},${bIncludeXTargets}].NotEqual[-]} || ${iniHeals[${iniIndex},${bOnlyXTargets}].NotEqual[-]})  {
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

	/if (${iniHeals[${iniIndex},${bOnlyXTargets}].NotEqual[-]}) /goto :doneWithNames

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
	/varset checkTeamIndexTimer 5s

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

		/varset myManaAmount ${Me.CurrentMana}
		/varset entryManaAmount ${NetBots[${entryName}].CurrentMana}
		|/varset myManaAmount ${Me.PctMana}
		|/varset entryManaAmount ${NetBots[${entryName}].PctMana}

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
		/if (${Select[${NetBots[${entryName}].State},DEAD,HOVER]}>0) {
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



SUB checkAutoRez() {
	/if (${Me.Moving}==TRUE) /RETURN
	/if (${Stick.Active}==TRUE) /RETURN
	/if (${Me.Silenced.ID}!=NULL) /RETURN
	/if (${Me.Invis}==TRUE) /RETURN

	/declare xSpawnID int local
	/declare corpseSearch string local pccorpse radius 150
	/declare useRezSpell string local
	/declare x int local
	/declare allcorpses[100] spawn local
	/declare corpseCount int local

	/if (${autoRezRequireHealIndex}!=0) {
		/if (${healIndex}<${autoRezRequireHealIndex}) /return
	}

	/vardata corpseCount SpawnCount[${corpseSearch}]
	/if (${corpseCount}==0) /return FALSE

	/for x 1 to ${corpseCount}
		/if (${isLockoutExpired[rez_${NearestSpawn[${x},${corpseSearch}].ID}]}==FALSE) {
			/continue
		} else {
			/varset xSpawnID ${NearestSpawn[${x},${corpseSearch}].ID}
		}
	/next x

	/if (${xSpawnID}==NULL) /return
	
	/call getReadyRezSpell
	/if (${Macro.Return.Equal[-]}) /return

	/bc AutoRez:  ${xSpawnID} ${Spawn[id ${xSpawnID}].Name}
	/invoke (${cq.Append[CAST;SPELL=${Macro.Return};TARGETID=${xSpawnID};ONRESULT=onRestCastResult;]})
	
	/RETURN
}


SUB onRestCastResult(string result, string opts) {

	/if (${result.Find[PASS_]}!=NULL) {
		/echo create lockout: rez_${xSpawnID}
		/call lockout rez_${xSpawnID} 45s

	}
	
	/RETURN
}

SUB getReadyRezSpell() {
	/declare i int local 

	/if (${autoRezSpells.Find[,]}==0) {
		/if (${isCastReady[${autoRezSpells}]}==FALSE) {
			/return -
		} else {
			/return ${autoRezSpells}
		}
	} else {
		/for i 1 to ${Math.Calc[${autoRezSpells.Count[,]}+1]}
			/if (${isCastReady[${autoRezSpells.Token[${i},,]}]}==TRUE) {
				/return ${autoRezSpells.Token[${i},,]}
			}
		/next i
	}

	/RETURN -
}