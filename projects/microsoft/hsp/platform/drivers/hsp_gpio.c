// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_gpio.h"
#include "common/unused.h"


bool hsp_gpio_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param)
{
	const struct hsp_gpio *gpio = (const struct hsp_gpio*) handler;
	uint32_t mask;
	uint32_t edge;
	uint32_t level;
	size_t i;

	UNUSED (param);

	if (gpio == NULL) {
		return false;
	}

	mask = (1U << 0) | (1U << gpio->count);

	for (i = 0; i < gpio->count; i++) {
		/* If there is no handler registered, don't even bother looking at the interrupts. */
		if (gpio->irq_handler[i]) {
			/* Filter out interrupts for other GPIOs and ones that aren't enabled. */
			edge = gpio->regs->gpc_interrupts.gpio_intsts_edge & mask &
				gpio->regs->gpc_interrupts.gpio_inten_edge;
			level = gpio->regs->gpc_interrupts.gpio_intsts_level & mask &
				gpio->regs->gpc_interrupts.gpio_inten_level;

			if (edge || level) {
				/* If there is at least one enabled interrupt that has triggered, call the
				 * registered handler.  It is expected that the registered handler will clear the
				 * interrupt status, as appropriate. */
				gpio->irq_handler[i]->handle_interrupt (gpio->irq_handler[i], (uintptr_t) gpio);
			}
		}

		mask <<= 1;
	}

	return true;
}

bool hsp_gpio_handle_interrupt_no_irq_support (const struct hsp_interrupt_handler *handler,
	uintptr_t param)
{
	UNUSED (handler);
	UNUSED (param);

	return false;
}

/**
 * Initialize a driver for HSP GPIOs.
 *
 * @param gpio The GPIO driver instance to initialize.
 * @param gpio_regs Register interface for the HSP GPIOs.
 * @param irq_handler Array of interrupt handlers to use for GPIO interrupts.  This array will not
 * be initialized in any way by the driver, so the caller can choose to pre-populate it with data,
 * if desired.
 * @param gpio_count The total number of GPIOs supported by the HSP.  The irq_handler array must be
 * at least this size.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int hsp_gpio_init (struct hsp_gpio *gpio, struct Creg_regs_gpc_regs *gpio_regs,
	const struct hsp_interrupt_handler **irq_handler, size_t gpio_count)
{
	if ((gpio == NULL) || (gpio_regs == NULL) || (irq_handler == NULL)) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	memset (gpio, 0, sizeof (struct hsp_gpio));

	gpio->base.handle_interrupt = hsp_gpio_handle_interrupt;

	gpio->regs = gpio_regs;
	gpio->irq_handler = irq_handler;
	gpio->count = gpio_count;

	return 0;
}

/**
 * Initialize a driver for HSP GPIOs that allows for reading and writing GPIO values but does not
 * support GPIO driven interrupts.
 *
 * @param gpio The GPIO driver instance to initialize.
 * @param gpio_regs Register interface for the HSP GPIOs.
 * @param gpio_count The total number of GPIOs supported by the HSP.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int hsp_gpio_init_no_irq_support (struct hsp_gpio *gpio, struct Creg_regs_gpc_regs *gpio_regs,
	size_t gpio_count)
{
	if ((gpio == NULL) || (gpio_regs == NULL)) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	memset (gpio, 0, sizeof (struct hsp_gpio));

	gpio->base.handle_interrupt = hsp_gpio_handle_interrupt_no_irq_support;

	gpio->regs = gpio_regs;
	gpio->count = gpio_count;

	return 0;
}

/**
 * Release an HSP GPIO driver.
 *
 * @param gpio The GPIO driver to release.
 */
void hsp_gpio_release (const struct hsp_gpio *gpio)
{
	UNUSED (gpio);
}

/**
 * Configure a GPIO for use.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to configure.
 * @param is_output Flag indicating if the GPIO should be configured as an output.  True for an
 * output GPIO and false for an input.
 * @param pull The type of internal resistor pull that should be used with the GPIO.
 * @param init_value For output GPIOs, the initial value that should be used.  This parameter will
 * be ignored for input GPIOs.
 *
 * @return 0 if the GPIO was configured successfully or an error code.
 */
int hsp_gpio_configure (const struct hsp_gpio *gpio, uint8_t gpio_num, bool is_output,
	enum hsp_gpio_internal_pull pull, bool init_value)
{
	uint32_t mask;

	if (gpio == NULL) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	mask = (1U << gpio_num);

