adb shell mkfifo /data/vendor/camera/fifo

adb remount
adb push camera.device@3.4-impl.so /vendor/lib64/
adb push ffmpeg/ffmpeg /data/vendor/camera/
adb push 720_h264.mp4 /data/vendor/camera/720_h264.mp4

adb shell "stop vendor.camera-provider-2-4 && sleep 1 && start vendor.camera-provider-2-4"

URL="/data/vendor/camera/720_h264.mp4"
#URL="rtsp://10.129.144.18:8086/"

adb shell "/data/vendor/camera/ffmpeg -y -i ${URL} -pix_fmt nv12 -f rawvideo -s 1440x1080 /data/vendor/camera/fifo"

#setsid sh -c "while true; do /data/vendor/camera/ffmpeg -y -i ${URL} -pix_fmt nv12 -f rawvideo -s 1440x1080 -r 8 /data/vendor/camera/fifo; sleep 1; done"
