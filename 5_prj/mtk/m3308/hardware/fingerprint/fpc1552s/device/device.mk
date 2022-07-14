# ----------------------------
# Add feature flags below
# ----------------------------
FPC_CONFIG_ALGO_ANTI_SPOOF=1
FPC_CONFIG_IDENTIFY_AT_ENROL=1
FPC_CONFIG_MALLOC_NOT_USE_EXFUN=1
FPC_CONFIG_MAX_NR_TEMPLATES=5
FPC_CONFIG_NO_TU=0
FPC_CONFIG_SENSE_TOUCH_CALIBRATION_PATH=/data/fpc/calibration_sense_touch.dat
FPC_CONFIG_SENSORTEST=1
FPC_CONFIG_TA_DB_BLOB=1
FPC_CONFIG_TA_GP_SUPPORT_ENABLE=0
FPC_DEFECTIVE_PIXEL_LIST_SIZE=5000
FPC_TBASE_VER=KINIBI
FPC_TEE_RUNTIME=TBASE
LIBFPC_NAME=libfpc1552_S_nav_as_debug.a
FPC_CONFIG_DEBUG=1
FPC_CONFIG_ENGINEERING=1
FPC_CONFIG_PRODUCT_DEFAULT=FPC_PRODUCT_TYPE1552_S
FPC_SENSOR_SDK_LOG_LEVEL=1
FPC_HAL_SHARED_LIB=1

#
# File included from device/<manufacture>/<>/<device>.mk
# Packages to include into the build
# PRODUCT_PACKAGES += \
#     fingerprint.fpc.default \
#     com.fingerprints.extension \
#     com.fingerprints.extension.xml

#RODUCT_PACKAGES += fpc_tee_test


# ifeq ($(FPC_CONFIG_SENSORTEST),1)
# PRODUCT_PACKAGES += SensorTestTool
# endif

# ifeq ($(FPC_CONFIG_ENGINEERING),1)
# PRODUCT_PACKAGES += ImageCollection \
#                    ImageTool \
#                    ImageSubscription \
#                    SensorTestTool \
#                    ImageInjection
# endif

# ifeq ($(FPC_CONFIG_NAVIGATION),1)
# PRODUCT_PACKAGES += NavigationTest
# endif
