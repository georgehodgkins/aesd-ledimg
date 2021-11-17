#include <asm/gpio.h>
#include <dt-bindings/gpio/tegra186-gpio.h>
#include <stdio.h>
#include <assert.h>

#define GRID_CTL_BITS 4
static const unsigned GRID_CTL_PINS[][2] = {
	{TEGRA_MAIN_GPIO_PORT_C, 0},
	{TEGRA_MAIN_GPIO_PORT_C, 1},
	{TEGRA_MAIN_GPIO_PORT_C, 2},
	{TEGRA_MAIN_GPIO_PORT_C, 3}	
};

// runtime pin numbers for LED address bits
static unsigned gridctl[GRID_CTL_BITS] = {UINT_MAX};

int grid_init () {
	_Static_assert(GRID_CTL_BITS < 16); // label can only acommodate a single hex digit
	_Static_assert(sizeof(GRID_CTL_BITS)/(sizeof(unsigned)*2) == GRID_CTL_BITS);

	// open syslog
	openlog("ledgrid", 
#ifdef NDEBUG
	0
#else
	LOG_PERROR
#endif
	, LOG_USER);

	// acquire GPIOs and set them as outputs
	char* pinlabel = "LEDGRID_x";
	for (unsigned p = 0; p < GRID_CTL_BITS; ++p) {
		syslog(LOG_DEBUG, "Initializing LEDGRID_%x at port %u, pin %u", 
				p, GRID_CTL_PINS[p][0], GRID_CTL_PINS[p][1]);
		gridctl[p] = TEGRA_MAIN_GPIO(GRID_CTL_PINS[p][0], GRID_CTL_PINS[p][1]);
		if (!gpio_is_valid((int) gridctl[p])) {
			syslog(LOG_ERR, "could not locate pin LEDGRID_%x at port %u, pin %u",
					p, GRID_CTL_PINS[p][0], GRID_CTL_PINS[p][1]);
			return -1;
		}
		size_t llen = strlen(pinlabel);
		int s = sprintf(pinlabel, "LEDGRID_%x", p);
		assert(s == llen);
		s = gpio_request(gridctl[p], pinlabel);
	   	if (s == -1) {
			syslog(LOG_ERR, "could not acquire pin %u (%s)",  gridctl[p], pinlabel);
			return -1;
		}
		s = gpio_direction_output(gridctl[p], 0);
		if (s == -1) {
			syslog(LOG_ERR, "could not set pin %u (%s) as output", gridctl[p], pinlabel);
			return -1;
		}
		syslog(LOG_DEBUG, "LEDGRID_%x initialized on pin %u", p, gridctl[p]);
	}
	return 0;
}

int grid_select (unsigned addr) {
	if (addr >= (1u << GRID_CTL_BITS)) {
		syslog(LOG_ERR, "address %x is invalid; max address is %x", addr, (1u << GRID_CTL_BITS));
		return -1;
	}

	for (unsigned p = 0; p < GRID_CTL_BITS; ++p) {
		if (gridctl[p] == UINT_MAX) {
			syslog(LOG_ERR, "Attempting to use grid without initializing it!");
			return -1;
		}
		int bit =  (addr & (1 << p)) ? 1 : 0;
		syslog(LOG_DEBUG, "Selecting address %x: LEDGRID_%x %s", addr, p, (bit) ? "HIGH" : "LOW");
		__gpio_set_value(gridctl[p], bit);
	}

	return 0;
}

void grid_free (void) {
	for (unsigned p = 0; p < GRID_CTL_BITS; ++p) {
		gpio_free(gridctl[p]);
		gridctl[p] = UINT_MAX;
	}
}
