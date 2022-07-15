#event followMe "<#1#> e3 follow me"
#event followMe "<#1#> e3 follow me #2#"

#event followMe "<#1#> p3 follow me"
#event followMe "<#1#> p3 follow me #2#"

#event followMe "[#1#(msg)] p3 follow me"
#event followMe "[#1#(msg)] p3 follow me #2#"

SUB EVENT_followMe(string line, string chatSender, string opts) {
	/if (${Defined[opts]}==FALSE) /declare opts string local ${defaultStickOpts}

	/if (${Spawn[pc =${chatSender}].ID}==${Me.ID}) /return

	/invoke ${cq.Append[FOLLOW;AUTHOR=${chatSender};SPAWNID=${Spawn[pc =${chatSender}].ID};STICKOPTS=${opts}]}
	/varset hardLoopReset TRUE
	/RETURN
}

#event idle "<#*#> p3 idle #1#"
SUB EVENT_idle(string line, string newMode) {
	/if (${newMode.Upper.Equal[ON]}) {
		/bc Idle on.
   		/notify AdvancedDisplayOptionsWindow ADOW_MaxBGFPSSlider newvalue ${Math.Calc[3-9]}
	} else {
		/bc Idle off.
   		/notify AdvancedDisplayOptionsWindow ADOW_MaxBGFPSSlider newvalue ${Math.Calc[25-9]}
	}
	/RETURN
}

#event stop "<#1#> e3 stop"
#event stop "<#1#> p3 stop"
SUB EVENT_stop(string line, string chatSender) {
	/invoke ${cq.Append[STOP;AUTHOR=${chatSender};ASSISTHOLDTIMER=50]}
	/RETURN
}

#event kill "[#1#(msg)] p3 kill #2#"
#event kill "<#1#> p3 kill #2#"
#event kill "[#1#(msg)] assist on #2#"
#event kill "<#1#> assist on #2#"
#event kill "[#1#(msg)] p3 assist on #2#"
#event kill "<#1#> p3 assist on #2#"

SUB event_kill(string line, string chatSender, string spawnID) {
	/if (${assistLock}==TRUE) /RETURN
	/invoke ${cq.Append[KILL;AUTHOR=${chatSender};SPAWNID=${spawnID}]}
	/RETURN
}

#event killLock "[#1#(msg)] p3 killlock #2#"
#event killLock "<#1#> p3 killlock #2#"
#event killLock "[#1#(msg)] assistlock on #2#"
#event killLock "<#1#> assistlock on #2#"

SUB event_killLock(string line, string chatSender, string spawnID) {
	/echo ${spawnID}
	/invoke ${cq.Append[KILL;AUTHOR=${chatSender};SPAWNID=${spawnID};LOCK=YES;]}
	/RETURN
}


#Event buffRequest "#*# tells you, '#2# #1#'" 
#Event buffRequest "#1# tells you, '#2#'" 
SUB EVENT_buffRequest(string line, string who, string msg)
	/declare i int local 0
	/declare requestKeyword ${msg.Upper.Replace[ ,]}
	/declare spawnID int local
	/declare spellName string local
	/declare queueEntry string local
	/declare repeatBuff int local 0

	/if (${brSameGuildOnly}==TRUE && ${Me.Guild.Length}==0) /RETURN
	/if (${brSameGuildOnly}==TRUE && ${Spawn[=${who}].Guild.Length}==0) /RETURN
	/if (${brSameGuildOnly}==TRUE && ${Spawn[=${who}].Guild.NotEqual[${Me.Guild}]}) /RETURN

	/if (${msg.Right[1].Equal[!]}) {
		/varset repeatBuff 1
		/varset requestKeyword ${requestKeyword.Left[-1]}
	}

	/for i 1 to ${iniBuffRequests.Size}
		/if (${iniBuffRequests[${i},1].Equal[-]}) /break

		/if (${Select[${requestKeyword},${iniBuffRequests[${i},${bKeyword}]}]}!=0) {
			/varset spawnID ${Spawn[pc ${who}].ID}
			/varset spellName ${iniBuffRequests[${i},1]}
			/varset queueEntry ${spawnID}_${spellName}_${i}_${repeatBuff}

			/if (${iniBuffRequests[${i},${bRequireTargetClass}].NotEqual[-]}) {
				/if (${Select[${Spawn[id ${spawnID}].Class.ShortName},${iniBuffRequests[${i},${bRequireTargetClass}]}]}==0) /continue
			}

			/if (${repeatBuff}==1) {
				/if (${repeatBuffsQueue.Index[${spawnID}_${i}]}>-1) {
					/tell ${who} You're already in my list for repeat buffs for ${spellName}.
					/continue
				} else {
					/invoke ${repeatBuffsQueue.Append[${spawnID}_${i}]}
					/em beams a smile at ${who}.
				}
			} else {
				/if (${buffQueue.Contains[${queueEntry}]}==TRUE) {
					/tell ${who} You're already queued to receive ${spellName}.
					/continue
				} else {
					/invoke ${buffQueue.Append[${queueEntry}]}
					/em nods at ${who}.
				}
			}

		}

	/next i
