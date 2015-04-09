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

#if defined(CONFIG_POWERSUSPEND)
#include <linux/powersuspend.h>
#elif defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#else
#include <linux/fb.h>
#endif  /* CONFIG_POWERSUSPEND || CONFIG_HAS_EARLYSUSPEND || FB NOTIFIER */

unsigned int scaling_min_suspend_freq = 378000;
unsigned int scaling_max_suspend_freq = CPU_MAX_FREQ;
unsigned int scaling_max_oncall_freq = CPU_MAX_ONCALL_FREQ;
unsigned int scaling_max_gps_freq = CPU_MAX_FREQ;

static bool suspended = false;
static bool gps_status = false;
static bool oncall_status = false;

#if !defined(CONFIG_POWERSUSPEND) && !defined(CONFIG_HAS_EARLYSUSPEND)
static struct notifier_block notif;
#endif

int update_cpufreq_limit(unsigned int limit_type, bool limit_status)
{
	unsigned int min_freq = 0;
	unsigned int max_freq = 0;
	bool immediately_update = true;
	unsigned int cpu;

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

	return 0;
}
EXPORT_SYMBOL_GPL(update_cpufreq_limit);

#if defined(CONFIG_POWERSUSPEND) || defined(CONFIG_HAS_EARLYSUSPEND)
#ifdef CONFIG_POWERSUSPEND
static void __ref cpufreq_limit_suspend(struct power_suspend *handler)
#else
static void __ref cpufreq_limit_suspend(struct early_suspend *handler)
#endif
#else
static void __cpufreq_limit_suspend(void)
#endif
{
 	/* SUSPEND */
	update_cpufreq_limit(0, true);
}

#if defined(CONFIG_POWERSUSPEND) || defined(CONFIG_HAS_EARLYSUSPEND)
#ifdef CONFIG_POWERSUSPEND
static void __cpuinit cpufreq_limit_late_resume(struct power_suspend *handler)
#else
static void __cpuinit cpufreq_limit_late_resume(
				struct early_suspend *handler)
#endif
#else
static void __cpufreq_limit_resume(void)
#endif
{
	/* RESUME */
	update_cpufreq_limit(0, false);
}

#if defined(CONFIG_POWERSUSPEND) || defined(CONFIG_HAS_EARLYSUSPEND)
#ifdef CONFIG_POWERSUSPEND
static struct power_suspend cpufreq_limit_suspend_driver = {
	.suspend = cpufreq_limit_suspend,
	.resume = cpufreq_limit_late_resume,
};
#else
static struct early_suspend cpufreq_limit_suspend_driver = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 10,
	.suspend = cpufreq_limit_suspend,
	.resume = cpufreq_limit_late_resume,
};
#endif
#else
static int prev_fb = FB_BLANK_UNBLANK;

static int fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;

	if (evdata && evdata->data && event == FB_EVENT_BLANK) {
		blank = evdata->data;
		switch (*blank) {
			case FB_BLANK_UNBLANK:
				if (prev_fb == FB_BLANK_POWERDOWN) {
					/* display on */
					__cpufreq_limit_resume();
					prev_fb = FB_BLANK_UNBLANK;
				}
				break;
			case FB_BLANK_POWERDOWN:
				if (prev_fb == FB_BLANK_UNBLANK) {
					/* display off */
					__cpufreq_limit_suspend();
					prev_fb = FB_BLANK_POWERDOWN;
				}
				break;
		}
	}

	return NOTIFY_OK;
}
#endif  /* CONFIG_POWERSUSPEND || CONFIG_HAS_EARLYSUSPEND || FB NOTIFIER */

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

#if defined(CONFIG_POWERSUSPEND)
	register_power_suspend(&cpufreq_limit_suspend_driver);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	register_early_suspend(&cpufreq_limit_suspend_driver);
#else
	notif.notifier_call = fb_notifier_callback;
	if (fb_register_client(&notif))
		pr_err("Failed to register FB notifier callback for cpufreq limit manager\n");
#endif  /* CONFIG_POWERSUSPEND || CONFIG_HAS_EARLYSUSPEND || FB NOTIFIER */

	return (rc);
}

MODULE_AUTHOR("Alucard_24 XDA");
MODULE_DESCRIPTION("CPUFREQ Limit manager");
MODULE_LICENSE("GPL");
late_initcall(cpufreq_limit_manager_init);
