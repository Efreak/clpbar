#include "config.h"

#include "headers.h"
#include "types.h"
#include "error.h"
#include "io.h"
#include "display.h"

#ifdef HAVE_UNISTD_H
#	ifdef HAVE_SIGNAL_H
#		ifdef HAVE_TERMIOS_H
#			ifdef HAVE_SYS_IOCTL_H
#				define CAN_RESIZE
#			endif
#		endif
#	endif
#endif

display d;

#ifdef CAN_RESIZE
static void displayGetSize(void)
{
	struct winsize size;

	if (d.manual_width && d.manual_height)
		return;
	if (ioctl(STDERR_FILENO, TIOCGWINSZ, (char *)&size) < 0)
		return;
	if (!d.manual_width) {
		d.screen_width = size.ws_col;
		if (d.screen_width_minus_one)
			d.screen_width--;
	}
	if (!d.manual_height) {
		d.screen_height = size.ws_row;
		if (d.screen_height_minus_one)
			d.screen_height--;
	}
}

int displaySetSigWinch(void);

static void sig_winch(int signo)
{
	displayGetSize();
	displaySetSigWinch();
}

int displaySetSigWinch(void)
{
	if (isatty(STDERR_FILENO) != 0) {
		if (signal(SIGWINCH, sig_winch) == SIG_ERR) {
			return(1);
		}
	}
	return(0);
}
#endif

int displayInit(void)
{
	int c;

	d.start_time = 0;
	d.total_time = 0;
	d.current_time = 0;
	d.elapsed_time = 0;
	d.percent_complete = 0.0;
	d.display_interval = 1;
	d.overtime_flag = 0;
	d.k = 1024;
	d.twiddle = '-';
	for (c = 0; c < 80; c++)
		d.title[c] = 0;
	d.screen_width = 79;
	d.manual_width = 0;
	d.screen_width_minus_one = DEFAULT_SW_MINUS_ONE;
	d.screen_height = 23;
	d.manual_height = 0;
	d.screen_height_minus_one = DEFAULT_SH_MINUS_ONE;
	d.display_wait = 0;
	d.display_numeric = 0;
	d.display_twiddle = DEFAULT_DISPLAY_TWIDDLE;
	d.display_title = 1;
	d.display_datacount = 1;
	d.display_throughput = 1;
	d.display_time = 1;
	d.display_elapsed_only = 0;
	d.display_percent = 1;
	d.display_bar = 1;
	d.display_summary = 1;
	d.display_ansi = 0;
	d.display_throughput_bits = 0;
	d.display_count_bits = 0;
	d.space_bg_color = 0;
	d.twiddle_fg_color = "[32m"; /* green */
	d.twiddle_bg_color = 0;
	d.twiddle_fg_bold = 0;
	d.title_fg_color = "[33m"; /* yellow */
	d.title_bg_color = 0;
	d.title_fg_bold = 0;
	d.datacount_fg_color = "[32m"; /* green */
	d.datacount_bg_color = 0;
	d.datacount_fg_bold = 1; /* bold */
	d.throughput_label_fg_color = 0;
	d.throughput_label_bg_color = 0;
	d.throughput_label_fg_bold = 0;
	d.throughput_fg_color = "[32m"; /* green */
	d.throughput_bg_color = 0;
	d.throughput_fg_bold = 1; /* bold */
	d.time_label_fg_color = 0;
	d.time_label_bg_color = 0;
	d.time_label_fg_bold = 0;
	d.time_fg_color = "[32m"; /* green */
	d.time_bg_color = 0;
	d.time_fg_bold = 1; /* bold */
	d.percent_fg_color = "[32m"; /* green */
	d.percent_bg_color = 0;
	d.percent_fg_bold = 1; /* bold */
	d.bar_fg_color = "[33m"; /* yellow */
	d.bar_bg_color = 0;
	d.bar_fg_bold = 1; /* bold */
	d.barbrace_fg_color = "[31m"; /* red */
	d.barbrace_bg_color = 0;
	d.barbrace_fg_bold = 0;
	d.bar_open_brace = '[';
	d.bar_close_brace = ']';
	d.bar_complete = '=';
	d.bar_incomplete = ' ';
	d.total_display_percent = 1;
	d.info_file = 0;
	d.info_fin = 0;
	d.info_num = 0;
	d.info_percent_threshold = 0.0;
	d.info_percent_count = 0.0;
	for (c = 0; c < 256; c++)
		d.info_line[c] = 0;
	return(0);
}

