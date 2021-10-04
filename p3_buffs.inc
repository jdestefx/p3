SUB p3_buffs_init {
	/declare updateNetnoteTimer timer outer 0
	/declare buffCheckTimer timer outer 30
	/declare buffsEnabled bool outer true

	/call validateSpellBasedArray iniBuffs
	/call processArrayTargets iniBuffs

	/netnote

	/RETURN
}

SUB buffsMaintenance() {
	/if (${buffsEnabled}==FALSE) /return

	/call updateNetnoteBuffDurations

	/if (${Me.Invis}==TRUE) /return
	/if (${Stick.Active}==TRUE && ${combatStateTimer}==0) /return
	/if (${Me.Casting.ID} !=0 && ${Twist.Twisting}==FALSE) /return
	/if (${movingTimer}>0 && ${canCastWhileMoving}==FALSE) /return

	/call checkBuffs
	/RETURN
}

SUB updateNetnoteBuffDurations {
	/if (${updateNetnoteTimer}>0) /return
	/varset updateNetnoteTimer 5

	/declare i int local
	/declare buffDurations string local

	/for i 1 to 30
		/if (${Me.Buff[${i}].ID}==NULL) /continue
		/varset buffDurations ${buffDurations};${Me.Buff[${i}].Spell.ID}_${Me.Buff[${i}].Duration.TotalSeconds}
	/next i

	/netnote ${buffDurations}

	/RETURN
}

