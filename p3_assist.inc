SUB p3_assist_init {
	/call p3RegisterCommand STOP assistStop 50
	/call p3RegisterCommand FOLLOW assistFollow 50
	/call p3RegisterCommand KILL assistKill 50

	/declare assistSpawnID int outer 0
	/declare combatStateCheckTimer timer outer 0
	/declare combatStateTimer timer outer 0
	/declare combatStateTimerMax int outer 50
	/declare autoAssistHoldTimer timer outer 0
	/declare autoAssistCheckTimer timer outer 0

	/declare autoAssist bool outer 				TRUE
	/declare maxAssistDistance int outer 		500
	/declare useAutoFire bool outer				FALSE
	/declare autoAssistPercent int outer		99

	/varset autoAssist 				${getBotIniOption[Assist.AutoAssist,${autoAssist}]}
	/varset autoAssistPercent		${getBotIniOption[Assist.AutoAssistPercent,${autoAssistPercent}]}
	/varset maxAssistDistance 		${getBotIniOption[Assist.MaxAssistDistance,${maxAssistDistance}]}
	/varset useAutoFire 			${getBotIniOption[Assist.UseAutoFire,${useAutoFire}]}

	/RETURN
}

SUB assistKill(string cmd, string opts) {
	/varset assistSpawnID ${getStackCommandOpt[${opts},SPAWNID]}
	
	/RETURN
}

SUB assistStop(string cmd, string opts) {
	/declare holdTimer string local ${getStackCommandOpt[${opts},ASSISTHOLDTIMER]}
	/if (${holdTimer.NotEqual[-]}) {
		/varset autoAssistHoldTimer ${holdTimer}
	}

	/varset combatStateTimer 5
	/varset assistSpawnID 0
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

	/if (${Me.Combat}==TRUE) {
		/varset combatStateTimer ${combatStateTimerMax}
		/return
	}

	/if (${combatStateCheckTimer}>0) /return
	/varset combatStateCheckTimer 3

	/for i 1 to ${NetBots.Counts}
		/varset botName ${NetBots.Client[${i}]}

		/if (${NetBots[${botName}].Attacking} == TRUE) {
			/if (${NetBots[${botName}].Zone} != ${Zone.ID}) /continue
			/if (${Spawn[pc ${botName}].Distance} > ${maxAssistDistance}) /continue

			/varset combatStateTimer ${combatStateTimerMax}
			/return
		}

	/next i
	/RETURN
}