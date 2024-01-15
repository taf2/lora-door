all: src/main.cpp
	../env/bin/pio run --target upload
build: src/main.cpp
	../env/bin/pio run
monitor:
	../env/bin/pio device monitor

# build for esp32s3
build-s3: src/main.cpp
	#../env/bin/pio run -e esp32s3 --target erase
	../env/bin/pio run -e tamc_termod_s3 --target upload && ../env/bin/pio device monitor
