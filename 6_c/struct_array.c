#include <stdio.h>

struct mtk_func_desc {
  	int muxval;
  	const char *name;
};


struct mtk_pin_desc {
  	unsigned int number;
  	const char *name;
  	struct mtk_func_desc *funcs;
};

static const struct mtk_pin_desc mtk_pins_mt6833[] = {
	{
		.number = 0,
		.name = "pin0",
		.funcs = (struct mtk_func_desc[]) {
			{.name = "func100", .muxval = 100}, 
			{.name = "func200", .muxval = 200}, 
		},
	},
	{
		.number = 2,
		.name = "pin2",
		.funcs = (struct mtk_func_desc[]) {
			{.name = "func210", .muxval = 210}, 
			{.muxval = 220, .name = "func220"}, 
		},
	},
	{
		.number = 3,
		.name = "pin3",
		.funcs = (struct mtk_func_desc[]) {
			{310, "func310" }, 
		},
	},
	{
		.number = 4,
		.name = "pin4",
		.funcs = /*(struct mtk_func_desc[])  Segmentation fault */ {
			{410, "func410" }, 
		},
	},
	{
	},
};
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(void)
{
	printf("Hello world!\n");
	
	int length = ARRAY_SIZE(mtk_pins_mt6833);
	int size = sizeof(mtk_pins_mt6833);
	printf("lenth = %d, size = %d\n", length, size);
	
	printf("pin_name = %s,\t func_name = %s\n", mtk_pins_mt6833[0].name,  mtk_pins_mt6833[0].funcs[1].name);
	printf("pin_number = %d,\t\t func_name = %s\n", mtk_pins_mt6833[1].number,  mtk_pins_mt6833[1].funcs[0].name);
	printf("pin_number = %d,\t\t func_muxval = %d\n", mtk_pins_mt6833[1].number,  mtk_pins_mt6833[1].funcs[1].muxval);
	printf("func_muxval = %d,\t func_name = %s\n", mtk_pins_mt6833[2].funcs[0].muxval, mtk_pins_mt6833[2].funcs[0].name);
	printf("func_muxval = %d,\t func_name = %s\n", mtk_pins_mt6833[3].funcs[0].muxval, mtk_pins_mt6833[3].funcs[0].name);
	return 0;
}
  