#!/usr/bin/env python

"""
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>

From: https://github.com/gstavrinos/calc-inertia
Modified by: DanielFLopez1620
"""
import sys
from stl import mesh

# Command line args:
# 1: STL file
# 2: Mass
# 3: Scale
# Make sure to have updated numpy stl
# pip install --upgrade numpy-stl

def get_dimensions(model):
    min_ = model.vectors.min(axis=(0, 1))
    max_ = model.vectors.max(axis=(0, 1))
    return tuple(max_ - min_)

def get_inertia(x, y, z, m, scale):
    xx = (1.0 / 12.0) * m * (y**2 + z**2) * scale
    yy = (1.0 / 12.0) * m * (x**2 + z**2) * scale
    zz = (1.0 / 12.0) * m * (x**2 + y**2) * scale
    return xx, yy, zz

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: python script.py model.stl mass scale")
        sys.exit(1)

    try:
        model = mesh.Mesh.from_file(sys.argv[1])
        m = float(sys.argv[2])
        scale = float(sys.argv[3])

        x, y, z = get_dimensions(model)
        ixx, iyy, izz = get_inertia(x, y, z, m, scale)

        print(f'<inertia ixx="{ixx}" ixy="0" ixz="0" iyy="{iyy}" iyz="0" izz="{izz}" />')

    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