/RETURN

#Event pleaseBuffs "#*#says, '#1# needs buffs'" 
#Event pleaseBuffs "#*#says, '#1# needs buffs.'" 
#Event pleaseBuffs "#1# needs buffs." 
SUB EVENT_pleaseBuffs(string line, string emoter, string hailee)
	/if (${brEnabled}==FALSE) /return
	/if (${needsBuffPlayerQueue.Index[${emoter}]}>-1) /return
	/invoke ${needsBuffPlayerQueue.Append[${emoter}]}
	/em looks ${emoter} up and down.
	/delay 2
/RETURN


#Event pleaseBuffsMe "#1# says, 'i need buffs'"
#Event pleaseBuffsMe "#1# says, 'i need buffs.'"
#Event pleaseBuffsMe "#1# tells you, 'i need buffs.'"
#Event pleaseBuffsMe "#1# tells you, 'i need buffs'"
SUB EVENT_pleaseBuffsMe(string line, string emoter, string hailee)
	/if (${brEnabled}==FALSE) /return
	/if (${needsBuffPlayerQueue.Index[${emoter}]}>-1) /return
	/invoke ${needsBuffPlayerQueue.Append[${emoter}]}
	/em looks ${emoter} up and down.
	/delay 2
/RETURN



#Event hailResponse "#1# says, 'Hail, #2#'" 
SUB EVENT_hailResponse(string line, string hailer, string hailee)
	/if (${brEnabled}==FALSE) /return
	/if (${hailee.NotEqual[${Me.CleanName}]}) /return

	/declare response string local 
	/declare i int local 0
	/declare spewedSpells string local
	/declare cleanKeywords string local

	/varset response Send me a tell with one of the following requests:

	/for i 1 to ${iniBuffRequests.Size}
		/if (${spewedSpells.Find[${iniBuffRequests[${i},1]},]}!=NULL) /continue
		/if (${iniBuffRequests[${i},1].Equal[-]}) /break

		/varset cleanKeywords ${iniBuffRequests[${i},${bKeyword}].Upper.Replace[EMOTABLE,]}
		/if (${cleanKeywords.Find[!HIDE]}!=NULL) /continue
		/if (${cleanKeywords.Right[1].Equal[,]}) /varset cleanKeywords ${cleanKeywords.Left[-1]}

		/varset response ${response} ... [ ${cleanKeywords} => ${iniBuffRequests[${i},1]} ]
		/varset spewedSpells ${iniBuffRequests[${i},1]},${spewedSpells}
	/next i

	/if (${brReplyViaTell}==TRUE) {
		/docommand /tell ${hailer} ${response} (${Me.PctMana}m).
	} else {
		/say Hail, ${hailer}. ${response} (${Me.PctMana}m).
	}
	
/RETURN


#EVENT clearBuffRequests "#*#p3 cbr#*#"
SUB event_clearBuffRequests(line, ChatSender)
	/bc Clearing all single, repeat, and emoted buff requests.
	/invoke ${repeatBuffsQueue.Clear}
	/invoke ${buffQueue.Clear}
	/invoke ${needsBuffPlayerQueue.Clear}
/RETURN


