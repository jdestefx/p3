SUB p3_autodebuff_init() {

	|/call p3RegisterCommand KILL autoMezKillCheck 5

	/declare autoDebuffIDsList list outer
	/declare autoDebuffCheckTimer timer outer 0
	/declare autoDebuffCheckTimerDelay int outer 10
	/declare autoDebuffMobRadius int outer 50
	/declare autoDebuffSpawnSearch string outer NPC LOS TARGETABLE

	/declare workingOnADebuff bool outer FALSE

	/declare autoDebuffEnabled bool outer TRUE

	/varset autoDebuffEnabled 				${getBotIniOption[AutoDebuff.AutoDebuffEnabled,${autoDebuffEnabled}]}
	/varset autoDebuffMobRadius 			${getBotIniOption[AutoDebuff.AutoDebuffRadius,${autoDebuffMobRadius}]}
	/varset autoDebuffSpawnSearch 			${getBotIniOption[AutoDebuff.AutoDebuffSpawnSearch,${autoDebuffSpawnSearch}]}

	/varset autoDebuffSpawnSearch ${autoDebuffSpawnSearch} RADIUS ${autoDebuffMobRadius} RANGE ${autoDebuffSpawnSearch}
	

	/declare autoDebuffSpellIDsList list outer
	/call populateAutoDebuffSpellIDs

	/if (${autoDebuffSpellIDsList.Count}==0) {
		/echo There are no AutoDebuff-flagged entries in [CombatSpells].  Disabling auto-debuff routines.
		/varset autoDebuffEnabled FALSE
	}

	/RETURN
}


SUB autodebuffMaintenance() {
	/if (${autoDebuffCheckTimer}>0) /return
	/varset autoDebuffCheckTimer ${autoDebuffCheckTimerDelay}

	/if (${autoDebuffEnabled}==FALSE) /RETURN
	/if (${Me.Casting.ID}!=NULL) /return

	/if (${Stick.Active}==TRUE || ${Nav.Active}==TRUE) /return

	/if (${workingOnADebuff}==FALSE) /call checkAutoDebuffs

	/RETURN
}

SUB checkAutoDebuffs() {
	/declare xspawn spawn local
	/declare i int local

	/declare targetEntry string local 
	/declare targetID string local 
	/declare targetZone string local 

	| SCAN AREA FOR NEW MOBS TO DEBUFF
	/declare mobCountInArea int local ${SpawnCount[${autoDebuffSpawnSearch}]}
	/if (${mobCountInArea}>0) {
		/for i 1 to ${mobCountInArea}
			/vardata xspawn NearestSpawn[${i},${autoDebuffSpawnSearch}]
			/if (${xspawn.Aggressive}==FALSE) /continue
			/call addTargetForDebuffs ${xspawn.ID}
		/next i
	}

	| CLEAN UP THE LIST OF ANYTHING DEAD/GONE, ETC
	/for i ${Math.Calc[${autoDebuffIDsList.Count}-1]} downto 0
		/varset targetEntry ${autoDebuffIDsList.Item[${i}]}
		/varset targetID ${targetEntry.Arg[1,_]}
		/varset targetZone ${targetEntry.Arg[2,_]}

		/if (${Zone.ID} != ${targetZone} || ${Spawn[id ${targetID}].Dead}==TRUE || ${SpawnCount[id ${targetID}]}==0) {
			/invoke ${autoDebuffIDsList.Erase[${i}]}
		}
	/next i

	/if (${autoDebuffIDsList.Count}==0) /RETURN

	/for i 0 to ${Math.Calc[${autoDebuffIDsList.Count}-1]}
		|/echo ${autoDebuffIDsList.Item[${i}]}
	/next i

	| PERFORM DEBUFFS
	/for i 0 to ${Math.Calc[${autoDebuffIDsList.Count}-1]}
		/if (${autoDebuffIDsList.Item[${i}].Arg[3,_].Equal[0]}) /continue
		/call processDebuffsForTargetID ${autoDebuffIDsList.Item[${i}].Arg[1,_]} TRUE
		/if (${Macro.Return}==TRUE) {
			/varset workingOnADebuff TRUE
			/break
		}
	/next i

	/varset workingOnADebuff FALSE

	/RETURN
}

