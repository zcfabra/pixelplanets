APP_NAME = planet
BUILD_DIR = ./bin
SRC_FILES = ./src/*.cc
C_FLAGS = -Wall -Wno-deprecated

APP_DEFINES:=
APP_INCLUDES:= -I./src/vendors/GLFW -framework Cocoa -framework OpenGL -framework IOKit
APP_LINKERS:= -L ./src/vendors/GLFW/lib -lglfw3

build:
	clang -std=c++17 $(SRC_FILES) -o $(BUILD_DIR)/$(APP_NAME) $(C_FLAGS) $(APP_INCLUDES) $(APP_LINKERS)