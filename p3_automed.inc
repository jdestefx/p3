SUB p3_automed_init() {
	/call p3RegisterCommand CAST autoMedCast 10


	/declare doAutoMed bool outer FALSE
	/declare checkAutoMedTimer timer outer 0
	/declare medLockoutTimer timer outer 0


	/call pauseAutoMed 2s

	/declare ignoreAutoMed bool outer TRUE
	/if (${Select[${Me.Class.ShortName},${manaRegenClasses}]}>0) /varset ignoreAutoMed FALSE
	/varset ignoreAutoMed ${getBotIniOption[Misc.IgnoreAutoMed,${ignoreAutoMed}]}


	/RETURN
}


SUB automedMaintenance() {
	/if (${doAutoMed}==TRUE && ${ignoreAutoMed}==FALSE) /call checkAutoMed
	/RETURN
}

SUB autoMedCast(string cmd, string opts) {
	/call pauseAutoMed 2s
	/RETURN
}

SUB setAutoMed(bool mode) {
	/varset doAutoMed ${mode}

	/if (${mode}==TRUE) /bc Auto-Med is now on.
	/if (${mode}==FALSE) /bc Auto-Med is now off.
	/RETURN
}

SUB checkAutoMed() {
	/if (${checkAutoMedTimer}>0) /return
	/varset checkAutoMedTimer 3s

	/if (${medLockoutTimer}>0) {
		|/bc Meditate locked out. Time remaining: ${medLockoutTimer}
		/return
	}

	/if (${movingTimer}>0) /return
	/if (${Me.State.Equal[MOUNT]}==TRUE) /return
	/if (${Me.Casting.ID}!=NULL) /return
	/if (${Me.PctMana} >= 99) /return
	/if (${checkOpenWindows[]}==TRUE) /return
	
	/if (${assistSpawnID}!=0 && (${useMeleeCombat}==TRUE || ${autoFireAssist}==TRUE)) /return

	| PREVENT MEDDING AND STAND, IF THERES A TIMER ACTIVE FROM BEING HIT
	/echo medlockouttime: ${medLockoutTimer}

	/if (${medLockoutTimer}>0) {
		/if (${Me.Sitting}==TRUE) /stand
		/return
	}

	/if (${Me.State.Equal[STAND]}==TRUE) {
		/if (${Me.Sitting}==FALSE) /sit
	}

	/RETURN
}


SUB pauseAutoMed(string waitTime) {
	/if (${Defined[waitTime]}==FALSE) {
		/declare waitTime string local 10s
	}

	/varset medLockoutTimer ${waitTime}
	|/echo Stopping automed for ${waitTime}
	/if (${Me.Sitting}==TRUE) /stand
	/RETURN
}