#event setAMTags "[#*#(msg)] e3 past #1#"
#event setAMTags "<#*#> e3 past #1#"
#event setAMTags "[#*#(msg)] p3 past #1#"
#event setAMTags "<#*#> p3 past #1#"
SUB EVENT_setAMTags(string line, string newTags)
	/if (${newTags.Upper.Equal[NONE]} || ${newTags.Upper.Equal[CLEAR]}) {
		/invoke ${assistTags.Clear}
		/bc Not using any assist spell tags now.
	} else {
		/invoke ${assistTags.Clear}
		/invoke ${assistTags.Append[${newTags}]}
		/bc Assist tags are now: ${newTags}
	}

	/invoke ${cq.Append[TAGSCHANGED;]}
/RETURN

#event addPASTTags "[#*#(msg)] p3 pastadd #1# #2#"
#event addPASTTags "<#*#> p3 pastadd #1# #2#"
#event addPASTTags "[#*#(msg)] p3 pastadd #1#"
#event addPASTTags "<#*#> p3 pastadd #1#"
SUB EVENT_addPASTTags(string line, string newTags, string dur)

	/declare i int local
	/declare tok string local

	/for i 1 to ${Math.Calc[${newTags.Count[,]}+1]}
		/varset tok ${newTags.Token[${i},,]}
		/if (${assistTags.Contains[${tok.Lower}]}==FALSE) {
			/invoke ${assistTags.Append[${tok}]}
			/bc Adding ${tok} to assist tags.
			/if (${Defined[dur]}==TRUE) {
				/call lockout past_${tok} ${dur}
				/echo adding lockout for ${tok} => ${dur}
			}
		}
	/next i

	/invoke ${cq.Append[TAGSCHANGED;]}
	/doevents flush addPASTTags
/RETURN


#event remPASTTags "[#*#(msg)] e3 pastrem #1#"
#event remPASTTags "<#*#> e3 pastrem #1#"
#event remPASTTags "[#*#(msg)] p3 pastrem #1#"
#event remPASTTags "<#*#> p3 pastrem #1#"
SUB EVENT_remPASTTags(string line, string remTags)
	/call removePastTags "${remTags}"
/RETURN



#event setTwist "[#1#(msg)] p3 twist #2#"
#event setTwist "<#1#> p3 twist #2#"

SUB event_setTwist(string line, string chatSender, string newOrder) {
	/if (${Me.Class.ShortName.NotEqual[BRD]}) /return

	/if (${newOrder.Upper.Equal[OFF]}) {
		/twist off 
		/timed 5 /twist off
		/varset doTwist FALSE
		/bc Twist is now off.
		/return
	} else {
		/varset doTwist TRUE
		/bc Twist is now on.
	}

	/invoke ${cq.Append[NEWTWIST;ORDER=${newOrder}]}

	/RETURN
}


#event perf "[#1#(msg)] p3 perf"
#event perf "<#1#> p3 perf"

SUB event_perf(string line, string chatSender, string newOrder) {
	/declare i int local

	/for i 0 to ${Math.Calc[ ${modules.Count}-1 ]}
		/echo ${modules.Item[${i}]} ::: ${Math.Calc[(${p3_${modules.Item[${i}]}_perftime}+1) / (${MacroQuest.Running} - ${perfStart}) * 100 ]}%
	/next i

	/echo idle:: ${Math.Calc[(${perfIdle}+1) / (${MacroQuest.Running} - ${perfStart}) * 100 ]}%

	/RETURN
}



#event toggleSlay "[#*#(msg)] e3 slay #1#"
#event toggleSlay "<#*#> e3 slay #1#"
#event toggleSlay "[#*#(msg)] p3 slay #1#"
#event toggleSlay "<#*#> p3 slay #1#"
SUB EVENT_toggleSlay(string line, string newMode) {
	/if (${amSlayLeader}==FALSE) /RETURN

	/if (${newMode.Upper.Equal[ON]}) {
		/varset slayEnabled TRUE
		/bc Slay Mode is now ON.
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset slayEnabled FALSE
		/bc Slay Mode is now OFF.
	}
	/RETURN
}

