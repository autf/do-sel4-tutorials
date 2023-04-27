
#include <assert.h>
#include <stdio.h>
#include <sel4/sel4.h>
#include <utils/util.h>
#include <autoconf.h>

#define FAULTER_BADGE_VALUE     (0xBEEF)
#define PROGNAME                "Handler: "
/* We signal on this notification to let the fauler know when we're ready to
 * receive its fault message.
 */
extern seL4_CPtr sequencing_ep_cap;

extern seL4_CPtr faulter_fault_ep_cap;

extern seL4_CPtr handler_cspace_root;
extern seL4_CPtr badged_faulter_fault_ep_cap;

extern seL4_CPtr faulter_tcb_cap;
extern seL4_CPtr faulter_vspace_root;
extern seL4_CPtr faulter_cspace_root;

int main(void)
{
    int error;
    seL4_Word tmp_badge;
    seL4_CPtr foreign_badged_faulter_empty_slot_cap;
    seL4_CPtr foreign_faulter_capfault_cap;
    seL4_MessageInfo_t seq_msginfo;

    printf(PROGNAME "Handler thread running!\n"
           PROGNAME "About to wait for empty slot from faulter.\n");


    seq_msginfo = seL4_Recv(sequencing_ep_cap, &tmp_badge);
    foreign_badged_faulter_empty_slot_cap = seL4_GetMR(0);
    printf(PROGNAME "Received init sequence msg: slot in faulter's cspace is "
           "%lu.\n",
           foreign_badged_faulter_empty_slot_cap);



    /* Mint the fault ep with a badge */

    puts("debug: src changed #7");
    printf("%p\n%p\n",
        foreign_badged_faulter_empty_slot_cap,
        faulter_fault_ep_cap);
    ZF_LOGF_IF(foreign_badged_faulter_empty_slot_cap != faulter_fault_ep_cap, "then why any one is ok?");

    error = seL4_CNode_Mint(
        handler_cspace_root,
        badged_faulter_fault_ep_cap,
        seL4_WordBits,
        handler_cspace_root,
        // /* EXERCISE: Which capability should be the source argument here? */-1,
        // foreign_badged_faulter_empty_slot_cap, // ok, why? :: looks like they are alias
        faulter_fault_ep_cap,
        // foreign_faulter_capfault_cap, //-> fail to mint
        seL4_WordBits,
        seL4_AllRights, FAULTER_BADGE_VALUE);

    ZF_LOGF_IF(error != 0, PROGNAME "Failed to mint ep cap with badge!");
    printf(PROGNAME "Successfully minted fault handling ep into local cspace.\n");


    /* This step is only necessary on the master kernel. On the MCS kernel it
     * can be skipped because we do not need to copy the badged fault EP into
     * the faulting thread's cspace on the MCS kernel.
     */

    error = seL4_CNode_Copy(
        faulter_cspace_root,
        // /* EXERCISE: What should the destination be for this cap copy? */-1,
        foreign_badged_faulter_empty_slot_cap,
        seL4_WordBits,
        handler_cspace_root,
        badged_faulter_fault_ep_cap,
        seL4_WordBits,
        seL4_AllRights);

    ZF_LOGF_IF(error != 0, PROGNAME "Failed to copy badged ep cap into faulter's cspace!");
    printf(PROGNAME "Successfully copied badged fault handling ep into "
           "faulter's cspace.\n"
           PROGNAME "(Only necessary on Master kernel.)\n");

    ZF_LOGF_IF(
        seL4_TCB_SetSpace(
            faulter_tcb_cap,
            foreign_badged_faulter_empty_slot_cap,
            faulter_cspace_root,
            0,
            faulter_vspace_root,
            0),
        PROGNAME "Failed SetSpace"
    );

    seL4_Reply(seL4_MessageInfo_new(0, 0, 0, 0));

    seL4_Recv(faulter_fault_ep_cap, &tmp_badge);
    ZF_LOGF_IF(tmp_badge != FAULTER_BADGE_VALUE, PROGNAME "Expect called via FAULTER_BADGE");
    foreign_faulter_capfault_cap = seL4_GetMR(seL4_CapFault_Addr);

    ZF_LOGF_IF(
        seL4_CNode_Copy(
            faulter_cspace_root,
            foreign_faulter_capfault_cap,
            seL4_WordBits,
            handler_cspace_root,
            sequencing_ep_cap,
            seL4_WordBits,
            seL4_AllRights),
        PROGNAME "Failed handling fault"
    );

    seL4_Reply(seL4_MessageInfo_new(0, 0, 0, 0));

    puts(PROGNAME "(DONE)");
    while(1);
    // return 0;
}
