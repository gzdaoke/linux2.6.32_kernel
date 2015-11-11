cmd_arch/arm/lib/strncpy_from_user.o := /root/CodeSourcery/Sourcery_G++_Lite//bin/arm-none-linux-gnueabi-gcc -Wp,-MD,arch/arm/lib/.strncpy_from_user.o.d  -nostdinc -isystem /root/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/arm-none-linux-gnueabi/4.3.3/include -Iinclude  -I/usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include -include include/linux/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-davinci/include -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork  -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -include asm/unified.h -msoft-float       -c -o arch/arm/lib/strncpy_from_user.o arch/arm/lib/strncpy_from_user.S

deps_arch/arm/lib/strncpy_from_user.o := \
  arch/arm/lib/strncpy_from_user.S \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/linkage.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/hwcap.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \

arch/arm/lib/strncpy_from_user.o: $(deps_arch/arm/lib/strncpy_from_user.o)

$(deps_arch/arm/lib/strncpy_from_user.o):
