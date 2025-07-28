#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","媒体处理"),
    name = "ACL_MEDIA_VDEC",
    tags = [oec.app_dev, oec.acl, oec.media],
    cmd = f"bash run.sh {oec.Context.data_path}/data {oec.Context.output_dir}/tmp"
    )
