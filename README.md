## Show diff from Infinitime 1.15 / 1.16

```
# 1.15.0:
git diff 1.15.0 --stat -- src ':!README.md' ':!src/libs' ':!src/displayapp/icons' ':!src/displayapp/fonts' ':!src/displayapp/widgets' ':!src/resources' ':!src/displayapp/screens' | cat

# 1.16.0:
git diff 1.16.0 --stat -- src ':!README.md' ':!src/libs' ':!src/displayapp/icons' ':!src/displayapp/fonts' ':!src/displayapp/widgets' ':!src/resources' ':!src/displayapp/screens' | cat
```

## Test using j-link

```
openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg -c "\
init; halt; reset_config none; \
flash erase_address 0x00000000 0x80000; \
program /media/oscar/disk3/var/www/html/pinetime-mcuboot-bootloader-10/bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.elf.bin verify 0x00000000; \
program /media/oscar/disk3/var/www/html/InfiniSim4/InfiniTime/build/output/src/pinetime-mcuboot-app-image-1.15.0.bin verify 0x8000; \
reset run; exit"
```

## Test mostrando logs:

```
openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg -c "\
init; \
halt; \
program /media/oscar/disk3/var/www/html/pinetime-mcuboot-bootloader-10/bin/targets/nrf52_boot/app/@mcuboot/boot/mynewt/mynewt.elf.bin verify 0x00000000; \
program /media/oscar/disk3/var/www/html/InfiniSim4/InfiniTime/build/output/src/pinetime-mcuboot-app-image-1.15.0.bin verify 0x8000; \
rtt setup 0x20000000 0x10000 \"SEGGER RTT\"; \
rtt start; \
rtt server start 9001 0; \
reset run; \
rtt polling_interval 100"
```

telnet localhost 9001

## Deploy

```
# BUILD FROM /Infinitime (cd InfiniTime):
docker pull --platform linux/amd64 infinitime/infinitime-build
rm -rf build
docker run --rm -it -v ${PWD}:/sources --user $(id -u):$(id -g) infinitime/infinitime-build
```