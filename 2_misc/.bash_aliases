alias hs='history'
alias cd..='cd ..'
alias fmt='indent -kr '
#alias   adb='~/c/windows/adb.exe'
alias  push='git push wiko HEAD:refs/for/MTK_S0_MP1_DRVONLY_V1.0'
alias start='explorer.exe'
alias     f='find . -name '
alias croot='cd ~/d/m3308/android'
alias  ctee='cd ~/d/m3308/android/vendor/mediatek/proprietary/trustzone/trustonic'
alias   cfp='cd ~/d/m3308/android/vendor/mediatek/proprietary/hardware/fingerprint '
alias   dtc='~/bin/dtc -I dtb -O dts'
alias   dts='croot &&  adb root && adb pull /sys/firmware/fdt && ~/bin/dtc -I dtb -O dts fdt -o fdt.txt && npp fdt.txt'

alias     b='t=$(date +%s) && \
	     croot && \
	     export ODM_TARGET_PROJECT=Romanee && source build/envsetup.sh && export OUT_DIR=out && lunch vnd_Romanee-userdebug && \
	     make -j16 bootimage dtboimage krn_images  2>&1 |tee ../log/b$t.log  && \
	     vendor/mediatek/proprietary/scripts/sign-image/sign_image.sh && \
	     mv ../log/b$t.log  ../log/b${t}_ok.log && \
	     echo && echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)'


alias mktee='t=$(date +%s) && \
	     croot && \
	     export ODM_TARGET_PROJECT=Romanee && source build/envsetup.sh && export OUT_DIR=out && lunch vnd_Romanee-userdebug && \
	     make -j16 tee.img 2>&1 |tee ../log/mktee$t.log && \
	     vendor/mediatek/proprietary/scripts/sign-image/sign_image.sh && \
	     mv ../log/mktee$t.log   ../log/mktee${t}_ok.log && \
	     echo && echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)'

alias  mktz='t=$(date +%s) && \
	     croot && \
	     export ODM_TARGET_PROJECT=Romanee && source build/envsetup.sh && export OUT_DIR=out && lunch vnd_Romanee-userdebug && \
	     make -j16 trustzone 2>&1 |tee ../log/mktz$t.log && \
	     vendor/mediatek/proprietary/scripts/sign-image/sign_image.sh && \
	     mv ../log/mktz$t.log   ../log/mktz${t}_ok.log && \
	     echo && echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)'

alias mksuper='t=$(date +%s) && \
	     croot && \
	     export ODM_TARGET_PROJECT=Romanee && source build/envsetup.sh && export OUT_DIR=out && lunch vnd_Romanee-userdebug && \
	     make -j16 superimage 2>&1 |tee ../log/mksuper$t.log && \
	     vendor/mediatek/proprietary/scripts/sign-image/sign_image.sh && \
	     mv ../log/mksuper$t.log   ../log/mksuper${t}_ok.log && \
	     echo && echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)'

alias  mkdtbo='t=$(date +%s) && \
	     croot && \
 	     export ODM_TARGET_PROJECT=Romanee && source build/envsetup.sh && export OUT_DIR=out && lunch vnd_Romanee-userdebug && \
	     make -j16 dtboimage 2>&1 |tee ../log/mkdtbo$t.log && \
	     vendor/mediatek/proprietary/scripts/sign-image/sign_image.sh && \
	     mv ../log/mkdtbo$t.log   ../log/mkdtbo${t}_ok.log && \
	     echo && echo $(date -d @$[$(date +%s)-$t-28800] +%H:%M:%S)'
alias       e='export ODM_TARGET_PROJECT=Romanee && source build/envsetup.sh && export OUT_DIR=out && lunch vnd_Romanee-userdebug'