int readline_infofile(void)
{
	int c = 0;

	for (c = 0; c < 256; c++) {
		d.info_line[c] = 0;
	}
	if (!feof(d.info_fin)) {
		fgets(d.info_line, (size_t)256, d.info_fin);
	}
	return(0);
}

int is_delimiter_infofile(void)
{
	if (strcmp(d.info_line, "@@@") == 0)
		return(1);
	if (strcmp(d.info_line, "@@@\n") == 0)
		return(1);
	return(0);
}

int init_infofile(void)
{
	d.info_fin = fopen(d.info_file, "r");
	if (d.info_fin == 0) {
		print_error(stderr, "Cannot open infofile for reading: %s", d.info_file);
		return(1);
	}

	d.info_num = 1;
	while (!feof(d.info_fin)) {
		if (readline_infofile() != 0) {
			print_error(stderr, "Read error on infofile: %s", d.info_file);
			return(1);
		}
		if (is_delimiter_infofile()) {
			d.info_num++;
		}
	}

	fclose(d.info_fin);
	d.info_fin = 0;

	d.info_fin = fopen(d.info_file, "r");
	if (d.info_fin == 0) {
		print_error(stderr, "Cannot reopen infofile for reading: %s", d.info_file);
		return(1);
	}

	d.info_percent_threshold = 100.0 / (float)(d.info_num);
	d.info_percent_count = d.info_percent_threshold;

	return(0);
}

int shutdown_infofile(void)
{
	fclose(d.info_fin);
	return(0);
}

void displayAnsiNormal(void)
{
	if (d.display_ansi) {
		fprintf(stderr, "[0m");
	}
}

void displayClearScreen(void)
{
	if (d.display_ansi) {
		fprintf(stderr, "[J");
	}
	else {
		int c = 0;

		for (c = 0; c < d.screen_height; c++) {
			fprintf(stderr, "\n");
		}
	}
}

/*
void displayAnsiMovetoXY(int x, int y)
{
	if (d.display_ansi) {
		fprintf(stderr, "[%d;%dH", x, y);
	}
}
*/

/*
void displayAnsiClearEOL(void)
{
	if (d.display_ansi) {
		fprintf(stderr, "[K");
	}
}
*/

int displayInfo(void)
{
	int c = 0;

	if (d.info_fin == 0)
		return(0);

	displayAnsiNormal();
	displayClearScreen();
	readline_infofile();
	while (!feof(d.info_fin) && !is_delimiter_infofile()) {
		fprintf(stderr, "%s", d.info_line);
		c++;
		readline_infofile();
	}
	while (c < d.screen_height) {
		c++;
		fprintf(stderr, "\n");
	}
	return(0);
}

int displayNextInfo(void)
{
	if (d.display_numeric != 0)
		return(0);
	if (d.info_fin == 0)
		return(0);

	if (d.info_percent_count <= d.percent_complete) {
		displayInfo();
		d.info_percent_count += d.info_percent_threshold;
	}
	return(0);
}

int displayBegin(void)
{
	d.start_time = time(0);
	d.current_time = time(0);

#ifdef CAN_RESIZE
	displayGetSize();
	if (displaySetSigWinch() != 0) {
		print_error(stderr, "Could not install SIGWINCH signal handler");
		print_esup(stderr, "Window resize events will be ignored");
	}
#endif

	if (d.info_file != 0) {
		if (init_infofile() != 0) {
			return(1);
		}
		displayInfo();
	}

	return(0);
}

#define sec_per_hour 3600
#define sec_per_minute 60

