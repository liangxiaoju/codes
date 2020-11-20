#!/system/bin/sh

gpio_output_value()
{
    id=$1
    value=$2

    echo $id > /sys/class/gpio/export
    echo "out" > /sys/class/gpio/gpio$id/direction
    echo $value > /sys/class/gpio/gpio$id/value
}

usb_connected()
{
    gpio_output_value 104 1
    gpio_output_value 105 0
    gpio_output_value 115 1
    gpio_output_value 116 0
}

usb_disconnected()
{
    echo 104 > /sys/class/gpio/unexport
    echo 105 > /sys/class/gpio/unexport
    echo 115 > /sys/class/gpio/unexport
    echo 116 > /sys/class/gpio/unexport
}

USB_STATE=$(cat /sys/class/android_usb/android0/state)

case ${USB_STATE} in
    "CONFIGURED")
        usb_connected
        ;;
    "DISCONNECTED")
        usb_disconnected
        ;;
    *)
        ;;
esac

