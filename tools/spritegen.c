/* 
 * This is a tool to convert PNG files into a 
 * simple Palette-based C array. It uses the standard
 * library for simplicity.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>

int main()
{
	int x, y, w, h, c;
	stbi_uc* image;
	FILE* file;
	
	image = stbi_load("tools/spritesheet.png", &w, &h, &c, 4);
	file = fopen("spritesheet.h", "w");
	
	fprintf(file, "#ifndef NB_SPRITESHEET_H\n");
	fprintf(file, "#define NB_SPRITESHEET_H\n\n");
	fprintf(file, "/* This file is generated automatically via `spritesheet.c` in tools */\n\n");
	fprintf(file, "#define NB_SPRITESHEET_WIDTH %i\n", w);
	fprintf(file, "#define NB_SPRITESHEET_HEIGHT %i\n", h);
    fprintf(file, "const unsigned char nb_spritesheet[] = {\n");

	for (y = 0; y < h; y ++)
	{
		fprintf(file, "    ");
		for (x = 0; x < w; x ++)
		{
			c = (x + y * w) * 4;

			if (image[c + 3] == 0)
				fprintf(file, "4, ");
			else
				fprintf(file, "%i, ", (int)((image[c] / 256.0f) * 4));
		}
		fprintf(file, "\n");
	}
	fprintf(file, "};\n");
	fprintf(file, "#endif\n");
	fclose(file);
	return 0;
}