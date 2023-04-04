deps_config := \
	drivers/Config_usb.in \
	drivers/Config.in \
	Config.in

.config include/autoconf.h: $(deps_config)

$(deps_config):
