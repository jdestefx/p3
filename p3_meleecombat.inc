SUB p3_meleecombat_init {
	/call p3RegisterCommand KILL meleecombatKill 55
	/call p3RegisterCommand STOP meleecombatStop 50
	/call p3RegisterCommand FOLLOW meleecombatFollow 50
	/call p3RegisterCommand ROAMMOVESTART meleecombatRoamMoveStart 45

	/declare meleeTargetID int outer 0

	/declare useMeleeCombat bool outer 				${Bool[${Select[${Me.Class.ShortName},${defaultMeleeCombatClasses}]}]}
	/declare meleeRetreatPct int outer 				45
	/declare meleeRetreatResumePct int outer 		80

	/varset useMeleeCombat 				${getBotIniOption[Assist.UseMeleeCombat,${useMeleeCombat}]}

	/RETURN
}

SUB meleecombatRoamMoveStart(string cmd, string opts) {
	/call meleecombatStop
	/RETURN
}

SUB meleecombatFollow(string cmd, string opts) {
	/call meleecombatStop
	/RETURN
}

SUB meleecombatKill(string cmd, string opts) {
	/varset meleeTargetID ${getStackCommandOpt[${opts},SPAWNID]}

	/RETURN
}

SUB meleecombatStop(string cmd, string opts) {
	/varset meleeTargetID 0
	/attack off

	/RETURN
}

SUB meleecombatMaintenance() {
	/declare killSpawn spawn local 

	/if (${meleeTargetID}==0) {
		/if (${Me.Combat}==TRUE) /attack off
		/return
	}

	/if (${Spawn[id ${meleeTargetID}].ID}==NULL || ${Spawn[id ${meleeTargetID}].Dead}==TRUE) {
		/call meleecombatStop
		/return
	}

	/vardata killSpawn Spawn[id ${meleeTargetID}]

	/if (${killSpawn.ID}==NULL || ${killSpawn.Dead}==TRUE) {
		/call meleecombatStop
		/return
	}

	/if (${useMeleeCombat}==TRUE) {
		/if (${killSpawn.Distance} > ${Math.Calc[${killSpawn.MaxRangeTo}+0.5]}) {
			/if (${Me.Combat}==TRUE) /attack off
		} else {
			/if (${Me.Combat}==FALSE) /attack on
		}
	}

	/RETURN
}

