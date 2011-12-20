/* 
 * This file is part of dcaenc.
 *
 * Copyright (c) 2008-2011 Alexander E. Patrakov <patrakov@gmail.com>
 *
 * dcaenc is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * dcaenc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dcaenc; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "dcaenc.h"
#include "wavfile.h"

extern const int32_t prototype_filter[512];

static int dcaenc_main(int argc, char *argv[])
{
	dcaenc_context c;
	int32_t data[512 * 6];
	uint8_t output[16384];
	wavfile * f;
	FILE * outfile;
	int bitrate;
	int wrote;
	int samples_total;
	int counter;
	
	fprintf(stderr, "%s-%s [%s]\n", PACKAGE_NAME, PACKAGE_VERSION, __DATE__);
	fprintf(stderr, "Copyright (c) 2008-2011 Alexander E. Patrakov <patrakov@gmail.com>\n\n");

	fprintf(stderr, "This program is free software: you can redistribute it and/or modify\n");
	fprintf(stderr, "it under the terms of the GNU General Public License <http://www.gnu.org/>.\n");
	fprintf(stderr, "Note that this program is distributed with ABSOLUTELY NO WARRANTY.\n\n");
	
	static const int channel_map[6] = {DCAENC_CHANNELS_MONO, DCAENC_CHANNELS_STEREO, 0,
	DCAENC_CHANNELS_2FRONT_2REAR, DCAENC_CHANNELS_3FRONT_2REAR, DCAENC_CHANNELS_3FRONT_2REAR };
	
	if (argc != 4) {
	    if (argc == 2 && !strcmp(argv[1], "--version")) {
	        fprintf(stderr, PACKAGE_NAME "-" PACKAGE_VERSION "\n");
		fprintf(stderr, PACKAGE_URL "\n");
		return 0;
	    } else {
	        fprintf(stderr, "Usage: dcaenc <input.wav> <output.dts> <kilo_bits_per_second>\n");
	        return 1;
	    }
	}
	f = wavfile_open(argv[1]);
	if (!f) {
	    fprintf(stderr, "Could not open or parse %s\n", argv[1]);
	    return 1;
	}
	bitrate = atoi(argv[3]) * 1000;
	
	samples_total = f->samples_left;
	c = dcaenc_create(f->sample_rate, channel_map[f->channels - 1], bitrate, f->channels == 6 ? DCAENC_FLAG_LFE : 0);
	
	if (!c) {
	    fprintf(stderr, "Wrong bitrate or sample rate\n");
	    return 1;
	}
	outfile = fopen_utf8(argv[2], "wb");
	if (!outfile) {
	    fprintf(stderr, "Could not open %s\n", argv[2]);
	    return 1;
	}
	
	fprintf(stderr, "Source: %s\n", argv[1]);
	fprintf(stderr, "Output: %s\n", argv[2]);
	fprintf(stderr, "KBit/s: %d\n\n", bitrate / 1000);
	
	fflush(stdout);
	fflush(stderr);
	
	counter = 0;
	
	while (wavfile_read_s32(f, data)) {
		wrote = dcaenc_convert_s32(c, data, output);
		fwrite(output, 1, wrote, outfile);
		if(counter == 0)
		{
			fprintf(stderr, "Encoding... [%3.1f%%]\r", ((double)(samples_total - f->samples_left)) / ((double)(samples_total)) * 100.0);
			fflush(stderr);
		}
		counter = (counter+1) % 128;
	}
	fprintf(stderr, "Encoding... [%3.1f%%]\n", 100.0);
	fflush(stderr);

	wrote = dcaenc_destroy(c, output);
	fwrite(output, 1, wrote, outfile);
	fclose(outfile);
	wavfile_close(f);

	fprintf(stderr, "Done.\n");
	return 0;
}

#ifdef _WIN32
#include <Windows.h>

int main( int argc, char **argv )
{
  int dcaenc_argc;
  char **dcaenc_argv;
  int exit_code;

  UINT old_cp = GetConsoleOutputCP();
  SetConsoleOutputCP(CP_UTF8);

  init_commandline_arguments_utf8(&dcaenc_argc, &dcaenc_argv);
  exit_code = dcaenc_main(dcaenc_argc, dcaenc_argv);
  free_commandline_arguments_utf8(&dcaenc_argc, &dcaenc_argv);

  SetConsoleOutputCP(old_cp);
  return exit_code;
}

#else //_WIN32

int main( int argc, char **argv )
{
    return dcaenc_main(argc, argv);
}

#endif //_WIN32