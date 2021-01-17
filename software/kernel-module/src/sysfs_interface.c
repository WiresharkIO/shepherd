#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <asm/io.h>

#include "commons.h"
#include "sysfs_interface.h"
#include "pru_comm.h"
#include "sync_ctrl.h"

int schedule_start(unsigned int start_time_second);

struct kobject *kobj_ref;
struct kobject *kobj_mem_ref;
struct kobject *kobj_sync_ref;

static ssize_t sysfs_sync_error_show(struct kobject *kobj,
				     struct kobj_attribute *attr, char *buf);

static ssize_t sysfs_sync_error_sum_show(struct kobject *kobj,
					 struct kobj_attribute *attr,
					 char *buf);

static ssize_t sysfs_sync_correction_show(struct kobject *kobj,
					  struct kobj_attribute *attr,
					  char *buf);

static ssize_t sysfs_SharedMem_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf);

static ssize_t sysfs_state_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf);

static ssize_t sysfs_state_store(struct kobject *kobj,
				 struct kobj_attribute *attr, const char *buf,
				 size_t count);

static ssize_t sysfs_mode_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf);

static ssize_t sysfs_mode_store(struct kobject *kobj,
				struct kobj_attribute *attr, const char *buf,
				size_t count);

static ssize_t sysfs_auxiliary_voltage_store(struct kobject *kobj,
					      struct kobj_attribute *attr,
					      const char *buf, size_t count);

static ssize_t sysfs_calibration_settings_store(struct kobject *kobj,
						struct kobj_attribute *attr,
						const char *buf, size_t count);

static ssize_t sysfs_calibration_settings_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf);

static ssize_t sysfs_virtsource_settings_store(struct kobject *kobj,
						struct kobj_attribute *attr,
						const char *buf, size_t count);

static ssize_t sysfs_virtsource_settings_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf);

struct kobj_attr_struct_s {
	struct kobj_attribute attr;
	unsigned int val_offset;
};

struct kobj_attribute attr_state =
	__ATTR(state, 0660, sysfs_state_show, sysfs_state_store);

struct kobj_attr_struct_s attr_mem_base_addr = {
	.attr = __ATTR(address, 0660, sysfs_SharedMem_show, NULL),
	.val_offset = offsetof(struct SharedMem, mem_base_addr)
};
struct kobj_attr_struct_s attr_mem_size = {
	.attr = __ATTR(size, 0660, sysfs_SharedMem_show, NULL),
	.val_offset = offsetof(struct SharedMem, mem_size)
};

struct kobj_attr_struct_s attr_n_buffers = {
	.attr = __ATTR(n_buffers, 0660, sysfs_SharedMem_show, NULL),
	.val_offset = offsetof(struct SharedMem, n_buffers)
};
struct kobj_attr_struct_s attr_samples_per_buffer = {
	.attr = __ATTR(samples_per_buffer, 0660, sysfs_SharedMem_show, NULL),
	.val_offset = offsetof(struct SharedMem, samples_per_buffer)
};
struct kobj_attr_struct_s attr_buffer_period_ns = {
	.attr = __ATTR(buffer_period_ns, 0660, sysfs_SharedMem_show, NULL),
	.val_offset = offsetof(struct SharedMem, buffer_period_ns)
};
struct kobj_attr_struct_s attr_mode = {
	.attr = __ATTR(mode, 0660, sysfs_mode_show, sysfs_mode_store),
	.val_offset = offsetof(struct SharedMem, shepherd_mode)
};
struct kobj_attr_struct_s attr_auxiliary_voltage = {
	.attr = __ATTR(dac_auxiliary_voltage_mV, 0660, sysfs_SharedMem_show,
		       sysfs_auxiliary_voltage_store),
	.val_offset = offsetof(struct SharedMem, dac_auxiliary_voltage_mV)
};
struct kobj_attr_struct_s attr_calibration_settings = {
	.attr = __ATTR(calibration_settings, 0660, sysfs_calibration_settings_show,
		       sysfs_calibration_settings_store),
	.val_offset = offsetof(struct SharedMem, calibration_settings)
};
struct kobj_attr_struct_s attr_virtsource_settings = {
	.attr = __ATTR(virtsource_settings, 0660, sysfs_virtsource_settings_show,
		       sysfs_virtsource_settings_store),
	.val_offset = offsetof(struct SharedMem, virtsource_settings)
};

