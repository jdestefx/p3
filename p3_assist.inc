SUB p3_assist_init {
	/call p3RegisterCommand STOP assistStop 50
	/call p3RegisterCommand FOLLOW assistFollow 50
	/call p3RegisterCommand KILL assistKill 50
	/call p3RegisterCommand KILL assistKillNameCheck 1
	/call p3RegisterCommand KILL assistKillLockCheck 5

	/declare assistSpawnID int outer 0
	/declare combatStateCheckTimer timer outer 0
	/declare combatStateTimer timer outer 0
	/declare combatStateTimerMax int outer 50
	/declare combatStateLongTimer timer outer 0
	/declare combatStateLongTimerMax int outer 150
	/declare assistTime timer outer 0
	/declare assistLock bool outer FALSE
	/declare assistMustHaveName string outer -
	/declare assistMustNotHaveName string outer -

	/declare autoAssistHoldTimer timer outer 0
	/declare autoAssistCheckTimer timer outer 0

	/declare autoAssist bool outer 				TRUE
	/declare maxAssistDistance int outer 		500
	/declare useAutoFire bool outer				FALSE
	/declare autoAssistPercent int outer		99
	/declare useEQCombatIndicator bool outer	FALSE

	/varset autoAssist 				${getBotIniOption[Assist.AutoAssist,${autoAssist}]}
	/varset autoAssistPercent		${getBotIniOption[Assist.AutoAssistPercent,${autoAssistPercent}]}
	/varset maxAssistDistance 		${getBotIniOption[Assist.MaxAssistDistance,${maxAssistDistance}]}
	/varset useAutoFire 			${getBotIniOption[Assist.UseAutoFire,${useAutoFire}]}
	/varset assistMustHaveName 		${getBotIniOption[Assist.MustHaveName,${assistMustHaveName}]}
	/varset assistMustNotHaveName 	${getBotIniOption[Assist.MustNotHaveName,${assistMustNotHaveName}]}
	/varset useEQCombatIndicator	${getBotIniOption[Misc.CombatStateUsesGameIndicator,${useEQCombatIndicator}]}

	/RETURN
}

SUB assistKillNameCheck(string cmd, string opts) {
	/declare checkSpawnID int local


	/if (${assistMustHaveName.NotEqual[FALSE]}) {
		/varset checkSpawnID ${getStackCommandOpt[${opts},SPAWNID]}
		/if (${Spawn[id ${checkSpawnID}].CleanName.Upper.Find[${assistMustHaveName.Upper}]}==NULL) /RETURN RESET
	}

	/if (${assistMustNotHaveName.NotEqual[FALSE]}) {
		/varset checkSpawnID ${getStackCommandOpt[${opts},SPAWNID]}
		/if (${Spawn[id ${checkSpawnID}].CleanName.Upper.Find[${assistMustNotHaveName.Upper}]}!=NULL) /RETURN RESET
	}

	/RETURN
}

SUB assistKillLockCheck(string cmd, string opts) {
	|/if (${assistLock}==TRUE) /RETURN RESET
	/RETURN
}

SUB assistKill(string cmd, string opts) {
	/varset assistSpawnID ${getStackCommandOpt[${opts},SPAWNID]}
	/varset assistTime 1200s

	/if (${Spawn[id ${assistSpawnID}].Distance}>${maxAssistDistance}) /RETURN RESET

	/declare lock string local
	/varset lock ${getStackCommandOpt[${opts},LOCK]}

	/if (${lock.NotEqual[-]}) {
		/bc locking on ${assistSpawnID}
		/varset assistLock TRUE
	}

	/RETURN
}

SUB assistStop(string cmd, string opts) {
	/declare holdTimer string local ${getStackCommandOpt[${opts},ASSISTHOLDTIMER]}
	/if (${holdTimer.NotEqual[-]}) {
		/varset autoAssistHoldTimer ${holdTimer}
	}

	| setting this low too fast will trigger buffs in the middle of combat
	|/varset combatStateTimer 5

	/varset assistSpawnID 0
	/varset assistTime 0
	/varset assistLock FALSE

	/RETURN
}

