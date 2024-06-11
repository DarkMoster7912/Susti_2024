

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "math.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "pid-pwm.h"
#include "device.h"
#include "isr.h"
#include "i2c-bus.h"
#include "VL6180X.h"
#include "VL53L1X.h"
#include "APDS.h"
#include "BNO.h"
#include "serial.h"
#include "hardware/i2c.h"
#include "pico/util/queue.h"


uint32_t timertime, oldtimertime, oldbluetime, maybeInTile=11;
double time,oldtime,tileTime,wholeTileTime;
int8_t sensnum=0, go=0, Go=0, finecorsa=0;
int provDist, blue=0, space=0, caselle=0;
int imp=0;
bool turning = false, center = false, black=false, victim=false, start=false, inTile=true,justEntered=false,send = false,justTurned=false,inTileToActCount=false, justFinecorsaed=false, slope=false, stop=false;
double absRelAngle;
int path[30]={'e'};//{0,0,0,0,0,1,2,2,2,1,0,0,3,3,2,2,2,2,2,'e'};
uint8_t currNTile[2],provCurrNTile[2], countForNTiles[2];
queue_t facingDirectionQueue,goQueue,GoQueue,victimQueue, startQueue, turningQueue, pathQueue, impQueue;
int delay, indeX;


void reset_imp(){
    queue_try_remove(&impQueue, &imp);
    imp=0;//imp=(distances[0]<600 || distances[5]<600)?((distances[0]<distances[5])?(distances[0]%TILE_LENGTH-50):(50-distances[5]%TILE_LENGTH))*1.25:0;
    queue_add_blocking(&impQueue, &imp);
    while (!queue_try_peek(&impQueue, &imp));
    //printf("corr:%d\n",((distances[0]<distances[5])?(distances[0]%TILE_LENGTH-50):(50-distances[5]%TILE_LENGTH))*1.25);
}

