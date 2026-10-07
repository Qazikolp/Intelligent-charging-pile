#include "framework.h"


extern drvp_wdt_t* drvp_wdt;
static void init( uint32_t tick )
{
	drvp_wdt->init( tick );
}

//Î¹¹·
static void feed(void)
{
	drvp_wdt->feed( );
}
	

//¿´ÃÅ¹·Æô¶¯
static void start(void)
{
	drvp_wdt->start( );
}

static void stop(void)
{
	drvp_wdt->stop( );
}

static drv_wdt_t do_drv_wdt = {
	.init = init,
	.feed = feed,
	.start = start,
	.stop = stop,
};

drv_wdt_t* wdt = &do_drv_wdt;
