import oec

class OpenMPICheckCase(oec.TestCase):
    def execute_command(self):
        super().execute_command()
        if self.is_passed():
            self.context.env["LD_LIBRARY_PATH"] = f'/usr/local/openmpi/lib:{self.context.env.get("LD_LIBRARY_PATH","")}'
            

OpenMPICheckCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_OPENMPI_CHECK",
    cmd=f"python3 check_mpi.py {oec.Context.data_path} {oec.Context.infomation.get('Count',1)}"
    )

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_BUILD",
    cmd=f"bash ./build_hccl_test.sh",
    exclude=None
    )

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_ALL_REDUCE_TEST",
    cmd=f"mpirun --allow-run-as-root -n 8 $ASCEND_HOME_PATH/tools/hccl_test/bin/all_reduce_test -b 8K -e 64M -f 2 -d fp32 -o sum -p 8",
    exclude=None
    )

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_ALL_GATHER_TEST",
    cmd=f"mpirun --allow-run-as-root -n 8 $ASCEND_HOME_PATH/tools/hccl_test/bin/all_gather_test -b 8K -e 64M -f 2 -d fp32 -p 8",
    exclude=None
    )

oec.TestCase(
    group= ("模型开发","集合通信"),
    name = "MODEL_HCCL_BROADCAST_TEST",
    cmd=f"mpirun --allow-run-as-root -n 8 $ASCEND_HOME_PATH/tools/hccl_test/bin/broadcast_test -b 8K -e 64M -f 2 -d fp32 -p 8",
    exclude=None
    )

