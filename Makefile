CFLAGS :=

ifdef NDEBUG
CFLAGS += -DNDEBUG=${NDEBUG}
endif

ifdef NO_CXXEXCEPTIONS
CFLAGS += -DNO_CXXEXCEPTIONS=${NO_CXXEXCEPTIONS}
endif

all: tests/cpp.out

src/lib.h: src/lib.rs
	cbindgen -o src/lib.h

test: tests/cpp.out
	./tests/cpp.out

tests/cpp.out: target/debug/libhttps_everywhere_lib_cpp.a tests/wrapper.o tests/cpp/main.cpp
	g++ $(CFLAGS) -std=gnu++0x tests/cpp/main.cpp tests/wrapper.o ./target/debug/libhttps_everywhere_lib_cpp.a -I ./src -lpthread -ldl -o tests/cpp.out

tests/wrapper.o: src/lib.h src/wrapper.cpp src/wrapper.hpp
	g++ $(CFLAGS) -std=gnu++0x src/wrapper.cpp -I src/ -c -o tests/wrapper.o

target/debug/libhttps_everywhere_lib_cpp.a: src/lib.rs Cargo.toml
	cargo build

clean:
	rm -rf target

lint:
	cargo fmt -- --check
	cargo clippy
