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
        };_'

alias b='_(){
	cd ~/MPSV_Quectel6490A13 &&\
	t=$(date +%s) &&\
	headid=$(git rev-parse HEAD | cut -b 1-11) &&\
	stamp=$(date +%m%d%H%M) &&\
	targetDir=~/ufs_${headid}_$stamp &&\
	echo -e "\e[1;33mstamp=$stamp headid=$headid targetDir=$targetDir\e[0m" &&\
	(rm ~/target ; ./build.sh --all -j20 && ./unpack_tools.sh ) |tee build_log_$stamp.txt &&\
	cp -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605/qfil_download_ufs $targetDir &&\
	cp UM.9.14/out/dist/merged-qssi_lahaina-ota.zip $targetDir/update.zip &&\
	echo $targetDir > ~/target &&\
	rm -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605 SG560DCNPAR03A02_BP01.002_prebuilt
	echo; echo -e "\e[1;33$(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S) stamp=$stamp headid=$headid targetDir=$targetDir\e[0m"; echo;
};_'
