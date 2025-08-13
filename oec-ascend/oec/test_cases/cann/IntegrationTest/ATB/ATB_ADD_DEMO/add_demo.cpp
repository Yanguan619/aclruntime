#include <acl/acl.h>
#include <atb/atb_infer.h>
#include <iostream>
#include <unistd.h>

using namespace atb;
void warmup(atb::Operation *operation, atb::VariantPack &pack, uint64_t &workspaceSize, atb::Context *context) {
    operation->Setup(pack, workspaceSize, context);
    void *workSpace = nullptr;
    int ret = 0;
    if (workspaceSize != 0) {
        ret = aclrtMalloc(&workSpace, workspaceSize, ACL_MEM_MALLOC_HUGE_FIRST);
        if (ret != 0) {
            std::cout << "alloc error!";
            exit(100);
        }
    }

    operation->Execute(pack, (uint8_t*)workSpace, workspaceSize, context);
}

void exeop(atb::Operation *operation, atb::VariantPack &pack, uint64_t &workspaceSize, atb::Context *context) {
    operation->Setup(pack, workspaceSize, context);
    void *workSpace = nullptr;
    int ret = 0;
    if (workspaceSize != 0) {
        ret =aclrtMalloc(&workSpace, workspaceSize, ACL_MEM_MALLOC_HUGE_FIRST);
        if(ret != 0) {
            std::cout<< "alloc error!";
            exit(200);
        }
    }

    operation->Execute(pack, (uint8_t*)workSpace, workspaceSize, context);
    sleep(1);
    std::cout<<"sleep(1)" <<std::endl;
}

int main() {
    int deviceId = 1;
    aclError status = aclrtSetDevice(deviceId);
    atb::infer::ElewiseParam param;
    param.elewiseType = atb::infer::ElewiseParam::ELEWISE_ADD;
    atb::Operation *op =nullptr;
    atb::Status st = atb::CreateOperation(param, &op);
    atb::Tensor a;
    a.desc.dtype = ACL_FLOAT16;
    a.desc.format = ACL_FORMAT_ND;
    a.desc.shape.dimNum = 2;
    a.desc.shape.dims[0] = 3;
    a.desc.shape.dims[1] = 3;
    a.dataSize = Utils::GetTensorSize(a);
    atb::Tensor b;
    b.desc.dtype = ACL_FLOAT16;
    b.desc.format = ACL_FORMAT_ND;
    b.desc.shape.dimNum = 2;
    b.desc.shape.dims[0] = 3;
    b.desc.shape.dims[1] = 3;
    b.dataSize = Utils::GetTensorSize(b);
    atb::Tensor output;
    output.desc.dtype = ACL_FLOAT16;
    output.desc.shape.dimNum=2;
    output.desc.shape.dims[0] = 3;
    output.desc.shape.dims[1] = 3;
    output.dataSize = Utils::GetTensorSize(b);
    status = aclrtMalloc(&a.deviceData, a.dataSize, ACL_MEM_MALLOC_HUGE_FIRST);
    status = aclrtMalloc(&b.deviceData, b.dataSize, ACL_MEM_MALLOC_HUGE_FIRST);
    status = aclrtMalloc(&output.deviceData, output.dataSize, ACL_MEM_MALLOC_HUGE_FIRST);
    atb::VariantPack variantPack;
    variantPack.inTensors = {a ,b};
    variantPack.outTensors = {output};
    atb::Context *context = nullptr;
    st = atb::CreateContext(&context);
    aclrtStream stream = nullptr;
    status = aclrtCreateStream(&stream);
    context->SetExecuteStream(stream);
    uint64_t workspaceSize = 0;
    warmup(op, variantPack, workspaceSize, context);
    exeop(op, variantPack, workspaceSize, context);
    exeop(op, variantPack, workspaceSize, context);
    st = op->Setup(variantPack, workspaceSize, context);
    void *workspace =nullptr;
    status = aclrtMalloc(&workspace, workspaceSize, ACL_MEM_MALLOC_HUGE_FIRST);
    st = op->Execute(variantPack, (uint8_t *)workspace, workspaceSize, context);
    status = aclrtDestroyStream(stream);
    status = aclrtFree(workspace);
    st =atb::DestroyOperation(op);
    st = atb::DestroyContext(context);
    status = aclrtFree(a.deviceData);
    a.deviceData = nullptr;
    a.dataSize = 0;
    status = aclrtFree(b.deviceData);
    b.deviceData = nullptr;
    b.dataSize = 0;
    status = aclrtFree(output.deviceData);
    output.deviceData = nullptr;
    output.dataSize = 0;
    return 0;
}