bool gyro(struct repeating_timer *t) {
    timertime = time_us_32();
    if(queue_try_remove(&goQueue, &go))printf("go: %d\n",go);
    if(queue_try_remove(&facingDirectionQueue, &facingDirection))printf("facingDirection: %d\n",facingDirection);
    queue_try_remove(&victimQueue, &victim);
    if(queue_try_remove(&startQueue, &start))printf("start: %d\n",start);
    if(!queue_try_remove(&pathQueue,&path[indeX++]))indeX=0;
    if(queue_try_peek(&impQueue, &imp))printf("imp: %d\n",imp);

    if(!start){
        uint16_t amb = readAmbientLight();
        if(usuwhite==(usuwhite+amb)/2)Serial_Send_String("x\n");
        usuwhite=(usuwhite+amb)/2;
        printf("white=%d\n",usuwhite);
        busy_wait_ms(100);
        //printf("avanti:%dmm,dietro:%dmm ",get_longdistance(0),get_longdistance(5));
        
        //0 caselle: davanti->117:12 dietro->0:102
        //1 casella: davanti->450:330 dietro->306:420
        
    }
    else {
        Go=go;
        //Go = (distances[0]<60)?0:Go;
        if(finecorsa){
            justFinecorsaed=true;
            Go=-1;
            facingDirection+=(finecorsa>0)?-1:1;
            finecorsa+=(finecorsa>0)?-1:1;
            //printf("facingDirecrtion:%d\n",facingDirection);
        }
        //else Go=1;
        if(black){
            if((tile!=3)&&((distances[5]%TILE_LENGTH)>50)&&((distances[5]%TILE_LENGTH)<70)){
                    Go = 0;

                    black=false;
            }
            else Go = -1;
            
        }
        //BNO055_GetVector(VECTOR_LINEARACCEL);

        BNO055_GetVector(VECTOR_EULER);
        //printf("%d,%d,%d,%f,%d\n",accVects[0],accVects[1],accVects[2],angles[0],(unsigned long)((timertime-oldtime)/1000));
        absRelAngle = ABS(relative180(facingDirection-angles[0]));
        justTurned=turning && !(absRelAngle>25);
        turning=(absRelAngle>25);
        //while(!queue_try_add(&turningQueue,&turning))queue_try_remove(&turningQueue,&tile);
        tile = APDS9960_GetTile();
        switch (tile)
        {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3: 
                Go=-1;
                black=true;
                break;
            default:
                break;
        }
        blue=(tile==2);
        if(tile!=3){
            vl53l1x_get_distance_nowait(0);
            vl53l1x_get_distance_nowait(5);
        }
        if(!victim && !finecorsa && !black){
            for(int i=1;i<5;i++)vl6180x_get_distance_nowait(i);
            if(distances[1]<170 && distances[2]<170 && distances[3]<170 && distances[4]<170 && ready[1] && ready[2] && ready[3] && ready[4])
                provDist = (-distances[3]+distances[1]-distances[4]+distances[2])/2-5;
            else if(distances[1]<170 && ready[1])provDist = (-70+distances[1]);
            else if(distances[2]<170 && ready[2])provDist = (-70+distances[2]);
            else if(distances[3]<170 && ready[3])provDist = (-distances[3]+60);
            else if(distances[4]<170 && ready[4])provDist = (-distances[4]+60);
            else provDist = 0;
            diffdistance = absRelAngle>30?0:provDist;
        }
        else diffdistance = 0;
         
        
        //Go = (victim)?0:Go;
        //inTile=(((distances[0]>9 && distances[0]<117)||(distances[0]>330 && distances[0]<450)||(distances[5]>0 && distances[5]<102)||(distances[5]>306 && distances[5]<420)||(absRelAngle>30)))||(space==tileImpulse);
        if((turning) || ((distances[0]<300 || distances[5]<300) && ((distances[0]<distances[5])?(distances[0]%TILE_LENGTH<100 && distances[0]%TILE_LENGTH>60):(distances[5]%TILE_LENGTH<80 && distances[5]%TILE_LENGTH>40)))){
            if(!inTile)maybeInTile++;
            else maybeInTile = 0;
        }
        else{
            if(inTile)maybeInTile++;
            else maybeInTile = 0;
        }
        if(maybeInTile>3){
            inTile=!inTile;
            maybeInTile = 0;    
        }
        //inTile= (turning) || ((distances[0]<distances[5])?(distances[0]%TILE_LENGTH<140 && distances[0]%TILE_LENGTH>20):(distances[5]%TILE_LENGTH<100 && distances[5]%TILE_LENGTH>20));
        if(justTurned)reset_imp();
        /*if(inTile){
            //if(((((currNTile[0]-1)==((distances[0]+100)/TILE_LENGTH)) && distances[0]<distances[5]) || ((currNTile[1]+1)==(distances[5]+150)/TILE_LENGTH && distances[5]<distances[0])) && !justTurned && !turning)justEntered=true;
            if(imp>(TILE_LENGTH_IMP/2)){
                justEntered=true;
                reset_imp();
                stop = true;
                oldtimertime=timertime;

            }
        }*/

        else if(imp>TILE_LENGTH_IMP){//((distances[0]>300 && distances[5]>300) && (imp>TILE_LENGTH_IMP)){
            justEntered=true;
            reset_imp();
            stop = true;
            oldtimertime=timertime;
        }
        if(stop)if((timertime-oldtimertime)>(blue?5000000L:100000L))stop=false;
        gpio_put(25,inTile);  
        //printf("current number of back tiles:%d, distance:%d, inTile:%d, ntiles??:%d\n",currNTile[1],distances[5],inTile,);
        //printf("inTile:%d, justentered:%d, disav:%dmm, disdi:%d, turning:%d\n",inTile,justEntered,distances[0],distances[5], turning);
        /*if(inTile&&!inTileToActCount){
            inTileToActCount=true;
            printf("caseella:%d\n",++caselle);
        }
        else if(!inTile&&inTileToActCount)inTileToActCount=false;*/
        /*if (inTile && !justTurned && (distances[0]<500 || distances[5]<500)){
            justTurned = true;
            //wholeTileTime=(timertime-oldtimertime)/1000.f;
            //oldtimertime = timertime;
            //wholeTileImpulse=space-oldImpulses;
            oldImpulses = space;
        }
        else if (!inTile && justTurned && (distances[0]<500 || distances[5]<500)){
            justTurned=false;
            //tileTime=(timertime-oldtimertime)/1000.f;
            //tileImpulse=space-oldImpulses;
        }
        else if(!inTile && distances[0]>500 && distances[5]>500){
            //printf("not in intile senza sensor\n");
            if((wholeTileImpulse-tileImpulse)<=(space-oldImpulses)){
                inTile=true;
                oldImpulses=space;
                //printf("viva il TEMPO\n");
            }
        }
        else if(inTile && distances[0]>500 && distances[5]>500){
            //printf("intile senza sensor\n");
            if((tileImpulse)<=(space-oldImpulses)){
                inTile=false;
                //printf("viva il TEMPO\n");
            }
        }
        //printf("whoTiletime:%f,tileTime%f,diff%f, time,%f\n",wholeTileTime,tileTime,(wholeTileTime-tileTime),(timertime-oldtimertime)/1000.f);
*/

        //if(Go==1 && (timertime-oldtimertime)%2000000L<100)printf("casella:%d\n",space++);
        //if(absRelAngle>45 && Go==1)oldtimertime = timertime;
        facingDirection%=360;
        while(facingDirection<0)facingDirection+=360;
        if(absRelAngle>15)Go=0;
        //printf("facingDirection:%d, diffdist:%d\n",facingDirection, diffdistance);
        setpwm(Go, calcpid());
    }
    return true;
}
bool laser(struct repeating_timer *t){
    timertime=time_us_64();
    printf("elapsed_time: %f, timertime:%f, oldtimertime:%f\n",(float)(timertime-oldtimertime)/1000,(float)timertime/1000,(float)oldtimertime/1000);
    oldtimertime = time_us_64();
    return true;
}
bool apds(struct repeating_timer *t){
	uint16_t* lghts = readLights();
    if(lghts!=0){
        for(int i;i<4;i++)lights[i] = lghts[i];
        printf("lights-> gen:%d red:%d green:%d blue%d\n", lights[0],lights[1],lights[2],lights[3]);
    }
    timertime=time_us_32();
    printf("elapsed_time: %d\n",(float)(timertime-oldtimertime));
    oldtimertime = time_us_32();
    return true;
}