#event buildSlayTeam "[#*#(msg)] e3 build slay team"
#event buildSlayTeam "<#*#> e3 build slay team"
#event buildSlayTeam "[#*#(msg)] p3 build slay team"
#event buildSlayTeam "<#*#> p3 build slay team"
SUB EVENT_buildSlayTeam(string line, string newMode) {
	/if (${amSlayLeader}==FALSE) /RETURN

	/call slayBuildTeam

	/RETURN
}


#EVENT campCommand "[#1#(msg)] p3 camp #2#"
#EVENT campCommand "<#1#> p3 camp #2#"
#EVENT campCommand "[#1#(msg)] e3 camp #2#"
#EVENT campCommand "<#1#> e3 camp #2#"
SUB EVENT_campCommand(string line, string source, string newMode)
	/if (${Spawn[=${source}].ID}==NULL) {
		/bc Ignoring because ${source} is not in my zone.
		/return
	}

	/if (${newMode.Upper.Equal[OFF]}) {
		/invoke ${cq.Append[CAMPOFF;SOURCENAME=${source}]}
		/return
	}

	/if (${newMode.Upper.Equal[HERE]}) {
		/invoke ${cq.Append[CAMPON;TYPE=LOC;SOURCENAME=${source};LOCX=${Spawn[${source}].X};LOCY=${Spawn[${source}].Y};LOCZ=${Spawn[${source}].Z}]}
		/return
	}

	/if (${newMode.Upper.Equal[THERE]}) {
		/invoke ${cq.Append[CAMPON;TYPE=SPAWN;SPAWNTARGETID=${Me.ID}]}
		/return
	}


	| IF "NEWMODE" WAS NOT HERE OR THERE, THEN ASSUME IT WAS A SPAWN
	/if (${Spawn[=${newMode}].ID}!=NULL) {
		/invoke ${cq.Append[CAMPON;TYPE=SPAWN;SPAWNTARGETID=${Spawn[=${newMode}].ID}]}
		/return
	}

/RETURN


#event retreatHere "[#1#(msg)] p3 retreat #2#"
#event retreatHere "<#1#> p3 retreat #2#"
SUB event_retreatHere(string line, string who, string newMode)
	/if (${newMode.Upper.Equal[HERE]}) {
		/invoke ${cq.Append[RETREATON;LOCXYZ=${Spawn[pc ${who}].X} ${Spawn[pc ${who}].Y} ${Spawn[pc ${who}].Z}]}
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/invoke ${cq.Append[RETREATOFF;]}
	}
/RETURN


#event anchorHere "[#1#(msg)] p3 anchor #2#"
#event anchorHere "<#1#> p3 anchor #2#"
SUB event_anchorHere(string line, string who, string newMode)
	/if (${newMode.Upper.Equal[HERE]}) {
		/invoke ${cq.Append[DROPANCHOR;LOCXYZ=${Spawn[pc ${who}].X} ${Spawn[pc ${who}].Y} ${Spawn[pc ${who}].Z}]}
		/bc Achor point set at ${who}'s location. 
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/invoke ${cq.Append[RAISEANCHOR;]}
		/bc Achor raised.
	}
/RETURN

#event adistChange "#*#tells the raid, 'p3 adist #1#'"
#event adistChange "[#*#(msg)] p3 adist #1#"
#event adistChange "<#*#> p3 adist #1#"
#event adistChange "[#*#(msg)] e3 adist #1#"
#event adistChange "<#*#> e3 adist #1#"
SUB EVENT_adistChange(string line, string newDist)
	/varset meleeStickDistance ${newDist}
	/call setActiveStickParams ${stickMode}

	/varset needRestick TRUE
	/bc Melee stick distance is now ${newDist}
/RETURN



#EVENT adhocCast "[#1#(msg)] e3 cast #2#@#3#"
#EVENT adhocCast "<#1#> e3 cast #2#@#3#"
#EVENT adhocCast "[#1#(msg)] p3 cast #2#@#3#"
#EVENT adhocCast "<#1#> p3 cast #2#@#3#"
SUB EVENT_adhocCast(string line, string chatSender, string castName, string castTarget)
	/call addCastOrder "${castName.Upper}" "${castTarget}" FALSE
/RETURN


