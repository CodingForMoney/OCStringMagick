//
//  OCStringMagick.h
//  OCStringMagick
//
//  Created by 罗贤明 on 2016/12/26.
//  Copyright © 2016年 罗贤明. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifdef DEBUG

// 在debug情况下 ， ENCode 就是 @ ,但是Release下编译，如果替换失败，会编译失败。
#define MUENCODESTRING @
#endif




/**
  解密字符串， 在Release的情况下，将密文解密成明文。

 @param cstr 加密字符串
 @return 返回一个解密后的NSString
 */
NSString *MUDECODESTRING(const char *cstr);

// 所以， 当使用我们的字符串加密时， 尽量使用 宏去设置字符串，而不要用 static NSString = @"" ，这种形式来初始化字符串， 使用这种形式，不能将@
// 直接转换为 MUENCODESTRING , 因为我们的解密函数是一个函数， 不能放在static区域去调用，只能放在函数中去执行。


/**
  重新设置一个加密密钥， 注意，这个函数应该放在所有解密操作之前，否则可能有些数据会解密出错。
    然后就是， 默认的key 是 "BundleID+VersionCode" ， 在Shell脚本中也如此传入密钥， 如果要修改，两处都应该进行修改。

 @param key 选取一个与APP相关的键值，作为加密密钥，以确保每次加密密钥不同。
 */
void resetEncryptKey(NSString *key);
