#event followMe "<#1#> e3 follow me #2#"
#event followMe "<#1#> e3 follow me"
SUB EVENT_followMe(string line, string chatSender, string opts) {
	/if (${Defined[opts]}==FALSE) /declare opts string local ${defaultStickOpts}

	/if (${Spawn[pc =${chatSender}].ID}==${Me.ID}) /return

	/invoke ${cq.Append[FOLLOW;AUTHOR=${chatSender};SPAWNID=${Spawn[pc =${chatSender}].ID};STICKOPTS=${opts}]}
	/RETURN
}

#event stop "<#1#> e3 stop"
SUB EVENT_stop(string line, string chatSender) {
	/invoke ${cq.Append[STOP;AUTHOR=${chatSender};ASSISTHOLDTIMER=50]}
	/RETURN
}

#event kill "[#1#(msg)] p3 kill #2#"
#event kill "<#1#> p3 kill #2#"
#event kill "[#1#(msg)] assist on #2#"
#event kill "<#1#> assist on #2#"

SUB event_kill(string line, string chatSender, string spawnID) {
	/invoke ${cq.Append[KILL;AUTHOR=${chatSender};SPAWNID=${spawnID}]}
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
/RETURN

#event addPASTTags "[#*#(msg)] e3 pastadd #1#"
#event addPASTTags "<#*#> e3 pastadd #1#"
#event addPASTTags "[#*#(msg)] p3 pastadd #1#"
#event addPASTTags "<#*#> p3 pastadd #1#"
SUB EVENT_addPASTTags(string line, string newTags)
	/declare i int local 
	/declare tok string local

	/for i 1 to ${Math.Calc[${newTags.Count[,]}+1]}
		/varset tok ${newTags.Token[${i},,]}
		/if (${assistTags.Contains[${tok.Lower}]}==FALSE) {
			/invoke ${assistTags.Append[${tok}]}
			/bc Adding ${tok} to assist tags.
		}
	/next i
/RETURN


#event remPASTTags "[#*#(msg)] e3 pastrem #1#"
#event remPASTTags "<#*#> e3 pastrem #1#"
#event remPASTTags "[#*#(msg)] p3 pastrem #1#"
#event remPASTTags "<#*#> p3 pastrem #1#"
SUB EVENT_remPASTTags(string line, string remTags)
	/declare i int local 
	/declare tok string local

	/for i 1 to ${Math.Calc[${remTags.Count[,]}+1]}
		/varset tok ${remTags.Token[${i},,]}
		/if (${assistTags.Contains[${tok.Lower}]}==TRUE) {
			/invoke ${assistTags.Remove[${tok}]}
			/bc Removed ${tok} from assist tags.
		}
	/next i
/RETURN



#event setTwist "[#1#(msg)] p3 twist #2#"
#event setTwist "<#1#> p3 twist #2#"

SUB event_setTwist(string line, string chatSender, string newOrder) {
	/if (${Me.Class.ShortName.NotEqual[BRD]}) /return

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
	/if (${Spawn[${source}].ID}==NULL) {
		/bc Can't find "${source}" to use as the camp location.
		/return
	}
	
	/if (${newMode.Upper.Equal[OFF]}) {
		/call stopCamping
		/return
	}

	/if (${newMode.Upper.Equal[HERE]}) {
		/varset campX ${Spawn[${source}].X}
		/varset campY ${Spawn[${source}].Y}
		/varset campZ ${Spawn[${source}].Z}
		/varset campEnabled TRUE
		/varset campStartZoneID ${Zone.ID}
		/bc Now camping at ${source} location.
	}

	/if (${newMode.Upper.Equal[THERE]}) {
		/varset campX ${Me.X}
		/varset campY ${Me.Y}
		/varset campZ ${Me.Z}
		/varset campEnabled TRUE
		/varset campStartZoneID ${Zone.ID}
		/bc Now camping at my location.
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
