## 说明
1. 在`SPI Flash`实现数据库功能；
2. `SPI Flash`驱动使用了[`SFUD`](https://github.com/armink/SFUD.git)开源库；
3. 数据库相关功能使用了[`FlashDB`](https://github.com/armink/FlashDB.git)开源库；

## `SFUD`移植
### 通用移植方式
1. 将`SFUD`工程通过`git submodule add`添加到工程；
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