#import <stdio.h>
#include <string.h>
#include <dirent.h>
#import <regex.h>
#import <stdlib.h>

// 最大字符串长度
static const int LINEMAXCHARNUMBER = 1024;
// 源文件中，放置 Encode ,放置原文。
static char *const ENCODE_TAG_STRING = "MUENCODESTRING\"";
static const int ENCODE_TAG_STRING_LENGTH = 15;
static char *const ENCODE_TAG_STRING_REG = "MUENCODESTRING\"";
// 然后我们加密， 并将Encode 改为decode ,运行时解密。
static char *const DECODE_TAG_STRING = "MUDECODESTRING(\"";
static const int DECODE_TAG_STRING_LENGTH = 16;

/*
	 在参数中，传入一个key的md5结果， 放在shell脚本中进行MD5.
*/
static unsigned char *encrypt_key_md5hash;
typedef enum { false, true }bool;  

/*通过正则表达式进行匹配判断*/
bool checkByRegex(char *src, char *regexExp)
{
	regex_t regex;
    int reti;
    bool ret;
    char msgbuf[100];
    reti = regcomp(&regex, regexExp, REG_EXTENDED);
    if( reti ) {
    	printf( "Could not compile regex\n");
    }
    reti = regexec(&regex,src, 0, NULL, 0);
    if(!reti){
        ret = true;
    }
    else if( reti == REG_NOMATCH ){
        ret = false;
    }
    else{
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        ret = false;
    }
    regfree(&regex);
    return ret;
}

/*转义字符转换*/
const static char ESCAPE_MAP[256] = {
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\"', '\0', '\0', '\0', '\0', '\'', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\?',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\\','\0', '\0', '\0',
	'\0', '\a', '\b', '\0', '\0', '\0', '\f', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\n', '\0',
	'\0', '\0', '\r', '\0', '\t', '\0', '\v', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0',
};

/*
	获取字符串，并将转义字符正确转义
	str 为后续字符串，而raw 为外部初始化好的放置字符串的地方。
	返回值为原有字符串长度。
	不允许 \xAA 和 \012 这种写法的转义字符， 太难解析了，不准写。
*/
int cutoutRawString(const char *str,char *raw) 
{
	int beforeLength = 0;
	if (*str == '\"' || *str == '\0')// 后面这种情况，应该不会存在，编译器会报错。
	{
		// 如果字符串为空， 返回NULL。
		return 0;
	}
	do
	{
		beforeLength ++;
		if (*str != '\\')// 如果不是转义，则直接复制。	
		{
			*raw ++ = *str++;
		}else {
			str ++;
			char escape = ESCAPE_MAP[(unsigned char) * str++];
			if (escape == '\0')
			{
				// 这里匹配转义字符出错，整个操作就是错误失败的，直接失败.
				printf("解析转义字符失败，这里原有字符串是%s\n",str );
				exit(1);
			}else{
				*raw ++ = escape;
				beforeLength ++;
			}
		}

	} while(*str != '\"');// 如果是双引号，表示读完字符串。
	return beforeLength;
}
// 以下16进制全为大写。
/**
 *  将一个字符转换为16进制.
 *
 *  @param num <#num description#>
 *
 *  @return <#return value description#>
 */
static inline  unsigned char changeNumberToChar(unsigned char num)
{
    return num < 10? num + '0' : num + 'A' - 10;
}

/**
 *  将一个16进制字符转换为数字,但是数字依然是char中
 *
 *  @param ch <#ch description#>
 *
 *  @return <#return value description#>
 */
static inline  unsigned char changeCharToNumber(unsigned char ch)
{
    return ch > '9' ? 10 + ch - 'A' : ch - '0';
}

/**
 *  将字节中保存的数据转换为16进制字符串.
 *
 *  @param bytes       字节数据
 *  @param str         输出字符串
 *  @param bytesLength 字节长度,而转换得到的字符串长度为字节长度x2, 但是创建的数组的大小得使字节长度x2 + 1.
 */
static inline  void changeBytesToHEXString(const unsigned char* bytes,unsigned char* str,const int bytesLength)
{
    int bi= 0 ,si = 0;
    for ( ; bi < bytesLength; bi++) {
        str[si++] =changeNumberToChar( bytes[bi] >> 4);
        str[si++] =changeNumberToChar( bytes[bi] & 0x0f);
    }
    str[si] = '\0';
}

