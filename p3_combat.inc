SUB p3_combat_init {
	/call p3RegisterCommand KILL combatKill 50
	/call p3RegisterCommand STOP combatStop 50

	/declare killTargetID int outer 0

	/RETURN
}

SUB combatKill(string cmd, string opts) {
	/varset killTargetID ${getStackCommandOpt[${opts},SPAWNID]}

	/RETURN
}

SUB combatStop(string cmd, string opts) {
	/varset killTargetID 0
	/attack off

	/RETURN
}

SUB combatMaintenance() {
	/declare killSpawn spawn local 

	/if (${killTargetID}==0) {
		/if (${Me.Combat}==TRUE) /attack off
		/return
	}

	/vardata killSpawn Spawn[id ${killTargetID}]

	/if (${killSpawn.ID}==NULL || ${killSpawn.Dead}==TRUE) {
		/call combatStop
		/return
	}

	/echo ${killSpawn.Distance} ${Math.Calc[${killSpawn.MaxRangeTo}+0.5]}

	/if (${useMeleeCombat}==TRUE) {
		/if (${killSpawn.Distance} > ${Math.Calc[${killSpawn.MaxRangeTo}+0.5]}) {
			/if (${Me.Combat}==TRUE) /attack off
		} else {
			/if (${Me.Combat}==FALSE) /attack on
		}
	}


	/RETURN
}