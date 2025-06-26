#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","媒体处理"),
    name = "ACL_MEDIA_VDEC",
    cmd = f"bash run.sh {oec.Context.data_path}/data {oec.Context.output_dir}/tmp"
    )
