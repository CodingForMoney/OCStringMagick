//
//  OCStringMagick.m
//  OCStringMagick
//
//  Created by 罗贤明 on 2016/12/26.
//  Copyright © 2016年 罗贤明. All rights reserved.
//

#import "OCStringMagick.h"
#import <CommonCrypto/CommonCrypto.h>


// 16进制采用小写。
///**
// *  将一个字符转换为16进制.
// *
// *  @param num <#num description#>
// *
// *  @return <#return value description#>
// */
//static inline  unsigned char changeNumberToChar(unsigned char num){
//    return num < 10? num + '0' : num + 'A' - 10;
//}

/**
 *  将一个16进制字符转换为数字,但是数字依然是char中
 *
 *  @param ch <#ch description#>
 *
 *  @return <#return value description#>
 */
static inline  unsigned char changeCharToNumber(unsigned char ch){
    return ch > '9' ? 10 + ch - 'A' : ch - '0';
}

///**
// *  将字节中保存的数据转换为16进制字符串.
// *
// *  @param bytes       字节数据
// *  @param str         输出字符串
// *  @param bytesLength 字节长度,而转换得到的字符串长度为字节长度x2, 但是创建的数组的大小得使字节长度x2 + 1.
// */
//static inline  void changeBytesToHEXString(const unsigned char* bytes,unsigned char* str,const NSUInteger bytesLength){
//    NSUInteger bi= 0 ,si = 0;
//    for ( ; bi < bytesLength; bi++) {
//        str[si++] =changeNumberToChar( bytes[bi] >> 4);
//        str[si++] =changeNumberToChar( bytes[bi] & 0x0f);
//    }
//    str[si] = '\0';
//}

/**
 *  将16进制字符串存放入字节中.
 *
 *  @param str       <#str description#>
 *  @param bytes     <#bytes description#>
 *  @param strLength 是HEX字符串的长度,也就是字节长度*2.
 */
static inline  void changeHEXStringToBytes(const unsigned char* str,unsigned char* bytes ,const NSUInteger strLength){
    for (NSUInteger i = 0; i < strLength; i+= 2) {
        bytes[i / 2] = (changeCharToNumber(str[i]) << 4 ) | changeCharToNumber(str[i+1]) ;
    }
}

static unsigned char *params = NULL;
// 16进制， md5值，
static __attribute__((always_inline)) char getXORParams(int i) {
    if (params == NULL) {
        NSString *appid = [[NSBundle mainBundle] bundleIdentifier];
        NSString *versionCode = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
        NSString *appinfo = [NSString stringWithFormat:@"%@+%@",appid,versionCode];
        const char *cStr = [appinfo UTF8String];
        params = (unsigned char *)malloc(sizeof(unsigned char) * 16);
        CC_MD5( cStr, (CC_LONG)strlen(cStr), params );
    }
    return params[i % 16];
}

__attribute__((always_inline))  void resetEncryptKey(NSString *key) {
    const char *cStr = [key UTF8String];
    params = (unsigned char *)malloc(sizeof(unsigned char) * 16);
    CC_MD5( cStr, (CC_LONG)strlen(cStr), params );
}


__attribute__((always_inline)) NSString *MUDECODESTRING(const char *cstr) {
    if (cstr == NULL) {
        return nil;
    }
    int length = (int)strlen(cstr) / 2;
    if (length == 0) {
        return @"";
    }
    char *plain = (char *)malloc(length + 1);
    changeHEXStringToBytes((unsigned char *)cstr,(unsigned char *) plain, length * 2);
    plain[length] = '\0';
    for (int i = 0; i < length; i++) {
        plain[i] = plain[i] ^ (char)i  ^ getXORParams(i);
    }
    return [NSString stringWithUTF8String:plain];
}


//__attribute__((always_inline)) NSString *MUENCODESTRING(const char *cstr) {
//    if (cstr == NULL) {
//        return nil;
//    }
//    int length = (int)strlen(cstr);
//    if (length == 0) {
//        return @"";
//    }
//    unsigned char *raw = (unsigned char *)malloc(length);
//    unsigned char *plain = (unsigned char *)malloc(length * 2 + 1);
//    plain[length * 2] = '\0';
//    for (int i = 0; i < length; i++) {
//        raw[i] = (cstr[i] ^ getXORParams(i)) ^ (char)i;
//    }
//    changeBytesToHEXString(raw, plain, length);
//    free(raw);
//    return [NSString stringWithUTF8String:(char *)plain];
//}


