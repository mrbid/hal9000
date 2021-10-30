# hal9000
A simple faux 3D space shooter. Completely software rendered using simple 3D point projection and vector maths.

**Video:** https://youtu.be/nTaHR6ta5i4

## Synopsis
hal9000 has gone haywire, he is firing plasma at you, luckily for you the plasma is harmless but hal does not know this. You have to disable hal but you cannot shoot directly at him! You have to shoot your plasma, at hal's plasma to make it bounce off in the direction of hal. You must destroy hal as fast as you can, the winner is the player with the lowest time taken.. or if playing alone you should aim to beat your last best time.

## Controls
Just move your mouse and click to fire.
- `Press S` to see your time taken and hal's current health.
- `Press R` to reset the game.

---

### Snapcraft / Software Center
https://snapcraft.io/hal9000

---

### AppImage
https://github.com/mrbid/hal9000/raw/main/hal9000-x86_64.AppImage

---

### Build Prerequisites
`sudo apt install --assume-yes libsdl2-2.0-0 libsdl2-dev`

---

### Compile & Install
```
gcc hal9000.c -Ofast -lSDL2 -lm -o hal9000
sudo cp hal9000 /usr/bin/hal9000
```

---

### Make & Install
```
make
sudo make install /usr/bin
```

---

### Linux Download (Ubuntu 21.04)
https://github.com/mrbid/hal9000/raw/main/hal9000

### Windows Download
https://github.com/mrbid/hal9000/raw/main/hal9000.exe<br>
https://github.com/mrbid/hal9000/raw/main/SDL2.dll