struct kobj_attribute attr_sync_error =
	__ATTR(error, 0660, sysfs_sync_error_show, NULL);

struct kobj_attribute attr_sync_correction =
	__ATTR(correction, 0660, sysfs_sync_correction_show, NULL);

struct kobj_attribute attr_sync_error_sum =
	__ATTR(error_sum, 0660, sysfs_sync_error_sum_show, NULL);

static struct attribute *pru_attrs[] = {
	&attr_n_buffers.attr.attr,
	&attr_samples_per_buffer.attr.attr,
	&attr_buffer_period_ns.attr.attr,
	&attr_mode.attr.attr,
	&attr_auxiliary_voltage.attr.attr,
	&attr_calibration_settings.attr.attr,
	&attr_virtsource_settings.attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = pru_attrs,
};

static struct attribute *pru_mem_attrs[] = {
	&attr_mem_base_addr.attr.attr,
	&attr_mem_size.attr.attr,
	NULL,
};

static struct attribute_group attr_mem_group = {
	.attrs = pru_mem_attrs,
};

static struct attribute *pru_sync_attrs[] = {
	&attr_sync_error.attr,
	&attr_sync_error_sum.attr,
	&attr_sync_correction.attr,
	NULL,
};

static struct attribute_group attr_sync_group = {
	.attrs = pru_sync_attrs,
};

static ssize_t sysfs_SharedMem_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf)
{
	struct kobj_attr_struct_s *kobj_attr_wrapped;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);
	return sprintf(
		buf, "%u",
		readl(pru_shared_mem_io + kobj_attr_wrapped->val_offset));
}

static ssize_t sysfs_sync_error_show(struct kobject *kobj,
				     struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%lld", sync_data->err);
}

static ssize_t sysfs_sync_error_sum_show(struct kobject *kobj,
					 struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%lld", sync_data->err_sum);
}

static ssize_t sysfs_sync_correction_show(struct kobject *kobj,
					  struct kobj_attribute *attr,
					  char *buf)
{
	return sprintf(buf, "%d", sync_data->clock_corr);
}

static ssize_t sysfs_state_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	switch (pru_comm_get_state()) {
	case STATE_IDLE:
		return sprintf(buf, "idle");
	case STATE_ARMED:
		return sprintf(buf, "armed");
	case STATE_RUNNING:
		return sprintf(buf, "running");
	case STATE_FAULT:
		return sprintf(buf, "fault");
	default:
		return sprintf(buf, "unknown");
	}
}

static ssize_t sysfs_state_store(struct kobject *kobj,
				 struct kobj_attribute *attr, const char *buf,
				 size_t count)
{
	struct timespec ts_now;
	int tmp;

	if (strncmp(buf, "start", 5) == 0) {
		if ((count < 5) || (count > 6))
			return -EINVAL;

		if (pru_comm_get_state() != STATE_IDLE)
			return -EBUSY;

		pru_comm_set_state(STATE_RUNNING);
		return count;
	}

	else if (strncmp(buf, "stop", 4) == 0) {
		if ((count < 4) || (count > 5))
			return -EINVAL;

		pru_comm_cancel_delayed_start();
		pru_comm_set_state(STATE_RESET);
		return count;
	}

	else if (sscanf(buf, "%d", &tmp) == 1) {
		/* Timestamp system clock */

		if (pru_comm_get_state() != STATE_IDLE)
			return -EBUSY;

		getnstimeofday(&ts_now);
		if (tmp < ts_now.tv_sec + 1)
			return -EINVAL;
		printk(KERN_INFO "shprd: Setting ts_start to %d", tmp);
		pru_comm_set_state(STATE_ARMED);
		pru_comm_schedule_delayed_start(tmp);
		return count;
	} else
		return -EINVAL;
}

