#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/tm4c123gh6pm.h"


#define RW 0x20
#define RS 0x40
#define E  0x80

volatile unsigned long delay;
int secim = 0;
int para = 0;
char paraChr[11];
char hizmetChr[11];

int besTL = 0, onTL = 0, yirmiTL = 0, elliTL = 0, yuzTL = 0;
int gBesTL = 0, gOnTL = 0, gYirmiTL = 0, gElliTL = 0, gYuzTL = 0;
int gKopukleme = 0, gYikama = 0, gKurulama = 0, gCilalama = 0;
int paraSikisma = 0;

void portAyarlari(void){
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;       // activate clock for Port A
    delay = SYSCTL_RCGC2_R;                     // allow time for clock to start
    GPIO_PORTA_AMSEL_R &= ~0b11100000;          // disable analog on PA
    GPIO_PORTA_PCTL_R &= ~0xFFF00000;           // PCTL GPIO on PA
    GPIO_PORTA_DIR_R |= 0b11100000;             // PA4-PA0 in, PA5-PA7 out
    GPIO_PORTA_AFSEL_R &= ~0b11100000;          // disable alt funct on PA
    GPIO_PORTA_DEN_R |= 0b11100000;             // enable digital I/O on PA7-0
    GPIO_PORTA_DR8R_R |= 0b11100000;            // activate 8mA output on PA

    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;       // activate clock for Port B
    delay = SYSCTL_RCGC2_R;                     // allow time for clock to start
    GPIO_PORTB_AMSEL_R &= ~0b11111111;          // disable analog on PB
    GPIO_PORTB_PCTL_R &= ~0xFFFFFFFF;           // PCTL GPIO on PB
    GPIO_PORTB_DIR_R |= 0b11111111;             // PB0-PB7 out
    GPIO_PORTB_AFSEL_R &= ~0b11111111;          // disable alt funct on PB
    GPIO_PORTB_DEN_R |= 0b11111111;             // enable digital I/O on PB7-0
    GPIO_PORTB_DR8R_R |= 0b11111111;            // activate 8mA output on PB

    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;       // activate clock for Port E
    delay = SYSCTL_RCGC2_R;                     // allow time for clock to start
    GPIO_PORTE_DIR_R |= 0x00;                   // PE7-0 in
    GPIO_PORTE_DEN_R |= 0b00111111;             // enable digital I/O on PE7-0
    GPIO_PORTE_PUR_R = 0b00111111;              // enable pull-up on PE0-5

    SYSCTL_RCGCGPIO_R |= 0x00000020; // 1) activate clock for Port F
    delay = SYSCTL_RCGCGPIO_R; // allow time for clock to start
    GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) unlock GPIO Port F
    GPIO_PORTF_CR_R = 0x1F; // allow changes to PF4-0
    GPIO_PORTF_AMSEL_R = 0x00; // 3) disable analog on PF
    GPIO_PORTF_PCTL_R = 0x00000000; // 4) PCTL GPIO on PF4-0
    GPIO_PORTF_DIR_R = 0x0E; // 5) PF4,PF0 in, PF3-1 out
    GPIO_PORTF_AFSEL_R = 0x00; // 6) disable alt funct on PF7-0
    GPIO_PORTF_PUR_R = 0x11; // enable pull-up on PF0 and PF4
    GPIO_PORTF_DEN_R = 0x1F; // 7) enable digital I/O on PF4-0
}

void LCDComment(unsigned char LCD_Comment){
    GPIO_PORTA_DATA_R &= ~(RS+RW+E);            // Tüm pinleri sýfýrla
    GPIO_PORTB_DATA_R = LCD_Comment;            // Komutu yazdýr
    GPIO_PORTA_DATA_R |= E;                     // E'yi aç
    GPIO_PORTA_DATA_R &= ~(RS+RW);              // RS ve RW kapat
    for (delay = 0 ; delay < 1; delay++);       // 1us bekle
    GPIO_PORTA_DATA_R &= ~(RS+RW+E);            // RS, RW ve E kapat
    for (delay = 0 ; delay < 1000; delay++);    // 1ms bekle
}

void LCDData(unsigned char LCD_Data){
    GPIO_PORTB_DATA_R = LCD_Data;               // Write Data
    GPIO_PORTA_DATA_R |= RS+E;                  // RS ve E aç
    GPIO_PORTA_DATA_R &= ~RW;                   // RW kapat
    for (delay = 0 ; delay < 23 ; delay++);     // 230ns bekle
    GPIO_PORTA_DATA_R &= ~(RS+RW+E);            // RS, RW ve E kapat
    for (delay = 0 ; delay < 1000; delay++);    // 1ms bekle
}

