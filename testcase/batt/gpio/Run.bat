
adb push event /data/
adb push action.sh /data/
adb push wrapper /data/

adb shell chmod 777 /data/event
adb shell chmod 777 /data/action.sh
adb shell chmod 777 /data/wrapper

adb shell /data/wrapper

pause