#EVENT roamWith "[#1#(msg)] e3 roam with #1#"
#EVENT roamWith "<#*#> e3 roam with #1#"
#EVENT roamWith "[#1#(msg)] p3 roam with #1#"
#EVENT roamWith "<#*#> p3 roam with #1#"
SUB EVENT_roamWith(string line, string spawnSearch)
	/if (${Spawn[=${spawnSearch}].ID}==NULL) {
		/bc Ignoring because ${spawnSearch} is not in my zone.
		/RETURN
	}

	/varset roamTargetID ${Spawn[${spawnSearch}].ID}
	/invoke ${cq.Append[ROAMLOCK;TARGETID=${roamTargetID};]}
/RETURN

#EVENT stopRoam "[#1#(msg)] e3 roam off"
#EVENT stopRoam "<#1#> e3 roam off"
#EVENT stopRoam "[#1#(msg)] p3 roam off"
#EVENT stopRoam "<#1#> p3 roam off"
SUB EVENT_stopRoam(string line, string source) {
	/invoke ${cq.Append[ROAMOFF;SOURCENAME=${source};]}
	/RETURN
}



#event autoMed "[#1#(msg)] p3 automed #2#"
#event autoMed "<#1#> p3 automed #2#"
SUB event_autoMed(string line, string who, string newMode)
	/if (${ignoreAutoMed}==TRUE) /return

	/if (${newMode.Upper.Equal[ON]}) {
		/call setAutoMed TRUE
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/call setAutoMed FALSE
	}
/RETURN


#EVENT medOffFromHit "#*# YOU for#*#points of dam#*#"
SUB EVENT_medOffFromHit
	/if (${doAutoMed}==TRUE) {
		/call pauseAutoMed 20s
	}
/RETURN

#event expSpammerResponder "#*#You gained raid experience#*#"
#event expSpammerResponder "#*#You gain party experience#*#"
#event expSpammerResponder "#*#You gain experience#*#"
SUB event_expSpammerResponder
	/call expTrackerNewExp
/return




#event buffsEnabled "[#1#(msg)] p3 buffs #2#"
#event buffsEnabled "<#1#> p3 buffs #2#"
SUB event_buffsEnabled(string line, string who, string newMode)
	/if (${newMode.Upper.Equal[ON]}) {
		/varset buffsEnabled TRUE
		/bc Buffs are now ON.
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset buffsEnabled FALSE
		/bc Buffs are now OFF.
	}
/RETURN


#event tauntSuccess "You capture #*# attention!"
#event tauntSuccess "You capture #*# with your unparalleled reproach!"
SUB EVENT_tauntSuccess(string line, string newMode)
	/if (${Target.ID}==NULL) /return
	
	/invoke ${tauntSuccessList.Append[${Target.ID}_${MacroQuest.Running}]}
/RETURN


#EVENT autoRezToggle "#*#p3 autorez #1#"
SUB event_autoRezToggle(string line, string newMode)
	/if (${newMode.Upper.Equal[ON]}) {
		/varset autoRezEnabled TRUE
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset autoRezEnabled FALSE
	} else {
		/bc Unknown AutoRez mode. Use ON or OFF.
		/return
	}

	/bc AutoRezEnabled is now: ${newMode}.

/RETURN



#EVENT toggleHeals "#*#p3 heals #1#"
SUB event_toggleHeals(string line, string newMode)
	/if (${newMode.Upper.Equal[ON]}) {
		/varset healsEnabled TRUE
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset healsEnabled FALSE
	} else {
		/bc Unknown Heals mode. Use ON or OFF.
		/return
	}

	/bc HealsEnabled is now: ${newMode}.

/RETURN


#EVENT toggleAutoMez "#*#p3 automez #1#"
SUB event_toggleAutoMez(string line, string newMode)
	/if (${newMode.Upper.Equal[ON]}) {
		/varset autoMezEnabled TRUE
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset autoMezEnabled FALSE
	} else {
		/bc Unknown autoMezEnabled mode. Use ON or OFF.
		/return
	}

	/bc AutoMezEnabled is now: ${newMode}.
	/em shrugs.

/RETURN




