#!/usr/bin/env python3

##
## Custom RayTracer
## Built from scratch by Cyrian Pittaluga, 2025
## Originally developed during an EPITECH project (B-OOP-400)
## This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
##

import os
import sys
from PIL import Image, ImageSequence

INPUT = "output.gif"
OUTPUT = "cropped.gif"
MAX_SIZE = 9.8 * 1024 * 1024 #9.8MB la limite discord
STEP = 5

def crop_and_save(frames, durations, crop):
    cropped = []
    for im in frames:
        w, h = im.size
        cropped.append(im.crop((crop, crop, w - crop, h - crop)))
    cropped[0].save(
        OUTPUT, save_all=True, append_images=cropped[1:],
        duration=durations, loop=0, optimize=True
    )

def main():
    gif = Image.open(INPUT)
    frames = [f.convert("RGBA") for f in ImageSequence.Iterator(gif)]
    durations = [gif.info.get("duration", 100)] * len(frames)

    crop = 0
    while True:
        print(f"Trying crop={crop}px...", end=" ")
        crop_and_save(frames, durations, crop)
        size = os.path.getsize(OUTPUT)
        print(f"size={(size/1024/1024):.2f}MB")
        if size <= MAX_SIZE or crop * 2 >= frames[0].width or crop * 2 >= frames[0].height:
            break
        crop += STEP

    if size > MAX_SIZE:
        print("⚠️  Could not reduce under 9.8 MB; maximum crop reached.")
        sys.exit(1)
    else:
        print(f"✅ Saved {OUTPUT} at {(size/1024/1024):.2f} MB with crop={crop}px.")

if __name__ == "__main__":
    main()
