#include "handlers.h"

void init_handlers()
{
	register_handler(GET, is_default_page, get_default_page, NULL, NULL);
	/* php proxy handler: must be before sdcard handler so .php doesn't get served as static file */
	register_handler(GET, is_php_request, get_php_response, NULL, NULL);
	register_handler(GET, is_sdcard_handler, get_sdcard_response, NULL, NULL);
	register_handler(GET, is_system_request, get_system_handler_response, pre_system_response, NULL);
	register_handler(GET, is_crypt_request, get_crypt_handler_response, NULL, NULL);
	register_handler(GET, is_read_request, get_read_handler_response, NULL, NULL);
	register_handler(GET, is_write_request, get_write_handler_response, NULL, NULL);
	register_handler(GET, is_favicon_request, get_favicon_icon,NULL,NULL);
}
