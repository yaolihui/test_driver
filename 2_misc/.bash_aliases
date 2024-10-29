alias hs='history'
alias npp='/mnt/c/Program\ Files\ \(x86\)/Notepad++/npp.exe '
alias htop='htop -d 50'

alias cd.='cd ..'
alias cd..='cd .. && cd ..'
alias cd...='cd .. && cd .. && cd ..'
alias cd....='cd .. && cd .. && cd .. && cd ..'

alias gitc='git clean -df . && git co .'

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


alias b='_(){
	t=$(date +%s)
	stamp=$(date +%m%d_%H%M) && echo $stamp
	cd ~/MPSV_Quectel6490A13
	( ./build.sh --all -j20 && ./unpack_tools.sh ) |tee build_log_$stamp.txt
	cp -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605/qfil_download_ufs ~/ufs_$stamp
	cp UM.9.14/out/dist/merged-qssi_lahaina-ota.zip ~/ufs_$stamp/update.zip
	echo ufs_$stamp > ~/target
	echo; echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)
};_'

alias bc='_(){
	t=$(date +%s)
	stamp=$(date +%m%d_%H%M) && echo $stamp
	cd ~/MPSV_Quectel6490A13
	rm -rf QSSI.13/out; rm -rf UM.9.14/out
	gitc && git pull
	( ./build.sh --all -j20 && ./unpack_tools.sh ) |tee build_log_$stamp.txt
	cp -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605/qfil_download_ufs ~/ufs_$stamp
	cp UM.9.14/out/dist/merged-qssi_lahaina-ota.zip ~/ufs_$stamp/update.zip
	echo ufs_$stamp > ~/target
	echo; echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)
};_'

alias bu='_(){
	t=$(date +%s)
	stamp=$(date +%m%d_%H%M) && echo $stamp
	cd ~/ngf
	( ./build.sh --user -j20 && ./unpack_tools.sh ) |tee build_log_$stamp.txt
	cp -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605/qfil_download_ufs ~/user_$stamp
	cp UM.9.14/out/dist/merged-qssi_lahaina-ota.zip ~/user_$stamp/update.zip
	echo; echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)
};_'

alias bcu='_(){
	t=$(date +%s)
	stamp=$(date +%m%d_%H%M) && echo $stamp
	cd ~/ngf
	rm -rf QSSI.13/out; rm -rf UM.9.14/out
	gitc && git pull
	( ./build.sh --user -j20 && ./unpack_tools.sh ) |tee build_log_$stamp.txt
	cp -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605/qfil_download_ufs ~/user_$stamp
	cp UM.9.14/out/dist/merged-qssi_lahaina-ota.zip ~/user_$stamp/update.zip
	echo; echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)
};_'