# Makefile for building BME280-I2C Sample code for libmpsse on Linux.

DEPENDENCIES := -lpthread -lrt -ldl
UNAME := $(shell uname)
# Assume target is Mac OS if build host is Mac OS; any other host targets Linux
ifeq ($(UNAME), Darwin)
	DEPENDENCIES += -lobjc -framework IOKit -framework CoreFoundation
else
	DEPENDENCIES += -lrt
endif

CFLAGS = -Wall -Wextra -I/usr/local/include

DYNAMIC_LINK_OPTIONS := -Wl,-rpath /usr/local/lib

APP = bme280
STATIC_APP = $(APP)-static
DYNAMIC_APP = $(APP)-dynamic

all: $(STATIC_APP) $(DYNAMIC_APP)

$(STATIC_APP): bme280.c	
	$(CC) bme280.c -o $(STATIC_APP) -lmpsse $(CFLAGS) $(DEPENDENCIES)

$(DYNAMIC_APP): bme280.c	
	$(CC) bme280.c -o $(DYNAMIC_APP) $(CFLAGS) -lmpsse $(DEPENDENCIES) $(DYNAMIC_LINK_OPTIONS)
	
clean:
	-rm -f *.o ; rm $(STATIC_APP); rm $(DYNAMIC_APP)