/**
 *  将16进制字符串存放入字节中.
 *
 *  @param str       <#str description#>
 *  @param bytes     <#bytes description#>
 *  @param strLength 是HEX字符串的长度,也就是字节长度*2.
 */
static inline  void changeHEXStringToBytes(const unsigned char* str,unsigned char* bytes ,const int strLength)
{
    for (int i = 0; i < strLength; i+= 2) {
        bytes[i / 2] = (changeCharToNumber(str[i]) << 4 ) | changeCharToNumber(str[i+1]) ;
    }
}

// 16进制， md5值，
char getXORParams(int i) {
    return encrypt_key_md5hash[i % 16];
}


char *MUENTRYSTRING(const char *cstr) {
    if (cstr == NULL) 
    {
        return NULL;
    }
    int length = (int)strlen(cstr);
    if (length == 0) 
    {
        return NULL;
    }
    unsigned char *raw = (unsigned char *)malloc(length);
    unsigned char *plain = (unsigned char *)malloc(length * 2 + 1);
    plain[length * 2] = '\0';
    for (int i = 0; i < length; i++) 
    {
        raw[i] = (cstr[i] ^ getXORParams(i)) ^ (char)i;
    }
    changeBytesToHEXString(raw, plain, length);
    free(raw);
    return (char*)plain;
}


/*详细检查一行数据，进行加密*/
// 单行匹配无法判断注释，所以忽略注释的判断
// 这里逻辑就比较坑了， 只能一个字符一个字符的去判断逻辑。
void EncryptStringOfLine(char *line){
	// 起始点
	char *startPoint = line;
	char *buff = (char *)malloc(LINEMAXCHARNUMBER);
	char *currentBuff = buff;
	memset(buff, '\0', LINEMAXCHARNUMBER);
	// 当前点。 首先匹配这个 xxx("  ,如果在字符串里面，必定有`\"`,即不会是`xxx("` ,所以可以放心，这个是函数的开始。
	char *currentPoint = strstr(line,ENCODE_TAG_STRING);
	while (currentPoint) 
	{
		// 找到第一次匹配后，先将原来部分的内容复制到buff上
		int length = currentPoint - startPoint;
		// 先复制匹配端之前的内容
		memcpy(currentBuff,startPoint,length);
		currentBuff += length;
		// 再复制匹配段
		memcpy(currentBuff,DECODE_TAG_STRING,DECODE_TAG_STRING_LENGTH);
		currentBuff += DECODE_TAG_STRING_LENGTH;
		currentPoint += ENCODE_TAG_STRING_LENGTH; // 再移动到双引号后
		// 找到匹配点后， 截取到字符串终点。
		char *rawString = (char *)malloc(LINEMAXCHARNUMBER);
		memset(rawString,'\0',LINEMAXCHARNUMBER);
		int beforeLength = cutoutRawString(currentPoint,rawString);
		currentPoint += beforeLength;// 移动匹配点。
		if (beforeLength > 0)
		{
			// printf("获取到字符串中内容，如下 ： %s \n", rawString);
			// 进行加密。
			char *encodedString = MUENTRYSTRING(rawString);
			if (!encodedString)
			{
				// 不会走到这里，因为外面已经判断字符串不为空。
				exit(1);
			}
			// 将加密后的字符串拼接到buff上。
			int encodedStringLength = strlen(encodedString);
			memcpy(currentBuff,encodedString,encodedStringLength);
			currentBuff += encodedStringLength;
			free(encodedString);
		}
        *currentBuff ++ = *currentPoint++ ;
        *currentBuff ++ = ')';// 在匹配字符串后面，再添加上括号，以将@""形式转换为 Decode("")的函数形式。
		free(rawString);
		// 再次移动起始点
		startPoint = currentPoint;
		// 同时再次寻找匹配点。
		currentPoint = strstr(currentPoint,ENCODE_TAG_STRING);
	}
	// 如果之后的内容，没有匹配，则将剩下内容，粘贴到buff 上。
	strncpy(currentBuff , startPoint , strlen(startPoint));
	memcpy(line,buff,LINEMAXCHARNUMBER);
	// printf("输出 Buffer 内容 : %s\n",buff);
	free(buff);
}