static ssize_t sysfs_mode_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf)
{
	struct kobj_attr_struct_s *kobj_attr_wrapped;
	unsigned int mode;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);

	mode = readl(pru_shared_mem_io + kobj_attr_wrapped->val_offset);

	switch (mode) {
	case MODE_HARVEST:
		return sprintf(buf, "harvesting");
	case MODE_HARVEST_TEST:
		return sprintf(buf, "harvesting (test)");
	case MODE_EMULATE:
		return sprintf(buf, "emulation");
	case MODE_EMULATE_TEST:
		return sprintf(buf, "emulation (test)"); // TODO: adapt modes to pru-changes
	case MODE_DEBUG:
		return sprintf(buf, "debug");
    case MODE_NONE:
        return sprintf(buf, "none");
	default:
		return -EINVAL;
	}
}

static ssize_t sysfs_mode_store(struct kobject *kobj,
				struct kobj_attribute *attr, const char *buf,
				size_t count)
{
	struct kobj_attr_struct_s *kobj_attr_wrapped;
	unsigned int mode;

	if (pru_comm_get_state() != STATE_IDLE)
		return -EBUSY;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);

	if (pru_comm_get_state() != STATE_IDLE)
		return -EBUSY;

	if (strncmp(buf, "harvesting", 10) == 0) {
		if ((count < 10) || (count > 11))
			return -EINVAL;

		mode = MODE_HARVEST;
	} else if (strncmp(buf, "harvesting (test)", 4) == 0) {
		if ((count < 4) || (count > 5))
			return -EINVAL;

		mode = MODE_HARVEST_TEST;
	} else if (strncmp(buf, "emulation", 9) == 0) {
		if ((count < 9) || (count > 10))
			return -EINVAL;

		mode = MODE_EMULATE;
	} else if (strncmp(buf, "emulation (test)", 7) == 0) {
		if ((count < 7) || (count > 8))
			return -EINVAL;

		mode = MODE_EMULATE_TEST;
	} else if (strncmp(buf, "debug", 5) == 0) {
		if ((count < 5) || (count > 6))
			return -EINVAL;

		mode = MODE_DEBUG;
	} else
		return -EINVAL;

	writel(mode, pru_shared_mem_io + kobj_attr_wrapped->val_offset);
	printk(KERN_INFO "shprd: new mode: %d", mode);
	pru_comm_set_state(STATE_RESET);
	return count;
}

static ssize_t sysfs_auxiliary_voltage_store(struct kobject *kobj,
					      struct kobj_attribute *attr,
					      const char *buf, size_t count)
{
	unsigned int tmp;
	struct kobj_attr_struct_s *kobj_attr_wrapped;

	if (pru_comm_get_state() != STATE_IDLE)
		return -EBUSY;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);

	if (sscanf(buf, "%u", &tmp) == 1) {
		printk(KERN_INFO "shprd: Setting auxiliary DAC-voltage to %u",
		       tmp);
		writel(tmp, pru_shared_mem_io + kobj_attr_wrapped->val_offset);

		pru_comm_set_state(STATE_RESET);
		return count;
	}

	return -EINVAL;
}

static ssize_t sysfs_calibration_settings_store(struct kobject *kobj,
						struct kobj_attribute *attr,
						const char *buf, size_t count)
{
	struct CalibrationSettings tmp;
	struct kobj_attr_struct_s *kobj_attr_wrapped;

	if (pru_comm_get_state() != STATE_IDLE)
		return -EBUSY;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);

	if (sscanf(buf,"%d %d %d %d", &tmp.adc_load_current_gain,
		   &tmp.adc_load_current_offset, &tmp.adc_load_voltage_gain,
		   &tmp.adc_load_voltage_offset) == 4) {
		printk(KERN_INFO
		       "shprd: Setting current calibration settings. Current gain: %d, current offset: %d\n",
		       tmp.adc_load_current_gain, tmp.adc_load_current_offset);

		printk(KERN_INFO
		       "shprd: Setting voltage calibration settings. Voltage gain: %d, voltage offset: %d\n",
		       tmp.adc_load_voltage_gain, tmp.adc_load_voltage_offset);

		writel(tmp.adc_load_current_gain,
		       pru_shared_mem_io + kobj_attr_wrapped->val_offset);
		writel(tmp.adc_load_current_offset,
		       pru_shared_mem_io + kobj_attr_wrapped->val_offset + 4);
		writel(tmp.adc_load_voltage_gain,
		       pru_shared_mem_io + kobj_attr_wrapped->val_offset + 8);
		writel(tmp.adc_load_voltage_offset,
		       pru_shared_mem_io + kobj_attr_wrapped->val_offset + 12);

		return count;
	}

	return -EINVAL;
}