int calculateCountDisplay(
	uint64 *total_count,
	char **total_count_units,
	float *short_count,
	char **short_count_units
	)
{
	uint64 num = 0;
	uint64 div = 1;

	*total_count = io.total_write;
	if (d.display_count_bits)
		*total_count_units = "b";
	else
		*total_count_units = "B";

	num = io.total_write;

	if (d.display_count_bits) {
		*short_count_units = "b ";
		num *= 8;
	}
	else {
		*short_count_units = "B ";
	}

	*short_count = 0.0;

	if (num >= d.k) {
		if (d.display_count_bits)
			*short_count_units = "Kb";
		else
			*short_count_units = "KB";
		div = d.k;
	}
	if (num >= (d.k * d.k)) {
		if (d.display_count_bits)
			*short_count_units = "Mb";
		else
			*short_count_units = "MB";
		num /= d.k;
	}
	if (num >= (d.k * d.k)) {
		if (d.display_count_bits)
			*short_count_units = "Gb";
		else
			*short_count_units = "GB";
		num /= d.k;
	}
	if (num >= (d.k * d.k)) {
		if (d.display_count_bits)
			*short_count_units = "Tb";
		else
			*short_count_units = "TB";
		num /= d.k;
	}
	if (num >= (d.k * d.k)) {
		if (d.display_count_bits)
			*short_count_units = "Pb";
		else
			*short_count_units = "PB";
		num /= d.k;
	}
	if (num >= (d.k * d.k)) {
		if (d.display_count_bits)
			*short_count_units = "Eb";
		else
			*short_count_units = "EB";
		num /= d.k;
	}

	*short_count = ((float)num / div);
	return(0);
}

int calculateThroughputDisplay(
	uint64 *total_throughput,
	char **total_throughput_units,
	float *short_throughput,
	char **short_throughput_units
	)
{
	uint64 num = 0;
	uint64 div = 1;

	if (d.elapsed_time > 0) {
		*total_throughput = io.total_write / d.elapsed_time;
		num = io.total_write / d.elapsed_time;
	}
	else {
		*total_throughput = 0;
		num = 0;
	}
	if (d.display_throughput_bits) {
		*total_throughput *= 8;
		*total_throughput_units = "b";
		*short_throughput_units = "b/s ";
		num *= 8;
	}
	else {
		*total_throughput_units = "B";
		*short_throughput_units = "B/s ";
	}

	*short_throughput = 0.0;

	if (num >= d.k) {
		if (d.display_throughput_bits)
			*short_throughput_units = "Kb/s";
		else
			*short_throughput_units = "KB/s";
		div = d.k;
	}
	if (num >= (d.k * d.k)) { 
		if (d.display_throughput_bits)
			*short_throughput_units = "Mb/s";
		else
			*short_throughput_units = "MB/s";
		num /= d.k;
	}
	if (num >= (d.k * d.k)) { 
		if (d.display_throughput_bits)
			*short_throughput_units = "Gb/s";
		else
			*short_throughput_units = "GB/s";
		num /= d.k; 
	}
	if (num >= (d.k * d.k)) { 
		if (d.display_throughput_bits)
			*short_throughput_units = "Tb/s";
		else
			*short_throughput_units = "TB/s";
		num /= d.k;
	}
	*short_throughput = ((float)num / div);
	return(0);
}

int calculateTimeDisplay(uint64 *sptr, uint64 *mptr, uint64 *hptr)
{
	if (*sptr >= sec_per_hour) {
		*hptr = *sptr / sec_per_hour;
		*sptr -= *hptr * sec_per_hour;
	}
	if (*sptr >= sec_per_minute) {
		*mptr = *sptr / sec_per_minute;
		*sptr -= *mptr * sec_per_minute;
	}
	return(0);
}

int calculatePercentComplete(void)
{
	if (io.total_size_known && (io.total_size > 0)) {
		d.percent_complete = (float)(io.total_write * 100.0 / io.total_size);
		if (d.percent_complete < 0) {
			d.percent_complete = 9999.9;
		}
	}
	return(0);
}