	/* Configure the internal PU/PD. */
	switch (pull) {
		case HSP_GPIO_INTERNAL_PULL_NONE:
			gpio->regs->gpc_config.gpio_pullup &= ~mask;
			gpio->regs->gpc_config.gpio_pulldown &= ~mask;
			break;

		case HSP_GPIO_INTERNAL_PULL_UP:
			gpio->regs->gpc_config.gpio_pulldown &= ~mask;
			gpio->regs->gpc_config.gpio_pullup |= mask;
			break;

		case HSP_GPIO_INTERNAL_PULL_DOWN:
			gpio->regs->gpc_config.gpio_pullup &= ~mask;
			gpio->regs->gpc_config.gpio_pulldown |= mask;
			break;

		default:
			return HSP_GPIO_INVALID_ARGUMENT;
	}

	/* Configure the GPIO direction. */
	if (is_output) {
		if (init_value) {
			gpio->regs->gpc_config.gpio_out |= mask;
		}
		else {
			gpio->regs->gpc_config.gpio_out &= ~mask;
		}

		gpio->regs->gpc_config.gpio_outen |= mask;
	}
	else {
		gpio->regs->gpc_config.gpio_outen &= ~mask;
	}

	return 0;
}

/**
 * Configure multiple HSP GPIOs for use.
 *
 * @param gpio The GPIO driver for the GPIOs to configure.
 * @param config List of configuration to apply.  This does not need to contain configuration data
 * for all supported GPIOs, nor does it need to be in GPIO number order.
 * @param count The number of configuration entries in the list.
 * @param is_por Flag indicating if the POR configuration should be applied.
 *
 * @return 0 if all GPIOs were successfully configured or an error code.  On failure, some GPIOs
 * may be configured while others may not be.
 */
