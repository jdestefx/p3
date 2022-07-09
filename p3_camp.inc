SUB p3_camp_init {
	/call p3RegisterCommand FOLLOW campFollow 50
	/call p3RegisterCommand ROAMLOCK campRoamStarted 55
	/call p3RegisterCommand CAMPON campCampOn 20

	/declare campCheckTimer timer outer 
	/declare campEnabled bool outer FALSE
	/declare campStartZoneID int outer 
	/declare campX int outer
	/declare campY int outer
	/declare campZ int outer
	/declare campSpawnID int outer 0

	/RETURN
}

SUB campRoamStarted(string cmd, string opts) {
	/if (${campEnabled}==TRUE) {
		/varset campEnabled FALSE
		/bc Camp disabled to allow roam.
	}
	/RETURN
}

SUB campFollow(string cmd, string opts) {
	/if (${campEnabled}==TRUE) {
		/varset campEnabled FALSE
		/bc Camp disabled to engage follow.
	}
	/RETURN
}

SUB campCampOff(string cmd, string opts) {
	/declare sourceName string local ${getStackCommandOpt[${opts},SOURCENAME]}
	/if (${Spawn[=${sourceName}].ID}==NULL) /RETURN

	/if (${campEnabled}==TRUE) {
		/varset campEnabled FALSE
		/bc Camp mode disabled.
	}
	/RETURN
}

SUB campCampOn(string cmd, string opts) {
	/declare campType string local ${getStackCommandOpt[${opts},TYPE]}
	/declare spawnTargetID int local
	/declare sourceName string local

	/if (${campType.Equal[SPAWN]}) {
		/varset spawnTargetID ${getStackCommandOpt[${opts},SPAWNTARGETID]}
		/if (${Spawn[id ${spawnTargetID}].ID}==NULL) /RETURN RESET

		/bc Using spawn ${Spawn[id ${spawnTargetID}]} as camp location.
		/varset campSpawnID ${spawnTargetID}
		/varset campEnabled TRUE
		/varset campStartZoneID ${Zone.ID}
	}

	/if (${campType.Equal[LOC]}) {
		/varset sourceName ${getStackCommandOpt[${opts},SOURCENAME]}
		/if (${Spawn[=${sourceName}].ID}==NULL) /RETURN RESET

		/varset campSpawnID 0
		/varset campX ${Spawn[${sourceName}].X}
		/varset campY ${Spawn[${sourceName}].Y}
		/varset campZ ${Spawn[${sourceName}].Z}
		/varset campEnabled TRUE
		/varset campStartZoneID ${Zone.ID}
		/bc Now camping at ${sourceName}'s location.
	}

	/RETURN
}


SUB campMaintenance() {
	/if (${campEnabled}==TRUE) /call checkCamp
	/RETURN
}

SUB checkCamp() {
	/if (${campCheckTimer}>0) /RETURN
	/if (${Me.Moving}==TRUE) /return
	/varset campCheckTimer 5
	
	/if (${Zone.ID}!=${campStartZoneID}) {
		/call stopCamping
		/return
	}

	/if (${Stick.Active}==TRUE || ${Navigation.Active}==TRUE) /RETURN
	/if (${combatStateTimer}>0) /RETURN

	/if (${campSpawnID}!=0) {
		/varset campX ${Spawn[id ${campSpawnID}].X}
		/varset campY ${Spawn[id ${campSpawnID}].Y}
		/varset campZ ${Spawn[id ${campSpawnID}].Z}
	}

	/if (${Math.Distance[${campY}, ${campX}, ${campZ}]}>15) {
		/if (${Navigation.MeshLoaded}==TRUE && ${Navigation.Active}==FALSE && ) {
			/bc Returning to camp.
			/nav locxyz ${campX} ${campY} ${campZ}
		} else {
			/moveto dist 15
			/moveto loc ${campY} ${campX}
		}
	} else {

	}
	/RETURN
}


SUB stopCamping() {
	/if (${campEnabled}==TRUE) {
		/varset campEnabled FALSE
		/bc Camp mode disabled.
	}
	/RETURN
}

