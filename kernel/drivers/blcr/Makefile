#
# 
#

MODULE_NAME = blcr

# small core, mostly translating board-specific
# config declarations into driver model code


#
obj-$(CONFIG_BLCR_SUPPORT)	:= $(MODULE_NAME).o
$(MODULE_NAME)-objs := cr_module.o cr_async.o cr_barrier.o cr_chkpt_req.o\
                        cr_compat.o cr_creds.o cr_dest_file.o cr_dump_self.o\
                        cr_fops.o cr_io.o cr_ktrace.o cr_mmaps.o cr_objects.o\
                        cr_pipes.o  cr_proc.o cr_relocate.o cr_rstrt_req.o cr_sync.o\
                        cr_task.o cr_timers.o cr_trigger.o cr_vmadump.o cr_watchdog.o\
                        vmadump_common.o vmadump_arm.o