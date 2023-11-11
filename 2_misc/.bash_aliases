alias hs='history'
alias cd..='cd ..'
alias fmt='indent -kr '
#alias   adb='~/c/windows/adb.exe'

alias mka='_(){ cd ~/T_DST_AOSP/aosp; t=$(date +%s);\
        . ./build/envsetup.sh && lunch 27 && \
        echo -e "\e[33m ~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        echo -e "\e[35m make -j20 $1 \e[0m"        &&\
        echo -e "\e[33m ~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        make -j20 $1 && \
        echo -e "\e[32m ####################\e[0m" &&\
        echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S);};_'