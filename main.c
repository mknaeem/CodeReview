#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include <zephyr/drivers/gpio.h>

// Get the PWM device specification from the devicetree alias
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

// Define the minimum and maximum period for PWM signal
#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)

// Define the desired high and low brightness values
#define HIGH_BRIGHTNESS 0.999
#define MED_BRIGHTNESS 0.01
#define LOW_BRIGHTNESS 0.001

// Define the duration for high and low brightness states in milliseconds
#define DURATION_MS 5000 // 5 seconds

//*********** for flash led   *****************
/* 1000 msec = 1 sec */
#define FAST_BLINK_TIME_MS   100
#define SLOW_BLINK_TIME_MS   1000
#define BLINK_DURATION_MS    10000  // 30 seconds

/* The devicetree node identifier for the "led0" alias. */
#define LED1_NODE DT_ALIAS(led1)
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
//*********************************************


void main(void)
{
    uint32_t max_period;
    uint32_t period;
    int ret;

    // Print a message indicating the start of the program
    printk("PWM-based blinky\n");

    // Check if the PWM device is ready for use
    if (!device_is_ready(pwm_led0.dev)) {
        printk("Error: PWM device %s is not ready\n", pwm_led0.dev->name);
        return;
    }

    /*
     * In case the default MAX_PERIOD value cannot be set for
     * some PWM hardware, decrease its value until it can.
     *
     * Keep its value at least MIN_PERIOD * 4 to make sure
     * the sample changes frequency at least once.
     */

    // Calibrate the PWM period for the specified channel
    printk("Calibrating for channel %d...\n", pwm_led0.channel);
    max_period = MAX_PERIOD;
    while (pwm_set_dt(&pwm_led0, max_period, max_period / 2U)) {
        max_period /= 2U;
        if (max_period < (4U * MIN_PERIOD)) {
            printk("Error: PWM device does not support a period at least %lu\n", 4U * MIN_PERIOD);
            return;
        }
    }

    // Print the calibration results
    printk("Done calibrating; maximum/minimum periods %u/%lu usec\n", max_period, MIN_PERIOD);

    period = max_period;

    //*********** for flash led   *****************
    int ret1;
    int elapsed_time = 0;

    if (!device_is_ready(led.port)) {
        return;
    }

    ret1 = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret1 < 0) {
        return;
    }
    //*********************************************

    // Enter an infinite loop
    while (1) {
        // Set the LED brightness to high -- period is 9.99e9 ns and pulse width is 9.99e9
        ret = pwm_set_dt(&pwm_led0, period * HIGH_BRIGHTNESS, period * HIGH_BRIGHTNESS / 1U);
        if (ret) {
            printk("Error %d: failed to set pulse width\n", ret);
            return;
        }

        // Sleep for the specified duration for high brightness state
        k_sleep(K_MSEC(DURATION_MS));

        // Set the LED brightness to med -- period is e7 nsec and pulse width is e6 nsec
        ret = pwm_set_dt(&pwm_led0, period * MED_BRIGHTNESS, period * MED_BRIGHTNESS / 10U);
        if (ret) {
            printk("Error %d: failed to set pulse width\n", ret);
            return;
        }

        // Sleep for the specified duration for med brightness state
        k_sleep(K_MSEC(DURATION_MS));

		// Set the LED brightness to low -- period is e7 nsec and pulse width is e6 nsec
        ret = pwm_set_dt(&pwm_led0, period * LOW_BRIGHTNESS, period * LOW_BRIGHTNESS / 100U);
        if (ret) {
            printk("Error %d: failed to set pulse width\n", ret);
            return;
        }

        // Sleep for the specified duration for low brightness state
        k_sleep(K_MSEC(DURATION_MS));

        //*********** for flash led   *****************
        while (elapsed_time < BLINK_DURATION_MS) {
        ret1 = gpio_pin_toggle_dt(&led);
        if (ret1 < 0) {
            return;
        }

        if (elapsed_time < (BLINK_DURATION_MS / 2)) {
            k_msleep(FAST_BLINK_TIME_MS);
        } else {
            k_msleep(SLOW_BLINK_TIME_MS);
        }

        elapsed_time += (elapsed_time < (BLINK_DURATION_MS / 2)) ? FAST_BLINK_TIME_MS : SLOW_BLINK_TIME_MS;
    }
	elapsed_time = 0;
    }
    
}
