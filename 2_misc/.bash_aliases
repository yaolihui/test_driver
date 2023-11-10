alias hs='history'
alias cd..='cd ..'
alias fmt='indent -kr '
#alias   adb='~/c/windows/adb.exe'

alias mk='_(){ t=$(date +%s); . build/envsetup.sh && lunch 27 && echo make -j16 $1 && make -j16 $1; echo && echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S);};_'