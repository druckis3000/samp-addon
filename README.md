# SA-MP Addon
## Compatible with most recent SA-MP 0.3.7 R4-2 version

This addon adds various San Andreas patches and additional features in order to make game easier and more entertaining.

## Features

- Fast loader (skip SA-MP loading screen)
- Fast connect (skip 3s delay before connect)
- No FPS limit
- FPS counter
- Lock weather
- Unlimited draw distance
- Infinite NOS, Oxygen
- Quick turn, bunny hop & speed hack for cars
- Toggle nametags & change max distance of them
- ESP (it actually just shows nametags through walls)
- ALT+TAB without pause (/zafk command for toggling)
- Anti speed cam (auto slow down when approaching speed cam)

It also contains few cheats/bots made specifically for one server that I used to play. Shouldn't be interesting for others, but I didn't bothered removing them, might still be useful someday.

Short video of features demonstration: https://youtu.be/pXdDJU7njDw

## Compiling

All this project was written on Linux machine, compiled with mingw32 compiler. Everything seems to be working, but I never tested it in windows environment, thus I cannot guarantee that it compiles and works for you 100% exactly like it does on my machine.

You can find already compiled file **scr.asi** located in build directory. Dynamic linking for some reason doesn't work for me, it is not being injected into game, so this compiled file is statically linked and I don't know whether it works on windows or not.

If for some reason it doesn't work, I included my own makefile for compiling, you can take a look at it to see how this addon is compiled then try compiling it on windows and test it.
