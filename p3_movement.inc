SUB p3_movement_init {
	/call p3RegisterCommand FOLLOW movementFollow 60
	/call p3RegisterCommand STOP movementStop 60
	/call p3RegisterCommand KILL movementKill 60
	/call p3RegisterCommand DROPANCHOR movementAnchor 55
	/call p3RegisterCommand RAISEANCHOR movementRaiseAnchor 55
	/call p3RegisterCommand ROAMMOVESTART movementRoamMoveStart 45

	/declare stickLockoutTimer timer outer 0
	/declare stickRoamLockoutTimer timer outer 0
	/declare feetWetStickTimer timer outer 0
	/declare anchorXYZ string outer -
	/declare needRestick bool outer FALSE
	/declare stickMode string outer -
	/declare refaceTimer timer outer 0

	/declare stickTargetID int outer 0
	/declare activeStickFlags string outer
	/declare activeStickDistance string outer
	/declare movingTimer timer outer 0
	/declare fearStartXYZ string outer -

	/declare followStickFlags string outer 			UW
	/declare followStickDistance string outer 		10
	/declare meleeStickFlags string outer 			BEHIND MOVEBACK
	/declare meleeStickDistance string outer 		10
	/declare followUseNavAssist bool outer 			TRUE
	/declare fearReturnToStart bool outer 			TRUE
	/declare feignAutoStand bool outer 				TRUE

	/varset followStickFlags ${getBotIniOption[Misc.FollowStickFlags,${followStickFlags}]}
	/varset followStickDistance ${getBotIniOption[Misc.FollowStickDistance,${followStickDistance}]}
	/varset meleeStickFlags ${getBotIniOption[Assist.MeleeStickFlags,${meleeStickFlags}]}
	/varset meleeStickDistance ${getBotIniOption[Assist.MeleeStickDistance,${meleeStickDistance}]}
	/varset followUseNavAssist ${getBotIniOption[Assist.FollowUseNavAssist,${followUseNavAssist}]}
	/varset fearReturnToStart ${getBotIniOption[Misc.FearReturnToStart,${fearReturnToStart}]}
	/varset feignAutoStand ${getBotIniOption[Misc.AutoStandFromFeign,${feignAutoStand}]}

	/RETURN
}


SUB movementMaintenance() {
	/if (${Me.Moving}==TRUE) /varset movingTimer 5

	/if (${Me.Feared.ID}!=NULL) {
		/if (${fearStartXYZ.NotEqual[-]}) /varset fearStartXYZ ${Me.X} ${Me.Y} ${Me.Z}
	} else {
		/if (${fearStartXYZ.NotEqual[-]} && ${MathDistanceXYZ[${fearStartXYZ.Replace[ ,,]}]}>20) {
			/nav locxyz ${fearStartXYZ}
			/delay 10
			/RETURN
		} else {
			/varset fearStartXYZ -
		}
	}

	/if (${Me.Feigning}==TRUE) {
		/if (${feignAutoStand}==TRUE) /timed 5 /stand
	}

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

	/if (${stickLockoutTimer}>0) /RETURN
	/if (${stickRoamLockoutTimer}>0) /RETURN

	/if (${Spawn[id ${stickTargetID}].LineOfSight}==FALSE && ${followUseNavAssist}==TRUE) {
		/if (${Nav.PathExists[id ${stickTargetID}]}==TRUE) {
			/if (${Nav.Active}==FALSE) /nav id ${stickTargetID}
			/RETURN
		}
	}

	/if (${Stick.StickTarget}!=${stickTargetID} || ${Stick.Active}==FALSE || ${needRestick}==TRUE || ${Me.FeetWet}==TRUE && ${feetWetStickTimer}==0) {
		/if (${Nav.Active}==TRUE) /nav stop
		|/bc /stick id ${stickTargetID} ${activeStickDistance} ${activeStickFlags} 
		/stick id ${stickTargetID} ${activeStickDistance} ${activeStickFlags}
		/varset needRestick FALSE
		/if (${Me.FeetWet}==TRUE) /varset feetWetStickTimer 40
	}

	| BECAUSE STICK WILL FREAK OUT ALONG WALLS, RE-FACING THE MOB MAINTAINS PROPER MELEEING
	/if (${refaceTimer}==0) {
		/face id ${stickTargetID}
		/varset refaceTimer 20
	}

	/RETURN
}

SUB movementLockoutStick(string t) {
	/if (${Stick.Active}==TRUE) {
		/stick off
		/moveto stop
		/delay 5
	}
	
	/varset stickLockoutTimer ${t}
	/ECHO LOCKING OUT STICK FOR ${t}
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

	/echo movement kill:
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
		/if (${tankModeEnabled}==TRUE) /varset activeStickFlags ${activeStickFlags.Replace[BEHIND,]}
	} else /if (${mode.Equal[FOLLOW]}) {
		/varset activeStickFlags ${followStickFlags}
		/varset activeStickDistance ${followStickDistance}
	}
	/RETURN
}
