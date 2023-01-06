# sdl-punk

# What?
An immediate mode UI library specifically for SDL2 written in C.

# Why?
I created punk for a couple of reasons:
* I wanted to experiment with immediate mode UI concepts. For those who haven't encountered these before: this is a UI paradigm where the user code is typically functional style and keeps track of some of the UI state. This means that the library code can be more lightweight.
* A lot of the existing immediate mode UI libraries which work with SDL2 seem bloated to me: I wanted something which was specifically targeting my use cases.

# Does it work?
It does the job for me, though only a small number of widgets are currently supported. I will be extending punk as needed.

Punk caches widget types and positions so that it only draws widgets on-screen when required. This is particularly important because it uses software rendering - no OpenGL contexts here!

# How do I use it? (WIP)
Punk uses CMake and pulls in its SDL dependencies via CPM. This should hopefully make it easy to include in other code bases.

A demo app is included which shows off the existing widgets.