SUB assistFollow(string cmd, string opts) {
	/call assistStop STOP STOP;ASSISTHOLDTIMER=20
	/RETURN
}


SUB assistMaintenance() {
	/call updateCombatStateTimer

	/if (${assistSpawnID}!=0) {
		/if (${Spawn[id ${assistSpawnID}].ID}==NULL || ${Spawn[id ${assistSpawnID}].Dead}==TRUE) /call assistStop
		
		/if (${assistMustHaveName.NotEqual[FALSE]}) {
			/if (${Spawn[id ${assistSpawnID}].CleanName.Upper.Find[${assistMustHaveName.Upper}]}==NULL) {
				/call assistStop
				/RETURN
			}
		}
		/if (${assistMustNotHaveName.NotEqual[FALSE]}) {
			/if (${Spawn[id ${assistSpawnID}].CleanName.Upper.Find[${assistMustNotHaveName.Upper}]}!=NULL) {
				/call assistStop
				/RETURN
			}
		}
	}

	/call checkAutoAssist

	/RETURN
}

SUB checkAutoAssist() {
	/if (${autoAssistHoldTimer}>0) /return
	/if (${autoAssistCheckTimer}>0) /return
	/if (${autoAssist}==FALSE) /return
	/if (${assistSpawnID}!=0) /return

	/varset autoAssistCheckTimer 5

	/declare i int local
	/declare netbotName string local
	/declare targetID int local 0

	/for i 1 to ${NetBots.Counts}
		/varset netbotName ${NetBots.Client[${i}]}

		/if (${NetBots[${netbotName}].InZone}==FALSE) /continue
		/if (${NetBots[${netbotName}].Attacking}==FALSE) /continue
		/if (${NetBots[${netbotName}].TargetID}==0) /continue
		/if (${Spawn[id ${NetBots[${netbotName}].TargetID}].Dead}==TRUE) /continue
		/if (${Spawn[pc ${netbotName}].Distance} > ${maxAssistDistance}) /continue

		/if (${Spawn[id ${NetBots[${netbotName}].TargetID}].Type.NotEqual[PC]} && ${NetBots[${netbotName}].TargetHP}<=${autoAssistPercent}) {
			/varset targetID ${NetBots[${netbotName}].TargetID}
		}

	/next i

	/if (${targetID}!=0) {
   		/invoke ${cq.Append[KILL;AUTHOR=${Me.CleanName};SPAWNID=${targetID}]}
	}

	/RETURN
}

SUB updateCombatStateTimer() {
	/declare i int local
	/declare botName string local

	/if (${combatStateCheckTimer}>0) /return
	/varset combatStateCheckTimer 5

	/if (${Me.CombatState.Equal[COMBAT]}) {
		/varset combatStateTimer ${combatStateTimerMax}
		/varset combatStateLongTimer ${combatStateLongTimerMax}		
		/return
	}

	/for i 1 to ${NetBots.Counts}
		/varset botName ${NetBots.Client[${i}]}

		/if (${useEQCombatIndicator}==TRUE) {
			/if (${NetBots[${botName}].CombatState}==0) {
				/varset combatStateTimer ${combatStateTimerMax}
				/varset combatStateLongTimer ${combatStateLongTimerMax}
				/RETURN
			}
		} else {
			/if (${NetBots[${botName}].Attacking} == TRUE) {
				/if (${NetBots[${botName}].Zone} != ${Zone.ID}) /continue
				/if (${Spawn[pc ${botName}].Distance} > ${maxAssistDistance}) /continue

				/varset combatStateTimer ${combatStateTimerMax}
				/varset combatStateLongTimer ${combatStateLongTimerMax}
				/return
			}
		}

	/next i
	/RETURN
}

