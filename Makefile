CFLAGS :=

ifdef NDEBUG
CFLAGS += -DNDEBUG=${NDEBUG}
endif

ifdef NO_CXXEXCEPTIONS
CFLAGS += -DNO_CXXEXCEPTIONS=${NO_CXXEXCEPTIONS}
endif

all: examples/cpp.out

src/lib.h: src/lib.rs
	cbindgen -o src/lib.h

test: examples/cpp.out
	./examples/cpp.out

examples/cpp.out: target/debug/libhttps_everywhere_lib_cpp.a examples/wrapper.o examples/cpp/main.cpp
	g++ $(CFLAGS) -std=gnu++0x examples/cpp/main.cpp examples/wrapper.o ./target/debug/libhttps_everywhere_lib_cpp.a -I ./src -lpthread -ldl -o examples/cpp.out

examples/wrapper.o: src/lib.h src/wrapper.cpp src/wrapper.hpp
	g++ $(CFLAGS) -std=gnu++0x src/wrapper.cpp -I src/ -c -o examples/wrapper.o

target/debug/libhttps_everywhere_lib_cpp.a: src/lib.rs Cargo.toml
	cargo build

clean:
	rm -rf target

lint:
	cargo fmt -- --check
	cargo clippy
