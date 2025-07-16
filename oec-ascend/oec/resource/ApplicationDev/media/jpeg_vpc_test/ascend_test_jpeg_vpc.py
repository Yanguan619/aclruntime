#encoding: utf-8
import oec

oec.TestCase(
    group= ("应用开发","媒体处理"),
    name = "ACL_MEDIA_JPEGD_VPC_CROP_PASTE",
    cmd = f"bash run.sh 2 {oec.Context.data_path} {oec.Context.output_dir}/tmp/dvpp_jpeg_vpc"
    )


oec.TestCase(
    group= ("应用开发","媒体处理"),
    name = "ACL_MEDIA_JPEGE",
    cmd = f"bash run.sh 3 {oec.Context.data_path} {oec.Context.output_dir}/tmp/dvpp_jpeg_vpc"
    )

oec.TestCase(
    group= ("应用开发","媒体处理"),
    name = "ACL_MEDIA_JPEG_YUV_VPC_RESIZE",
    cmd = f"bash run.sh 4 {oec.Context.data_path} {oec.Context.output_dir}/tmp/dvpp_jpeg_vpc"
    )