#ifndef MALI_H
#define MALI_H
struct mali_device {
	int (*suspend)(u32 cores);
	int (*resume)(u32 cores);
	void (*enable_clock)(int enable);
	void (*enable_power)(int enable);
	void (*set_memory_base)(unsigned int val);
	void (*set_memory_size)(unsigned int val);
	void (*set_mem_validation_base)(unsigned int val);
	void (*set_mem_validation_size)(unsigned int val);
	void (*get_memory_base)(unsigned int *val);
	void (*get_memory_size)(unsigned int *val);
	void (*get_mem_validation_base)(unsigned int *val);
	void (*get_mem_validation_size)(unsigned int *val);
};
struct mali_device *create_mali_device(void);
void release_mali_device(struct mali_device *dev);
#endif /* MALI_H */
