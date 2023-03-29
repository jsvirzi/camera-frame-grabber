cmake_minimum_required(VERSION 3.20)

set(TOOLS_COMPILER_PATH /usr/bin)
set(CMAKE_C_COMPILER ${TOOLS_COMPILER_PATH}/gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_COMPILER_PATH}/g++)

set(CMAKE_CXX_STANDARD 17)

project(frame-grabber C CXX)

#include_directories(/usr/local/include/opencv4)

#add_executable(frame-grabber src/main.cpp)

execute_process(
        COMMAND root-config --cflags
        OUTPUT_VARIABLE ROOT_CFLAGS
)

execute_process(
        COMMAND root-config --libs
        OUTPUT_VARIABLE ROOT_LIBS
)

message(pre-this)
message(${ROOT_CFLAGS})
message(post-this)

add_link_options(-pthread)

# find_package(ROOT REQUIRED COMPONENTS RIO Net)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package( OpenCV REQUIRED )

include_directories(${OpenCV_INCLUDE_DIRS} )
include_directories(/home/jsvirzi/utils/root/include)
# on macbook
# include_directories(/usr/local/Cellar/root/6.26.06_2/include/root)
# on linux
# include_directories(/snap/root-framework/919/usr/local/include)
add_executable(frame-grabber src/main.cpp src/udp.c)
add_executable(v4l src/v4l-xface.c)
add_executable(v4l-mmap src/v4l-mmap.c src/frame-process.cpp src/plot.cpp src/udp.c)
add_executable(test-plot src/test-plot.cpp src/plot.cpp src/udp.c)
target_link_libraries(frame-grabber -L/home/jsvirzi/utils/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -Wl,-rpath,/home/jsvirzi/utils/root/lib -pthread -lm -ldl -rdynamic)
target_link_libraries(frame-grabber ${OpenCV_LIBS})
target_link_libraries(v4l-mmap ${OpenCV_LIBS})
add_compile_options(-lpthread -lm)
# target_compile_options(-stdlib=libc++ -pthread -std=c++17 -m64)

# on macbook
# target_link_libraries(test-plot -L/usr/local/Cellar/root/6.26.06_2/lib/root -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -Wl,-rpath,/usr/local/Cellar/root/6.26.06_2/lib/root -stdlib=libc++ -lpthread -lm -ldl)
# on linux machine
# target_link_libraries(test-plot -L/snap/root-framework/919/usr/local/lib -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -Wl,-rpath,/snap/root-framework/919/usr/local/lib -pthread -lm -ldl -rdynamic)
# target_link_libraries(test-plot -lpthread -lm)
target_link_libraries(test-plot -L/home/jsvirzi/utils/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -Wl,-rpath,/home/jsvirzi/utils/root/lib -pthread -lm -ldl -rdynamic)
add_compile_options(-pthread -m64 -I/home/jsvirzi/utils/root/include)
include_directories(/home/jsvirzi/utils/root/include)