SUB processDebuffsForTargetID(int targetID, bool requireAutoDebuffFlag) {
	/declare i int local
	/declare castName string local
	/declare spellTime int local
	/declare debuffResult string local
	/declare castSpell spell local
	/declare timerName string local
	/declare minDuration int local 30
	/declare requireMana int local 0
	/declare entry string local
	/declare maxResists int local 5
	/declare currentResistCount int local 0
	/declare cancelConditions string local
	/declare lifeDeficit int local
	/declare lifeRatio int local

	/for i 1 to ${iniCombatSpells.Size}
		/varset currentResistCount 0
		/varset cancelConditions

		/if (${iniCombatSpells[${i},1].Equal[-]}) /break

		/if (${requireAutoDebuffFlag}==TRUE) {
			/if (${iniCombatSpells[${i},${bAutoDebuff}].Equal[-]} && ${iniCombatSpells[${i},${bAutoDebuffOnly}].Equal[-]}) /continue
		}

		/if (${iniCombatSpells[${i},${bEnabled}].Equal[NO]}) /continue

		/if (${iniCombatSpells[${i},${bNotIfAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}>0 || ${persistentAssistTags.Count}>0) {
				/call validateAssistTags ${iniCombatSpells[${i},${bNotIfAssistTag}]} TRUE
				/if (${Macro.Return}==TRUE) /continue
			}
		}

		/if (${iniCombatSpells[${i},${bRequireAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}==0 && ${persistentAssistTags.Count}==0) /continue
			/call validateAssistTags ${iniCombatSpells[${i},${bRequireAssistTag}]}
			/if (${Macro.Return}==FALSE) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetOverLevel}].NotEqual[-]}) {
			/if (${Spawn[id ${targetID}].Level} < ${iniCombatSpells[${i},${bTargetOverLevel}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetUnderLevel}].NotEqual[-]}) {
			/if (${Spawn[id ${targetID}].Level} >= ${iniCombatSpells[${i},${bTargetUnderLevel}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bSelfOverMana}].NotEqual[-]}) {
			/if (${Me.PctMana} < ${iniCombatSpells[${i},${bSelfOverMana}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 45]} < ${iniCombatSpells[${i},${bRequireMobsInProximity}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireMaxMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 45]} > ${iniCombatSpells[${i},${bRequireMaxMobsInProximity}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireTargetClass}].NotEqual[-]}) {
			|/echo ${Spawn[id ${targetID}].Class.ShortName} === ${iniCombatSpells[${i},${bRequireTargetClass}]}
			|/echo ${Select[${Spawn[id ${targetID}].Class.ShortName},${iniCombatSpells[${i},${bRequireTargetClass}]}]}
			/if (${Select[${Spawn[id ${targetID}].Class.ShortName},${iniCombatSpells[${i},${bRequireTargetClass}]}]}==0) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetUnderHP}].NotEqual[-]}) {
			/call fullTarget ${targetID}
			/if (${Target.PctHPs} > ${iniCombatSpells[${i},${bTargetUnderHP}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetOverHP}].NotEqual[-]}) {
			/call fullTarget ${targetID}
			/if (${Target.PctHPs} <= ${iniCombatSpells[${i},${bTargetOverHP}]}) /continue
			/varset cancelConditions ${cancelConditions},TARGETUNDERHP:${iniCombatSpells[${i},${bTargetOverHP}]}
		}

		/if (${iniCombatSpells[${i},${bTargetLifeManaRatio}].NotEqual[-]}) {
			/call fullTarget ${targetID}
			/varset lifeDeficit ${Math.Calc[100-${Target.PctHPs}]}
			/varset lifeRatio ${iniCombatSpells[${i},${bTargetLifeManaRatio}]}
			/if (${Me.PctMana} < ${Math.Calc[100-(${lifeDeficit}/${lifeRatio})]}) {
				/continue
			}
		}

		/if (${iniCombatSpells[${i},${bMaxResists}].NotEqual[-]}) {
			/varset maxResists ${iniCombatSpells[${i},${bMaxResists}]}
		}

		/varset castName ${iniCombatSpells[${i},1]}

		/call getSpellFromCastRequest "${castName}"
		/vardata castSpell ${Macro.Return}

		/varset spellTime ${castSpell.Duration.TotalSeconds}

		/if (${iniCombatSpells[${i},${bRecastDelay}].NotEqual[-]}) {
			/varset spellTime ${Math.Calc[${spellTime} + ${iniCombatSpells[${i},${bRecastDelay}]}]}
		}

		/varset timerName detriTimer_${targetID}_${castSpell.ID}

		| ALREADY TIMER FOR IT? ITS LOCKED OUT...
		/if (${Defined[${timerName}]}==TRUE) {
			/if (${${timerName}}>0) /continue
			/deletevar ${timerName}
		}

		|/if (${FindItemCount[=${castName}]}>0) {
		|	/call waitForInt "FindItem[=${castName}].TimerReady" 0
		|}

		| LAST CHANCE DEAD/GONE CHECK
		/if (${Spawn[id ${targetID}].Dead}==TRUE || ${SpawnCount[id ${targetID}]}==0) /break

		/if (${iniCombatSpells[${i},${bMemInGem}].NotEqual[-]}) {
			/if (${Me.Gem[${castName}]}==NULL) {
				/call memorizeSpellInGem "${castName}" ${iniCombatSpells[${i},${bMemInGem}]}
			}
		}

		| A CAST WILL HAPPEN -- PRE-TARGET AND CHECK FOR BUFF
		/call fullTarget ${targetID}
		/if (${Target.Buff[${castSpell.Name}].ID}!=NULL) {
			/if (${Math.Calc[${Target.BuffDuration[${castSpell.Name}].TotalSeconds} * ${castSpell.MyDuration.TotalSeconds} > 0.3]}) {
				/varset spellTime ${Target.BuffDuration[${castSpell.Name}].TotalSeconds}
				/echo Detected debuff already on target. Locking out for duration: ${spellTime}
				/call lockout ${timerName} ${spellTime}
				/continue
			}
		}

		/if (${isCastReady[${castName}]}==FALSE) {
			/echo spell was not ready:  ${castName}
			/RETURN FALSE
		}
		
		/if (${iniCombatSpells[${i},${bDoCommand}].NotEqual[-]}) {
			/docommand ${iniCombatSpells[${i},${bDoCommand}]}
			/delay 3
		}

		|/echo invokecastadd: SPELL=${castName};TARGETID=${targetID};
		/invoke (${cq.Append[CAST;SPELL=${castName};TARGETID=${targetID};SRC=iniCombatSpells;SRCIDX=${i};ONRESULT=onDebuffCastResult]})		
		/RETURN TRUE
	/next i

	/RETURN FALSE
}

