import acl
print(acl.get_soc_name())
Count,ret = acl.rt.get_device_count()
if ret !=0:
    exit(1)
print(Count,end='')