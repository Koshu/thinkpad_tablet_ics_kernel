cmd_drivers/media/video/tegra/avp/headavp.o := /home/koshu/androidtest/cm9/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-gcc -Wp,-MD,drivers/media/video/tegra/avp/.headavp.o.d  -nostdinc -isystem /home/koshu/androidtest/cm9/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/../lib/gcc/arm-eabi/4.4.3/include -I/home/koshu/androidtest/kernel/kernel/arch/arm/include -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-tegra/include -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables  -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float -gdwarf-2            -c -o drivers/media/video/tegra/avp/headavp.o drivers/media/video/tegra/avp/headavp.S

deps_drivers/media/video/tegra/avp/headavp.o := \
  drivers/media/video/tegra/avp/headavp.S \
  /home/koshu/androidtest/kernel/kernel/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /home/koshu/androidtest/kernel/kernel/arch/arm/include/asm/linkage.h \
  /home/koshu/androidtest/kernel/kernel/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
  /home/koshu/androidtest/kernel/kernel/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /home/koshu/androidtest/kernel/kernel/arch/arm/include/asm/hwcap.h \
  drivers/media/video/tegra/avp/headavp.h \

drivers/media/video/tegra/avp/headavp.o: $(deps_drivers/media/video/tegra/avp/headavp.o)

$(deps_drivers/media/video/tegra/avp/headavp.o):
