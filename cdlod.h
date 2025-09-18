/* cdlod.h - v0.2 - public domain data structures - nickscha 2025

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

typedef float (*cdlod_height_function)(float x, float z);

/* quadtree node */
typedef struct cdlod_quadtree_node
{
  float x, z; /* center position */
  float size; /* patch size */

} cdlod_quadtree_node;

CDLOD_API CDLOD_INLINE float cdlod_dist2(float dx, float dy, float dz)
{
  return dx * dx + dy * dy + dz * dz;
}

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
    int lod_count, float *lod_ranges,
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
    float half, max_size;
    cdlod_quadtree_node children[4];

    /* distance to node center */
    dx = camera_x - node.x;
    dy = camera_y - height(node.x, node.z);
    dz = camera_z - node.z;
    dist = cdlod_dist2(dx, dy, dz);

    /* LOD selection: 0 = highest detail */
    lod = 0;
    while (lod + 1 < lod_count && dist > lod_ranges[lod + 1] * lod_ranges[lod + 1])
    {
      lod++;
    }

    /* determine maximum allowed patch size for this LOD */
    max_size = patch_size;
    {
      int i;
      for (i = lod_count - 1; i > lod; --i)
      {
        max_size *= 0.5f; /* halve per step above current */
      }
    }

    /* leaf node: generate patch */
    if (node.size <= max_size)
    {
      cdlod_generate_patch(vertices, vertices_capacity, vertices_count,
                           indices, indices_capacity, indices_count,
                           &node, height, skirt_depth);
      continue;
    }

    /* subdivide into 4 children */
    half = node.size * 0.5f;

    children[0].x = node.x - half * 0.5f;
    children[0].z = node.z - half * 0.5f;
    children[0].size = half;

    children[1].x = node.x + half * 0.5f;
    children[1].z = node.z - half * 0.5f;
    children[1].size = half;

    children[2].x = node.x + half * 0.5f;
    children[2].z = node.z + half * 0.5f;
    children[2].size = half;

    children[3].x = node.x - half * 0.5f;
    children[3].z = node.z + half * 0.5f;
    children[3].size = half;

    /* push children on stack */
    if (stack_size + 4 <= 64)
    {
      stack[stack_size++] = children[0];
      stack[stack_size++] = children[1];
      stack[stack_size++] = children[2];
      stack[stack_size++] = children[3];
    }
  }
}

CDLOD_API CDLOD_INLINE void cdlod(
    float *vertices, int vertices_capacity, int *vertices_count,
    int *indices, int indices_capacity, int *indices_count,
    float camera_x, float camera_y, float camera_z,
    cdlod_height_function height,
    float patch_size,
    int lod_count,
    float *lod_ranges,
    int grid_radius,
    float skirt_depth)
{
  int cam_patch_x, cam_patch_z;
  int gx, gz;
  cdlod_quadtree_node root;

  /* reset counts */
  *vertices_count = 0;
  *indices_count = 0;

  /* camera patch coordinates */
  cam_patch_x = (int)(camera_x / patch_size);
  cam_patch_z = (int)(camera_z / patch_size);

  for (gx = -grid_radius; gx <= grid_radius; ++gx)
  {
    for (gz = -grid_radius; gz <= grid_radius; ++gz)
    {
      root.x = (float)(cam_patch_x + gx) * patch_size + patch_size * 0.5f;
      root.z = (float)(cam_patch_z + gz) * patch_size + patch_size * 0.5f;
      root.size = patch_size;

      cdlod_quadtree_traverse(vertices, vertices_capacity, vertices_count,
                              indices, indices_capacity, indices_count,
                              root, camera_x, camera_y, camera_z,
                              height, lod_count, lod_ranges,
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
