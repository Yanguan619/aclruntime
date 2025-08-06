cd AddKernelInvocationNeo
SOC_VERSION=$(python3 -c "
try:
    import acl
    print(acl.get_soc_name())
except:
    print('unknow')
")
bash run.sh -r sim -v $SOC_VERSION -o "$OEC_OUTPUT_PATH"
