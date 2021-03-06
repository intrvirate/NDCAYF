# NDCAYF 
![cover art](/lore/coverart.png)
![manifest](/lore/Manifest.png)
### "Numbers Don't Care About Your Feelers"
Lightweight first person shooter focused on multiplayer LAN-based gameplay, and geared towards native Linux compatibility.

## Contributors
* **Nathan Jonson** - [intrvirate](https://github.com/intrvirate)
* **Kevin Lockwood** - [kevin-b-lockwood](https://github.com/kevin-b-lockwood)
* **Matthew Tiemersma** - [NottMatt](https://github.com/NottMatt)
* **Luke Gantar** - [lukeg32](https://github.com/lukeg32)

## Objectives
* Lightweight FPS game
* Native Linux compatibility

## Keybinds
`q` enable debug mesh layer

`c` show backs of meshes

`r` increment through translation, rotation, and scaling

`F1` quit

## Dependencies
* cmake
* glfw3 (glfw-x11 on ar(ch|tix)linux)
* glew
* assimp
* bullet
* glm
* openal (freealut on ar(ch|tix)linux)

## Linux Installation:
1. Clone this git repo
2. Ensure you have all proper dependencies installed
3. `cd` into the repo directory and run `cmake ./*`
4. Run `make`
5. Run `./NDCAYF`

### Documentation:

What little there is can be found in
[docs](https://github.com/intrvirate/NDCAYF/tree/master/docs)
in manpage format.

To integrate this with your linux install, simply make the following
`~/.manpath` file:

```
MANDATORY_MANPATH path/to/NDCAYF/docs
```

This tells `man` where to find our docs so that you don't need to specify the
entire path every time.

## Windows Installation:
1. [Our more detailed, recommended method](https://wiki.artixlinux.org/Main/Installation)

## Current Progress
* Render engine

## Feature Ideas:
* CTF game mode in which both teams can move both flags.
* Quake game mode, modeled after jaun tap, start with one bullet and get one per kill.
* Mario Kart Game mode, vehicle-based FFA.

## Links and Resources
https://trello.com/b/23Q76eGP/ndcayf

### GitHub Repository
[NDCAYF](https://github.com/intrvirate/NDCAYF)
[NDCAYF-Server](https://github.com/lukeg32/NDCAYF-Server)

### Lore and Story Concepts
[No_Longer_Human.pdf](https://github.com/intrvirate/NDCAYF/tree/master/lore)

### Acknowledgements
* Inspired in part by Halo CE 2001
* Written using OpenGL