void LCDActivate(){
    portAyarlari();                      // Portlarý aktifleþtir
    for (delay = 0 ; delay < 15000; delay++);   // 15ms bekle
    LCDComment(0x38);                          // 0b00111000 -> PortB
    for (delay = 0 ; delay < 5000; delay++);    // 5ms bekle
    LCDComment(0x38);                          // 0b00111000 -> PortB
    for (delay = 0 ; delay < 150; delay++);     // 150us bekle
    LCDComment(0x0C);                          // 0b00001010 -> PortB
    LCDComment(0x01);                          // Ekraný Temizle
    LCDComment(0x06);                          // 0b00000110 -> PortB
    for (delay = 0 ; delay < 50000; delay++);   // 50ms bekle
}

void LCDWrite(unsigned int line,unsigned int digit, unsigned char *str){
    unsigned int lineCode = line==1 ?0x80:0xC0;
    LCDComment(lineCode + digit);
    while(*str != 0){ LCDData(*str++); }
}

int basiliButon(){
    return    (GPIO_PORTE_DATA_R & 0b00000001) == 0 ? 1
            : (GPIO_PORTE_DATA_R & 0b00000010) == 0 ? 2
            : (GPIO_PORTE_DATA_R & 0b00000100) == 0 ? 3
            : (GPIO_PORTE_DATA_R & 0b00001000) == 0 ? 4
            : (GPIO_PORTE_DATA_R & 0b00010000) == 0 ? 5
            : (GPIO_PORTE_DATA_R & 0b00100000) == 0 ? 6
            : 0 ;
}


struct hizmet
{
     int id;
    unsigned char ad[50];
     int adet;
    int ucret;

};
struct hizmet hizmet[4];


void paraBozdurma() {

    int kopuklemeFiyat = hizmet[0].ucret;
    int yikamaFiyat = hizmet[1].ucret;
    int kurulamaFiyat = hizmet[2].ucret;
    int cilalamaFiyat = hizmet[3].ucret;

    // hizmetlerin fiyatýný yüklenen paradan çýkar.
    para -= kopuklemeFiyat*gKopukleme + yikamaFiyat*gYikama + kurulamaFiyat*gKurulama + cilalamaFiyat*gCilalama;

    int paraAdet = 0;

    int kasaToplamPara = yuzTL*100+elliTL*50+yirmiTL*20+onTL*10+besTL*5;

    int besTLYok = 0;

    if(para%10==5 && besTL == 0) {
        besTLYok = 1;
    }

    int hizmetYok = 0;

    if((gKopukleme>0 && hizmet[0].adet<gKopukleme) || (gYikama>0 && hizmet[1].adet<gYikama) || (gKurulama>0 && hizmet[2].adet<gKurulama) || (gCilalama>0 && hizmet[3].adet<gCilalama) ) {
        hizmetYok = 1;
    }


    if(para<=kasaToplamPara && besTLYok == 0 && hizmetYok == 0) {

        // hizmet adetini azalt.
        hizmet[0].adet -= gKopukleme;
        hizmet[1].adet -= gYikama;
        hizmet[2].adet -= gKurulama;
        hizmet[3].adet -= gCilalama;

        // yüklenen paralarý para stoklarýna ekle
        besTL += gBesTL;
        onTL += gOnTL;
        yirmiTL += gYirmiTL;
        elliTL += gElliTL;
        yuzTL += gYuzTL;

    while(para>=5) {


                 if(yuzTL>0 && para>=100) {
                      paraAdet = para/100;
                     if(paraAdet>=1){
                        para-=100;
                        yuzTL-=1;
                     }
                 }
                 else if(elliTL>0 && para>=50) {
                       paraAdet = para/50;
                     if(paraAdet>=1){
                        para-=50;
                        elliTL-=1;
                     }
                 }
                 else if(yirmiTL>0 && para>=20) {
                       paraAdet = para/20;
                     if(paraAdet>=1){
                        para-=20;
                        yirmiTL-=1;
                     }
                 }
                 else if(onTL>0 && para>=10) {
                        paraAdet = para/10;
                     if(paraAdet>=1){
                        para-=10;
                        onTL-=1;
                     }
                 }
                 else if(besTL>0 && para>=5) {
                       paraAdet = para/5;
                     if(paraAdet>=1){
                        para-=5;
                        besTL-=1;
                     }
                 }


    }

    }

    else {

        if(para>kasaToplamPara) {

            //kasada yetersiz para var.
            char eksik[9] = {'E','K','S','I','K', 'P','A','R','A'};
            LCDWrite(1,0,eksik);
        }
        else if(besTLYok == 1) {
            //kasada bozuk yok.
            char bestlYOK[6] = {'5','T','L','Y','O', 'K'};
            LCDWrite(1,0,bestlYOK);
        }
        else if(hizmetYok == 1) {
            //hizmet yok.
            char hizmetYOK[9] = {'H','I','Z','M','E', 'T','Y','O','K'};
            LCDWrite(1,0,hizmetYOK);
        }

    }

    FILE *myfile2 = fopen("C:\\Users\\merto\\workspace_v10\\project1\\hizmetler.txt", "w");
        if (myfile2 != NULL) {


            fprintf(myfile2, "%d %d %d %d %d\n", besTL, onTL, yirmiTL, elliTL, yuzTL);
            int i;
            for (i = 0; i < 4; i++) {
                fprintf(myfile2, "%d %s %d %d\n", hizmet[i].id, hizmet[i].ad, hizmet[i].adet, hizmet[i].ucret);
            }

        }

        fclose(myfile2);

}



