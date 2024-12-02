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

alias lo='_(){
    if [ ! -L $1 ]; then
		if [ -d $1 ]; then
			rm -rf $1
			echo -e "\e[1;31m [rm -rf $1]         finish ... \e[0m"
        fi
		if [ ! -d ~/out/$1 ]; then
			mkdir -p ~/out/$1
			echo -e "\e[1;33m [mkdir -p ~/out/$1] finish ... \e[0m"
		fi
		ln -s ~/out/$1 $1
		echo -e "\e[1;33m [ln -s ~/out/$1 $1] finish ... \e[0m"
    fi
};_'

alias b='_(){
    if [ "clean" == "$2" ]; then
        rm -rf QSSI.13/out/* UM.9.14/out/* && echo -e "\e[1;36m [rm */out]      finish ... \e[0m"
        git clean -df .                    && echo -e "\e[1;36m [git clean -df] finish ... \e[0m"
        git co .                           && echo -e "\e[1;36m [git co .]      finish ... \e[0m"
        if [ "pull" == "$3" ]; then
            git pull --rebase          && echo -e "\e[1;36m [git pull]      finish ... \e[0m"
        fi
    fi &&\

	lo QSSI.13/out &&\
	lo UM.9.14/out &&\

	t=$(date +%s) &&\
	branch=$(git rev-parse --abbrev-ref HEAD | cut -b 1-8) &&\
	headid=$(git rev-parse HEAD | cut -b 1-6) &&\
	stamp=$(date +%m%d%H%M) &&\
	target=${1/"all"/"usdg"}_${branch}_${stamp}_${headid} &&\
	echo -e "\e[1;33m $target \e[0m" &&\
    (rm /home2/target ; ./build.sh --$1 -j$(nproc) && ./unpack_tools.sh ) |tee build_${stamp}_log &&\
    mv -f QCM6490_Android13_R03_r028_unpacking_tools_20240605/qfil_download_ufs /home2/$target &&\
    cp UM.9.14/out/dist/merged-qssi_lahaina-ota.zip /home2/${target}/update.zip &&\
    echo $target > /home2/target &&\
    rm -rf QCM6490_Android13_R03_r028_unpacking_tools_20240605 SG560DCNPAR03A02_BP01.002_prebuilt &&\
    echo; echo -e "\e[1;32m$(date -d @$[$(date +%s)-$t] +%H:%M:%S) stamp=$stamp headid=$headid target=$target \e[0m"; echo;
};_'

alias ba='cd ~/MPSV_Quectel6490A13 && b all'
alias bac='cd ~/MPSV_Quectel6490A13 && b all clean'
alias bacp='cd ~/MPSV_Quectel6490A13 && b all clean pull'

alias ma='cd ~/mdm && b all'
alias mac='cd ~/mdm && b all clean'
alias macp='cd ~/mdm && b all clean pull'

alias mu='cd ~/mdm && b user'
alias muc='cd ~/mdm && b user clean'
alias mucp='cd ~/mdm && b user clean pull'

alias tt='c=0; while [ ! -e ~/target ]; do echo waiting... $((c++)); sleep 5; done'

