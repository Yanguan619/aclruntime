if [[ -f /etc/Ascend/ascend_cann_install.info ]]; then
    mv /etc/Ascend/ascend_cann_install.info /etc/Ascend/ascend_cann_install.info.bac
fi
bash install_cann_packages.sh "$1" "$2"
rst=$?
if [[ -f /etc/Ascend/ascend_cann_install.info.bac ]]; then
    mv /etc/Ascend/ascend_cann_install.info.bac /etc/Ascend/ascend_cann_install.info
fi
exit $rst