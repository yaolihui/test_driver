#include <stdio.h>

struct pinctrl_desc {
	const char *name;
	const struct pinctrl_pin_desc *pins;
	unsigned int npins;
	const struct pinctrl_ops *pctlops;
	const struct pinmux_ops *pmxops;
	const struct pinconf_ops *confops;
	struct module *owner;
#ifdef CONFIG_GENERIC_PINCONF
	unsigned int num_custom_params;
	const struct pinconf_generic_params *custom_params;
	const struct pin_config_item *custom_conf_items;
#endif
};

void main(void)
{
	printf("Hello world!\n\n");

	struct pinctrl_desc desc = {
		 .name = "test-pinctrl_desc",
		 .npins = 100,
	};
	
	printf("desc=%s, pnis=%d\n", desc.name, desc.npins);
	
}