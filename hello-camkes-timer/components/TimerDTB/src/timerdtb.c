#include <stdio.h>

#include <timer_driver/driver.h>

#include <camkes.h>

#define NS_IN_SECOND 1000000000ull
#define DEFAULT_TIMER_ID 0

timer_drv_t timer_drv;

void tmr_irq_handle(ps_irq_t *irq) {
    int error;

    /* Part 2, TASK 4: call into the supplied driver to handle the interrupt. */
    /* hint: timer_handle_irq
     */
    timer_handle_irq(&timer_drv);

    /* Part 2, TASK 5: stop the timer. */
    /* hint: timer_stop
     */
    timer_stop(&timer_drv);

    /* signal the rpc interface. */
    error = sem_post();
    ZF_LOGF_IF(error != 0, "failed to post to semaphore");

    /* Part 2, TASK 6: acknowledge the interrupt */
    /* hint 1: use the function <to_interface_name>_irq_acknowledge()
     * hint 2: pass in the 'irq' variable to the function
     */
    tmr_irq_acknowledge(irq);
}

void hello__init() {
    /* Part 2, TASK 7: call into the supplied driver to get the timer handler */
    /* hint1: timer_init
     * hint2: The timer ID is supplied as a #define in this file
     * hint3: The register's name is follows the format of <interface name>_<register number>,
     * where "interface name" is the name of the
     * interface where you set the path of the DTB (foo.dtb = dtb({...}))
     * and "register number" is the index of the register block in the device
     * node in the devicetree blob
     */
    timer_init(&timer_drv, DEFAULT_TIMER_ID, tmr_0);

    /* Part 2, TASK 8: start the timer
     * hint: timer_start
     */
    timer_start(&timer_drv);
}

/* Part 2, TASK 9: implement the rpc function. */
/* hint 1: the name of the function to implement is a composition of an interface name and a function name:
 * i.e.: <interface>_<function>
 * hint 2: the interfaces available are defined by the component, e.g. in components/timer/timer.camkes
 * hint 3: the function name is defined by the interface definition, e.g. in interfaces/timer.camkes
 * hint 4: so the function would be: hello_sleep()
 * hint 5: the camkes 'int' type maps to 'int' in c
 * hint 6: invoke a function in the supplied driver to set a timeout
 * hint 7: look at https://github.com/sel4/camkes-tool/blob/master/docs/index.md#creating-an-application
 */
void hello_sleep(int sec) {
    int error = 0;

    /* Part 2, TASK 10: invoke a function in the supplied driver to set a timeout */
    /* hint1: timer_set_timeout
     * hint2: periodic should be set to false
     */
    timer_set_timeout(&timer_drv, 2 * NS_IN_SECOND, false);

    error = sem_wait();
    ZF_LOGF_IF(error != 0, "failed to wait on semaphore");
}
