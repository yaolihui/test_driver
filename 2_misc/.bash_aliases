alias hs='history'
alias cd..='cd ..'
alias fmt='indent -kr '
#alias   adb='~/c/windows/adb.exe'

alias mka='_(){\
        t=$(date +%s); cd ~/e/T_DST_AOSP/aosp ;\
        . ./build/envsetup.sh && lunch aosp_oriole-eng && \
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        echo -e "\e[1;33mlunch aosp_oriole-eng   			\e[0m" &&\
        echo -e "\e[1;33mmake -j16 $@            			\e[0m" &&\
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        (make -j16 $@ |tee build.log);\
        echo -e "\e[1;32m ####################\e[0m" &&\
        echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S) ;\
        };_'