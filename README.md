# Realm of Beasts (a.k.a Project Predators)
Realm of beasts is a rogue-lite Deck Building game taking inspiration from Hearthstone & Slay the Spire. It is already released for all Apple systems and can also be played unofficially on Windows (without Store/Cloud support as of yet, see Running the Game below).
[https://youtu.be/AosWRDqO3H0](https://www.youtube.com/watch?v=wgY9K8_8jrU)

The game has a dedicated page detailing the latest released cards and a few FAQs:
https://www.realmofbeasts.com/

## Implementation
The project was developed using C++ & SDL2 and is Windows/MacOS/iOS compatible. 

## Running the game
### Windows
1) Visit the Released tags in the project page.
2) Download and extract the zipped folder
3) Navigate to bin/release and run the executable

### MacOS/iOS
Download the game for your respective platform from https://apps.apple.com/us/app/realm-of-beasts/id6476781038

## Building the game
### Windows
#### Dependencies
* Make sure you have CMake with version >= 3.1
* Make sure you have >= Visual studio 2015 installed.

#### Instructions
1) Clone the project: Run `git clone https://github.com/AlexKoukoulas2074245K/Predators.git` on the directory of your choice
2) Navigate to the newly cloned project's root folder: Run `cd Predators`
3) Checkout the gtest submodule dependency:  Run `git submodule update --init lib/googletest`
4) Generate the project files: Run `make_project.bat`
5) The generated project will reside inside the build directory
6) Open the Predators.sln file with visual studio
7) Compile and run with.

## Known issues/Tasks in Progress
Sound support is unfortunately not yet implemented for Windows as of yet. I'm planning to integrate OpenAL for the windows version in the future.