void main(void)
{

    FILE *myfile = fopen("C:\\Users\\merto\\workspace_v10\\project1\\hizmetler.txt", "r");
    if (myfile != NULL) {


        fscanf(myfile, "%d %d %d %d %d", &besTL, &onTL, &yirmiTL, &elliTL, &yuzTL);
        int i;
        for (i = 0; i < 4; i++) {
            fscanf(myfile, "%d %s %d %d\n", &hizmet[i].id, hizmet[i].ad, &hizmet[i].adet, &hizmet[i].ucret);
        }

    }

    fclose(myfile);

    LCDActivate();

    char Str[6] = {'P','A','R','A','-', '\0'};


    LCDWrite(1,8,Str);

    int hizmetSec = 0;

    while(1){
        if(hizmetSec == 0) {


            if(secim == 0){
                secim = basiliButon(
                );
            }else{
                if(secim!=0){
                         if(secim==1){
                             para+=5;
                             gBesTL++;
                             for (delay = 0 ; delay < 2000000 ; delay++);
                         }
                    else if(secim==2){
                        para+=10;
                        gOnTL++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                    }
                    else if(secim==3){
                        para+=20;
                        gYirmiTL++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                    }
                    else if(secim==4){
                        para+=50;
                        gElliTL++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                    }
                    else if(secim==5){
                        para+=100;
                        gYuzTL++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                    }
                    else if(secim==6){
                           hizmetSec = 1;
                           for (delay = 0 ; delay < 2000000 ; delay++);

                    }
                }
                secim=0;

            sprintf(paraChr,"%d", para);
            LCDWrite(1,0,paraChr);
            }
        }
        if(hizmetSec == 1) {
            if(secim == 0){
                secim = basiliButon(
                );
            }else{
                if(secim!=0){
                    if(secim==1){

                        gKopukleme = 0;
                        gYikama = 0;
                        gKurulama = 0;
                        gCilalama = 0;


                        for (delay = 0 ; delay < 2000000 ; delay++);
                        sprintf(hizmetChr,"%d", gKopukleme);
                        LCDWrite(2,0,hizmetChr);
                        sprintf(hizmetChr,"%d", gYikama);
                        LCDWrite(2,2,hizmetChr);
                        sprintf(hizmetChr,"%d", gKurulama);
                        LCDWrite(2,4,hizmetChr);
                        sprintf(hizmetChr,"%d", gCilalama);
                        LCDWrite(2,6,hizmetChr);

                        if(paraSikisma == 1) {

                            gBesTL = 0;
                            gOnTL = 0;
                            gYirmiTL = 0;
                            gElliTL = 0;
                            gYuzTL = 0;
                            para = 0;


                            hizmetSec = 0;
                            LCDActivate();
                            LCDWrite(1,8,Str);
                            paraSikisma = 0;
                            GPIO_PORTF_DATA_R &= ~(0b00010);
                            GPIO_PORTF_DATA_R &= ~(0b01000);
                        }



                    }

                    else if(secim==2){
                        gKopukleme++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                        sprintf(hizmetChr,"%d", gKopukleme);
                        LCDWrite(2,0,hizmetChr);
                    }
                    else if(secim==3){
                        gYikama++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                        sprintf(hizmetChr,"%d", gYikama);
                        LCDWrite(2,2,hizmetChr);
                    }
                    else if(secim==4){
                        gKurulama++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                        sprintf(hizmetChr,"%d", gKurulama);
                        LCDWrite(2,4,hizmetChr);
                    }
                    else if(secim==5){
                        gCilalama++;
                        for (delay = 0 ; delay < 2000000 ; delay++);
                        sprintf(hizmetChr,"%d", gCilalama);
                        LCDWrite(2,6,hizmetChr);
                    }
                    else if(secim==6){

                        int r = rand() % 8;
                                    if(r == 2){
                                        //para sýkýþtýðý için ledi yak
                                        // parayý geri ver.
                                        //0b0010 kýrmýzý
                                        //0b0100 mavi
                                        //0b1000 yesil
                                        //GPIO_PORTF_DATA_R &= ~(0b01000);
                                        GPIO_PORTF_DATA_R |= 0b00010;

                                        paraSikisma = 1;



                                    } else {
                                        hizmetSec = -1;
                                        // parayý bozdur
                                        //GPIO_PORTF_DATA_R &= ~(0b00010);
                                        GPIO_PORTF_DATA_R |= 0b01000;

                                        paraBozdurma();





                                    }

                                    for (delay = 0 ; delay < 2000000 ; delay++);

                    }
                }
                secim=0;

            }

        }


            for (delay = 0 ; delay < 1000 ; delay++);

        }



}


