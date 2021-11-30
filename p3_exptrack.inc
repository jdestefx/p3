SUB p3_exptrack_init() {
	
	/declare doExpReport bool outer TRUE
	/varset doExpReport ${getBotIniOption[Misc.ExperienceReports,${doExpReport}]}

	/declare curRegPct float outer ${Math.Calc[${Me.Exp}/1000].Precision[3]}
	/declare curAAPct float outer ${Math.Calc[${Me.AAExp}/1000].Precision[3]}


	/RETURN
}


SUB exptrackMaintenance() {
	/RETURN
}


SUB expTrackerNewExp() {
	/declare newRegPct float local ${Math.Calc[${Me.Exp}/1000].Precision[3]}
	/declare newAAPct float local ${Math.Calc[${Me.AAExp}/1000].Precision[3]}

	/declare gain float local

	/if (${doExpReport}==TRUE) {

		/if (${newRegPct}<${curRegPct}) {
			/varset gain ${Math.Calc[(${newRegPct.Precision[3]})- (100-${curRegPct.Precision[3]})]}
		} else /if (${newRegPct}>${curRegPct}) {
			/varset gain ${Math.Calc[${newRegPct.Precision[3]} - ${curRegPct.Precision[3]} ]}
		}

		/if (${gain}>0) {
			/bc Regular Exp Gain: ${gain.Precision[3]}% (${Math.Calc[${Me.Exp}/1000].Precision[3]}% through ${Me.Level})
		}

		/varset gain 0

		/if (${newAAPct}<${curAAPct}) {
			/varset gain ${Math.Calc[(100-${curAAPct.Precision[3]}) + (${newAAPct.Precision[3]})]}
		} else /if (${newAAPct}>${curAAPct}) {
			/varset gain ${Math.Calc[${newAAPct.Precision[3]} - ${curAAPct.Precision[3]} ]}
		}

		/if (${gain}>0) {
			/bc AA Exp Gain: ${gain.Precision[3]}%  (${Me.AAPoints} banked)
		}

		/varset curRegPct ${newRegPct.Precision[3]}
		/varset curAAPct ${newAAPct.Precision[3]}
	}
	/RETURN
}
