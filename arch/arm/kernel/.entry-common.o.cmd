cmd_arch/arm/kernel/entry-common.o := /root/CodeSourcery/Sourcery_G++_Lite//bin/arm-none-linux-gnueabi-gcc -Wp,-MD,arch/arm/kernel/.entry-common.o.d  -nostdinc -isystem /root/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/arm-none-linux-gnueabi/4.3.3/include -Iinclude  -I/usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include -include include/linux/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-davinci/include -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork  -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -include asm/unified.h -msoft-float       -c -o arch/arm/kernel/entry-common.o arch/arm/kernel/entry-common.S

deps_arch/arm/kernel/entry-common.o := \
  arch/arm/kernel/entry-common.S \
    $(wildcard include/config/function/tracer.h) \
    $(wildcard include/config/dynamic/ftrace.h) \
    $(wildcard include/config/cpu/arm710.h) \
    $(wildcard include/config/oabi/compat.h) \
    $(wildcard include/config/arm/thumb.h) \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/aeabi.h) \
    $(wildcard include/config/alignment/trap.h) \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/unistd.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/ftrace.h \
    $(wildcard include/config/frame/pointer.h) \
    $(wildcard include/config/arm/unwind.h) \
  arch/arm/mach-davinci/include/mach/entry-macro.S \
    $(wildcard include/config/aintc.h) \
    $(wildcard include/config/cp/intc.h) \
  arch/arm/mach-davinci/include/mach/io.h \
    $(wildcard include/config/pci.h) \
  arch/arm/mach-davinci/include/mach/irqs.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/unwind.h \
  arch/arm/kernel/entry-header.S \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/linkage.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/linkage.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/ptrace.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/hwcap.h \
  include/asm/asm-offsets.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  /usr/local/dvsdk/psp/linux-2.6.32-rc2-psp03.01.00.37/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  arch/arm/kernel/calls.S \

arch/arm/kernel/entry-common.o: $(deps_arch/arm/kernel/entry-common.o)

$(deps_arch/arm/kernel/entry-common.o):
