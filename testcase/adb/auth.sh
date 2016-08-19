
setprop ro.adb.secure 1
stop adbd
nohup /cache/adb_auth >/cache/log 2>&1 &
