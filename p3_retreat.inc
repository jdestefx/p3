SUB p3_retreat_init() {
	/call p3RegisterCommand RETREATON retreatOn 50
	/call p3RegisterCommand RETREATOFF retreatOff 50

	/declare meleeRetreat bool outer FALSE
	/declare meleeRetreatPercent int outer 40
	/declare meleeRetreatResumePercent int outer 75
	/declare meleeRetreatLoc string outer -
	/declare amMeleeRetreating bool outer FALSE

	/varset meleeRetreatPercent ${getBotIniOption[Assist.RetreatPercent,${meleeRetreatPercent}]}
	/varset meleeRetreatResumePercent ${getBotIniOption[Assist.RetreatResumePercent,${meleeRetreatResumePercent}]}

	/RETURN
}

SUB retreatMaintenance() {
	/if (${meleeRetreat}==TRUE) /call checkMeleeRetreat

	/RETURN	
}

SUB retreatOn(string cmd, string opts) {
	/varset meleeRetreat TRUE
	/varset meleeRetreatLoc ${getStackCommandOpt[${opts},LOCXYZ]}
	/bc Retreating at ${meleeRetreatLoc}.
	/RETURN
}

SUB retreatOff(string cmd, string opts) {
	/varset meleeRetreat FALSE
	/varset meleeRetreatLoc -
	/bc Retreat mode is now off.
	/RETURN
}


SUB checkMeleeRetreat() {
	/if (${Me.PctHPs} < ${meleeRetreatPercent} && ${amMeleeRetreating}==FALSE) {
		/varset amMeleeRetreating TRUE
		/invoke (${cq.Append[DROPANCHOR;LOCXYZ=${meleeRetreatLoc};]})
		/RETURN
	}

	/if (${Me.PctHPs} >= ${meleeRetreatResumePercent} && ${amMeleeRetreating}==TRUE) {
		/varset amMeleeRetreating FALSE
		/invoke (${cq.Append[RAISEANCHOR;]})
		/RETURN
	}

	/RETURN
}
