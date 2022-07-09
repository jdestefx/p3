SUB p3_buffrequests_init {
	|WORKING VARS
	/declare buffQueue list outer
	/declare brOOMEmoteTimer timer outer 0
	/declare repeatBuffsQueue list outer
	/declare needsBuffPlayerQueue list outer 

	| INI DEFAULTS
	/declare brEnabled bool outer 				FALSE
	/declare brSameGuildOnly bool outer 		TRUE
	/declare brMinMana int outer 				25
	/declare brReplyViaTell bool outer 			TRUE
	/declare brDoEmotes bool outer 				TRUE

	| IMPORT INI
	/varset brEnabled 				${getBotIniOption[BuffRequestOptions.AcceptBuffRequests,${brEnabled}]}
	/varset brSameGuildOnly 		${getBotIniOption[BuffRequestOptions.SameGuildOnly,${brSameGuildOnly}]}
	/varset brMinMana				${getBotIniOption[BuffRequestOptions.MinimumMana,${brMinMana}]}
	/varset brReplyViaTell			${getBotIniOption[BuffRequestOptions.ReplyViaTell,${brReplyViaTell}]}
	/varset brDoEmotes				${getBotIniOption[BuffRequestOptions.DoEmotes,${brDoEmotes}]}

	/if (${iniBuffRequests[1,1].Equal[-]}) /varset brEnabled FALSE

	/RETURN
}


SUB buffrequestsMaintenance() {
	/if (${recentlyZoned}==TRUE) /return
	/if (${brEnabled}==TRUE) {
		/if (${Me.Invis}==TRUE) /return
		/if (${Me.Casting.ID}!=0) /return
		/if (${movingTimer}>0 && ${canCastWhileMoving}==FALSE) /return

		/call checkNeedsBuffsQueue
		/call checkBuffRequestQueue
		/call checkRepeatBuffsQueue
	}

	/RETURN
}


SUB checkNeedsBuffsQueue() {
	/if (${needsBuffPlayerQueue.Count}==0) /return
	/if (${buffQueue.Count}>0) /return

	/declare i int local
	/declare buffName string local
	/declare spawnID int local 
	/declare queueEntry string local
	/declare spellName string local
	/declare spawnName string local

	/varset spawnID ${Spawn[pc ${needsBuffPlayerQueue.Head[]}].ID}
	/varset spawnName ${Spawn[id ${spawnID}].CleanName}

	/call fullTarget ${spawnID}
	/if (${Macro.Return}==FALSE) /RETURN

	/for i 1 to ${iniBuffRequests.Size}
		/if (${iniBuffRequests[${i},1].Equal[-]}) /break

		/varset spellName ${iniBuffRequests[${i},1]}
		/varset queueEntry ${spawnID}_${spellName}_${i}

		/if (${Select[EMOTABLE,${iniBuffRequests[${i},${bKeyword}]},]}==0) /continue

		/if (${iniBuffRequests[${i},${bRequireTargetClass}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spawnID}].Class.ShortName},${iniBuffRequests[${i},${bRequireTargetClass}]}]}==0) /continue
		}

		/if (${Target.Buff[${spellName}].ID}==NULL) {
			/invoke ${buffQueue.Append[${queueEntry}]}
		}

	/next i

	/RETURN
}

SUB checkBuffRequestQueue() {
	/declare e string local
	/declare targetID int local
	/declare buffName string local
	/declare createRecheckTimer bool local FALSE
	/declare ei int local
	/declare i int local
	/declare lockoutSecs int local

	/if (${buffQueue.Count}==0) /return

	/if (${Me.PctMana} < ${brMinMana} && ${buffQueue.Count}>0) {
		/if (${brOOMEmoteTimer}==0) {
			/em is low on mana and will continue buff requests later.
			/varset brOOMEmoteTimer 60s
		}
		/return
	}

	/for i 0 to ${Math.Calc[ ${buffQueue.Count}-1 ]}
		/varset e ${buffQueue.Item[${i}]}
		
		/varset targetID ${e.Token[1,_]}
		/varset buffName ${e.Token[2,_]}
		/varset ei ${e.Token[3,_]}
		/varset createRecheckTimer ${e.Token[4,_]}

		/if (${Spawn[pc id ${targetID}].ID}==NULL) {
			/invoke ${buffQueue.Erase[${i}]}
			/break
		}

		/if (${iniBuffRequests[${ei},${bSelfOverMana}].NotEqual[-]}) {
			/if (${Me.PctMana} < ${iniBuffRequests[${ei},${bSelfOverMana}]}) /continue
		}

		/if (${iniBuffRequests[${ei},${bMemInGem}].NotEqual[-]} && ${Me.Gem[${buffName}]}==NULL) {
			/call memorizeSpellInGem "${buffName}" ${iniBuffRequests[${ei},${bMemInGem}]}
		}

		/if (${isCastReady[${buffName}]}==FALSE) {
			/continue
		}

		/invoke (${cq.Append[CAST;SPELL=${buffName};TARGETID=${targetID};SRC=iniBuffRequests;SRCIDX=${ei};ONRESULT=onBuffRequestCastResult;BUFFQUEUEIDX=${i}]})

	/next i

	/RETURN
}

SUB onBuffRequestCastResult(string result, string opts) {
	/declare buffQueueIndex int local ${getStackCommandOpt[${opts},BUFFQUEUEIDX]}
	/declare buffIniIndex int local ${getStackCommandOpt[${opts},SRCIDX]}
	/declare lockoutSecs int local 0

	/declare targetID int local ${buffQueue.Item[${buffQueueIndex}].Token[1,_]}
	/declare buffName string local ${buffQueue.Item[${buffQueueIndex}].Token[2,_]}
	/declare repeatMode bool local ${buffQueue.Item[${buffQueueIndex}].Token[4,_]}

	/if (${result.Find[PASS_]}!=NULL || ${result.Find[FAIL_NOHOLD]}!=NULL) {
		/echo Buff request satisfied. Removing from queue.
		/invoke ${buffQueue.Erase[${buffQueueIndex}]}		

		/if (${repeatMode}==TRUE) {
			/varset lockoutSecs ${getSpellDurationByRequestName[${buffName}]}
			/if (${lockoutSecs}==0) /varset lockoutSecs 30
			/if (${lockoutSecs}>45) /varset lockoutSecs ${Math.Calc[${lockoutSecs}-12]}
			/call lockout "${targetID}_${buffIniIndex}" ${lockoutSecs}s
			/echo Locked out ${buffName} from repeat-buff queue for ${lockoutSecs}s.
		}


	} else {
		/echo Buff did not apply. Will leave in queue.
	}

	/RETURN
}

SUB checkRepeatBuffsQueue
	/declare i int local 
	/declare e string local
	/declare targetID int local
	/declare entryIndex int local
	/declare castRequest string local

	/if (${repeatBuffsQueue.Count}==0) /return

	/for i 0 to ${Math.Calc[ ${repeatBuffsQueue.Count}-1 ]}
		/varset e ${repeatBuffsQueue.Item[${i}]}
		/if (${isLockoutExpired[${e}]}==TRUE) {
			/varset targetID ${e.Token[1,_]}
			/varset entryIndex ${e.Token[2,_]}
			/varset castRequest ${iniBuffRequests[${entryIndex},1]}

			/if (${buffQueue.Index[${targetID}_${castRequest}_${entryIndex}_1]}>-1) /continue
			/invoke ${buffQueue.Append[${targetID}_${castRequest}_${entryIndex}_1]}
		}
	/next i
/RETURN

