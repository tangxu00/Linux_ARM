    #include <stdio.h>    
    #include <sys/types.h>    
    #include <sys/stat.h>    
    #include <fcntl.h>    
      
      
    #define LED_OFF     0  
    #define LED_ON      1  
    #define LED_1_ON    2  
    #define LED_1_OFF   3  
    #define LED_2_ON    4  
    #define LED_2_OFF   5  
    #define LED_3_ON    6  
    #define LED_3_OFF   7  
    #define LED_4_ON    8  
    #define LED_4_OFF   9  
      
    int main (void)    
    {    
        int  i=0;  
        int  fd;    
        char buf[10]={  
                LED_ON ,   LED_OFF ,  
                LED_1_ON,  LED_1_OFF,  
                LED_2_ON,  LED_2_OFF,  
                LED_3_ON,  LED_3_OFF,  
                LED_4_ON,  LED_4_OFF,             
             };    
      
        fd = open("/dev/led",O_RDWR);    
        if (fd < 0)    
        {    
            printf ("Open /dev/led file error\n");    
            return -1;    
        }       
      
        while(i<10)    
        {    
            write(fd,&buf[i],4);    
            sleep(1);    
            i++;  
        }    
        close (fd);    
        return 0;    
        
    }    
