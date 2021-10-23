SUB p3_camp_init {
	/call p3RegisterCommand FOLLOW campFollow 50

	/declare campCheckTimer timer outer 
	/declare campEnabled bool outer FALSE
	/declare campStartZoneID int outer 
	/declare campX int outer
	/declare campY int outer	
	/declare campZ int outer

	/RETURN
}

SUB campFollow(string cmd, string opts) {
	/if (${campEnabled}==TRUE) {
		/varset campEnabled FALSE
		/bc Camp disabled to engage follow.
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

