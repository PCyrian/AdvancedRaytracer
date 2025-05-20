##
## Custom RayTracer
## Built from scratch by Cyrian Pittaluga, 2025
## Originally developed during an EPITECH project (B-OOP-400)
## This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
##

import math
import subprocess
from pathlib import Path
from PIL import Image

output_dir = Path("scripts/frames")
static_scene = Path("scripts/scene_static.cfg").read_text()
num_frames = 60
radius = 10.0
center = (0.0, 3.0, 0.0)
raytracer = "./raytracer"
threads = 32
spin_rate = 360.0 / num_frames

output_dir.mkdir(exist_ok=True)
ppm_files = []

poses = []
for i in range(num_frames):
    a = math.radians(i * (360.0 / num_frames))
    x = center[0] + radius * math.sin(a)
    y = center[1]
    z = center[2] + radius * math.cos(a)
    spin = i * spin_rate
    poses.append((x, y, z, spin))

print("Frame |    pos (x,y,z)    | rot (x,y,z)")
for i,(x,y,z,sp) in enumerate(poses):
    print(f"{i:02}    | ({x:.2f},{y:.2f},{z:.2f}) | (0.00,{sp:.2f},0.00)")

# === Render ===
for i,(x,y,z,sp) in enumerate(poses):
    print(f"[Render] F{i:02}: rot_y={sp:.2f}°")
    cfg = f"""camera: {{
    position: {{ x = {x:.4f}; y = {y:.4f}; z = {z:.4f}; }};
    rotation: {{ x = 0.0; y = {sp:.4f}; z = 0.0; }};
    fieldOfView = 60.0;
    resolution: {{ width = 1280; height = 720; }};
}}
// static scene
{static_scene}
"""
    cfg_path = output_dir / f"scene_{i:02}.cfg"
    cfg_path.write_text(cfg)
    ppm = output_dir / f"frame_{i:02}.ppm"
    ppm_files.append(ppm)
    with open(ppm, "w") as f:
        subprocess.run([raytracer, str(cfg_path), "-t", str(threads)], stdout=f)

pngs = []
for ppm in ppm_files:
    img = Image.open(ppm)
    p = ppm.with_suffix(".png")
    img.save(p)
    pngs.append(p)
frames = [Image.open(p) for p in pngs]
frames[0].save("output.gif", save_all=True, append_images=frames[1:], duration=50, loop=0)

for f in output_dir.iterdir():
    if f.suffix in {".cfg", ".ppm", ".png"}: f.unlink()
output_dir.rmdir()
