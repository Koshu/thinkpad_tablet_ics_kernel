/* linux/spi/ntrig_spi */
#ifndef _NTRIG_SPI_H
#define _NTRIG_SPI_H

/**
 * platform data for ntrig sensor driver over SPI 
 */
struct ntrig_spi_platform_data {
	/** the	gpio line used as "output enable". We must set this
	 *  line in	order to activate the SPI link */
	unsigned oe_gpio;
	/** if 1, the output enable	line is	connected to an
	 *  "inverter" - set it	to reverse value (0	for	1, 1 for 0) */
	int oe_inverted;
	/** the gpio line used for power*/
	unsigned pwr_gpio;
};

#endif
