alias hs='history'
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
        };_'

alias gitc='git clean -df . && git co . && echo -e "\e[1;36mgit clean finish ...\e[0m"'

alias b='_(){
	t=$(date +%s) &&\
	branch=$(git rev-parse --abbrev-ref HEAD | cut -b 1-8) &&\
	headid=$(git rev-parse HEAD | cut -b 1-6) &&\
	stamp=$(date +%m%d%H%M) &&\
	target=${1/"all"/"usdg"}_${branch}_${stamp}_${headid} &&\
	echo -e "\e[1;33m $target \e[0m" &&\
	if [ "clean" == "$2" ]; then
                rm -rf QSSI.13/out UM.9.14/out && echo -e "\e[1;36m [remove out] finish ...    \e[0m"
                git clean -df .                && echo -e "\e[1;36m [git clean -df] finish ... \e[0m"
                git co .                       && echo -e "\e[1;36m [git co .] finish ...      \e[0m"
                git pull                       && echo -e "\e[1;36m [git pull] finish ...      \e[0m"
    fi &&\
	(rm ~/target ; ./build.sh --$1 -j20 && ./unpack_tools.sh ) |tee build_${stamp}_log &&\
	mv -f QCM6490_Android13_R03_r028_unpacking_tools_20240605/qfil_download_ufs ~/$target &&\
	cp UM.9.14/out/dist/merged-qssi_lahaina-ota.zip ~/${target}/update.zip &&\
	echo $target > ~/target &&\
	rm -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605 SG560DCNPAR03A02_BP01.002_prebuilt &&\
	echo; echo -e "\e[1;32m$(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S) stamp=$stamp head=$head target=$target \e[0m"; echo;
};_'

alias ba='cd ~/MPSV_Quectel6490A13 && b all'

alias b2='cd ~/ngf && b all'

alias bu='cd ~/ngf && b user'

alias tt='c=0; while [ ! -e ~/target ]; do echo waiting... $((c++)); sleep 5; done'
