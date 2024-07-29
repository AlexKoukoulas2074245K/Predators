 ![loading](https://github.com/user-attachments/assets/aaae2560-9014-4371-a13d-f7c983f47126)
 
# Realm of Beasts (a.k.a Project Predators)
Realm of beasts is a rogue-lite Deck Building game taking inspiration from Hearthstone & Slay the Spire. It is already released for all Apple systems and can also be played unofficially on Windows (without Store/Cloud support as of yet, see Running the Game below).

Trailer: [https://youtu.be/AosWRDqO3H0](https://www.youtube.com/watch?v=wgY9K8_8jrU)

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
7) Compile and run the default Startup Project which should automatically be "Predators".
8) (optional) The imgui debug widgets can be locked/unlocked by pressing the mouse wheel in case they need to be moved around.
9) (optional) The unit tests can also be run by making the Predators_test the startup project for the solution.

## Code Structure Details
Due to the project being cross-platform, I've split the code is split into the following directories:
* source_common: All game and engine platform agnostic code lives here.
* source_test: All gtest suites live here aimed at unit testing different aspects of the project.
* source_desktop: All desktop common code (think keyboard input, desktop specific rendering, etc) lives here.
* source_ios: All ios specific code (think touch input, mobile rendering, etc) lives here.
* source_windows_utilities: All Windows-specific utilities (e.g. Internet connection tests via the Win API) live here.
* source_apple_utilities: All bridge code that interfaces with Apple's Objective C APIs live here (think Cloudkit, StoreKit, etc).

## Known issues/Tasks in Progress
Sound support is unfortunately not yet implemented for Windows as of yet. I'm planning to integrate OpenAL for the windows version in the future.
