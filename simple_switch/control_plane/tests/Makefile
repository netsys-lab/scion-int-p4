CXX = clang++
CXXFLAGS += -Wall -Wextra -Wno-unused-parameter -Werror -std=c++20 -MMD -MP -I../../control_plane

VPATH = ..
# Add source files needed by the tests to SRC
SRC = $(wildcard *.cpp)
OBJS := $(SRC:%=%.o)
DEPS := $(OBJS:.o=.d)

TARGET = tests

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET) *.cpp.o *.cpp.d