static ssize_t sysfs_calibration_settings_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf)
{
	struct kobj_attr_struct_s *kobj_attr_wrapped;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);
	return sprintf(
		buf, "%d %d %d %d",
		readl(pru_shared_mem_io + kobj_attr_wrapped->val_offset),
		readl(pru_shared_mem_io + kobj_attr_wrapped->val_offset + 4),
		readl(pru_shared_mem_io + kobj_attr_wrapped->val_offset + 8),
		readl(pru_shared_mem_io + kobj_attr_wrapped->val_offset + 12));
}

static ssize_t sysfs_virtsource_settings_store(struct kobject *kobj,
						struct kobj_attribute *attr,
						const char *buf, size_t count)
{
	struct kobj_attr_struct_s *kobj_attr_wrapped;
	int pos = 0;
	int i = 0;

	if (pru_comm_get_state() != STATE_IDLE)
		return -EBUSY;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);

	for (i = 0; i < sizeof(struct VirtSourceSettings); i += 4)
	{
		int read, n;
		int ret = sscanf(&buf[pos],"%d%n",&read,&n);
        pos += n;

		if (ret != 1)
			return -EINVAL;
		
		writel(read, pru_shared_mem_io + kobj_attr_wrapped->val_offset + i);	
	}

	return count;
}

static ssize_t sysfs_virtsource_settings_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf)
{
	struct kobj_attr_struct_s *kobj_attr_wrapped;
	int count = 0;
	int i = 0;

	kobj_attr_wrapped = container_of(attr, struct kobj_attr_struct_s, attr);


	for (i = 0; i < sizeof(struct VirtSourceSettings); i += 4)
	{
		count += sprintf(strlen(buf) + buf,"%d ", 
			readl(pru_shared_mem_io + kobj_attr_wrapped->val_offset + i));
	}

	return count;
}

int sysfs_interface_init(void)
{
	int retval = 0;

	kobj_ref = kobject_create_and_add("shepherd", NULL);

	if ((retval = sysfs_create_file(kobj_ref, &attr_state.attr))) {
		printk(KERN_ERR "Cannot create sysfs state attrib\n");
		goto r_sysfs;
	}

	if ((retval = sysfs_create_group(kobj_ref, &attr_group))) {
		printk(KERN_ERR "shprd: cannot create sysfs attrib group\n");
		goto r_state;
	};

	kobj_mem_ref = kobject_create_and_add("memory", kobj_ref);

	if ((retval = sysfs_create_group(kobj_mem_ref, &attr_mem_group))) {
		printk(KERN_ERR
		       "shprd: cannot create sysfs memory attrib group\n");
		goto r_group;
	};

	kobj_sync_ref = kobject_create_and_add("sync", kobj_ref);

	if ((retval = sysfs_create_group(kobj_sync_ref, &attr_sync_group))) {
		printk(KERN_ERR
		       "shprd: cannot create sysfs sync attrib group\n");
		goto r_mem;
	};

	return 0;

r_mem:
	kobject_put(kobj_mem_ref);
r_group:
	sysfs_remove_group(kobj_ref, &attr_group);
r_state:
	sysfs_remove_file(kobj_ref, &attr_state.attr);
r_sysfs:
	kobject_put(kobj_ref);

	return retval;
}

void sysfs_interface_exit(void)
{
	sysfs_remove_group(kobj_ref, &attr_group);
	sysfs_remove_file(kobj_ref, &attr_state.attr);
	kobject_put(kobj_sync_ref);
	kobject_put(kobj_mem_ref);
	kobject_put(kobj_ref);
}