void displayAnsi(char *fg, char *bg, int b)
{
	if (d.display_ansi) {
		if (fg != 0) {
			fprintf(stderr, "%s", fg);
		}
		if (bg != 0) {
			fprintf(stderr, "%s", bg);
		}
		if (b) {
			fprintf(stderr, "[1m");
		}
	}
}

void displayTwiddle(void)
{
	static uint64 last_write = 0;
	static time_t last_time = 0;

	if (last_time == 0) last_time = time(0);
	if (last_time == time(0)) return;
	if (last_write == io.total_write) return;

	last_time = time(0);
	last_write = io.total_write;

	switch (d.twiddle) {
		case '\\': d.twiddle = '|';  break;
		case '|':  d.twiddle = '/';  break;
		case '/':  d.twiddle = '-';  break;
		case '-':  d.twiddle = '\\'; break;
	}
	displayAnsi(d.twiddle_fg_color, d.twiddle_bg_color, d.twiddle_fg_bold);
	fprintf(stderr, "%c\r", d.twiddle);
}

int displayPrint(void)
{
	uint64 eta = 0;
	uint64 total_throughput = 0;
	char *total_throughput_units = "";
	float short_throughput = 0.0;
	char* short_throughput_units = "";
	uint64 hours = 0;
	uint64 minutes = 0;
	uint64 seconds = 0;
	uint64 total_count = 0;
	char *total_count_units = "";
	float short_count = 0.0;
	char *short_count_units = "";
	char *time_title = "eta";
	int screen_used = 0;
	int this_width = 0;

	if (d.display_wait && (io.total_write == 0)) {
		return(0);
	}

	displayNextInfo();

	d.current_time = time(0);
	d.elapsed_time = d.current_time - d.start_time;

	if (
		calculateThroughputDisplay(
			&total_throughput,
			&total_throughput_units,
			&short_throughput,
			&short_throughput_units
			)
		!= 0
		)
	{
		return(1);
	}

	if (calculatePercentComplete() != 0) {
		return(1);
	}

	if (d.display_numeric != 0) {
		fprintf(stderr, "%d\n",  (int)d.percent_complete);
		return(0);
	}

	if ((io.total_size_known == 1) && (!d.display_elapsed_only)) {
		if (io.total_write > 0) {
			if (io.total_size >= io.total_write) {
				if (d.percent_complete > 0.0) {
					eta = 
						(uint64)(
							100 * d.elapsed_time / d.percent_complete
						)
						- d.elapsed_time;
				}
				else {
					eta = (uint64)(-1);
				}
			}
			else {
				if (!d.overtime_flag) {
					d.overtime_flag = 1;
					d.total_time = d.elapsed_time;
				}
				eta = d.elapsed_time - d.total_time;
				time_title = "ovr";
			}
		}
		else {
			eta = 0;
		}
		seconds = eta;
	}
	else
	{
		seconds = d.elapsed_time;
		time_title = "elapsed";
	}
	if (calculateTimeDisplay(&seconds, &minutes, &hours) != 0) {
		return(1);
	}

	if (
		calculateCountDisplay(
			&total_count,
			&total_count_units,
			&short_count,
			&short_count_units)
		!= 0)
	{
		return(1);
	}

	/*
	 * Display twiddle
	 */
	this_width = 1;
	if ((d.display_twiddle) && (screen_used+this_width < d.screen_width)) {
		displayAnsi(
			d.twiddle_fg_color, 
			d.twiddle_bg_color, 
			d.twiddle_fg_bold);
		fprintf(stderr, "%c", d.twiddle);
		screen_used += this_width;
	}

	/*
	 * Display title, if set
	 */
	if ((d.display_title) && (d.title[0] != 0)) {
		this_width = strlen(d.title);
		if (screen_used > 0) {
			displayAnsiNormal();
			displayAnsi(0,d.space_bg_color,0);
			fprintf(stderr, " ");
			this_width++;
		}
		displayAnsiNormal();
		displayAnsi(
			d.title_fg_color,
			d.title_bg_color,
			d.title_fg_bold);
		fprintf(stderr, "%s", d.title);
		screen_used += this_width;
	}

	/*
	 * Display data count
	 */
	this_width = 8;
	if (screen_used > 0)
		this_width++;
	if ((d.display_datacount) && (screen_used+this_width < d.screen_width)) {
		if (screen_used > 0) {
			displayAnsiNormal();
			displayAnsi(0,d.space_bg_color,0);
			fprintf(stderr, " ");
		}
		displayAnsiNormal();
		displayAnsi(
			d.datacount_fg_color,
			d.datacount_bg_color,
			d.datacount_fg_bold);
		if (short_count > 9999.9) {
			fprintf(stderr, "+999.9%2.2s", short_count_units);
		}
		else {
			fprintf(stderr, "%6.1f%2.2s", short_count, short_count_units);
		}
		screen_used += this_width;
	}
		
	/*
	 * Display throughput
	 */
	this_width = 13;
	if (!d.display_datacount)
		this_width -= 3;
	if (screen_used > 0)
		this_width++;
	if ((d.display_throughput) && (screen_used+this_width < d.screen_width)) {
		if (screen_used > 0) {
			displayAnsiNormal();
			displayAnsi(0,d.space_bg_color,0);
			fprintf(stderr, " ");
		}
		if (d.display_datacount) {
			displayAnsiNormal();
			displayAnsi(
				d.throughput_label_fg_color,
				d.throughput_label_bg_color,
				d.throughput_label_fg_bold);
			fprintf(stderr, "at");
			displayAnsiNormal();
			displayAnsi(0,d.space_bg_color,0);
			fprintf(stderr, " ");
		}
		displayAnsiNormal();
		displayAnsi(
			d.throughput_fg_color,
			d.throughput_bg_color,
			d.throughput_fg_bold);
		fprintf(stderr, "%6.1f%4.4s", short_throughput, short_throughput_units);
		screen_used += this_width;
	}

	/*
	 * Display time
	 */
	this_width = 11+strlen(time_title);
	if (screen_used > 0)
		this_width += 2;
	if ((d.display_time) && (screen_used+this_width < d.screen_width)) {
		if (screen_used > 0) {
			displayAnsiNormal();
			displayAnsi(0,d.space_bg_color,0);
			fprintf(stderr, "  ");
		}
		displayAnsiNormal();
		displayAnsi(
			d.time_label_fg_color,
			d.time_label_bg_color,
			d.time_label_fg_bold);
		fprintf(stderr, "%s:", time_title);
		displayAnsiNormal();
		displayAnsi(0,d.space_bg_color,0);
		fprintf(stderr, " ");
		displayAnsiNormal();
		displayAnsi(
			d.time_fg_color,
			d.time_bg_color,
			d.time_fg_bold);
		if (hours > 99) {
			fprintf(stderr, "+99:99:99");
		}
		else
			fprintf(stderr, "%3u:%2.2u:%2.2u", 
				(unsigned int)hours, 
				(unsigned int)minutes, 
				(unsigned int)seconds);
		screen_used += this_width;
	}

	/*
	 * Display percent
	 */
	this_width = 5;
	if (screen_used > 0)
		this_width++;
	if ((d.display_percent) && (io.total_size_known)
		&& (screen_used+this_width < d.screen_width))
	{
		if (screen_used > 0) {
			displayAnsiNormal();
			displayAnsi(0,d.space_bg_color,0);
			fprintf(stderr, " ");
		}
		displayAnsiNormal();
		displayAnsi(
			d.percent_fg_color,
			d.percent_bg_color,
			d.percent_fg_bold);
		if (d.percent_complete > 999) {
			fprintf(stderr, "+999%%");
		}
		else {
			fprintf(stderr, "%4d%%", (int)d.percent_complete);
		}
		screen_used += this_width;
	}

	/*
	 * Display progress bar
	 */
	this_width = 5;
	if (screen_used > 0)
		this_width++;
	if ((d.display_bar) && (io.total_size_known)
		&& (screen_used+this_width < d.screen_width))
	{
		int c;
		int line_length;
		int completed_length = 0;

		if (screen_used > 0) {
			displayAnsiNormal();
			displayAnsi(0,d.space_bg_color,0);
			fprintf(stderr, " ");
			screen_used++;
		}
		this_width = d.screen_width - screen_used + 1;
		line_length = this_width - 3;
		completed_length = line_length * d.percent_complete / 100;
		displayAnsiNormal();
		displayAnsi(
			d.barbrace_fg_color,
			d.barbrace_bg_color,
			d.barbrace_fg_bold);
		fprintf(stderr, "%c", d.bar_open_brace);
		displayAnsiNormal();
		displayAnsi(
			d.bar_fg_color,
			d.bar_bg_color,
			d.bar_fg_bold);
		for (c = 0; c < line_length; c++) {
			if (c <= completed_length) {
				fprintf(stderr, "%c", d.bar_complete);
			}
			else {
				fprintf(stderr, "%c", d.bar_incomplete);
			}
		}
		displayAnsiNormal();
		displayAnsi(
			d.barbrace_fg_color,
			d.barbrace_bg_color,
			d.barbrace_fg_bold);
		fprintf(stderr, "%c", d.bar_close_brace);
	}

	displayAnsiNormal();
	fprintf(stderr, "\r");

	return(0);
}

