SUB p3_bardtwist_init {
	/if (${Me.Class.ShortName.NotEqual[BRD]}) /RETURN

	/call p3RegisterCommand NEWTWIST bardtwistNewTwist 50


	/declare notCastingTimer timer outer 30

	/declare twistOrder string outer		1 2 3 4
	/varset twistOrder 						${getBotIniOption[Bard.TwistOrder,${twistOrder}]}

	/RETURN
}


SUB bardtwistMaintenance {
	/if (${Me.Class.ShortName.NotEqual[BRD]}) /RETURN

	/if (${Me.Casting.ID}!=NULL) /varset notCastingTimer 30

	/if (${notCastingTimer}==0) {
		/if (${Twist.Twisting}==FALSE) /twist ${twistOrder}
	}

	/RETURN
}

SUB bardtwistNewTwist(string cmd, string opts) {
	/declare newOrder string local ${getStackCommandOpt[${opts},ORDER]}

	/if (${Twist.Twisting}==TRUE) /twist ${newOrder}

	/call writeBotIniOption Bard.TwistOrder "${newOrder}"
	/bc New twist: ${newOrder} (saved to ini)

	/RETURN
}