#/bin/sh
bashpath="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
installpath=/usr/local/arm_inst/sdk
rm $installpath/lib/libh264enc_arm.a  $installpath/lib/libh264dec_arm.a  $installpath/lib/libsvcdec_arm.a  $installpath/lib/libsvcenc_arm.a  $installpath/lib/libcommonandroidneon.a $installpath/lib/libffmpegdemux_arm.a
cd $bashpath/H264encoder/platform/android/;make;
cd $bashpath/H264decoder/platform/android/;make;
cd $bashpath/svcencoder/platform/android/;make;
cd $bashpath/svcdecoder/platform/android/;make;
cd $bashpath/neon_asm/common_android_neon/;make;
cd $bashpath/ffmpeg_demux/platform/android/;make;
cp $bashpath/H264encoder/inc/h264_enc_api.h $installpath/include/
cp $bashpath/H264decoder/inc/h264_dec_api.h $installpath/include/
cp $bashpath/svcdecoder/inc/svc_dec_api.h $installpath/include/
cp $bashpath/svcencoder/inc/svc_enc_api.h $installpath/include/

