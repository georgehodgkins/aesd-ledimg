#include <gpiod.h>
#include <stdio.h>
#include <assert.h>
#include <syslog.h>

#define GPIO_CHIP_PATH "/dev/gpiochip0"
#define GRID_CTL_BITS 4
static const unsigned GRID_CTL_PINS[] = {
	16, // SPI_1_MOSI / header pin 19
	17, // SPI_1_MISO / header pin 21
	18, // SPI_1_SCK / header pin 23
	19, // SPI_1_CS0 / header pin 24
};

static struct gpiod_chip* chip;
static struct gpiod_line* gridctl[GRID_CTL_BITS] = {NULL};

int grid_init () {
	_Static_assert(GRID_CTL_BITS < 16, "LEDGRID label only supports a single hex digit");
	_Static_assert(sizeof(GRID_CTL_PINS)/sizeof(unsigned) == GRID_CTL_BITS, "Wrong number of pins");

	// open syslog
	openlog("ledgrid", 
#ifdef NDEBUG
	0
#else
	LOG_PERROR
#endif
	, LOG_USER);

	// acquire GPIOs and set them as outputs
	chip = gpiod_chip_open(GPIO_CHIP_PATH);
	if (chip == NULL) {
		syslog(LOG_ERR, "Could not open GPIO chip at %s", GPIO_CHIP_PATH);
		goto fail;
	} else {
		syslog(LOG_DEBUG, "Opened chip %s as %s [%p]", GPIO_CHIP_PATH, gpiod_chip_name(chip), chip);
	}
	const char* pinlab = "LEDGRID";
	for (unsigned p = 0; p < GRID_CTL_BITS; ++p) {
		syslog(LOG_DEBUG, "Initializing LEDGRID_%x at pin %u of chip %s", 
				p, GRID_CTL_PINS[p], gpiod_chip_name(chip));
		assert(chip);
		gridctl[p] = gpiod_chip_get_line(chip, GRID_CTL_PINS[p]);
		if (gridctl[p] == NULL) {
			syslog(LOG_ERR, "Could not get pin %u of chip %s", GRID_CTL_PINS[p], gpiod_chip_name(chip));
			goto fail;
		}
		syslog(LOG_DEBUG, "Got line");
		int s = gpiod_line_request_output(gridctl[p], pinlab, 1);
		if (s == -1) {
			syslog(LOG_ERR, "Could not reserve pin %u as an output", GRID_CTL_PINS[p]);
			goto fail;
		}
		syslog(LOG_DEBUG, "LEDGRID_%x initialized on pin %u of chip %s", p, gridctl[p], gpiod_chip_name(chip));
	}
	return 0;

fail:
	for (unsigned p = 0; p < GRID_CTL_BITS; ++p) {
		if (gridctl[p]) {
			gpiod_line_release(gridctl[p]);
			gridctl[p] = NULL;
		}
	}
	if (chip)
		gpiod_chip_close(chip);
	chip = NULL;
	return -1;	
}

int grid_select (unsigned addr) {
	if (addr >= (1u << GRID_CTL_BITS)) {
		syslog(LOG_ERR, "address %x is invalid; max address is %x", addr, (1u << GRID_CTL_BITS));
		return -1;
	}

	if (chip == NULL) {
		syslog(LOG_ERR, "Attempting to use grid without initializing it!");
		return -1;
	}

	for (unsigned p = 0; p < GRID_CTL_BITS; ++p) {
		if (gridctl[p] == NULL) {
			syslog(LOG_ERR, "Pin LEDGRID_%x is not initialized!", p);
			return -1;
		}
		// logic between Nano and Arduino is inverted so we can use pull-ups on ard
		int bit =  (addr & (1 << p)) ? 0 : 1;
		syslog(LOG_DEBUG, "Selecting address %x: LEDGRID_%x %s (inverted)", addr, p, (bit) ? "HIGH" : "LOW");
		int s = gpiod_line_set_value(gridctl[p], bit);
		if (s == -1) {
			syslog(LOG_ERR, "Error setting LEDGRID_%x", p);
			return -1;
		}
	}

	return 0;
}

void grid_free (void) {
	for (unsigned p = 0; p < GRID_CTL_BITS; ++p) {
		gpiod_line_release(gridctl[p]);
		gridctl[p] = NULL;
	}
	gpiod_chip_close(chip);
	chip = NULL;
}
