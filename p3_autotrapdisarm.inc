SUB p3_autotrapdisarm_init() {
	/declare canDoAutoDisarm bool outer TRUE
	/declare checkAutoDisarmTimer timer outer

	/if (${Select[${Me.Class.ShortName},BRD,ROG]}==0) {
		/varset canDoAutoDisarm FALSE
		/return
	}

	/declare doAutoTrapDisarm bool outer FALSE
	/declare trapNames list outer

	/varset doAutoTrapDisarm ${getBotIniOption[Misc.AutoTrapDisarm,${doAutoTrapDisarm}]}


	/invoke ${trapNames.Append[DESERT CURSE,A BLADE,FESTERING OOZE,JAGGED BLADES,A DARK ENTITY,A SHIMMERING ORB]}
	/invoke ${trapNames.Append[A FORMLESS TERROR,A LOW HUM,A SINGULARITY,A WARDSTONE,A NULL FIELD,AN IMMOBILIZER,TRAP]}
	/invoke ${trapNames.Append[A PRESSURE PLATE,DECAYING REMAINS]}

	/RETURN
}


SUB autotrapdisarmMaintenance() {
	/if (${canDoAutoDisarm}==FALSE) /RETURN
	/if (${checkAutoDisarmTimer}>0) /RETURN 

	/varset checkAutoDisarmTimer 5

	/if (${Me.AbilityReady[Sense Traps]}==TRUE) {
		/doability "sense traps"
		/delay 3
	}
	/if (${Me.AbilityReady[Disarm Traps]}==TRUE) /call checkAutoDisarm

	/RETURN
}


SUB checkAutoDisarm() {
   /declare i int local 0
   /declare j int local 1
   /declare tName string local
   /declare xspawn spawn local

   /for i 0 to ${Math.Calc[${trapNames.Count}-1]}
      /varset tName ${trapNames.Item[${i}]}

      /varset j 1
      /while (1) {
         /if (${NearestSpawn[${j},radius 70].ID}==NULL) /break
         /vardata xspawn NearestSpawn[${j},radius 70]

         /if (${xspawn.CleanName.Find[${tName}]}!=NULL) {
            /tar id ${xspawn.ID}
            /delay 3
            /doability "disarm traps"
            /delay 3
            /RETURN
         }

         /varset j ${Math.Calc[${j}+1]}
      }

   /next i

   /RETURN
}

