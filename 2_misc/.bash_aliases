alias hs='history'
alias cd..='cd ..'
alias fmt='indent -kr '
#alias   adb='~/c/windows/adb.exe'

alias mka='_(){\
        t=$(date +%s); cd ~/d/T_DST_AOSP/aosp ;\
        . ./build/envsetup.sh && lunch 27 && \
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        echo -e "\e[1;35;47m lunch 27           \e[0m" &&\
        echo -e "\e[1;35;47m make -j16 $1       \e[0m" &&\
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        (make -j16 $1 |tee build.log);\
        echo -e "\e[1;32m ####################\e[0m" &&\
        echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S) ;\
        };_'