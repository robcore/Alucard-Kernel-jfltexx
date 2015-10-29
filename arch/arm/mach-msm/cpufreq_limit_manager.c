/*
 * Author: Alucard_24 <alucard_24 XDA>
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <mach/cpufreq.h>
#include <linux/cpu.h>
#include <linux/cpufreq_limit_manager.h>
#ifdef CONFIG_STATE_NOTIFIER
#include <linux/state_notifier.h>
#endif

unsigned int scaling_min_suspend_freq = 378000;
unsigned int scaling_max_suspend_freq = CPU_MAX_FREQ;
unsigned int scaling_max_oncall_freq = CPU_MAX_ONCALL_FREQ;
unsigned int scaling_max_gps_freq = CPU_MAX_FREQ;

static bool suspended = false;
static bool gps_status = false;
static bool oncall_status = false;

#ifdef CONFIG_STATE_NOTIFIER
static struct notifier_block notif;
#endif

/*
 * cpufreq_limit_mutex protects cpu frequency changing in update_cpufreq_limit fn.
 */
static DEFINE_MUTEX(cpufreq_limit_mutex);

int update_cpufreq_limit(unsigned int limit_type, bool limit_status)
{
	unsigned int min_freq = 0;
	unsigned int max_freq = 0;
	bool immediately_update = true;
	unsigned int cpu;

	mutex_lock(&cpufreq_limit_mutex);
	switch (limit_type) {
	case 0:
		/* SUSPEND */
		suspended = limit_status;
		break;
	case 1:
		/* CALL */
		oncall_status = limit_status;
		break;
	case 2:
		/* GPS */
		gps_status = limit_status;
		break;
	default:
		break;
	}

	if (suspended) {
		min_freq = scaling_min_suspend_freq;
		max_freq = scaling_max_suspend_freq;
	}
	
	if (oncall_status && suspended) {
		min_freq = 0;
		max_freq = scaling_max_oncall_freq;
	}
		
	if (gps_status) {
		min_freq = 0;
		max_freq = scaling_max_gps_freq;
	}

	set_min_lock(min_freq);
	set_max_lock(max_freq);

	if (immediately_update) {
		unsigned int cur = 0;

		for_each_online_cpu(cpu) {
			cur = cpufreq_quick_get(cpu);
			if (cur) {
				struct cpufreq_policy policy;
				policy.cpu = cpu;

				if (cur < min_freq)
					cpufreq_driver_target(&policy,
						min_freq, CPUFREQ_RELATION_H);
				else if (cur > max_freq)
					cpufreq_driver_target(&policy,
						max_freq, CPUFREQ_RELATION_L);
			}
		}
	}
	mutex_unlock(&cpufreq_limit_mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(update_cpufreq_limit);

#ifdef CONFIG_STATE_NOTIFIER
static void cpufreq_limit_suspend(void)
{
 	/* SUSPEND */
	update_cpufreq_limit(0, true);
}

static void cpufreq_limit_resume(void)
{
	/* RESUME */
	update_cpufreq_limit(0, false);
}

static int state_notifier_callback(struct notifier_block *this,
 				unsigned long event, void *data)
 {
	switch (event) {
		case STATE_NOTIFIER_ACTIVE:
			cpufreq_limit_resume();
			break;
		case STATE_NOTIFIER_SUSPEND:
			cpufreq_limit_suspend();
			break;
		default:
			break;
 	}
 
 	return NOTIFY_OK;
 }
#endif

static ssize_t show_scaling_min_suspend_freq(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", scaling_min_suspend_freq);
}

static ssize_t show_scaling_max_suspend_freq(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", scaling_max_suspend_freq);
}

static ssize_t show_scaling_max_oncall_freq(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", scaling_max_oncall_freq);
}

static ssize_t show_scaling_max_gps_freq(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", scaling_max_gps_freq);
}

static ssize_t store_scaling_min_suspend_freq(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1) {
		return -EINVAL;
	}

	input = max(min(input, CPU_MAX_FREQ), CPU_MIN_FREQ);

	if (input != scaling_min_suspend_freq) {
		/* update only if valid value provided */
		scaling_min_suspend_freq = input;
	}

	return count;
}

static ssize_t store_scaling_max_suspend_freq(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1) {
		return -EINVAL;
	}

	input = max(min(input, CPU_MAX_FREQ), CPU_MIN_FREQ);

	if (input != scaling_max_suspend_freq) {
		/* update only if valid value provided */
		scaling_max_suspend_freq = input;
	}

	return count;
}

static ssize_t store_scaling_max_oncall_freq(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1) {
		return -EINVAL;
	}

	input = max(min(input, CPU_MAX_FREQ), CPU_MIN_FREQ);

	if (input != scaling_max_oncall_freq) {
		/* update only if valid value provided */
		scaling_max_oncall_freq = input;
	}

	return count;
}

static ssize_t store_scaling_max_gps_freq(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1) {
		return -EINVAL;
	}

	input = max(min(input, CPU_MAX_FREQ), CPU_MIN_FREQ);

	if (input != scaling_max_gps_freq) {
		/* update only if valid value provided */
		scaling_max_gps_freq = input;
	}

	return count;
}

static struct kobj_attribute scaling_min_suspend_freq_attr =
	__ATTR(scaling_min_suspend_freq, 0666, show_scaling_min_suspend_freq,
			store_scaling_min_suspend_freq);

static struct kobj_attribute scaling_max_suspend_freq_attr =
	__ATTR(scaling_max_suspend_freq, 0666, show_scaling_max_suspend_freq,
			store_scaling_max_suspend_freq);

static struct kobj_attribute scaling_max_oncall_freq_attr =
	__ATTR(scaling_max_oncall_freq, 0666, show_scaling_max_oncall_freq,
			store_scaling_max_oncall_freq);

static struct kobj_attribute scaling_max_gps_freq_attr =
	__ATTR(scaling_max_gps_freq, 0666, show_scaling_max_gps_freq,
			store_scaling_max_gps_freq);

static struct attribute *cpufreq_limit_manager_attrs[] = {
	&scaling_min_suspend_freq_attr.attr,
	&scaling_max_suspend_freq_attr.attr,
	&scaling_max_oncall_freq_attr.attr,
	&scaling_max_gps_freq_attr.attr,
	NULL,
};

static struct attribute_group cpufreq_limit_manager_attr_group = {
	.attrs = cpufreq_limit_manager_attrs,
	.name = "cpufreq_limit_manager",
};

static int __init cpufreq_limit_manager_init(void)
{
	int rc;

	rc = sysfs_create_group(kernel_kobj, &cpufreq_limit_manager_attr_group);

	if (rc) {
		pr_info("%s sysfs create failed!\n", __FUNCTION__);
		return -EFAULT;
	}

#ifdef CONFIG_STATE_NOTIFIER
	notif.notifier_call = state_notifier_callback;
	if (state_register_client(&notif))
		pr_err("Failed to register STATE notifier callback for cpufreq limit manager\n");
#endif

	return (rc);
}

MODULE_AUTHOR("Alucard_24 XDA");
MODULE_DESCRIPTION("CPUFREQ Limit manager");
MODULE_LICENSE("GPL");
late_initcall(cpufreq_limit_manager_init);
