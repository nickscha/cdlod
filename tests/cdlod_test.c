/* cdlod.h - v0.2 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) Continuous Distance-Dependent Level of Detail (CDLOD).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../cdlod.h"     /* Continuous Distance-Dependent Level of Detail */
#include "../deps/test.h" /* Simple Testing framework    */
#include "../deps/perf.h" /* Simple Performance profiler */

/* (1) Define a height function for a given x and z coordinate */
static float custom_height_function(float x, float z)
{
  /* Flat Plane (replace with perlin/simplex noise or other height algorithms) */
  return 0.0f * (x + z);
}

void cdlod_test_simple(void)
{
  /* (2) Define a memory buffer for the gneerated cdlod vertice and indices */
#define VERTICES_CAPACITY 10000
#define INDICES_CAPACITY 10000
  float vertices[VERTICES_CAPACITY];
  int indices[INDICES_CAPACITY];
  int vertices_count = 0;
  int indices_count = 0;

  /* (3) Define when different lod ranges should be applied (distance to camera) */
  /*     This has to be sorted in an ascending order                             */
  /*     First = Highest Level of Detail                                         */
  float lod_ranges[] = {0.0f, 50.0f, 100.0f, 200.0f, 400.0f};
  float patch_size = 64.0f;
  int grid_radius = 9; /* 1 = 3x3 patches */
  float skirt_depth = 10.0f;

  /* (4) Define an eye/camera position from which the CDLOD grid should be generated */
  float camera_position_x = 0.0f;
  float camera_position_y = 10.0f;
  float camera_position_z = 0.0f;

  float camera_front_x = 0.0f;
  float camera_front_z = -1.0f;

  /* (5) Generate the CDLOD vertices and indices */
  PERF_PROFILE_WITH_NAME(
      { cdlod(
            vertices, VERTICES_CAPACITY, &vertices_count,            /* Vertices data                                   */
            indices, INDICES_CAPACITY, &indices_count,               /* Indices data                                    */
            camera_position_x, camera_position_y, camera_position_z, /* Camera position                                 */
            camera_front_x, camera_front_z,                          /* Camera front vector                             */
            custom_height_function,                                  /* Y-Heightmap function                            */
            patch_size,                                              /* How large is each patch                         */
            5, lod_ranges,                                           /* Number of lod levels and the ranges             */
            grid_radius,                                             /* How big is the grid (1=3x3, 3=5x5 patches, ...) */
            skirt_depth); }, "cdlod");

  test_print_string("vertices count: ");
  test_print_int(vertices_count);
  test_print_string("\n");

  test_print_string(" indices count: ");
  test_print_int(indices_count);
  test_print_string("\n");

  assert(vertices_count < VERTICES_CAPACITY);
  assert(indices_count < INDICES_CAPACITY);

  assert(vertices_count > 0);
  assert(indices_count > 0);
}

int main(void)
{
  cdlod_test_simple();

  return 0;
}

/*
   -----------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
