##
## Custom RayTracer
## Built from scratch by Cyrian Pittaluga, 2025
## Originally developed during an EPITECH project (B-OOP-400)
## This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
##

CXX        := g++
CXXFLAGS   := -std=c++17 -Wall -Wextra -fopenmp -Iinclude -O3 -march=native -mfma -mavx2
LDFLAGS    := -lconfig++
MAKEFLAGS += -j$(shell nproc)

GRAPH_SRC := src/core/Visualizer.cpp
GRAPH_OBJ := $(GRAPH_SRC:.cpp=.o)

SRC := src/core/main.cpp                \
       src/core/Raytracer.cpp           \
       src/core/Utils.cpp               \
       src/core/Vector3D.cpp            \
       src/core/Primitive.cpp			\
       src/loaders/STLLoader.cpp        \
       src/loaders/SceneParser.cpp   	\
       src/rendering/BVH.cpp            \
       src/rendering/Camera.cpp      	\
       src/rendering/CheckerTexture.cpp	\
       src/rendering/ImageTexture.cpp	\
       src/rendering/Material.cpp       \
       src/rendering/Mesh.cpp           \
       src/rendering/PostFX.cpp         \
       src/rendering/Light.cpp          \
       src/rendering/Scene.cpp          \
       src/rendering/ShadowShading.cpp	\
       src/rendering/Skybox.cpp         \
       src/rendering/FASTRenderer.cpp   \
       src/rendering/PerlinNoise.cpp    \
       src/rendering/NoiseTexture.cpp   \
       src/primitives/Box.cpp           \
       src/primitives/Cone.cpp          \
       src/primitives/Cylinder.cpp      \
       src/primitives/Menger.cpp        \
       src/primitives/Plane.cpp         \
       src/primitives/Rectangle.cpp     \
       src/primitives/Sierpinski.cpp	\
       src/primitives/Sphere.cpp        \
       src/primitives/TangleCube.cpp	\
       src/primitives/Tetrahedron.cpp	\
       src/primitives/Torus.cpp			\
       src/primitives/Triangle.cpp		\
       src/geoTransf/Shear.cpp

OBJ := $(SRC:.cpp=.o)
EXEC := raytracer

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

graph: CXXFLAGS += -DUSE_SFML
graph: LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system
graph: $(OBJ) $(GRAPH_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $(EXEC) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(GRAPH_OBJ)

fclean: clean
	rm -f $(EXEC)

.PHONY: all clean fclean re graph

re:
	$(MAKE) fclean
	$(MAKE) all

test: all
	./$(EXEC) assets/scenes/test_showcase.cfg -f -t 32 > outputs/test_image.ppm
	./$(EXEC) assets/scenes/test_texture.cfg -f -t 32 > outputs/test_image2.ppm
	./$(EXEC) assets/scenes/test_checker.cfg -f -t 32 > outputs/test_image3.ppm

tclean: fclean
	rm -f outputs/test_image.ppm
	rm -f outputs/test_image2.ppm
	rm -f outputs/test_image3.ppm

.PHONY: all clean fclean re