int displayUpdate(void)
{
	if (d.display_twiddle) displayTwiddle();
	if (time(0) < d.current_time + d.display_interval) return(0);
	if (displayPrint() != 0) return(1);
	return(0);
}

int displayEnd(void)
{
	uint64 total_throughput = 0;
	char *total_throughput_units = "";
	float short_throughput = 0.0;
	char *short_throughput_units = "";
	uint64 total_count = 0;
	char *total_count_units = "";
	float short_count = 0.0;
	char *short_count_units = "";
	uint64 hours = 0;
	uint64 minutes = 0;
	uint64 seconds = 0;

	displayPrint();
	d.elapsed_time = d.current_time - d.start_time;

	if (calculatePercentComplete() != 0) {
		return(1);
	}

	if (
		calculateCountDisplay(
			&total_count,
			&total_count_units,
			&short_count,
			&short_count_units
			)
		!= 0
		)
	{
		return(1);
	}

	if (
		calculateThroughputDisplay(
			&total_throughput,
			&total_throughput_units,
			&short_throughput,
			&short_throughput_units
			)
		!= 0
		)
	{
		return(1);
	}

	seconds = d.elapsed_time;
	if (calculateTimeDisplay(&seconds, &minutes, &hours) != 0) {
		return(1);
	}

	fprintf(stderr, "\n");
	if (d.display_summary) {
		fprintf(stderr, "Copied: %llu%s (%.1f%s)",
			(long long unsigned int)total_count,
			total_count_units,
			short_count,
			short_count_units
			);
		if (io.total_size_known && d.total_display_percent) {
			fprintf(stderr, " (%d%% of expected input)", (int)d.percent_complete);
		}
		fprintf(stderr, "\n");

		fprintf(stderr, "Time: ");
		if (hours > 0) {
			fprintf(stderr, "%3u:%2.2u:%2.2u", 
				(unsigned int)hours,
				(unsigned int)minutes,
				(unsigned int)seconds);
		}
		else if (minutes > 0) {
			fprintf(stderr, "%2.2u:%2.2u",
				(unsigned int)minutes,
				(unsigned int)seconds);
		}
		else {
			fprintf(stderr, "%2u seconds",
				(unsigned int)seconds);
		}
		fprintf(stderr, "\n");

		if ((hours != 0) || (minutes != 0) || (seconds != 0)) {
			fprintf(stderr, "Throughput: %llu%s (%.1f%s)\n\n",
				(long long unsigned int)total_throughput,
				total_throughput_units,
				short_throughput,
				short_throughput_units
			);
		}
		else {
			fprintf(stderr, "Throughput: (infinite)\n\n");
		}
	}

	d.start_time = 0;
	
	return(0);
}
