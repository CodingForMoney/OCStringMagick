## OCStringMagick

OCStringMagick 进行字符串加密，将代码中大量的字符串进行简单快速的加密，以提高静态分析的难度。

## 原理以及流程说明

我们声明一个标记， 如 `OCSMENTRYSTRING` , 将原有的字符串，`@"hello"`改写成 `OCSMENTRYSTRING"hello` , 而这个标记本身 `OCSMENTRYSTRING` 在debug情况下，是

	#define OCSMENTRYSTRING @

即还是通过@来创建字符串。 

然后我们在 `Build Phases`中编写的脚本，在`Release`编译前，执行脚本，进行加密替换操作。 将指定目录下的文件中 `OCSMENTRYSTRING"**"` 这种形式的内容，都替换成 `OCSMDECODESTRING("xxxx")`, 即我们会把 `@"hello` 改写成`OCSMDECODESTRING("xxxxxxxxx")` ,  `xxxxxxx`是 `hello`加密后的结果。

然后我们在项目中定义了函数`OCSMDECODESTRING` ，所以在运行时期，正确的将加密内容解密出来，正常运行。 

## 接入指导

首先，将`OCStringMagick.h` 和 `OCStringMagick.m` 文件导入项目中， `#import "OCStringMagick.h"` 写在`pch`文件中，确保项目中所有需要加密的内容，都会受到影响。

然后，将`ocsm`可执行文件放到自己喜欢的地方，或者给其添加到系统`path`上。

然后，项目设置中的 `Build Phases`中，添加一个 `Run Script` ，将`EncryptString.sh`中的脚本复制到到新添加的`Run Script`中。拖动新添加的`Run Script`将其移到第一位，要放在`Compile Sources` ,否则项目会先编译文件，然后才去加密字符串。

脚本中有一处 `${SRCROOT}/pathto/ocsm` ， 要修改为正确调用`ocsm`的路径。

脚本中有一处 `OCSM_NEED_ENCRYPT_DIRS`变量赋值，设置需要加密的文件夹路径，用空格分开。

	${SRCROOT}/xxxx ${SRCROOT}/xxx/xx

需要注意的事，我们将脚本放在`Run Script`，项目编译的时候才会执行，而`Pod`项目编译在`Run Script`之前，所以我们不能加密`Pod`中的字符串。

然后就可以使用我们的字符串加密了，将需要加密的字符串中的`@`替换成 `MUENCODESTRING` ，当然我们有限制，由于我们替换的结果是将`@""`改成 `function("xxx")`的形式，去调用一个函数。 所以，在使用时，我们需要明白，只有在函数中才可以使用。即首先，我们在使用上就有限制。

然后，在这个前提下，我们建议字符串的初始化，尽量不要放在静态区域，即这种形式：

	static NSString *Str = @"hello";

而是将其以宏的形式去定义：

	#define STR @"hello";


而我们也对使用的地方有一个指导性的限制，只有需要加密的东西，才设置进行加密， 如下面这些字符串，显然是不需要加密的 ：

	@"349CF3"
	@" "
	@"%@%@"
	
而我们常用的加密，用于一些提示输出，URL，参数设置，等等，有用的地方。


