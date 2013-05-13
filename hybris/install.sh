LIB_TARGET=/data/ubuntu/usr/lib/arm-linux-gnueabihf
TEST_TARGET=/data/ubuntu/root

adb push libEGL.so.1.0 $LIB_TARGET
adb push libEGL.so.1 $LIB_TARGET
adb push libsf.so.1.0 $LIB_TARGET
adb push libsf.so.1 $LIB_TARGET
adb push libis.so.1.0 $LIB_TARGET
adb push libis.so.1 $LIB_TARGET
adb push libhardware.so.1.0 $LIB_TARGET
adb push libhardware.so.1 $LIB_TARGET
adb push libhybris_ics.so $LIB_TARGET
adb push libGLESv2.so.2 $LIB_TARGET
adb push libGLESv2.so.2.0 $LIB_TARGET
adb push libcamera.so.1.0 $LIB_TARGET
adb push libcamera.so.1 $LIB_TARGET
adb push libcamera.so $LIB_TARGET
adb push libmediaplayer.so.1.0 $LIB_TARGET
adb push test_egl $TEST_TARGET
adb push test_glesv2 $TEST_TARGET
adb push test_hw $TEST_TARGET
adb push test_sensors $TEST_TARGET
adb push test_sf $TEST_TARGET
adb push test_camera $TEST_TARGET
adb push test_recorder $TEST_TARGET

