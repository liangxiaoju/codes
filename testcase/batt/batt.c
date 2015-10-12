#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATH	"/sys/devices/platform/s3c2440-i2c.0/i2c-0/0-0034/axp192-batt-coulomb.9/pmic-batt/"

int main(int args, char *argv[])
{
	char buf[128];
	int interval;
	int volt_a, volt_b, curr, cap;
	FILE *fd_adc_interval, *fd_volt_a, *fd_volt_b, *fd_curr, *fd_cap;

	if (args != 2) {
		fprintf(stderr, "Usage: %s val\n", argv[0]);
		return -1;
	}

	snprintf(buf, sizeof(buf), "%s%s", PATH, "adc_interval");
	fd_adc_interval = fopen(buf, "r+");

	snprintf(buf, sizeof(buf), "%s%s", PATH, "volt_a");
	fd_volt_a = fopen(buf, "r");

	snprintf(buf, sizeof(buf), "%s%s", PATH, "volt_b");
	fd_volt_b = fopen(buf, "r");

	snprintf(buf, sizeof(buf), "%s%s", PATH, "curr");
	fd_curr = fopen(buf, "r");

	snprintf(buf, sizeof(buf), "%s%s", PATH, "cap");
	fd_cap = fopen(buf, "r");

	if (!(fd_adc_interval && fd_volt_a && fd_volt_b && fd_curr && fd_cap)) {
		fprintf(stderr, "Failed to open.\n");
		return -1;
	}

	interval = atoi(argv[1]);
	if (interval <= 0) {
		fprintf(stderr, "illegal value !\n");
		return -1;
	}

	sprintf(buf, "%d\n", interval);
	fputs(buf, fd_adc_interval);
	fflush(fd_adc_interval);

	fprintf(stdout, "interval\tvolt_a\tvolt_b\tcurrent\tcapacity\n");

	for (;;) {

		volt_a = atoi(fgets(buf, 64, fd_volt_a));
		volt_b = atoi(fgets(buf, 64, fd_volt_b));
		curr = atoi(fgets(buf, 64, fd_curr));
		cap= atoi(fgets(buf, 64, fd_cap));
		rewind(fd_volt_a);
		rewind(fd_volt_b);
		rewind(fd_curr);
		rewind(fd_cap);
		fflush(fd_volt_a);
		fflush(fd_volt_b);
		fflush(fd_curr);
		fflush(fd_cap);

		fprintf(stdout, "%d\t%d\t%d\t%d\t%d\n", interval, volt_a, volt_b, curr, cap);
		usleep(interval*1000);
	}
}
