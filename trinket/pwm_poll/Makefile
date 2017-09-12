CC = avr-gcc
LOADER = avrdude
OBJDIR = ./obj
OBJCOPY = avr-objcopy 

## sources

SRCS := \
	main.c

OBJS := $(SRCS:.c=.o)
OBJS := $(addprefix $(OBJDIR)/,$(OBJS))

## flags

CFLAGS	= -Wall -O2 $(PFLAGS)
CFLAGS	+= -I$(PRJ_DIR)/include
CFLAGS	+= -I$(PRJ_DIR)/boards/$(PLAT)/apps/led-gpio
CFLAGS  += -DF_CPU=8000000 -mmcu=attiny85

LDFLAGS	= -Wl,-Map,$(PRG).map

DUDEFLAGS = -v -c usbtiny -p attiny85 -D -U 
DUDEFLAGS2 = -v -c usbtiny -p attiny85 -U 
## rules

led-gpio: $(OBJDIR)/poll.hex
	cp $(OBJDIR)/poll.hex $(OBJDIR)/firmware.hex
	avr-size $(OBJDIR)/poll.elf

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $^ $@

$(OBJDIR)/poll.elf: $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $^

upload:
	$(LOADER) $(DUDEFLAGS) flash:w:$(OBJDIR)/poll.hex

upload2:
	$(LOADER) $(DUDEFLAGS2) flash:w:$(OBJDIR)/poll.hex:i
		
clean:
	rm $(OBJDIR)/*.o
