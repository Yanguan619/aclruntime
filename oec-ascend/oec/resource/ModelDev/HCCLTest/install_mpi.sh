#!/bin/bash

#!/bin/bash
echo $1 $2

# 检查是否提供了参数
if [ $# -ne 2 ]; then
    echo "Usage: $0 <data_path> <radix_value>"
    exit 1
fi
cd $1
new_radix=$2
mkdir -p tmp
cd tmp
if [[ -d "openmpi-4.1.5" ]]; then
    echo the directory openmpi-4.1.5 is existing, remove it and continue
    rm -rf openmpi-4.1.5
fi
tar -zxf ../openmpi-4.1.5.tar.gz
cd openmpi-4.1.5

# 使用sed进行替换
sed -i "s/mca_routed_radix_component\.radix = 64;/mca_routed_radix_component.radix = ${new_radix};/" orte/mca/routed/radix/routed_radix_component.c
sed -i "s/mca_plm_rsh_component\.num_concurrent = 128;/mca_plm_rsh_component\.num_concurrent = ${new_radix};/" orte/mca/plm/rsh/plm_rsh_component.c
echo ==========================================================
./configure --disable-fortran --enable-ipv6 --prefix=/usr/local
echo ----------------------------------------------------------
make -j
echo ----------------------------------------------------------
make install
echo ==========================================================