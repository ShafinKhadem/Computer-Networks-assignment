#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int piggybacking, crc_steps;
char generator_polynomial[10];

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: SLIGHTLY MODIFIED
 FROM VERSION 1.1 of J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
       are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
       or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
       (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 1 /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

/* a "pkt" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct pkt
{
    char data[4];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct frm
{
    int seqnum;
    int acknum;
    int checksum;
    char payload[4];
    int type;
};

/********* FUNCTION PROTOTYPES. DEFINED IN THE LATER PART******************/
void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer1(int AorB, struct frm packet);
void tolayer3(int AorB, char datasent[4]);

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

bool a_next_frame_to_send,a_frame_expected,a_pendingAck,a_outstandingAck;
struct frm a_next_f,a_last_received;
const int a_timer_inc=25;

bool b_next_frame_to_send,b_frame_expected,b_pendingAck,b_outstandingAck;
struct frm b_next_f,b_last_received;
const int b_timer_inc=25;

char *bin(int x, char *tmp) {
    int i;
    for (i = 0; i < 32; ++i) tmp[31-i] = "01"[x&1], x >>= 1;
    tmp[32] = '\0';
    return tmp;
}

int calc_checksum(struct frm f) {
    char inp[1000] = "", tmp[33];
    strcat(inp, bin(f.type, tmp));
    strcat(inp, bin(f.seqnum, tmp));
    strcat(inp, bin(f.acknum, tmp));
    int ret = 0, i, payloadint = 0;
    for (i = 0; i < 4; ++i) payloadint = payloadint<<8|f.payload[i];
    strcat(inp, bin(payloadint, tmp));
    printf("\ninput to crc: %s\n", inp);
    int len = strlen(inp), gpl = strlen(generator_polynomial);
    bool cur[gpl];
    for (i = 0; i < gpl; ++i) {
        cur[i] = inp[i] == '1';
    }
    for (i = 0; i < gpl; ++i) printf("%d", cur[i]);
    printf("\n");
    for (i = 0; i < len; ++i) {
        bool msb = cur[0];
        int j;
        for (j = 0; j <= gpl-2; ++j) cur[j] = cur[j+1]^(msb && generator_polynomial[j+1] == '1');
        cur[gpl-1] = inp[i+gpl] == '1';
        if (crc_steps && i < len-1) {
            int k;
            for (k = 0; k < gpl; ++k) printf("%d", cur[k]);
            printf("\n");
        }
    }
    printf("crc: ");
    for (i = 0; i < gpl-1; ++i) {
        printf("%d", cur[i]);
        if (cur[i]) ret |= 1<<(gpl-2-i);
    }
    printf("\n");
    return ret;
}

/* called from layer 3, passed the data to be sent to other side */
void A_output(struct pkt packet)
{
    if (a_pendingAck) {
        printf("\nA's previous frame hasn't been acknowledged yet, hence discarding: %s\n", packet.data);
        return;
    }
    struct frm f;
    int i;
    for (i = 0; i < 4; ++i) f.payload[i] = packet.data[i];
    f.seqnum = a_next_frame_to_send;
    f.acknum = !a_frame_expected;
    f.type = piggybacking && a_outstandingAck ? 2 : 0;
    f.checksum = calc_checksum(f);
    a_next_f = f;
    printf("\nA is sending %d %d %s, checksum=%d\n", f.seqnum, f.acknum, f.payload, f.checksum);
    starttimer(0, a_timer_inc);
    a_pendingAck = 1;
    tolayer1(0, f);
}

void B_output(struct pkt packet)
{
    if (b_pendingAck) {
        printf("\nB's previous frame hasn't been acknowledged yet, hence discarding: %s\n", packet.data);
        return;
    }
    struct frm f;
    int i;
    for (i = 0; i < 4; ++i) f.payload[i] = packet.data[i];
    f.seqnum = b_next_frame_to_send;
    f.acknum = !b_frame_expected;
    f.type = piggybacking && b_outstandingAck ? 2 : 0;
    f.checksum = calc_checksum(f);
    b_next_f = f;
    printf("\nB is sending %d %d %s, checksum=%d\n", f.seqnum, f.acknum, f.payload, f.checksum);
    starttimer(1, b_timer_inc);
    b_pendingAck = 1;
    tolayer1(1, f);
}

/* called from layer 1, when a frame arrives for layer 2 */
void A_input(struct frm f)
{
    if (f.checksum != calc_checksum(f)) {
        printf("\nA received wrong checksum hence discarding, received checksum: %d, expected checksum: %d\n", f.checksum, calc_checksum(f));
        return;
    }
    if (f.type != 0 && f.acknum == a_next_frame_to_send) {
        stoptimer(0);
        printf("\nA is accepting acknowledgement %d %d %s, checksum=%d\n", f.acknum, f.seqnum, f.payload, f.checksum);
        a_pendingAck = 0;
        a_next_frame_to_send ^= 1;
        if (a_next_f.type == 2) a_outstandingAck = 0;
    }
    if (f.type != 1) {
        printf("\nA outstandingAck: %d, received: seqnum %d checksum %d, expected: seqnum %d checksum %d\n", a_outstandingAck, f.seqnum, f.checksum, a_frame_expected, a_last_received.checksum);
        if (f.seqnum == a_frame_expected) {
            tolayer3(0, f.payload);
            a_outstandingAck = 1;
            a_frame_expected ^= 1;
            a_last_received = f;
        }
        if (!piggybacking || (a_outstandingAck && f.checksum == a_last_received.checksum)) {
            f.acknum = !a_frame_expected;
            f.type = 1;
            f.checksum = calc_checksum(f);
            printf("\nA is acknowledging: %d\n", !a_frame_expected);
            tolayer1(0, f);
        }
    }
}


/* called when A's timer goes off */
void A_timerinterrupt(void)
{
    printf("\nResending packet from A\n");
    starttimer(0, a_timer_inc);
    tolayer1(0, a_next_f);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void)
{
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct frm f)
{
    if (f.checksum != calc_checksum(f)) {
        printf("\nB received wrong checksum hence discarding, received checksum: %d, expected checksum: %d\n", f.checksum, calc_checksum(f));
        return;
    }
    if (f.type != 0 && f.acknum == b_next_frame_to_send) {
        stoptimer(1);
        printf("\nB is accepting acknowledgement %d %d %s, checksum=%d\n", f.acknum, f.seqnum, f.payload, f.checksum);
        b_pendingAck = 0;
        b_next_frame_to_send ^= 1;
        if (b_next_f.type == 2) b_outstandingAck = 0;
    }
    if (f.type != 1) {
        printf("\nB outstandingAck: %d, received: seqnum %d checksum %d, expected: seqnum %d checksum %d\n", b_outstandingAck, f.seqnum, f.checksum, b_frame_expected, b_last_received.checksum);
        if (f.seqnum == b_frame_expected) {
            tolayer3(1, f.payload);
            b_outstandingAck = 1;
            b_frame_expected ^= 1;
            b_last_received = f;
        }
        if (!piggybacking || (b_outstandingAck && f.checksum == b_last_received.checksum)) {
            f.acknum = !b_frame_expected;
            f.type = 1;
            f.checksum = calc_checksum(f);
            printf("\nB is acknowledging: %d\n", !b_frame_expected);
            tolayer1(1, f);
        }
    }
}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
    printf("\nResending packet from B\n");
    starttimer(1, b_timer_inc);
    tolayer1(1, b_next_f);
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void)
{

}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
    - emulates the tranmission and delivery (possibly with bit-level corruption
        and packet loss) of packets across the layer 3/4 interface
    - handles the starting/stopping of a timer, and generates timer
        interrupts (resulting in calling students timer handler).
    - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
    float evtime;       /* event time */
    int evtype;         /* event type code */
    int eventity;       /* entity where event occurs */
    struct frm *frmptr; /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL; /* the event list */

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER3 1
#define FROM_LAYER1 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

