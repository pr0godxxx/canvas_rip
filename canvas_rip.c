#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lodepng.h"

typedef uint32_t pixel_t; //32 bit rgba pixel

int width;
int height;
pixel_t* pixels;

pixel_t corners[4];

typedef struct
{
	int x;
	int y;
	int w;
	int h;
}image_t;

int main(int argc, char** argv)
{
	width = 0;
	height = 0;
	pixels = NULL;

	pixel_t transparent_key = 0; //black
	
	image_t image;

	lodepng_decode32_file(&pixels, &width, &height, argv[1]);

	if(width > 0 && height > 0 && pixels)
	{
		corners[0] = pixels[0];
		corners[1] = pixels[width-1];
		corners[2] = pixels[(width*height)-width-1];
		corners[3] = pixels[(width*height)-1];

		int colors[4] = { 0, 0, 0, 0 }; //times a color is shared
		int most = 0;

		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4-i;j++)
			{
				if(corners[i] == corners[i+j])
				{
					++colors[i];
				}
			}
		}
		for(int i=0;i<4;i++)
		{
			if(colors[i]>most) most = i;
		}

		transparent_key = corners[most]; //transparent key will default to top left if there are no identical colors (this would normally mean no transparent key, it's fullscreen)

		if(most == 0)
		{
			//the image is likely (definitely???) fullscreen, return
			printf("Image doesn't have transparent key, stopping.\n");
			return 0;
		}
		else
		{
			//need to handle two colors being equally present in the corners (2/2)	

			pixel_t* current_pixel;


			//find the y that the image begins at
			current_pixel = pixels;
			while(*current_pixel == transparent_key) ++current_pixel; //if the image is fully transparent this might go out of bounds and crash..?
			image.y = (current_pixel - pixels) / width;

			printf("Image Y: %d\n", image.y);

			//find the x that the image begins at
			current_pixel = pixels;
			while(*current_pixel == transparent_key)
			{
					if( (current_pixel - pixels) + width < width*height)
						current_pixel += width;
					else
						current_pixel = pixels + ( (current_pixel - pixels) - ( ( (current_pixel - pixels) / width) * width) + 1);
			}
		
			image.x = (current_pixel - pixels) - ( ( (current_pixel - pixels) / width) * width);

			printf("Image X: %d\n", image.x);

			//find the y that the image ends at
			current_pixel = pixels + (width*height)-1; //bottom right corner
			while(*current_pixel == transparent_key) --current_pixel;
			image.h = ((current_pixel - pixels) / width) - image.y + 1;

			printf("Image Height: %d\n", image.h);

			//find the x that the image ends at
			current_pixel = pixels + width - 1;
            while(*current_pixel == transparent_key)
            {
                    if( (current_pixel - pixels) + width < width*height)
                        current_pixel += width;
                    else
                        current_pixel = pixels + ( (current_pixel - pixels) - ( ( (current_pixel - pixels) / width) * width) - 1);
            }

            image.w = (current_pixel - pixels) - ( ( (current_pixel - pixels) / width) * width) - image.x + 1;

			printf("Image Width: %d\n", image.w);

			pixel_t* new_image = (pixel_t*)malloc(image.w * image.h * sizeof(pixel_t));
			
			int canvas_offset = (image.x + image.y * width);
			int img_offset = 0;

			for(int i=0;i<image.h;i++)
			{
				memcpy(&new_image[img_offset], &pixels[canvas_offset], image.w * sizeof(pixel_t));
				img_offset += image.w;
        		canvas_offset += width;
			}

			printf("Saving image..\n");
			char* path = (char*)malloc(strlen("output/") + strlen(argv[1]) + 1);
			strcpy(path, "output/");
			strcat(path, argv[1]);
			lodepng_encode32_file(path, new_image, image.w, image.h);
			free(new_image);

		}


		printf("Transparent color: %08x", transparent_key);
		
		return 0;
	}
	else
	{
		printf("An error occurred while loading the image.\n");
		return -1;
	}
}
