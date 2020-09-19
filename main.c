#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


main(int argc,char*argv[]) {	
	
	// LOAD IMAGE
	int width, height, original_no_channels;
	int desired_no_channels = 3;

	/*
	unsigned char *image = stbi_load("Lena.png",
										&width,
										&height,
										&original_no_channels,
										desired_no_channels);
		
	//*/
	
	/*
	unsigned char *image = stbi_load("building.jpg",
										&width,
										&height,
										&original_no_channels,
										desired_no_channels);

*/		
	/*			
	unsigned char *image = stbi_load("landscape.jpg",
										&width,
										&height,
										&original_no_channels,
										desired_no_channels);																			
	*/
									
	if(image == NULL)	{
		printf("Error in loading the image \n");
		//exit(1);
	}

	int kernel_size = 3;	
	int image_size = width * height * original_no_channels;
	int grey_image_size = width * height;
	unsigned char *grey_image = malloc(grey_image_size);
	
	if(grey_image == NULL){
		printf("\nUnable to allocate memory for the grey image \n");
		exit(1);
	}		
	

	unsigned char *pg = grey_image;
	int grey_channels = 1;
	 	
		
	//LUMINOSITY METHOD TO CONVERT IMAGE TO GRAY LEVELS
	clock_t tic = clock();
	unsigned char *grey_image_luminosity = malloc(grey_image_size);
	unsigned char *pg_luminosity = grey_image_luminosity;
	unsigned char *p2 = image;
	for(p2,pg_luminosity;p2 != image + image_size; p2+= original_no_channels, pg_luminosity += grey_channels){		
		*pg_luminosity = (uint8_t) (*p2 * .21 + *(p2+1) * .72 + *(p2+2) * .07);		
	}
	
	
		clock_t toc = clock();
	printf("\n Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);
	
	stbi_write_png("greyImg_luminosity.png", width, height, grey_channels, grey_image_luminosity, width * grey_channels);
	
	//CONVOLUTION			
		
		int width_L, height_L, no_channels_L;
		int desired_no_channels_L = 1;
		
		unsigned char *luminosity_image = stbi_load("greyImg_luminosity.png",
											&width_L,
											&height_L,
											&no_channels_L,
											desired_no_channels_L);
	
		
		
		float conv_kernel[kernel_size][kernel_size];
	    int i, j;
		//CREATE KERNEL WITH RANDOM VALUES
	    srand(time(NULL));
	    
	    for(i = 0; i<kernel_size; i++){
	        for(j = 0; j<kernel_size; j++){		
	            conv_kernel[i][j] = rand();
	        }
		}
		
		//LAPLACEAN KERNEL
		float lap_kernel[3][3]={
		{0,-1,0},
		{-1,4,-1},	
		{0,-1,0}};
	
		//Convolution of laplacean kernel with luminosity image
		
		int grey_image_size_L = width_L * height_L;
		unsigned char *lapl_img = malloc(grey_image_size_L);
	
		if(lapl_img == NULL){
		printf("\nUnable to allocate memory for the grey image \n");
		exit(1);	
		}
		
		unsigned char *pg_lapl = lapl_img;
		unsigned char *cont = luminosity_image;
		int luminosity_size = width_L * height_L;
				
		int contador = 0;
		for(cont,pg_lapl; cont != luminosity_image + luminosity_size ; cont += no_channels_L,pg_lapl += no_channels_L){						
			contador += 1;												
			if(cont < (luminosity_image + width_L) ||  luminosity_image + (luminosity_size - width_L-1) < cont || contador % width_L == 0 || contador % width_L-1 == 0){				
			// it is at the edge of the image
				*pg_lapl = (uint8_t) 0;				
			}else
			{				
			// perform convolution here
				int f,k;
				int suma = 0;
				for(f = 0; f<kernel_size; f+=1){				
					for(k =0;k<kernel_size;k+=1){
												
						int centro_kernel = f/kernel_size;
						int rowKernel = ((f+1) - (centro_kernel + 1))*width_L;												
						int colKernel = (k+1) - (centro_kernel + 1);
						
						suma += lap_kernel[f][k] * *(cont + colKernel + rowKernel);																		
					}					
				}
				*pg_lapl = (uint8_t) suma;															
			}			
		}
	
	// PRINTING THE TIME ELAPSED

	
	// SAVE ORIGINAL IMAGE, GREY IMAGE AND LUMINOSITY IMAGE
	stbi_write_png("laplacean.png", width_L, height_L, grey_channels, lapl_img, width_L * grey_channels);
	
	// FREE THE MEMORY OF THE IMAGES
	stbi_image_free(image);
	stbi_image_free(grey_image);
	stbi_image_free(grey_image_luminosity);
	stbi_image_free(lapl_img);	
	
    return 0;
}
