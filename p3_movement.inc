SUB p3_movement_init {
	/call p3RegisterCommand FOLLOW movementFollow 60
	/call p3RegisterCommand STOP movementStop 60
	/call p3RegisterCommand KILL movementKill 60

	/declare stickTargetID int outer 0

	/RETURN
}

SUB movementFollow (string cmd, string opts) {
	/varset stickTargetID ${getStackCommandOpt[${opts},SPAWNID]}

	/RETURN
}


SUB movementStop (string cmd, string opts) {
	/varset stickTargetID 0

	/RETURN
}

SUB movementKill(string cmd, string opts) {
	/if (${useMeleeCombat}==TRUE) {
		/varset stickTargetID ${getStackCommandOpt[${opts},SPAWNID]}
	} else {
		/varset stickTargetID 0
	}

	/RETURN
}

SUB movementMaintenance() {
	/if (${stickTargetID}==0) {
		/if (${Nav.Active}) /nav stop
		/if (${Stick.Active}) /stick off
		/return
	}

	/if (${Spawn[id ${stickTargetID}].ID}==NULL || ${Spawn[id ${stickTargetID}].Dead}==TRUE) {
		/if (${Nav.Active}) /nav stop
		/if (${Stick.Active}) /stick off
		/return
	}


	/if (${useMeleeCombat}==TRUE) {
		/if (${Stick.StickTarget}!=${stickTargetID} || ${Stick.Active}==FALSE) {
			/stick id ${stickTargetID} ${defaultStickOpts}
		}
	}

	/RETURN
}