void initIsr(void){
    sleep_ms(10);
    queue_init(&pathQueue,2,1);
    queue_init(&facingDirectionQueue,2,1);
    queue_init(&goQueue,1,1);
    queue_init(&GoQueue,1,1);
    queue_init(&impQueue,sizeof(uint),1);
    queue_init(&victimQueue,1,1);
    queue_init(&startQueue,1,1);
    queue_init(&turningQueue,1,1);
    for(int i=26;i<28;i++){
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
        gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    }
    //gpio_set_irq_enabled_with_callback(21 , GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    irq_set_priority(13,0);

    static repeating_timer_t timer;
    // Create the repeating timer
    //add_repeating_timer_us(500, gyro, NULL, &timer);
    //add_repeating_timer_us(100, laser, NULL, &timer);
    //add_repeating_timer_ms(100, messageForRasp, NULL, &timer);
    printf("fine\n");

}
// Interrupt handler function for the timer




void gpio_callback(uint gpio, uint32_t events){
    switch (gpio)
    {
        case 26:
            if(!finecorsa){
                finecorsa = -40;
                facingDirection+=(int)finecorsa*1;
            }
            break;
        case 27:
            if(!finecorsa){
                finecorsa = +40;
                facingDirection+=(int)finecorsa*1;
            }
            break;
    
    default:
        break;
    }
    
    
}

void encoder_callback(uint gpio, uint32_t events){
    //timertime = time_us_64() ;
    //if(!finecorsa)
    printf("imp: %d\n", space++);//, time:%d\n",space++, timertime-oldtimertime);
    //oldtimertime = timertime;
    
}




void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}

bool messageForRasp(){
    Serial_Send_Int((distances[1]<170 || distances[2]<170) | (distances[3]<170 || distances[4]<170)<<1);
    Serial_Send_Int((distances[0]<170) | (distances[5]<170)<<1);
    Serial_Send_Int(tile);
    Serial_Send_Int(inTile);
    Serial_Send_Int((int)round(angles[0]/90)%4); 
    //Serial_Send_Int(turning);
    /*printf("message for rasp turning: %d\n",turning);
    Serial_Send_String("d");                    //message[message.index('d'):]=angle -> round(angle/90)=actualFacingDirection
    Serial_Send_Float(angles[0]);*/
    Serial_Send_String("e");
    return true;
}
void messageForRaspSV(){
    Serial_Send_Int((distances[1]<170 || distances[2]<170) | (distances[3]<170 || distances[4]<170)<<1);
    Serial_Send_Int(inTile);
    Serial_Send_Int((int)round(angles[0]/90)%4); 
    Serial_Send_String("e");

}