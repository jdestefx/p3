SUB p3_movement_init {
	/call p3RegisterCommand FOLLOW movementFollow 60
	/call p3RegisterCommand STOP movementStop 60
	/call p3RegisterCommand KILL movementKill 60

	/declare stickTargetID int outer 0
	/declare activeStickFlags string outer
	/declare activeStickDistance string outer
	/declare movingTimer timer outer 0

	/declare followStickFlags string outer 			UW
	/declare followStickDistance string outer 		10
	/declare meleeStickFlags string outer 			BEHIND
	/declare meleeStickDistance string outer 		10

	/varset followStickFlags ${getBotIniOption[Misc.FollowStickFlags,${followStickFlags}]}
	/varset followStickDistance ${getBotIniOption[Misc.FollowStickDistance,${followStickDistance}]}
	/varset meleeStickFlags ${getBotIniOption[Assist.MeleeStickFlags,${meleeStickFlags}]}
	/varset meleeStickDistance ${getBotIniOption[Assist.MeleeStickDistance,${meleeStickDistance}]}

	/RETURN
}


SUB movementFollow (string cmd, string opts) {
	/varset stickTargetID ${getStackCommandOpt[${opts},SPAWNID]}
	/varset activeStickFlags ${followStickFlags}
	/varset activeStickDistance ${followStickDistance}

	/RETURN
}

SUB movementStop (string cmd, string opts) {
	/varset stickTargetID 0
	/MOVETO STOP
	/if (${Nav.Active}==TRUE) /nav stop

	/RETURN
}

SUB movementKill(string cmd, string opts) {
	/if (${useMeleeCombat}==TRUE) {
		/varset stickTargetID ${getStackCommandOpt[${opts},SPAWNID]}
		/varset activeStickFlags ${meleeStickFlags}
		/varset activeStickDistance ${meleeStickDistance}
	} else {
		/varset stickTargetID 0
	}

	/RETURN
}

SUB movementMaintenance() {
	/if (${Me.Moving}==TRUE) /varset movingTimer 10

	/if (${stickTargetID}==0) {
		/if (${Stick.Active}==TRUE) /stick off
		/return
	}

	/if (${Spawn[id ${stickTargetID}].ID}==NULL || ${Spawn[id ${stickTargetID}].Dead}==TRUE) {
		/if (${Nav.Active}) /nav stop
		/if (${Stick.Active}) /stick off
		/call movementStop
		/return
	}

	/if (${Nav.Active}==TRUE) /nav stop

	/if (${Spawn[id ${stickTargetID}].LineOfSight}==FALSE && ${navPlugin}==TRUE) {
		/if (${Nav.Active}==FALSE) /nav id ${stickTargetID}
	} else {
		/if (${Stick.StickTarget}!=${stickTargetID} || ${Stick.Active}==FALSE) {
			/stick id ${stickTargetID} ${activeStickDistance} ${activeStickFlags} 
		}
	}


	/RETURN
}

SUB amMoving {
	/if (${movingTimer}==0) /RETURN FALSE
	/RETURN TRUE
}