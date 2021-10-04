SUB p3_medbreak_init() {
	/declare medBreakActive bool outer FALSE
	/declare checkMedBreakTimer timer outer 0


	/RETURN
}


SUB medbreakMaintenance() {
	
	/call checkMedBreak
	/RETURN
}

SUB setMedBreak(bool mode) {
	

	/RETURN
}

SUB checkMedBreak() {
	/if (${checkMedBreakTimer}>0) /return
	
	/call checkOpenWindows
	/if (${Macro.Return}==TRUE) /return
	
	/if (${assistTarget}!=0 && (${meleeAssist}==TRUE || ${autoFireAssist}==TRUE)) /return

	/if (${Me.Moving}) /return

	| PREVENT MEDDING AND STAND, IF THERES A TIMER ACTIVE FROM BEING HIT
	/if (${meditateBlockTimer}>0) {
		/if (${Me.Sitting}==TRUE) /stand
		/return
	}

	| MOUNTED? BAIL..
	/if (${Me.Casting.ID}!=NULL) /return

	/if (${Me.State.Equal[MOUNT]}==TRUE) /return

	/if (${Me.PctMana} >= 99) {
		/echo I am at full mana, pausing medbreak for 10s.
		/if (${Me.Sitting}) /stand
		/call pauseMedBreak 10s
		/return
	}

	/if (${Me.State.Equal[STAND]}==TRUE) {
		/timed 20 /if (${meditateBlockTimer}==0 && ${Me.Sitting}==FALSE && ${Me.Moving}==FALSE) /sit
		|/delay 15
	}

	/RETURN
}


SUB pauseMedBreak(string waitTime) {
	/if (${Defined[waitTime]}==FALSE) {
		/declare waitTime string local 10s
	}

	/echo Pausing Medidate: ${waitTime}
	/varset meditateBlockTimer ${waitTime}
	/if (${Me.Sitting}==TRUE) /stand
	/RETURN
}