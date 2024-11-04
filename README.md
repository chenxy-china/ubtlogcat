要编译到android系统中，需要在设备的mk文件中（比如SDK/device/rockchip/rk3588/V8/V8.mk)增加
PRODUCT_PACKAGES += ubtlogcat

要实现开机自动启动,需要在SDK/system/core/rootdir/init.rc中增加
service ubtlogcat /system/bin/ubtlogcat
    class core
    seclabel u:r:ueventd:s0
启动后，会自动运行保存dmesg和logdata数据