int TRACE = 1;     /* for my debugging */
int nsim = 0;      /* number of messages from 5 to 4 so far */
int nsimmax = 0;   /* number of pkts to generate, then stop */
float time = 0.000;
float lossprob;    /* probability that a packet is dropped  */
float corruptprob; /* probability that one bit is packet is flipped */
float lambda;      /* arrival rate of messages from layer 5 */
int ntolayer1;     /* number sent into layer 3 */
int nlost;         /* number lost in media */
int ncorrupt;      /* number corrupted by media*/

void init();
void generate_next_arrival(void);
void insertevent(struct event *p);

int main()
{
    struct event *eventptr;
    struct pkt pkt2give;
    struct frm frm2give;

    int i, j;
    char c;

    printf("crc_steps: ");
    scanf("%d", &crc_steps);
    printf("piggybacking: ");
    scanf("%d", &piggybacking);
    printf("generator polynomial: ");
    scanf("%s", generator_polynomial);
    printf("%s\n", generator_polynomial);
    init();
    A_init();
    B_init();

    while (1)
    {
        eventptr = evlist; /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next; /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2)
        {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer3 ");
            else
                printf(", fromlayer1 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER3)
        {
            if (nsim < nsimmax)
            {
                if (nsim + 1 < nsimmax)
                    generate_next_arrival(); /* set up future arrival */
                /* fill in pkt to give with string of same letter */
                j = nsim % 26;
                for (i = 0; i < 4; i++)
                    pkt2give.data[i] = 97 + j;
                pkt2give.data[19] = 0;
                if (TRACE > 2)
                {
                    printf("          MAINLOOP: data given to student: ");
                    for (i = 0; i < 4; i++)
                        printf("%c", pkt2give.data[i]);
                    printf("\n");
                }
                nsim++;
                if (eventptr->eventity == A)
                    A_output(pkt2give);
                else
                    B_output(pkt2give);
            }
        }
        else if (eventptr->evtype == FROM_LAYER1)
        {
            frm2give.seqnum = eventptr->frmptr->seqnum;
            frm2give.acknum = eventptr->frmptr->acknum;
            frm2give.checksum = eventptr->frmptr->checksum;
            frm2give.type = eventptr->frmptr->type;
            for (i = 0; i < 4; i++)
                frm2give.payload[i] = eventptr->frmptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(frm2give); /* appropriate entity */
            else
                B_input(frm2give);
            free(eventptr->frmptr); /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT)
        {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else
        {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(
        " Simulator terminated at time %f\n after sending %d pkts from layer3\n",
        time, nsim);
}

void init() /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("Enter the number of messages to simulate: ");
    scanf("%d",&nsimmax);
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf("%f",&lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f",&corruptprob);
    printf("Enter average time between messages from sender's layer3 [ > 0.0]:");
    scanf("%f",&lambda);
    printf("Enter TRACE:");
    scanf("%d",&TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(1);
    }

    ntolayer1 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand(void)
{
    double mmm = RAND_MAX;
    float x;                 /* individual students may need to change mmm */
    x = rand() / mmm;        /* x should be uniform in [0,1] */
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(void)
{
    double x, log(), ceil();
    struct event *evptr;
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
    /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER3;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}

void insertevent(struct event *p)
{
    struct event *q, *qold;

    if (TRACE > 2)
    {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist;      /* q points to header of list in which p struct inserted */
    if (q == NULL)   /* list is empty */
    {
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL)   /* end of list */
        {
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist)     /* front of list */
        {
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else     /* middle of list */
        {
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist(void)
{
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next)
    {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype,
               q->eventity);
    }
    printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB /* A or B is trying to stop timer */)
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;          /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist)   /* front of list - there must be event after */
            {
                q->next->prev = NULL;
                evlist = q->next;
            }
            else     /* middle of list */
            {
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB /* A or B is trying to start timer */, float increment)
{
    struct event *q;
    struct event *evptr;

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}

/************************** TOLAYER1 ***************/
void tolayer1(int AorB, struct frm frame)
{
    struct frm *myfrmptr;
    struct event *evptr, *q;
    float lastime, x;
    int i;

    ntolayer1++;

    /* simulate losses: */
    if (jimsrand() < lossprob)
    {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER1: frame being lost\n");
        return;
    }

    /* make a copy of the frame student just gave me since he/she may decide */
    /* to do something with the frame after we return back to him/her */
    myfrmptr = (struct frm *)malloc(sizeof(struct frm));
    myfrmptr->seqnum = frame.seqnum;
    myfrmptr->acknum = frame.acknum;
    myfrmptr->checksum = frame.checksum;
    myfrmptr->type = frame.type;
    for (i = 0; i < 4; i++)
        myfrmptr->payload[i] = frame.payload[i];
    if (TRACE > 2)
    {
        printf("          TOLAYER1: seq: %d, ack %d, check: %d type: %d", myfrmptr->seqnum,
               myfrmptr->acknum, myfrmptr->checksum, myfrmptr->type);
        for (i = 0; i < 4; i++)
            printf("%c", myfrmptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of frame at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER1;      /* frame will pop out from layer1 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->frmptr = myfrmptr;         /* save ptr to my copy of frame */
    /* finally, compute the arrival time of frame at the other end.
       medium can not reorder, so make sure frame arrives between 1 and 10
       time units after the latest arrival time of fs
       currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER1 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob)
    {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            myfrmptr->payload[0] = 'Z'; /* corrupt payload */
        else if (x < .875)
            myfrmptr->seqnum = 999999;
        else
            myfrmptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER1: frame being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER1: scheduling arrival on other side\n");
    insertevent(evptr);
}

void tolayer3(int AorB, char datasent[4])
{
    int i;
    if (TRACE > 2)
    {
        printf("          TOLAYER3: data received: ");
        for (i = 0; i < 4; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }
}
