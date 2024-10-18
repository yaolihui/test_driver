alias hs='history'
alias cd..='cd ..'
alias npp='/mnt/c/Program\ Files\ \(x86\)/Notepad++/npp.exe '
alias htop='htop -d 50'

alias cd.='cd ..'
alias cd..='cd .. && cd ..'
alias cd...='cd .. && cd .. && cd ..'
alias cd....='cd .. && cd .. && cd .. && cd ..'

alias mkk='_(){\
        t=$(date +%s); cd ~/e/T_DST_AOSP/kernel ;\
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        echo -e "\e[1;33m./build/build.sh        \e[0m" &&\
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        (./build/build.sh |tee $(date +%y%m%d_%H%M%S)_build.log) && \
        echo -e "\e[1;32m ####################\e[0m" &&\
        echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S) ;\
        export PATH=~/bin:$PATH
        };_'

alias mka='_(){\
        t=$(date +%s); cd ~/e/T_DST_AOSP/aosp ;\
        . ./build/envsetup.sh && lunch aosp_oriole-eng && \
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        echo -e "\e[1;33mlunch aosp_oriole-eng   			\e[0m" &&\
        echo -e "\e[1;33mmake -j16 $@            			\e[0m" &&\
        echo -e "\e[1;33m~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\e[0m" &&\
        (make -j16 $@ |tee $(date +%y%m%d_%H%M%S)_build.log);\
        echo -e "\e[1;32m ####################\e[0m" &&\
        echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S) ;\
        export PATH=~/bin:$PATH
        };_'
