import open3d as o3d
import numpy as np

# Define input and output file paths
input_file = "bunny.ply"
output_file = "translated_bunny.ply"

# Read the PLY file
try:
    pcd = o3d.t.io.read_point_cloud(input_file)
    print(f"Successfully read {input_file} with {len(pcd.points)} points.")
except Exception as e:
    print(f"Error reading {input_file}: {e}")
    exit()

# Define the translation vector
translation_vector = [0.5, 0.5, 0.5]

# Translate the point cloud
pcd.translate(translation_vector)
print(f"Point cloud translated by {translation_vector}.")


# Write the translated point cloud to a new PLY file
try:
    o3d.t.io.write_point_cloud(output_file, pcd)
    print(f"Translated point cloud saved to {output_file}.")
except Exception as e:
    print(f"Error writing to {output_file}: {e}")
