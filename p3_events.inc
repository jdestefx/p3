#event followMe "<#1#> e3 follow me #2#"
#event followMe "<#1#> e3 follow me"
SUB EVENT_followMe(string line, string chatSender, string opts) {
   /if (${Defined[opts]}==FALSE) /declare opts string local ${defaultStickOpts}
   /invoke ${cq.Append[FOLLOW:AUTHOR=${chatSender}:SPAWNID=${Spawn[pc ${chatSender}].ID}:STICKOPTS=${opts}]}
   /RETURN
}

#event stop "<#1#> e3 stop"
SUB EVENT_stop(string line, string chatSender) {
   /invoke ${cq.Append[STOP:AUTHOR=${chatSender}]}
   /RETURN
}

#event kill "[#1#(msg)] e3 kill #2#"
#event kill "<#1#> e3 kill #2#"
SUB event_kill(string line, string chatSender, string spawnID) {
   /echo kill ${spawnID}
   /invoke ${cq.Append[KILL:AUTHOR=${chatSender}:SPAWNID=${spawnID}]}
   /RETURN
}
