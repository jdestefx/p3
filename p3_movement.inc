SUB p3_movement_init {
	/call p3RegisterCommand FOLLOW movementFollow 60
	/call p3RegisterCommand STOP movementStop 60
	/call p3RegisterCommand KILL movementKill 60
	/call p3RegisterCommand DROPANCHOR movementAnchor 55
	/call p3RegisterCommand RAISEANCHOR movementRaiseAnchor 55
	/call p3RegisterCommand ROAMMOVESTART movementRoamMoveStart 45


	/declare stickRoamLockoutTimer timer outer 0
	/declare feetWetStickTimer timer outer 0
	/declare anchorXYZ string outer -
	/declare needRestick bool outer FALSE
	/declare stickMode string outer -

	/declare stickTargetID int outer 0
	/declare activeStickFlags string outer
	/declare activeStickDistance string outer
	/declare movingTimer timer outer 0

	/declare followStickFlags string outer 			UW
	/declare followStickDistance string outer 		10
	/declare meleeStickFlags string outer 			BEHIND MOVEBACK
	/declare meleeStickDistance string outer 		10
	/declare followUseNavAssist bool outer 			TRUE

	/varset followStickFlags ${getBotIniOption[Misc.FollowStickFlags,${followStickFlags}]}
	/varset followStickDistance ${getBotIniOption[Misc.FollowStickDistance,${followStickDistance}]}
	/varset meleeStickFlags ${getBotIniOption[Assist.MeleeStickFlags,${meleeStickFlags}]}
	/varset meleeStickDistance ${getBotIniOption[Assist.MeleeStickDistance,${meleeStickDistance}]}
	/varset followUseNavAssist ${getBotIniOption[Assist.FollowUseNavAssist,${followUseNavAssist}]}

	/RETURN
}


SUB movementMaintenance() {
	/if (${Me.Moving}==TRUE) /varset movingTimer 15


	|/if (${Me.Casting.ID}!=NULL && ${Stick.Active}==TRUE && ${canCastWhileMoving}==FALSE)  {
	|	/stick off
	|	/RETURN
	|}

	/if (${stickTargetID}==0) {
		/if (${Stick.Active}==TRUE) /stick off
		/RETURN
	}

	/if (${anchorXYZ.NotEqual[-]}) {
		/if (${MathDistanceXYZ[${anchorXYZ.Replace[ ,,]}]}>10) {
			/if (${Stick.Active}==TRUE) /stick off
			/if (${Nav.Active}==FALSE) /nav locxyz ${anchorXYZ}
		}
		/RETURN
	}

	/if (${Spawn[id ${stickTargetID}].ID}==NULL || ${Spawn[id ${stickTargetID}].Dead}==TRUE) {
		/if (${Nav.Active}) /nav stop
		/if (${Stick.Active}) /stick off
		/call movementStop
		/RETURN
	}

	/if (${stickRoamLockoutTimer}>0) /RETURN

	/if (${Stick.StickTarget}!=${stickTargetID} || ${Stick.Active}==FALSE || ${needRestick}==TRUE || ${Spawn[id ${stickTargetID}].LineOfSight}==FALSE || ${Me.FeetWet}==TRUE && ${feetWetStickTimer}==0) {
		/if (${Spawn[id ${stickTargetID}].LineOfSight}==FALSE) {
			/if (${navPlugin}==TRUE && ${followUseNavAssist}==TRUE) {
				/if (${Stick.Active}==TRUE) /stick off
				/if (${Nav.Active}==FALSE) /nav id ${stickTargetID}
			}
		} else {
			/if (${Nav.Active}==TRUE) /nav stop
			/bc /stick id ${stickTargetID} ${activeStickDistance} ${activeStickFlags} 
			/stick id ${stickTargetID} ${activeStickDistance} ${activeStickFlags}
			/varset needRestick FALSE
			/if (${Me.FeetWet}==TRUE) /varset feetWetStickTimer 40
		}		
	}

	/RETURN
}


SUB movementRoamMoveStart(string cmd, string opts) {
	/echo lockout stick!
	/varset stickRoamLockoutTimer 30

	/RETURN
}

SUB movementAnchor(string cmd, string opts) {
	/varset anchorXYZ ${getStackCommandOpt[${opts},LOCXYZ]}
	/RETURN
}


SUB movementRaiseAnchor(string cmd, string opts) {
	/varset anchorXYZ -
	/RETURN
}

SUB movementFollow (string cmd, string opts) {
	/varset stickMode FOLLOW
	/varset stickTargetID ${getStackCommandOpt[${opts},SPAWNID]}
	/call setActiveStickParams ${stickMode}

	/RETURN
}

SUB movementStop (string cmd, string opts) {
	/varset stickTargetID 0
	/moveto stop
	/if (${Nav.Active}==TRUE) /nav stop

	/RETURN
}

SUB movementKill(string cmd, string opts) {
	/varset stickMode MELEE

	/if (${useMeleeCombat}==TRUE) {
		/varset stickTargetID ${getStackCommandOpt[${opts},SPAWNID]}
		/call setActiveStickParams ${stickMode}
	} else {
		/varset stickTargetID 0
	}

	/RETURN
}

SUB setActiveStickParams(string mode) {
	/if (${mode.Equal[MELEE]}) {
		/varset activeStickFlags ${meleeStickFlags}
		/varset activeStickDistance ${meleeStickDistance}
	} else /if (${mode.Equal[FOLLOW]}) {
		/varset activeStickFlags ${followStickFlags}
		/varset activeStickDistance ${followStickDistance}
	}
	/RETURN
}
