if [[ -f /etc/Ascend/ascend_cann_install.info ]]; then
    mv /etc/Ascend/ascend_cann_install.info /etc/Ascend/ascend_cann_install.info.bac
fi
bash install_cann_packages.sh "$OEC_WORKDIR" "$OEC_OUTPUT_PATH"
rst=$?
if [[ -f /etc/Ascend/ascend_cann_install.info.bac ]]; then
    mv /etc/Ascend/ascend_cann_install.info.bac /etc/Ascend/ascend_cann_install.info
fi
exit $rst