/*对整个文件进行读写替换*/
void EncrptStringOfFile(const char *filename) 
{   
	if (filename == NULL)
	{
		printf("warn : 当前文件不存在\n");
		return;
	}
	// 读取文件
	FILE *sfp;
	if((sfp = fopen(filename,"r")) == NULL) //判断文件是否存在及可读  
	{   
		printf("当前文件不存在，或者不可读写 : %s\n",filename);   
		return;
	}   
	char bufffileName[LINEMAXCHARNUMBER];
	sprintf(bufffileName,"%s.buff",filename);
	// 读文件到缓冲文件中
	FILE *dfp = fopen(bufffileName,"w");
	char strLine[LINEMAXCHARNUMBER];             //每行最大读取的字符数,可根据实际情况扩大  
	// 会不会出现文本为空得情况？
    while (!feof(sfp))
    {
        // 按行读写文本，是一个困难的事情。。。
        if(fgets(strLine,LINEMAXCHARNUMBER,sfp) == NULL) {//读取一行
            // 如果为NULL ，表示遇到终点，进行feofpanda。
            if (feof(sfp))
            {
                break;
            }
        }else {
            if (checkByRegex(strLine,ENCODE_TAG_STRING_REG))
            {
                EncryptStringOfLine(strLine);
            }
            fputs(strLine,dfp);
        }
    }
	fclose(sfp);                     //关闭文件  
	fclose(dfp);
	remove(filename);  
	rename(bufffileName,filename);
 } 



/*检测文件名是否是需要加密字符串的类型， 一般只加密 .m文件，之后再考虑.mm和.c 等文件*/
bool fileNeedCheck(char *filename){
	if (filename == NULL)
	{
		return false;
	}
	int charLength = strlen(filename);
	if (charLength < 3)
	{
		return false;
	}
	return checkByRegex(filename,"\\.(m|h)$");
}

/*
	递归加密目录中的字符串
*/
void EncryptStringOfDictionary(char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];
    char fileFullPath[LINEMAXCHARNUMBER];

    if (basePath == NULL || (dir=opendir(basePath)) == NULL)
    {
        printf("文件夹不存在 %s",basePath);
    }
    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8){    ///file
        	char *fileName = ptr->d_name;
        	sprintf(fileFullPath,"%s/%s",basePath,ptr->d_name);
        	if (fileNeedCheck(fileFullPath))
        	{
        		EncrptStringOfFile(fileFullPath);	
        	}
        }
        else if(ptr->d_type == 10){
        	continue;
        }
        else if(ptr->d_type == 4)    ///dir
        {
            memset(base,'\0',sizeof(base));
            sprintf(base,"%s/%s",basePath,ptr->d_name);
            EncryptStringOfDictionary(base);
        }
    }
    closedir(dir);
}



/*
	接口调用形式 ， EncryptString MD5HashKey path1 path2 path3 ,传入的是文件夹目录。

	MD5HashKey : 表示一个加密密钥，为32位大写 MD5值 建议使用跟APP打包信息相关的内容，一般为APP bundleid 加 版本号。

	path : 默认是 项目中的所有 .m 文件 ，路径是项目路径 加 pod 路径。
	当然也可以继续详细指定，如 项目路径中的具体路径。
*/
int main(int argc,char *argv[])
{
    // const unsigned char inputHashKey[] = "EF9158BF809CE42704BEA52A196BCBC0"; // argv[1]
    if (argc < 3)
    {
    	// 如果参数数量少于 3个， 则不处理
    	return 1;
    }
    unsigned char *inputHashKey = (unsigned char *)argv[1];
    encrypt_key_md5hash = (unsigned char *)malloc(16);
    changeHEXStringToBytes(inputHashKey,encrypt_key_md5hash,32);
    for (int i = 2; i < argc; ++i)
    {
    	printf("输出参数%d :  %s\n",i,argv[i]);
    	EncryptStringOfDictionary(argv[i]);
    }
    free(encrypt_key_md5hash);
    return 0;
}
