SUB p3_bardtwist_init {
	/if (${Me.Class.ShortName.NotEqual[BRD]}) /RETURN

	/call p3RegisterCommand NEWTWIST bardtwistNewTwist 50

	/declare p3TwistEnabled bool outer true

	/declare notCastingTimer timer outer 30
	/declare doTwist bool outer TRUE

	/declare twistOrder string outer		1 2 3 4
	
	/varset twistOrder ${getBotIniOption[Bard.TwistOrder,${twistOrder}]}
	/varset p3TwistEnabled ${getBotIniOption[Bard.MacroControlledTwisting,${p3TwistEnabled}]}

	/RETURN
}


SUB bardtwistMaintenance {
	/if (${p3TwistEnabled}==FALSE) /RETURN

	/if (${Me.Class.ShortName.NotEqual[BRD]}) /RETURN

	/if (${Me.Casting.ID}!=NULL) /varset notCastingTimer 30

	/if (${notCastingTimer}==0) {
		/if (${Twist.Twisting}==FALSE && ${doTwist}==TRUE) {
			/echo do ${twistOrder}
			/twist ${twistOrder}
		}
	}

	/RETURN
}

SUB bardtwistNewTwist(string cmd, string opts) {
	/declare newOrder string local ${getStackCommandOpt[${opts},ORDER]}

	/if (${Cast.Effect.ID}!=NULL) {
		/twist off
		/delay 3
		/echo new twist start: ${newOrder}
		/twist ${newOrder}
		/delay 5
	}

	/varset twistOrder ${newOrder}

	/call writeBotIniOption Bard.TwistOrder "${newOrder}"
	/bc New twist: ${newOrder} (saved to ini)

	/RETURN
}