SUB checkBuffs {
	/if (${buffCheckTimer}>0) /return
	/varset buffCheckTimer 15

	|/echo buffecheck: combatTimer: ${combatStateTimer}

	/declare i int local 
	/declare targetNames list local 
	/declare targetName string local
	/declare buffCastName string local
	/declare buffSpell spell local
	/declare buffSpellID int local
	/declare netbotName string local
	/declare checkSpellID int local 
	/declare buffSpawn spawn local
	/declare noholdTimerName string local
	/declare buffSpellRange int local
	/declare cancelCheck string local
	/declare note string local 
	/declare cancelConditions string local

	/declare j int local
	/declare k int local

	/for i 1 to ${iniBuffs.Size}
		/invoke ${targetNames.Clear}

		/if (${iniBuffs[${i},${bTargets}].Length}==0) /continue

		/if (${iniBuffs[${i},1].Equal[-]}) /break
		/if (${iniBuffs[${i},${bEnabled}].Equal[NO]}) /continue

		/if (${combatStateTimer}>0) {
			/if (${iniBuffs[${i},${bCastableDuringAssist}].Equal[-]}) /continue
		}

		/if (${iniBuffs[${i},${bNotIfAssistTag}].NotEqual[-]}) {
			/if (${tempCombatTags.Count}>0 || ${assistTags.Count}>0) {
				/call validateAssistTags ${iniBuffs[${i},${bNotIfAssistTag}]} TRUE
				/if (${Macro.Return}==TRUE) /continue
			}
		}

		/if (${iniBuffs[${i},${bRequireAssistTag}].NotEqual[-]}) {
			/if (${tempCombatTags.Count}==0 && ${assistTags.Count}==0) /continue
			/call validateAssistTags ${iniBuffs[${i},${bRequireAssistTag}]}
			/if (${Macro.Return}==FALSE) /continue
		}

		/if (${iniBuffs[${i},${bNotIfMounted}].NotEqual[-]}) {
			/if (${Me.Mount.ID}!=NULL) /continue
		}
		/if (${iniBuffs[${i},${bRequireCastIdle}].NotEqual[-]}) {
			/if (${idleCastingTimer}>0) /continue
		}
		/if (${iniBuffs[${i},${bRequireAssistTarget}].NotEqual[-]}) {
			/if (${assistSpawnID}==0) /continue
		}
		/if (${iniBuffs[${i},${bSelfOverHP}].NotEqual[-]}) {
			/if (${Me.PctHPs} < ${iniBuffs[${i},${bSelfOverHP}]}) /continue
		}
		/if (${iniBuffs[${i},${bSelfUnderHP}].NotEqual[-]}) {
			/if (${Me.PctHPs} >= ${iniBuffs[${i},${bSelfUnderHP}]}) /continue
		}
		/if (${iniBuffs[${i},${bSelfOverMana}].NotEqual[-]}) {
			/if (${Me.PctMana} < ${iniBuffs[${i},${bSelfOverMana}]}) /continue
		}
		/if (${iniBuffs[${i},${bSelfUnderMana}].NotEqual[-]}) {
			/if (${Me.PctMana} >= ${iniBuffs[${i},${bSelfUnderMana}]}) /continue
		}
		/if (${iniBuffs[${i},${bCastableOnlyResting}].NotEqual[-]}) {
			/if (${combatStateTimer}>0) /continue
		}
		/if (${iniBuffs[${i},${bCastOnlyDuringAssist}].NotEqual[-]}) {
			/if (${combatStateTimer}==0) /continue
		}
		/if (${iniBuffs[${i},${bSelfUnderEnd}].NotEqual[-]}) {
			/if (${Me.PctEndurance} >= ${iniBuffs[${i},${bSelfUnderEnd}]} ) /continue
		}
		/if (${iniBuffs[${i},${bSelfOverEnd}].NotEqual[-]}) {
			/if (${Me.PctEndurance} < ${iniBuffs[${i},${bSelfUnderEnd}]} ) /continue
		}
		/if (${iniBuffs[${i},${bRequireMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 45]} < ${iniBuffs[${i},${bRequireMobsInProximity}]}) /continue
		}
		/if (${iniBuffs[${i},${bRequireUW}].NotEqual[-]}) {
			/if (${Me.FeetWet}==FALSE) /continue
		}

		| OBTAIN BUFF SPELL DETAIL
		/varset buffCastName ${iniBuffs[${i},${bName}]}

		/call getSpellFromCastRequest "${buffCastName}"
		/vardata buffSpell ${Macro.Return}
		/varset buffSpellID ${buffSpell.ID}
		
		/if (${iniBuffs[${i},${bForceSpellID}].NotEqual[-]}) {
			/varset buffSpellID ${iniBuffs[${i},${bForceSpellID}]}
		}
		
		/if (${iniBuffs[${i},${bIgnoreTargetType}].Equal[-]}) {
			/if (${buffSpell.TargetType.Equal[Pet]} || ${iniBuffs[${i},${bTargetType}].Upper.Equal[PET]} ) {
				/if (${Me.Pet.ID}==NULL) /continue
				/if (${Me.PetBuff[${buffSpell.Name}]}!=NULL) /continue
				/invoke ${targetNames.Append[${Me.Pet.Name}]}
				/goto :skipNetBots
			} else /if (${buffSpell.TargetType.Equal[Self]}) {
				/invoke ${targetNames.Append[${Me.CleanName}]}
			} else {
				/invoke ${targetNames.Append[${iniBuffs[${i},${bTargets}]}]}
			}
		} else {
			/invoke ${targetNames.Append[${iniBuffs[${i},${bTargets}]}]}
		}

		/if (${buffSpell.TargetType.Equal[Group v1]}) /varset buffSpellRange ${buffSpell.AERange}
		/if (${buffSpell.TargetType.Equal[Group v2]}) /varset buffSpellRange ${buffSpell.AERange}
		/if (${buffSpell.TargetType.Equal[Single]}) /varset buffSpellRange ${buffSpell.MyRange}
		/if (${buffSpell.TargetType.Equal[Self]} && ${iniBuffs[${i},${bIgnoreTargetType}].NotEqual[-]}) /varset buffSpellRange 50

		| TARGETNAMES ARE ALL POSSIBLE TARGETS REGARDLESS OF WHETHER OR NOT THEY HAVE THE BUFF.
		| THIS LOOP REMOVES PEOPLE WHO DONT QUALIFY FOR A CAST, TYPICALLY BECAUSE THEY HAVE THE
		| BUFF. BUFF ENTRIES WITH REBUFFSECONDS MAY KEEP PEOPLE IN THE LIST WHO ALREADY HAVE
		| THE BUFF IF THE TIME ON THE BUFF IS LOW ENOUGH.

		/for j ${Math.Calc[${targetNames.Count}-1]} downto 0
			/varset targetName ${targetNames.Item[${j}]}
			/if (${targetName.Length}==0) /continue

			/if (${Spawn[${targetName}].ID}==NULL) {
				/invoke ${targetNames.Remove[${targetName}]}
				/continue
			}

			/if (${NetBots[${targetName}].State.Equal[DEAD]}) /continue

			| HAVE THE BUFF?
			/if (${Select[${buffSpellID},${NetBots[${targetName}].Buff}]}>0) {

				| REBUFF SECONDS DONT MATER? REMOVE THIS PERSON FROM BUFF CANDIDATES
				/if (${iniBuffs[${i},${bRebuffSeconds}].Equal[-]}) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}

				| REBUFFSECONDS ENTRY PRESENT, EXAMINE NETNOTE

				| EMPTY NETNOTE.  REMOVE FROM CANDIDATES SINCE THEY HAVE THE BUFF
				/if (${NetBots[${targetName}].Note.Length}==0) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}

				| NO NETNOTE FOR SPELL FOUND - ASSUME PERSON NEEDS BUFF - LEAVE THEM IN LIST

				/if (${NetBots[${targetName}].Note.Find[;${buffSpellID}_]}==NULL) /continue
				
				| NETNOTE FOR SPELLID EXISTS, CHECK
				/varset note ${NetBots[${targetName}].Note}
				/varset note ${note.Right[ -${note.Find[;${buffSpellID}_]} ]}

				| REMAINING BUFF TIME IS SUFFICIENT, REMOVE PLAYER FROM BUFF CANDIDATES
				/if (${note.Token[1,;].Token[2,_]} > ${iniBuffs[${i},${bRebuffSeconds}]}) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}

				| REMAINING TIME IS NOT SUFFICIENT, LEAVE THEM IN THE LIST BY DOING NOTHING
			}

			/if (${Select[${buffSpellID},${NetBots[${targetName}].ShortBuff}]}>0) {
				/invoke ${targetNames.Remove[${targetName}]}
				/continue
			}

			/if (${NetBots[${targetName}].InZone}==FALSE) { 
				/invoke ${targetNames.Remove[${targetName}]}
				/continue
			}

			/if (${iniBuffs[${i},${bExtendedStackCheck}].NotEqual[-]}) {
				/dquery ${targetName} -q "Spell[${buffSpell}].Stacks" 
				|/echo res: ${DanNet.Q}  ${buffSpell}
				/if (${DanNet.Q}==FALSE) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${iniBuffs[${i},${bRequireGroupInRange}].NotEqual[-]}) {
				/if (${targetName.NotEqual[${Me.CleanName}]} && ${Group.Member[${targetName}].ID}==NULL) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}

				/if (${Spawn[pc ${targetName}].Distance} > ${buffSpellRange}) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${iniBuffs[${i},${bTargetUnderHP}].NotEqual[-]}) {
				/if (${NetBots[${targetName}].PctHPs} > ${iniBuffs[${i},${bTargetUnderHP}]}) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${iniBuffs[${i},${bTargetOverHP}].NotEqual[-]}) {
				/if (${NetBots[${targetName}].PctHPs} <= ${iniBuffs[${i},${bTargetOverHP}]}) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${iniBuffs[${i},${bNotIfShortBuff}].NotEqual[-]}) {
				/varset checkSpellID ${Spell[${iniBuffs[${i},${bNotIfShortBuff}]}].ID}
				/if (${Select[${checkSpellID},${NetBots[${targetName}].ShortBuff}]}>0) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${iniBuffs[${i},${bNotIfLongBuff}].NotEqual[-]}) {
				/varset checkSpellID ${Spell[${iniBuffs[${i},${bNotIfLongBuff}]}].ID}
				/if (${Select[${checkSpellID},${NetBots[${targetName}].Buff}]}>0) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${iniBuffs[${i},${bTargetOverLevel}].NotEqual[-]}) {
				/if (${NetBots[${targetName}].Level} < ${iniBuffs[${i},${bTargetOverLevel}]}) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${iniBuffs[${i},${bTargetUnderLevel}].NotEqual[-]}) {
				/if (${NetBots[${targetName}].Level} >= ${iniBuffs[${i},${bTargetUnderLevel}]}) {
					/invoke ${targetNames.Remove[${targetName}]}
					/continue
				}
			}

			/if (${Spawn[pc ${targetName}].Distance} > ${buffSpellRange}) {
				/invoke ${targetNames.Remove[${targetName}]}
				/continue
			}
		/next j

		/if (${targetNames.Count}==0) {
			/if (${iniBuffs[${i},${bRevertToBandolier}].NotEqual[-]}) {
				/if (${bandolierProfileIsActive[${iniBuffs[${i},${bRevertToBandolier}]}]}==FALSE) {
					/band activate ${iniBuffs[${i},${bRevertToBandolier}]}
				}
			}
			/continue
		}

		/if (${iniBuffs[${i},${bActivateBandolier}].NotEqual[-]}) {
			/if (${bandolierProfileIsActive[${iniBuffs[${i},${bActivateBandolier}]}]}==FALSE) {
				/band activate ${iniBuffs[${i},${bActivateBandolier}]}
			}
		}

		/if (${iniBuffs[${i},${bRequireGroupInRange}].NotEqual[-]}) {
			/for k 1 to ${Group.Members}
				/if (${Group.Member[${k}].ID}==NULL) /continue
				/if (${Group.Member[${k}].Distance} > ${buffSpellRange}) /continue
			/next k
		}

		:skipNetBots

		/if (${iniBuffs[${i},${bUseTeamIndex}].NotEqual[-]}) {
			/if (${targetNames.Count}<${teamIndex}) /continue
			/if (${Spawn[pc ${targetNames.Item[${teamIndex}]}].ID}==NULL) /continue
			/vardata buffSpawn Spawn[pc ${targetNames.Item[${teamIndex}]}]
		} else {
			/if (${iniBuffs[${i},${bTargetType}].Upper.Equal[PET]}) {
				/vardata buffSpawn Spawn[pcpet ${targetNames.Item[0]}]
			} else {
				/vardata buffSpawn Spawn[pc ${targetNames.Item[0]}]
			}
		}
		
		/if (${buffSpawn.ID}==NULL) /continue

		/if (${isLockoutExpired[noHoldTimer_${buffSpawn.ID}_${buffSpellID}]}==FALSE) /break

		/if (${iniBuffs[${i},${bNoCast}].NotEqual[-]}) /continue

		/if (${iniBuffs[${i},${bMemInGem}].NotEqual[-]} && ${Me.Gem[${buffCastName}]}==NULL) {
			/call memorizeSpellInGem "${buffCastName}" ${iniBuffs[${i},${bMemInGem}]}
		}

		/if (${isCastReady[${buffCastName}]}==FALSE) {
			/continue
		}
		
		/if (${iniBuffs[${i},${bDoCommand}].NotEqual[-]}) {
			/docommand ${iniBuffs[${i},${bDoCommand}]}
			/delay 3
		}

		/invoke (${cq.Append[CAST;SPELL=${buffCastName};TARGETID=${buffSpawn.ID};BUFFSPELLID=${buffSpellID};CANCELCONDITIONS=${cancelConditions};SRC=iniBuffs;SRCIDX=${i};ONRESULT=onBuffCastResult]})
		/break

	/next i

	/RETURN
}

SUB onBuffCastResult(string result, string opts) {
	/echo BuffCR: ${result} ${opts}

	/declare i int local ${getStackCommandOpt[${opts},SRCIDX]}
	/declare targetID int local ${getStackCommandOpt[${opts},TARGETID]}
	/declare buffSpellID ${getStackCommandOpt[${opts},BUFFSPELLID]}

	/if (${iniBuffs[${i},${bNoSitTimer}].NotEqual[-]}) {
		/call pauseAutoMed ${iniBuffs[${i},${bNoSitTimer}]}
	}

	/if (${result.Equal[FAIL_CAST_TAKEHOLD]}==TRUE) {
		/call lockout noHoldTimer_${buffSpawn.ID}_${buffSpellID}
	}

	/if (${result.Equal.Equal[PASS_CAST_SUCCESS]}) {
		/if (${NetBots[${Spawn[id ${targetID}].CleanName}].Level}!=NULL) {
			/bc tell ${NetBots[${Spawn[id ${targetID}].CleanName} NETNOTE
			/squelch /bct ${buffSpawn.CleanName} //netnote
		}
	}


	/RETURN
}

