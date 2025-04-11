#ifndef COLOH_H
#define COLOH_H
#include <stdlib.h>
#include "vec3.h"

/*
 * write_color - Write the color to a file in PPM format.
 *
 * @fp: The file pointer to write to. 
 * @pixel_color: The color to write, represented as a vec3.
 */
void write_color(FILE* fp, const vec3 pixel_color) {
  // Write the translated [0,255] value of each color component.
  int ir = (int) (255.999 * pixel_color.x);
  int ig = (int) (255.999 * pixel_color.y);
  int ib = (int) (255.999 * pixel_color.z);

  fprintf(fp, "%d %d %d\n", ir, ig, ib);
}

#endif