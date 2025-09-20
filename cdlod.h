/* cdlod.h - v0.4- public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) Continuous Distance-Dependent Level of Detail (CDLOD).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef CDLOD_H
#define CDLOD_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define CDLOD_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
#define CDLOD_INLINE __inline__
#elif defined(_MSC_VER)
#define CDLOD_INLINE __inline
#else
#define CDLOD_INLINE
#endif

#define CDLOD_API static

/* Maximium number of lod levels */
#ifndef CDLOD_MAX_LODS
#define CDLOD_MAX_LODS 8
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4699) /* MSVC-specific aliasing warning */
#endif
CDLOD_API CDLOD_INLINE float cdlod_invsqrt(float number)
{
  union
  {
    float f;
    long i;
  } conv;

  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  conv.f = number;
  conv.i = 0x5f3759df - (conv.i >> 1); /* Magic number for approximation */
  y = conv.f;
  y = y * (threehalfs - (x2 * y * y)); /* One iteration of Newton's method */

  return (y);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

CDLOD_API CDLOD_INLINE float cdlod_sqrtf(float x)
{
  return (x * cdlod_invsqrt(x));
}

typedef float (*cdlod_height_function)(float x, float z);

/* quadtree node */
typedef struct cdlod_quadtree_node
{
  float x, z; /* center position */
  float size; /* patch size */

} cdlod_quadtree_node;

/* generate a single quad patch (two triangles) */
CDLOD_API CDLOD_INLINE void cdlod_generate_patch(
    float *vertices, int vertices_capacity, int *vertices_count,
    int *indices, int indices_capacity, int *indices_count,
    cdlod_quadtree_node *node, cdlod_height_function height, float skirt_depth)
{
  int base_vertex;
  float half;
  float x0, x1, z0, z1;
  float h00, h10, h11, h01;

  /* check capacity (4 verts + 4*2 skirt verts = 12 verts, each 3 floats = 36) */
  if (*vertices_count + 36 > vertices_capacity || *indices_count + (6 + 4 * 6) > indices_capacity)
  {
    return;
  }

  base_vertex = *vertices_count / 3;
  half = node->size * 0.5f;

  x0 = node->x - half;
  x1 = node->x + half;
  z0 = node->z - half;
  z1 = node->z + half;

  /* Cache corner heights */
  h00 = height(x0, z0);
  h10 = height(x1, z0);
  h11 = height(x1, z1);
  h01 = height(x0, z1);

  /* vertices */
  vertices[(*vertices_count)++] = x0;
  vertices[(*vertices_count)++] = h00;
  vertices[(*vertices_count)++] = z0;
  vertices[(*vertices_count)++] = x1;
  vertices[(*vertices_count)++] = h10;
  vertices[(*vertices_count)++] = z0;
  vertices[(*vertices_count)++] = x1;
  vertices[(*vertices_count)++] = h11;
  vertices[(*vertices_count)++] = z1;
  vertices[(*vertices_count)++] = x0;
  vertices[(*vertices_count)++] = h01;
  vertices[(*vertices_count)++] = z1;

  /* indices (CCW winding) */
  indices[(*indices_count)++] = base_vertex + 0;
  indices[(*indices_count)++] = base_vertex + 2;
  indices[(*indices_count)++] = base_vertex + 1;

  indices[(*indices_count)++] = base_vertex + 0;
  indices[(*indices_count)++] = base_vertex + 3;
  indices[(*indices_count)++] = base_vertex + 2;

  /* skirts: each edge = 2 new verts + 2 triangles */

  /* left edge (v0 -> v3) */
  vertices[(*vertices_count)++] = x0;
  vertices[(*vertices_count)++] = h00 - skirt_depth;
  vertices[(*vertices_count)++] = z0;

  vertices[(*vertices_count)++] = x0;
  vertices[(*vertices_count)++] = h01 - skirt_depth;
  vertices[(*vertices_count)++] = z1;

  indices[(*indices_count)++] = base_vertex + 0;
  indices[(*indices_count)++] = base_vertex + 4;
  indices[(*indices_count)++] = base_vertex + 3;

  indices[(*indices_count)++] = base_vertex + 3;
  indices[(*indices_count)++] = base_vertex + 4;
  indices[(*indices_count)++] = base_vertex + 5;

  /* right edge (v1 -> v2) */
  vertices[(*vertices_count)++] = x1;
  vertices[(*vertices_count)++] = h10 - skirt_depth;
  vertices[(*vertices_count)++] = z0;

  vertices[(*vertices_count)++] = x1;
  vertices[(*vertices_count)++] = h11 - skirt_depth;
  vertices[(*vertices_count)++] = z1;

  indices[(*indices_count)++] = base_vertex + 1;
  indices[(*indices_count)++] = base_vertex + 2;
  indices[(*indices_count)++] = base_vertex + 6;

  indices[(*indices_count)++] = base_vertex + 2;
  indices[(*indices_count)++] = base_vertex + 7;
  indices[(*indices_count)++] = base_vertex + 6;

  /* bottom edge (v0 -> v1) */
  vertices[(*vertices_count)++] = x0;
  vertices[(*vertices_count)++] = h00 - skirt_depth;
  vertices[(*vertices_count)++] = z0;

  vertices[(*vertices_count)++] = x1;
  vertices[(*vertices_count)++] = h10 - skirt_depth;
  vertices[(*vertices_count)++] = z0;

  indices[(*indices_count)++] = base_vertex + 0;
  indices[(*indices_count)++] = base_vertex + 1;
  indices[(*indices_count)++] = base_vertex + 8;

  indices[(*indices_count)++] = base_vertex + 1;
  indices[(*indices_count)++] = base_vertex + 9;
  indices[(*indices_count)++] = base_vertex + 8;

  /* top edge (v3 -> v2) */
  vertices[(*vertices_count)++] = x0;
  vertices[(*vertices_count)++] = h01 - skirt_depth;
  vertices[(*vertices_count)++] = z1;

  vertices[(*vertices_count)++] = x1;
  vertices[(*vertices_count)++] = h11 - skirt_depth;
  vertices[(*vertices_count)++] = z1;

  indices[(*indices_count)++] = base_vertex + 3;
  indices[(*indices_count)++] = base_vertex + 10;
  indices[(*indices_count)++] = base_vertex + 2;

  indices[(*indices_count)++] = base_vertex + 2;
  indices[(*indices_count)++] = base_vertex + 10;
  indices[(*indices_count)++] = base_vertex + 11;
}

/* quadtree traversal */
/* iterative quadtree traversal using manual stack */
CDLOD_API CDLOD_INLINE void cdlod_quadtree_traverse(
    float *vertices, int vertices_capacity, int *vertices_count,
    int *indices, int indices_capacity, int *indices_count,
    cdlod_quadtree_node root,
    float camera_x, float camera_y, float camera_z,
    cdlod_height_function height,
    int lod_count, float *lod_ranges_sq,
    float patch_size, float skirt_depth)
{
  /* stack-based traversal */
  cdlod_quadtree_node stack[64]; /* supports depth ~64, more than enough */
  int stack_size = 0;

  stack[stack_size++] = root;

  while (stack_size > 0)
  {
    cdlod_quadtree_node node = stack[--stack_size];
    float dx, dy, dz, dist;
    int lod;
    float max_size;
    int i;

    /* distance to node center */
    dx = camera_x - node.x;
    dy = camera_y - height(node.x, node.z);
    dz = camera_z - node.z;
    dist = dx * dx + dy * dy + dz * dz;

    /* LOD selection: 0 = highest detail */
    lod = 0;
    while (lod + 1 < lod_count && dist > lod_ranges_sq[lod + 1])
    {
      lod++;
    }

    /* determine maximum allowed patch size for this LOD */
    max_size = patch_size;

    for (i = lod_count - 1; i > lod; --i)
    {
      max_size *= 0.5f; /* halve per step above current */
    }

    /* leaf node: generate patch */
    if (node.size <= max_size)
    {
      cdlod_generate_patch(vertices, vertices_capacity, vertices_count,
                           indices, indices_capacity, indices_count,
                           &node, height, skirt_depth);
      continue;
    }

    /* subdivide into 4 children  & push children on stack */
    if (stack_size + 4 <= 64)
    {
      float half = node.size * 0.5f;

      stack[stack_size].x = node.x - half * 0.5f;
      stack[stack_size].z = node.z - half * 0.5f;
      stack[stack_size++].size = half;

      stack[stack_size].x = node.x + half * 0.5f;
      stack[stack_size].z = node.z - half * 0.5f;
      stack[stack_size++].size = half;

      stack[stack_size].x = node.x + half * 0.5f;
      stack[stack_size].z = node.z + half * 0.5f;
      stack[stack_size++].size = half;

      stack[stack_size].x = node.x - half * 0.5f;
      stack[stack_size].z = node.z + half * 0.5f;
      stack[stack_size++].size = half;
    }
  }
}

CDLOD_API CDLOD_INLINE void cdlod(
    float *vertices, int vertices_capacity, int *vertices_count,
    int *indices, int indices_capacity, int *indices_count,
    float camera_x, float camera_y, float camera_z,
    float forward_x, float forward_z,
    cdlod_height_function height,
    float patch_size,
    int lod_count,
    float *lod_ranges,
    int grid_radius,
    float skirt_depth)
{
  int gx, gz;
  int grid_center_x, grid_center_z;
  float lod_ranges_sq[CDLOD_MAX_LODS];
  int i;

  float fx = forward_x;
  float fz = forward_z;
  float len;
  float offset_x, offset_z;

  cdlod_quadtree_node root;

  /* reset counts */
  *vertices_count = 0;
  *indices_count = 0;

  /* pre-cache lod_ranges squared assuming lod_count <= CDLOD_MAX_LODS */
  for (i = 0; i < lod_count; ++i)
  {
    lod_ranges_sq[i] = lod_ranges[i] * lod_ranges[i];
  }

  /* normalize forward vector (XZ only) */
  len = (float)(fx * fx + fz * fz);

  if (len > 0.0001f)
  {
    len = 1.0f / cdlod_sqrtf(len);
    fx *= len;
    fz *= len;
  }
  else
  {
    fx = 0.0f;
    fz = 1.0f; /* default forward = +Z */
  }

  /* compute forward shift in patch units */
  offset_x = fx * (float)(grid_radius - 1);
  offset_z = fz * (float)(grid_radius - 1);

  /* find grid center in patch coords */
  grid_center_x = (int)(camera_x / patch_size + offset_x);
  grid_center_z = (int)(camera_z / patch_size + offset_z);

  for (gx = -grid_radius; gx <= grid_radius; ++gx)
  {
    for (gz = -grid_radius; gz <= grid_radius; ++gz)
    {
      root.x = (float)(grid_center_x + gx) * patch_size + patch_size * 0.5f;
      root.z = (float)(grid_center_z + gz) * patch_size + patch_size * 0.5f;
      root.size = patch_size;

      cdlod_quadtree_traverse(vertices, vertices_capacity, vertices_count,
                              indices, indices_capacity, indices_count,
                              root, camera_x, camera_y, camera_z,
                              height, lod_count, lod_ranges_sq,
                              patch_size, skirt_depth);
    }
  }
}

#endif /* CDLOD_H */

/*
   ------------------------------------------------------------------------------
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