SUB populateAutoDebuffSpellIDs() {
	/declare i int local
	/declare cSpell spell local

	/for i 1 to ${iniCombatSpells.Size}
		/if (${iniCombatSpells[${i},1].Equal[-]}) /break
		/if (${iniCombatSpells[${i},${bEnabled}].Equal[NO]}) /continue
		/if (${iniCombatSpells[${i},${bAutoDebuff}].Equal[-]} && ${iniCombatSpells[${i},${bAutoDebuffOnly}].Equal[-]}) /continue

		/call getSpellFromCastRequest "${iniCombatSpells[${i},1]}"
		/vardata cSpell ${Macro.Return}

		/invoke ${autoDebuffSpellIDsList.Append[${cSpell.ID}]}
	/next i

	/return

}

SUB queueContainsCast(string castName, string targetID) {
	/declare i int local 

	/for i 0 to ${Math.Calc[${cq.Count}-1]}
		/if (${cq.Item[${i}].Find[SPELL=${castName}]}!=NULL && ${cq.Item[${i}].Find[TARGETID=${targetID}]}) /RETURN TRUE
	/next i
	
	/RETURN FALSE
}

SUB onDebuffCastResult(string result, string opts) {
	/varset workingOnADebuff FALSE

	/declare targetID string local ${getStackCommandOpt[${castingOpts},TARGETID]}
	/declare cSpellName string local ${getStackCommandOpt[${castingOpts},SPELL]}
	/declare cSpell spell local

	/call getSpellFromCastRequest "${cSpellName}"
	/vardata cSpell ${Macro.Return}

	/declare before string local ${targetID}_${Zone.ID}_${cSpell.ID}_0
	/declare after string local ${targetID}_${Zone.ID}_${cSpell.ID}_1

	/invoke ${autoDebuffIDsList.Replace[${before},${after}]}

	/RETURN
}


SUB addTargetForDebuffs(int targetID) {
	/declare i int local
	/declare spellID string local

	/for i 0 to ${Math.Calc[${autoDebuffSpellIDsList.Count}-1]}

		/varset spellID ${autoDebuffSpellIDsList.Item[${i}]}

		/if (${autoDebuffIDsList.Contains[${targetID}_${Zone.ID}_${spellID}_0]}) /continue
		/if (${autoDebuffIDsList.Contains[${targetID}_${Zone.ID}_${spellID}_1]}) /continue

		|/echo add: ${targetID}_${Zone.ID}_${autoDebuffSpellIDsList.Item[${i}]}_0
		/invoke ${autoDebuffIDsList.Append[${targetID}_${Zone.ID}_${autoDebuffSpellIDsList.Item[${i}]}_0]}
	/next i

	/RETURN
}


SUB xcheckAutoMez() {
	/if (${autoMezCheckTimer}>0) /RETURN FALSE
	/if (${Stick.Active}==TRUE) /RETURN FALSE
	/if (${Me.Moving}==TRUE) /RETURN FALSE
	/if (${Me.Invis}==TRUE) /RETURN FALSE

	/varset autoMezCheckTimer 5

}

SUB xmezCastRequestResult(string result, string opts) {
	/declare targetID int local ${getStackCommandOpt[${opts},TARGETID]}
	/declare cSpellName string local ${getStackCommandOpt[${opts},SPELL]}
	/declare cSpell spell local

	/call getSpellFromCastRequest "${cSpellName}"
	/vardata cSpell ${Macro.Return}

	/if (${result.Find[PASS_]}!=NULL) {
		/call lockout mez_${targetID} ${Math.Calc[${cSpell.Duration.TotalSeconds}-${autoMezTimerMarginSeconds}]}s
	}

	/RETURN
}