int hsp_gpio_configure_multiple (const struct hsp_gpio *gpio, const struct hsp_gpio_config *config,
	size_t count, bool is_por)
{
	size_t i;
	int status;

	if ((gpio == NULL) || ((config == NULL) && (count != 0))) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	for (i = 0; i < count; i++) {
		status = hsp_gpio_configure (gpio, config[i].gpio_num, config[i].is_output, config[i].pull,
			(is_por) ? config[i].init_value_por : config[i].init_value);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Read the current value of a GPIO.  For input GPIOs, this is the value detected on the pin.  For
 * output GPIOs, this is the value being driven.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to query.
 *
 * @return The GPIO value, which will be either 0 or 1, or an error code.
 */
int hsp_gpio_read (const struct hsp_gpio *gpio, uint8_t gpio_num)
{
	uint32_t mask;
	int value;

	if (gpio == NULL) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	mask = (1U << gpio_num);

	if (gpio->regs->gpc_config.gpio_outen & mask) {
		value = !!(gpio->regs->gpc_config.gpio_out & mask);
	}
	else {
		value = !!(gpio->regs->gpc_config.gpio_in & mask);
	}

	return value;
}

/**
 * Write a value to the GPIO.  This is only valid for output GPIOs.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to update.
 * @param value The value to write to the GPIO.
 *
 * @return 0 if the GPIO was updated successfully or an error code.
 */
int hsp_gpio_write (const struct hsp_gpio *gpio, uint8_t gpio_num, bool value)
{
	uint32_t mask;

	if (gpio == NULL) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	mask = (1U << gpio_num);

	if (!(gpio->regs->gpc_config.gpio_outen & mask)) {
		return HSP_GPIO_NOT_OUTPUT;
	}

	if (value) {
		gpio->regs->gpc_config.gpio_out |= mask;
	}
	else {
		gpio->regs->gpc_config.gpio_out &= ~mask;
	}

	return 0;
}

/**
 * Toggle the value of a GPIO.  This is only valid for output GPIOs.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to update.
 *
 * @return 0 if the GPIO was updated successfully or an error code.
 */
int hsp_gpio_toggle (const struct hsp_gpio *gpio, uint8_t gpio_num)
{
	uint32_t mask;

	if (gpio == NULL) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	mask = (1U << gpio_num);

	if (!(gpio->regs->gpc_config.gpio_outen & mask)) {
		return HSP_GPIO_NOT_OUTPUT;
	}

	gpio->regs->gpc_config.gpio_out ^= mask;

	return 0;
}

/**
 * Enable interrupts for a single GPIO.  This only controls interrupt enablement at the GPIO level.
 * Top-level HSP interrupts for all GPIOs would need to managed separately.
 *
 * Existing interrupts for those being enabled will be cleared.  Only new events will trigger
 * interrupts.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier to the GPIO to configure.
 * @param irq_type Bitmask of enum hsp_gpio_irq values for the interrupts to enable.  Providing a
 * value of 0 will register the handler without enabling any interrupts.
 * @param handler The handler that should be called when an interrupt has occurred.  The handler
 * will only get called in response to interrupts that have been enabled.  If the provided handler
 * is the same as the existing handler, the memory containing the handler will not be written again,
 * enabling read-only access to the GPIO handlers for static configurations.  The context provided
 * to the interrupt handler will be the GPIO driver instance.
 *
 * @return 0 if the GPIO interrupts were enabled successfully or an error code.
 */
int hsp_gpio_enable_interrupt (const struct hsp_gpio *gpio, uint8_t gpio_num, uint8_t irq_type,
	const struct hsp_interrupt_handler *handler)
{
	uint32_t mask;
	uint32_t mask_upper;

	if ((gpio == NULL) || (handler == NULL)) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	if (gpio->irq_handler == NULL) {
		return HSP_GPIO_IRQS_UNSUPPORTED;
	}

	mask = (1U << gpio_num);
	mask_upper = mask << gpio->count;

	if (gpio->irq_handler[gpio_num] != handler) {
		gpio->irq_handler[gpio_num] = handler;
	}

	/* Clear out any existing status for the interrupts being enabled. */
	hsp_gpio_clear_irq_status (gpio, gpio_num, irq_type);

	/* NOTE: GPIO interrupts are not as clean as other pieces, since there are not separate
	 * registers for each interrupt type.  Both edge types are packed into one register and both
	 * level types into another.  How these are packed seem to vary based on the number of GPIOs
	 * supported.  It is unclear what would happen if there were more than 16 GPIOs.  If that
	 * scenario is ever encountered, this driver may need to be updated. */

	if (irq_type & HSP_GPIO_IRQ_RISING_EDGE) {
		gpio->regs->gpc_interrupts.gpio_inten_edge |= mask;
	}

	if (irq_type & HSP_GPIO_IRQ_FALLING_EDGE) {
		gpio->regs->gpc_interrupts.gpio_inten_edge |= mask_upper;
	}

	if (irq_type & HSP_GPIO_IRQ_LEVEL_HIGH) {
		gpio->regs->gpc_interrupts.gpio_inten_level |= mask;
	}

	if (irq_type & HSP_GPIO_IRQ_LEVEL_LOW) {
		gpio->regs->gpc_interrupts.gpio_inten_level |= mask_upper;
	}

	return 0;
}

/**
 * Disable specific interrupts for a single GPIO.  The registered handler is never removed, even if
 * all interrupts are disabled, but it would no longer be called in response to disabled interrupts.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to configure.
 * @param irq_type Bitmask of enum hsp_gpio_irq values for the interrupts to disable.
 *
 * @return 0 if the GPIO interrupts were disabled successfully or an error code.
 */
int hsp_gpio_disable_interrupt (const struct hsp_gpio *gpio, uint8_t gpio_num, uint8_t irq_type)
{
	uint32_t mask;
	uint32_t mask_upper;

	if (gpio == NULL) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	/* Allow interrupt disablement even if the driver was not configured to support IRQs.  Disabling
	 * IRQs doesn't trigger any workflows that would be broken. */

	mask = (1U << gpio_num);
	mask_upper = ~(mask << gpio->count);
	mask = ~mask;

	if (irq_type & HSP_GPIO_IRQ_RISING_EDGE) {
		gpio->regs->gpc_interrupts.gpio_inten_edge &= mask;
	}

	if (irq_type & HSP_GPIO_IRQ_FALLING_EDGE) {
		gpio->regs->gpc_interrupts.gpio_inten_edge &= mask_upper;
	}

	if (irq_type & HSP_GPIO_IRQ_LEVEL_HIGH) {
		gpio->regs->gpc_interrupts.gpio_inten_level &= mask;
	}

	if (irq_type & HSP_GPIO_IRQ_LEVEL_LOW) {
		gpio->regs->gpc_interrupts.gpio_inten_level &= mask_upper;
	}

	return 0;
}

/**
 * Disable all interrupts for a single GPIO.  The registered handler is not removed, but it will no
 * longer be called.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to configure.
 *
 * @return 0 if the GPIO interrupts were disabled successfully or an error code.
 */
int hsp_gpio_disable_all_interrupts (const struct hsp_gpio *gpio, uint8_t gpio_num)
{
	return hsp_gpio_disable_interrupt (gpio, gpio_num, HSP_GPIO_IRQ_RISING_EDGE |
		HSP_GPIO_IRQ_FALLING_EDGE | HSP_GPIO_IRQ_LEVEL_HIGH | HSP_GPIO_IRQ_LEVEL_LOW);
}

/**
 * Read the current interrupt status for a specific GPIO.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to query.
 * @param only_enabled Flag indicating only enabled interrupt types should be returned.
 * @param irq_type Output for the bitmask of enum hsp_gpio_irq values representing the active
 * interrupts.
 *
 * @return 0 if the interrupt status was read successfully or an error code.
 */
static int hsp_gpio_read_irq_status (const struct hsp_gpio *gpio, uint8_t gpio_num,
	bool only_enabled, uint8_t *irq_type)
{
	uint32_t mask;
	uint32_t mask_upper;

	if ((gpio == NULL) || (irq_type == NULL)) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	/* Allow interrupt status operations even if the driver was not configured to support IRQs.
	 * Reading and clearing status only trigger register reads/writes and can be supported in all
	 * cases. */

	mask = (1U << gpio_num);
	mask_upper = mask << gpio->count;
	*irq_type = 0;

	if (gpio->regs->gpc_interrupts.gpio_intsts_edge & mask) {
		if (!only_enabled || (gpio->regs->gpc_interrupts.gpio_inten_edge & mask)) {
			*irq_type |= HSP_GPIO_IRQ_RISING_EDGE;
		}
	}

	if (gpio->regs->gpc_interrupts.gpio_intsts_edge & mask_upper) {
		if (!only_enabled || (gpio->regs->gpc_interrupts.gpio_inten_edge & mask_upper)) {
			*irq_type |= HSP_GPIO_IRQ_FALLING_EDGE;
		}
	}

	if (gpio->regs->gpc_interrupts.gpio_intsts_level & mask) {
		if (!only_enabled || (gpio->regs->gpc_interrupts.gpio_inten_level & mask)) {
			*irq_type |= HSP_GPIO_IRQ_LEVEL_HIGH;
		}
	}

	if (gpio->regs->gpc_interrupts.gpio_intsts_level & mask_upper) {
		if (!only_enabled || (gpio->regs->gpc_interrupts.gpio_inten_level & mask_upper)) {
			*irq_type |= HSP_GPIO_IRQ_LEVEL_LOW;
		}
	}

	return 0;
}

/**
 * Read the current interrupt status for a specific GPIO.  This will only return the current status
 * of interrupts that are enabled.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to query.
 * @param irq_type Output for the bitmask of enum hsp_gpio_irq values representing the active
 * interrupts.
 *
 * @return 0 if the interrupt status was read successfully or an error code.
 */
int hsp_gpio_get_irq_status (const struct hsp_gpio *gpio, uint8_t gpio_num,	uint8_t *irq_type)
{
	return hsp_gpio_read_irq_status (gpio, gpio_num, true, irq_type);
}

/**
 * Read the current interrupt status for a specific GPIO.  This will return the current status of
 * all interrupts, including those that may not be enabled.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to query.
 * @param irq_type Output for the bitmask of enum hsp_gpio_irq values representing the active
 * interrupts.
 *
 * @return 0 if the interrupt status was read successfully or an error code.
 */
int hsp_gpio_get_raw_irq_status (const struct hsp_gpio *gpio, uint8_t gpio_num, uint8_t *irq_type)
{
	return hsp_gpio_read_irq_status (gpio, gpio_num, false, irq_type);
}

/**
 * Clear the interrupt status for a specific GPIO.  If the interrupt condition is still present, the
 * interrupt status will not clear and another interrupt may get triggered.
 *
 * @param gpio The GPIO driver for the desired GPIO.
 * @param gpio_num Identifier for the GPIO to update.
 * @param irq_type Bitmask of enum hsp_gpio_irq values for the interrupt status bits to clear.
 *
 * @return 0 if the interrupt status was clear successfully or an error code.
 */
int hsp_gpio_clear_irq_status (const struct hsp_gpio *gpio, uint8_t gpio_num, uint8_t irq_type)
{
	uint32_t mask;
	uint32_t mask_upper;

	if (gpio == NULL) {
		return HSP_GPIO_INVALID_ARGUMENT;
	}

	if (gpio_num >= gpio->count) {
		return HSP_GPIO_UNKNOWN_GPIO;
	}

	mask = (1U << gpio_num);
	mask_upper = mask << gpio->count;

	if (irq_type & HSP_GPIO_IRQ_RISING_EDGE) {
		gpio->regs->gpc_interrupts.gpio_intsts_edge = mask;
	}

	if (irq_type & HSP_GPIO_IRQ_FALLING_EDGE) {
		gpio->regs->gpc_interrupts.gpio_intsts_edge = mask_upper;
	}

	if (irq_type & HSP_GPIO_IRQ_LEVEL_HIGH) {
		gpio->regs->gpc_interrupts.gpio_intsts_level = mask;
	}

	if (irq_type & HSP_GPIO_IRQ_LEVEL_LOW) {
		gpio->regs->gpc_interrupts.gpio_intsts_level = mask_upper;
	}

	return 0;
}
