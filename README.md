# sdl-punk

# What?
An immediate mode UI library specifically for SDL2 written in C.

# Why?
I created punk for a couple of reasons:
* I wanted to experiment with immediate mode UI concepts. For those who haven't encountered these before: this is a UI paradigm where the calling code is typically functional style and defines how the UI looks "on-the-fly". To make this work smoothly the calling code keeps track of a lot of the UI state. In theory this means that the library code can be more lightweight, though I've ended up with some slightly tricky caching code to keep things efficient.
* A lot of the existing immediate mode UI libraries which work with SDL2 seem bloated to me: I wanted something simple, hackable, and tailored to my use-cases.

# Does it work?
I've used punk with success in a number of personal projects. Only a small number of widgets are currently supported so it's only really sufficient for simple UIs (I'll be extending as needed).

Punk caches widget types and positions so that it only draws widgets on-screen when required. This is particularly important because it uses software rendering - no OpenGL contexts here!

I'm pretty pleased with how simple the layout mechanism is. Punk supports horizontal and vertical grid layouts, which can be nested. Within each layout, the slots for widgets can have sizes which are specified proportionally, or they can have exact pixel widths.

# How do I use it?
Punk uses CMake and pulls in its SDL dependencies via CPM. This should hopefully make it easy to include in other code bases.

Before you get hacking I would recommend having a look at the demo apps which are included (particularly `demo.c`) which show off all of the existing widget types and how UIs are implemented.