#EVENT toggleAutoAssist "#*#p3 autoassist #1#"
#EVENT toggleAutoAssist "#*#p3 autoass #1#"
#EVENT toggleAutoAssist "#*#p3 aa #1#"
SUB event_toggleAutoAssist(string line, string newMode)
	/if (${newMode.Upper.Equal[ON]}) {
		/varset autoAssist TRUE
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset autoAssist FALSE
	} else {
		/bc Unknown autoAssist mode. Use ON or OFF.
		/return
	}

	/bc AutoAssist is now: ${newMode}.
	/em shrugs.
/RETURN



#EVENT mezBroke "#1# has been awakened by#*#"
SUB EVENT_mezBroke(string line, string mobName)
	/declare i int local
	/declare mob spawn local

	/declare mobCount int local ${SpawnCount[=${mobName}]}

	| FIND THE CORRECT MOB. INITIATE A RE-MEZ BY REMOVING FROM MEZLIST AND LET MEZ CHECK FIND IT AGAIN
	/for i 1 to ${mobCount}
		/vardata mob NearestSpawn[${i},=${mobName}]
		/if (${autoMezList.Index[${mob.ID}]}!=-1) {
			/tar id ${mob.ID}
			/delay 10
			/if (${Target.Mezzed.ID}==NULL) {
				/invoke ${autoMezList.Erase[${autoMezList.Index[${mob.ID}]}]}
				/call wipeLockout mez_${mob.ID}
				/return
			}
		}
	/next i
/RETURN



#event charmOn "[#*#(msg)] p3 charm #1#"
SUB event_charmOn(string line, int spawnID)
	/if (${Bool[${charmUseSpell}]}==FALSE) {
		/bc I am not configured to accept charm commands.
		/RETURN
	}

	/varset charmSpawnID ${spawnID}
	/bc I will attempt to keep spawnID ${spawnID} ( ${Spawn[id ${charmSpawnID}]} ) charmed.
/RETURN

#event charmOff "[#*#(msg)] p3 charm off"
#event charmOff "<#*#> p3 charm off"
SUB event_charmOff(string line)
	/if (${Bool[${charmUseSpell}]}==FALSE) /return
	/if (${charmSpawnID}==0) /return

	/bc I will no longer try to charm spawnID ${charmSpawnID}.

	/varset charmSpawnID 0
/RETURN

#event useMelee "[#*#(msg)] p3 melee #1#"
#event useMelee "<#*#> p3 melee #1#"
SUB event_useMelee(string line, string newMode)
	/if (${newMode.Upper.Equal[ON]}) {
		/varset useMeleeCombat TRUE
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset useMeleeCombat FALSE
	} else {
		/bc Unknown useMeleeCombat mode. Use ON or OFF.
		/return
	}

	/bc useMeleeCombat is now: ${newMode}.
/RETURN


#event castFizzle "#*#Your spell fizz#*#!"
SUB event_castFizzle(string line)
	/varset castResult FIZZLE
/RETURN

#event castPower "#*#Your spell is too power#*#"
SUB event_castPower(string line)
	/varset castResult NOHOLD
/RETURN


#event castNoFunction "#*#have entered an area where levitation effects do not function#*#"
SUB event_castNoFunction(string line)
	/varset castResult NOHOLD
/RETURN

#event castResist "#*# resisted your #*#!"
#event castResist "#*# resisted the #*# spell."
SUB event_castResist(string line)
	/varset castResult RESIST
/RETURN

#event castImmune "Your target is immune to #*#"
SUB event_castImmune(string line)
	/varset castResult IMMUNE
/RETURN

#event castNoTakeHold "Your #*# did not take hold#*#"
#event castNoTakeHold "Your spell did not take hold#*#"
SUB event_castNoTakeHold(string line)
	/varset castResult NOHOLD
/RETURN


#event castInterrupted "Your #*# spell is interrupted."
#event castInterrupted "Your spell is interrupted."
SUB event_castInterrupted(string line)
	/varset castResult INTERRUPT
/RETURN


