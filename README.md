## 说明
1. 在`SPI Flash`实现数据库功能；
2. `SPI Flash`驱动使用了[`SFUD`](https://github.com/armink/SFUD.git)开源库；
3. 数据库相关功能使用了[`FlashDB`](https://github.com/armink/FlashDB.git)开源库；

## `SFUD`移植
### 通用移植方式
1. 将[`SFUD`](https://github.com/armink/SFUD.git)库通过`git submodule add`添加到工程；
2. 将`sfud/inc`的头文件添加到工程，将`sfud/src`的源文件添加到工程；
3. 新建`sfud_port`文件夹，并将`sfud/inc/sfud_cfg.h`和`sfud/src/sfud_port.c`文件复制到`sfud_port`文件夹；
4. 将`sfud_port/sfud_cfg.h`和`sfud_port/sfud_port.c`添加到工程；（注意，一定要确保`sfud_port/sfud_cfg.h`的查找优先级高于`sfud/inc/sfud_cfg.h`，即工程头文件路径列表中`sfud_port`要在`sfud/inc`前面，否则配置不起作用！）
5. 完成`sfud_port/sfud_port.c`文件中的函数，详情见 [sfud_port.c](./ThirdUtils/SFUD_port/sfud_port.c)；
6. 配置文件`sfud_port/sfud_cfg.h`说明：
```c
// 日志功能开启
#define SFUD_DEBUG_MODE

// 1. 一下两个定义可以都定义，但至少要定义一个；可以根据需要定义，以减少代码编译后的大小；
// 2. SFDP 是指 SPI Flash 的一个标准协议，可以通过特定的指令读取 Flash 的所有操作信息；
// 3. 开启 SFDP 表示对 Flash 操作之前，先从 Flash 芯片中读取其信息，然后根据读出来的信息再操作 Flash 芯片；但是一些老 Flash 芯片不支持 SFDP 协议，这时候就需要指定对其操作的一些信息了，这些信息可以定义在 SFUD_FLASH_CHIP_TABLE 中(这个 SFUD_FLASH_CHIP_TABLE 的定义，需要先定义 SFUD_USING_FLASH_INFO_TABLE)；
// 4. 当只定义了 SFUD_USING_FLASH_INFO_TABLE 时，代码是通过 SFUD_FLASH_CHIP_TABLE 中的 md_if, type_id, capacity_id 这三个字段和 Flash 芯片内部的 ID 信息作比较判断是否为同类芯片的；
#define SFUD_USING_SFDP
#define SFUD_USING_FLASH_INFO_TABLE

// #define SFUD_USING_FAST_READ

// 1. 定义板子上的 Flash 芯片；其中 enum 中的序号很重要，必须确保这序号定义的 Flash 芯片和 sfud_port.c 中定义的 flashPort 是相匹配的；SFUD_FLASH_CHIP_TABLE 中的名字按实际情况修改即可；
// 2. SFUD 可以支持多个 FLASH 芯片，直接在下面的定义中添加，然后在 sfud_port.c 中相应的添加即可；
enum {
    SFUD_W25Q64JV_DEVICE_INDEX = 0,
};
#define SFUD_FLASH_DEVICE_TABLE                                                \
{                                                                              \
    [SFUD_W25Q64JV_DEVICE_INDEX] = {.name = "W25Q64JV", .spi.name = "SPI3"},   \
}

// #define SFUD_USING_QSPI      // 不使用 QSPI 模式
```
7. 使用：
```c
    // 初始化所有在 SFUD_FLASH_DEVICE_TABLE 中定义的 Flash 芯片
    sfud_sta = sfud_init();
	if(sfud_sta != SFUD_SUCCESS) {
		LOG_ERR("sfud_device_init() err[%d]\r\n", sfud_sta);
		while(1);
	}
    // 获取第一个 Flash 对象（后面加 1 就是获取第二个 Flash 对象）
    const sfud_flash *flash = sfud_get_device_table() + 0;
    // 通过获取的 Flash 对象，对其进行读写擦除的操作
    result = sfud_erase(flash, 0, size);
    result = sfud_write(flash, 0, size, data);
    result = sfud_read(flash, 0, size, data);
```
### 最小编译代码移植
1. 和上面的`通用移植方式`基本一样，只是在`sfud_port/sfud_cfg.h`中，`SFUD_USING_SFDP`和`SFUD_USING_FLASH_INFO_TABLE`都不需要定义；
2. 使用：
```c
// 定义 SPI Flash 芯片信息（这种方式也可以定义多个 Flash 对象，只要保证每个 Flash 对象的 name 字段各不相同，然后在 sfud_port.c 中相关的地方添加一些代码即可）
sfud_flash spi_flash_1 = {
    .name = SPI_FLASH_1_NAME,
    .spi.name = "SPI3",
    .chip = { "W25Q64JV", SFUD_MF_ID_WINBOND, 0x40, 0x17, 8L * 1024L * 1024L, SFUD_WM_PAGE_256B, 4096, 0x20 }
};

// 初始化 SPI Flash 信息
sfud_sta = sfud_device_init(&spi_flash_1);
// 读写擦除 SPI Flash
result = sfud_erase(&spi_flash_1, 0, size);
result = sfud_write(&spi_flash_1, 0, size, data);
result = sfud_read(&spi_flash_1, 0, size, data);
```
## `FlashDB`移植
1. 将[`FlashDB`](https://github.com/armink/FlashDB.git)库通过`git submodule add`添加到工程；
2. 将如下文件添加到工程中：
```c
// 添加头文件路径
FlashDB\inc
FlashDB\port\fal\inc
// 添加源文件
FlashDB\src\fdb.c                       // 必须
FlashDB\src\fdb_utils.c                 // 必须
FlashDB\src\fdb_tsdb.c                  // 非必须，只有在使用 Time Series Database 时需要
FlashDB\src\fdb_kvdb.c                  // 非必须，只有在使用 key value database 时需要
FlashDB\src\fdb_file.c                  // 非必须，只有在定义了 FDB_USING_FILE_MODE 时需要
FlashDB\port\fal\src\fal.c              // 必须
FlashDB\port\fal\src\fal_flash.c        // 必须
FlashDB\port\fal\src\fal_partition.c    // 必须
```
3. 新建`FlashDB_port`文件夹，并将如下文件剪切(不是复制！)到`FlashDB_port`文件夹：
```c
FlashDB\inc\fdb_cfg.h
FlashDB\port\fal\samples\porting\fal_cfg.h
FlashDB\port\fal\samples\porting\fal_flash_sfud_port.c
```
4. 将`FlashDB_port/`添加到工程头文件路径，将`FlashDB\port\fal\samples\porting\fal_flash_sfud_port.c`添加到工程；
5. 按自己的需求修改`FlashDB_port`里面的文件即可；
6. `FlashDB`不仅可以用于`SPI Flash`，也可以用于`MCU`内部`Flash`，而且可以同时使用；`FlashDB`在每个`Flash`上都可以分多个区域，创建多个数据库文件，详情可以参考`FlashDB`库提供的例程，比如`FlashDB\demos\stm32f405rg_spi_flash`；

## 总结
1. 本工程设计到三个开源库的使用，分别是`SFUD`, `FAL`, `FlashDB`，这三个库的代码结果都很相似，除了源代码，都有一个`xxx_cfg.h`和`xxx_def.h`文件；那这两个文件有什么区别呢？
>   对于用户来说，最重要的区别就是：我们需要根据需要修改`xxx_cfg.h`文件，而不要改动`xxx_def.h`文件！
>   以`fal_cfg.h`和`fal_def.h`文件举例，我们可以看到在`fal_def.h`文件中并没有包含`fal_cfg.h`文件，那么`fal_cfg.h`中的宏定义又是如何影响`fal_def.h`中的配置呢？再搜索一下就会发现，每次某个文件在包含`fal_def.h`之前，都一定会包含`fal_cfg.h`，这就使得这两个头文件都包含在了同一个源文件中，所以`fal_cfg.h`的宏定义也就可以作用在之后的`fal_def.h`中了。但是个人认为这样就需要用户一直记得这个成对调用的要求，要是在`xxx_def.h`中加上`#include "xxx_cfg.h"`不就没有问题了吗？比如`sfud_def.h`就是这样做的。
2. 自己项目中使用`git`开源库的一般流程：
```
(1) 通过`git submodule add <url> <path>`添加指定的库到工程指定的路径；
(2) 将开源库中需要修改的文件复制到本工程开源库路径之外的一个文件夹，然后进行修改，并添加到工程中；
(3) 编译测试通过后提交代码即可；
```
