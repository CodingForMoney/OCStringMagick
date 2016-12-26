#!/bin/bash
#如果不是Release,则不进行加密
if [[ $CONFIGURATION != "Release" ]]; then
	exit 0
fi
# 设置加密密钥。#$PRODUCT_BUNDLE_IDENTIFIER # bundle ID.
CFBundleShortVersionString=`/usr/libexec/PlistBuddy -c "Print CFBundleShortVersionString" ${PRODUCT_SETTINGS_PATH}`
raw_key="${PRODUCT_BUNDLE_IDENTIFIER}+${CFBundleShortVersionString}"
encrypt_key=`echo -n "${raw_key}" | md5 | tr '[:lower:]' '[:upper:]'`
# OCSM_NEED_ENCRYPT_DIRS 设置需要加密的文件路径列表 一般这样设置 ${SRCROOT}/TestStringDecode ${SRCROOT}/TestStringDecode2/xxx
# 需要注意的是 pod编译比 这个shell脚本要快，因为先编译pod再编译项目，编译项目时，才会使用shell脚本，所以，不要企图使用这里去加密Pod中的内容。
OCSM_NEED_ENCRYPT_DIRS="${SRCROOT}/TestStringDecode"
# 然后进行加密
${SRCROOT}/pathto/ocsm $encrypt_key $OCSM_NEED_ENCRYPT_DIRS
if [[ $? != 0 ]]; then
cat << EOM
error: 使用ocsm 加密失败，请检查代码中是否编写有问题。
EOM
exit 1
fi
exit 0