#event spewItem "[#*#(msg)] e3 item #1#"
#EVENT spewItem "<#*#> e3 item #1#"
#event spewItem "[#*#(msg)] p3 item #1#"
#EVENT spewItem "<#*#> p3 item #1#"
   SUB EVENT_spewItem(string line, string slotNumber)
      /if (${slotNumber.Upper.Equal[charm]}) /varset slotNumber 0
      /if (${slotNumber.Upper.Equal[leftear]}) /varset slotNumber 1
      /if (${slotNumber.Upper.Equal[head]}) /varset slotNumber 2
      /if (${slotNumber.Upper.Equal[face]}) /varset slotNumber 3
      /if (${slotNumber.Upper.Equal[rightear]}) /varset slotNumber 4
      /if (${slotNumber.Upper.Equal[neck]}) /varset slotNumber 5
      /if (${slotNumber.Upper.Equal[shoulder]}) /varset slotNumber 6
      /if (${slotNumber.Upper.Equal[shoulders]}) /varset slotNumber 6
      /if (${slotNumber.Upper.Equal[arms]}) /varset slotNumber 7
      /if (${slotNumber.Upper.Equal[back]}) /varset slotNumber 8
      /if (${slotNumber.Upper.Equal[leftwrist]}) /varset slotNumber 9
      /if (${slotNumber.Upper.Equal[rightwrist]}) /varset slotNumber 10
      /if (${slotNumber.Upper.Equal[ranged]}) /varset slotNumber 11
      /if (${slotNumber.Upper.Equal[hands]}) /varset slotNumber 12
      /if (${slotNumber.Upper.Equal[mainhand]}) /varset slotNumber 13
      /if (${slotNumber.Upper.Equal[offhand]}) /varset slotNumber 14
      /if (${slotNumber.Upper.Equal[leftfinger]}) /varset slotNumber 15
      /if (${slotNumber.Upper.Equal[rightfinger]}) /varset slotNumber 16
      /if (${slotNumber.Upper.Equal[chest]}) /varset slotNumber 17
      /if (${slotNumber.Upper.Equal[legs]}) /varset slotNumber 18
      /if (${slotNumber.Upper.Equal[feet]}) /varset slotNumber 19
      /if (${slotNumber.Upper.Equal[waist]}) /varset slotNumber 20
      /if (${slotNumber.Upper.Equal[powersource]}) /varset slotNumber 21
      /if (${slotNumber.Upper.Equal[ammo]}) /varset slotNumber 22

      |/bc ${Me.Inventory[${slotNumber}].HP} HP / ${Me.Inventory[${slotNumber}].Mana} MP / ${Me.Inventory[${slotNumber}].AC} AC (${Me.Inventory[${slotNumber}].ItemLink[CLICKABLE]})
      /say ${Me.Inventory[${slotNumber}].HP} HP / ${Me.Inventory[${slotNumber}].Mana} MP / ${Me.Inventory[${slotNumber}].AC} AC (${Me.Inventory[${slotNumber}].ItemLink[CLICKABLE]})
   /RETURN

#event p3find "[#*#(msg)] p3 find #1#"
#event p3find "<#*#> p3 find #1#"
SUB EVENT_p3find(string line, string itemName) {
	/if (${FindItem[${itemName}].ID}!=NULL) {
		/say Found in my inventory: ${FindItem[${itemName}].ItemLink[CLICKABLE]}
	}

	/if (${FindItemBank[${itemName}].ID}!=NULL) {
		/say Found in my bank: ${FindItemBank[${itemName}].ItemLink[CLICKABLE]}
	}

	/RETURN
}

#event mydeath "#*#You have been slain#*#"
SUB EVENT_mydeath(string line)
	/if (${beepOnDeath}==TRUE) {
		/if (${beepOnDeathSound.Equal[FALSE]}) {
			/beep
		} else {
			/beep sounds/${beepOnDeathSound}
		}
	}
/RETURN


#event takeTrade "#*#take trade!#*#"
SUB EVENT_takeTrade(string line) {
	/notify TradeWnd TRDW_Trade_Button leftmouseup
}





#event autoLootToggle "[#1#(msg)] p3 autoloot #2#"
#event autoLootToggle "<#1#> p3 autoloot #2#"
SUB event_autoLootToggle(string line, string who, string newMode)
	/if (${newMode.Upper.Equal[ON]}) {
		/varset looter TRUE
		/bc AutoLoot is now ON.
	} else /if (${newMode.Upper.Equal[OFF]}) {
		/varset looter FALSE
		/bc AutoLoot is now OFF.
	}
/RETURN
