cmake_minimum_required(VERSION 3.20)
project(MoonLander C)

set(CMAKE_C_STANDARD 11)

# HOW-TO
#
# Ahhoz, hogy ez a projekt működjön menj CLion-ban a File -> Settingsbe és a Build, Execution, Deployment menü alatt navigálja a Toolchains-hez
# Fájlböngészőben menj a MinGW mappájába
# Töltsd le az alábbi mappát https://infoc.eet.bme.hu/sdl_telepito/sdl2-codeblocks2003-windows.zip
# Navigálj a .zip-ben a MinGW mappába és másold a bin, include és lib mappákat a MinGW mappájába
# Ezután a könyvtárak a helyükön vannak, a projekthez tartozó .cmake fájloknak meg kéne találniuk az SDL2-t

add_executable(MoonLander main.c Terrain.c Terrain.h General.h Physics.c Physics.h Renderer.c Renderer.h Particles.c Particles.h Lander.c Lander.h UI.c UI.h FileManager.c FileManager.h)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(SDL2_PATH "${PROJECT_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_gfx REQUIRED)

include_directories( ${SDL2_INCLUDE_DIR} )
target_link_libraries(
        MoonLander
        ${SDL2_LIBRARY}
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_GFX_LIBRARIES}
)


