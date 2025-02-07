#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "Bsp_otm8009a.h"

#define OTM8009A_GPIO_TYPE         GPIOC
#define OTM8009A_DATA_OUT(data)    GPIOB->ODR=data

//OTM8009A control pin define
#define OTM8009A_CS   9       //chip select pin           PC9
#define OTM8009A_RS   8       //reset pin                 PC8 
#define OTM8009A_RST  4       //                          PC4
#define OTM8009A_WR   7       //data write pin            PC7
#define OTM8009A_RD   6       //data read pin             PC6
#define OTM8009A_LED  10


//Set pin to low level
#define OTM8009A_CS_CLR       OTM8009A_GPIO_TYPE->BRR=1<<OTM8009A_CS
#define OTM8009A_WR_CLR       OTM8009A_GPIO_TYPE->BRR=1<<OTM8009A_WR
#define OTM8009A_RS_CLR       OTM8009A_GPIO_TYPE->BRR=1<<OTM8009A_RS
#define OTM8009A_RST_CLR      OTM8009A_GPIO_TYPE->BRR=1<<OTM8009A_RST

//Set pin to high level
#define OTM8009A_CS_SET      OTM8009A_GPIO_TYPE->BSRR=1<<OTM8009A_CS
#define OTM8009A_WR_SET      OTM8009A_GPIO_TYPE->BSRR=1<<OTM8009A_WR
#define OTM8009A_RS_SET      OTM8009A_GPIO_TYPE->BSRR=1<<OTM8009A_RS
#define OTM8009A_RST_SET     OTM8009A_GPIO_TYPE->BSRR=1<<OTM8009A_RST

#define OTM8009A_RGB_8BIT          0

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

#define GPIOC_ODR_Addr    (GPIOC_BASE+12)
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)
#define	LCD_LED PCout(OTM8009A_LED)



typedef struct lcd_dev
{										    
	uint16_t width;
	uint16_t height;
	uint16_t id;  
	uint8_t  dir;
	uint16_t	 wramcmd;
	uint16_t  rramcmd;
	uint16_t  setxcmd;
	uint16_t  setycmd;
}lcd_dev_type;

//function define
static void Otm8009a_Write_Data(uint16_t value);
static void Otm8009a_Write(uint16_t value);
static void Otm8009a_direction(uint8_t direction);
static void Otm8009a_Clear(uint16_t Color);
static void Otm8009a_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
static void Otm8009a_WriteRAM_Prepare(void);
static void Otm8009a_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);
static void Otm8009a_SetPixelPosition(uint16_t sx, uint16_t sy);

//variable
lcd_dev_type lcddev = {0};

//constant value
uint16_t const rainbow_color[7] = 
{
	LCD_RED,
	LCD_ORANGE,
	LCD_YELLOW,
	LCD_GREEN,
	LCD_BLUE,
	LCD_INDIGO,
	LCD_PURPLE
};

void Otm8009a_PinInit(void)
{
    //GPIO init
    GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_7| GPIO_Pin_8| GPIO_Pin_9 | GPIO_Pin_6 | GPIO_Pin_4; //GPIOC10,6,7,8,9,4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	GPIO_SetBits(GPIOC,GPIO_Pin_10|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_6|GPIO_Pin_4);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_All);
}

void Otm8009a_Init_Command(void)
{   
    
    Otm8009a_Write_Command(0xff00);
	Otm8009a_Write_Data(0x80);
	Otm8009a_Write_Command(0xff01);
	Otm8009a_Write_Data(0x09);
	Otm8009a_Write_Command(0xff02);
	Otm8009a_Write_Data(0x01);

	Otm8009a_Write_Command(0xff80);
	Otm8009a_Write_Data(0x80);
	Otm8009a_Write_Command(0xff81);
	Otm8009a_Write_Data(0x09);

	Otm8009a_Write_Command(0xff03);
	Otm8009a_Write_Data(0x01);
    
	//add ==========20131216============================//
	Otm8009a_Write_Command(0xf5b6); 
	Otm8009a_Write_Data(0x06); 
	Otm8009a_Write_Command(0xc480); 
	Otm8009a_Write_Data(0x30); 
	Otm8009a_Write_Command(0xc48a); 
	Otm8009a_Write_Data(0x40); 
	//===================================================//
	Otm8009a_Write_Command(0xc0a3);
	Otm8009a_Write_Data(0x1B);

	//Otm8009a_Write_Command(0xc0ba);  //No
	//Otm8009a_Write_Data(0x50);

	Otm8009a_Write_Command(0xc0ba); //--> (0xc0b4); // column inversion //  2013.12.16 modify
	Otm8009a_Write_Data(0x50); 

	Otm8009a_Write_Command(0xc181);
	Otm8009a_Write_Data(0x66);

	Otm8009a_Write_Command(0xc1a1);
	Otm8009a_Write_Data(0x0E);

	Otm8009a_Write_Command(0xc481);
	Otm8009a_Write_Data(0x83);

	Otm8009a_Write_Command(0xc582);
	Otm8009a_Write_Data(0x83);

	Otm8009a_Write_Command(0xc590);
	Otm8009a_Write_Data(0x96);

	Otm8009a_Write_Command(0xc591);
	Otm8009a_Write_Data(0x2B);

	Otm8009a_Write_Command(0xc592);
	Otm8009a_Write_Data(0x01);


	Otm8009a_Write_Command(0xc594);
	Otm8009a_Write_Data(0x33);

	Otm8009a_Write_Command(0xc595);
	Otm8009a_Write_Data(0x34);


	Otm8009a_Write_Command(0xc5b1);
	Otm8009a_Write_Data(0xa9);

	Otm8009a_Write_Command(0xce80);
	Otm8009a_Write_Data(0x86);
	Otm8009a_Write_Command(0xce81);
	Otm8009a_Write_Data(0x01); 
	Otm8009a_Write_Command(0xce82);
	Otm8009a_Write_Data(0x00); 

	Otm8009a_Write_Command(0xce83);
	Otm8009a_Write_Data(0x85); 
	Otm8009a_Write_Command(0xce84);
	Otm8009a_Write_Data(0x01); 
	Otm8009a_Write_Command(0xce85);
	Otm8009a_Write_Data(0x00);
	Otm8009a_Write_Command(0xce86);
	Otm8009a_Write_Data(0x00);
	Otm8009a_Write_Command(0xce87);
	Otm8009a_Write_Data(0x00);
	Otm8009a_Write_Command(0xce88);
	Otm8009a_Write_Data(0x00);
	Otm8009a_Write_Command(0xce89);
	Otm8009a_Write_Data(0x00);
	Otm8009a_Write_Command(0xce8A);
	Otm8009a_Write_Data(0x00);
	Otm8009a_Write_Command(0xce8B);
	Otm8009a_Write_Data(0x00);

	Otm8009a_Write_Command(0xcea0);// cea1[7:0] : clka1_width[3:0], clka1_shift[11:8]                         
	Otm8009a_Write_Data(0x18); 
	Otm8009a_Write_Command(0xcea1);// cea2[7:0] : clka1_shift[7:0]                                            
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcea2);// cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]   
	Otm8009a_Write_Data(0x03); 
	Otm8009a_Write_Command(0xcea3);// cea4[7:0] : clka1_switch[7:0]                                               
	Otm8009a_Write_Data(0x21); 
	Otm8009a_Write_Command(0xcea4);// cea5[7:0] : clka1_extend[7:0]                                           
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcea5);// cea6[7:0] : clka1_tchop[7:0]                                            
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcea6);// cea7[7:0] : clka1_tglue[7:0]                                            
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcea7);// cea8[7:0] : clka2_width[3:0], clka2_shift[11:8]                         
	Otm8009a_Write_Data(0x18); 
	Otm8009a_Write_Command(0xcea8);// cea9[7:0] : clka2_shift[7:0]                                            
	Otm8009a_Write_Data(0x03);
	Otm8009a_Write_Command(0xcea9);// ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]   
	Otm8009a_Write_Data(0x03); 
	Otm8009a_Write_Command(0xceaa);// ceab[7:0] : clka2_switch[7:0]                                                
	Otm8009a_Write_Data(0x22);
	Otm8009a_Write_Command(0xceab);// ceac[7:0] : clka2_extend                                                
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xceac);// cead[7:0] : clka2_tchop                                                 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcead);// ceae[7:0] : clka2_tglue 
	Otm8009a_Write_Data(0x00);    

	Otm8009a_Write_Command(0xceb0);// ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8]                          
	Otm8009a_Write_Data(0x18);
	Otm8009a_Write_Command(0xceb1);// ceb2[7:0] : clka3_shift[7:0]                                             
	Otm8009a_Write_Data(0x02); 
	Otm8009a_Write_Command(0xceb2);// ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]    
	Otm8009a_Write_Data(0x03); 
	Otm8009a_Write_Command(0xceb3);// ceb4[7:0] : clka3_switch[7:0]                                               
	Otm8009a_Write_Data(0x23); 
	Otm8009a_Write_Command(0xceb4);// ceb5[7:0] : clka3_extend[7:0]                                            
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xceb5);// ceb6[7:0] : clka3_tchop[7:0]                                             
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xceb6);// ceb7[7:0] : clka3_tglue[7:0]                                             
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xceb7);// ceb8[7:0] : clka4_width[3:0], clka2_shift[11:8]                          
	Otm8009a_Write_Data(0x18);
	Otm8009a_Write_Command(0xceb8);// ceb9[7:0] : clka4_shift[7:0]                                             
	Otm8009a_Write_Data(0x01); 
	Otm8009a_Write_Command(0xceb9);// ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]    
	Otm8009a_Write_Data(0x03); 
	Otm8009a_Write_Command(0xceba);// cebb[7:0] : clka4_switch[7:0]                                                
	Otm8009a_Write_Data(0x24); 
	Otm8009a_Write_Command(0xcebb);// cebc[7:0] : clka4_extend                                                 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcebc);// cebd[7:0] : clka4_tchop                                                  
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcebd);// cebe[7:0] : clka4_tglue                                                  
	Otm8009a_Write_Data(0x00); 


	Otm8009a_Write_Command(0xcfc0);// cfc1[7:0] : eclk_normal_width[7:0]   
	Otm8009a_Write_Data(0x01); 
	Otm8009a_Write_Command(0xcfc1);// cfc2[7:0] : eclk_partial_width[7:0]                                                                                  
	Otm8009a_Write_Data(0x01); 
	Otm8009a_Write_Command(0xcfc2);// cfc3[7:0] : all_normal_tchop[7:0]                                                                                    
	Otm8009a_Write_Data(0x20); 
	Otm8009a_Write_Command(0xcfc3);// cfc4[7:0] : all_partial_tchop[7:0]                                                                                   
	Otm8009a_Write_Data(0x20); 
	Otm8009a_Write_Command(0xcfc4);// cfc5[7:0] : eclk1_follow[3:0], eclk2_follow[3:0]                                                                     
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcfc5);// cfc6[7:0] : eclk3_follow[3:0], eclk4_follow[3:0]                                                                     
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcfc6);// cfc7[7:0] : 00, vstmask, vendmask, 00, dir1, dir2 (0=VGL, 1=VGH)                                                     
	Otm8009a_Write_Data(0x01); 
	Otm8009a_Write_Command(0xcfc7);// cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4  
	Otm8009a_Write_Data(0x00);    // GND OPT1 (00-->80  2011/10/28)
	Otm8009a_Write_Command(0xcfc8);// cfc9[7:0] : duty_block[3:0], DGPM[3:0]                                                                               
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcfc9);// cfca[7:0] : reg_goa_gnd_period[7:0]                                                                                  
	Otm8009a_Write_Data(0x00);    // Gate PCH (CLK base) (00-->0a  2011/10/28)

	Otm8009a_Write_Command(0xcfd0);// cfd1[7:0] : 0000000, reg_goa_frame_odd_high
	Otm8009a_Write_Data(0x00); 

	Otm8009a_Write_Command(0xcbc0);//cbc1[7:0] : enmode H-byte of sig1  (pwrof_0, pwrof_1, norm, pwron_4 )           
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbc1);//cbc2[7:0] : enmode H-byte of sig2  (pwrof_0, pwrof_1, norm, pwron_4 )          
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbc2);//cbc3[7:0] : enmode H-byte of sig3  (pwrof_0, pwrof_1, norm, pwron_4 )           
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbc3);//cbc4[7:0] : enmode H-byte of sig4  (pwrof_0, pwrof_1, norm, pwron_4 )        
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbc4);//cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 )             
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbc5);//cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 )           
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbc6);//cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 )           
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbc7);//cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 )           
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbc8);//cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 )           
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbc9);//cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 )        
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbca);//cbcb[7:0] : enmode H-byte of sig11 (pwrof_0, pwrof_1, norm, pwron_4 )        
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbcb);//cbcc[7:0] : enmode H-byte of sig12 (pwrof_0, pwrof_1, norm, pwron_4 )        
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbcc);//cbcd[7:0] : enmode H-byte of sig13 (pwrof_0, pwrof_1, norm, pwron_4 )        
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbcd);//cbce[7:0] : enmode H-byte of sig14 (pwrof_0, pwrof_1, norm, pwron_4 ) 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbce);//cbcf[7:0] : enmode H-byte of sig15 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 

	Otm8009a_Write_Command(0xcbd0);//cbd1[7:0] : enmode H-byte of sig16 (pwrof_0, pwrof_1, norm, pwron_4 )           
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbd1);//cbd2[7:0] : enmode H-byte of sig17 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbd2);//cbd3[7:0] : enmode H-byte of sig18 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbd3);//cbd4[7:0] : enmode H-byte of sig19 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbd4);//cbd5[7:0] : enmode H-byte of sig20 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbd5);//cbd6[7:0] : enmode H-byte of sig21 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbd6);//cbd7[7:0] : enmode H-byte of sig22 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbd7);//cbd8[7:0] : enmode H-byte of sig23 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbd8);//cbd9[7:0] : enmode H-byte of sig24 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbd9);//cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbda);//cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x04); 
	Otm8009a_Write_Command(0xcbdb);//cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbdc);//cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbdd);//cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbde);//cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 

	Otm8009a_Write_Command(0xcbe0);//cbe1[7:0] : enmode H-byte of sig31 (pwrof_0, pwrof_1, norm, pwron_4 )             
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe1);//cbe2[7:0] : enmode H-byte of sig32 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe2);//cbe3[7:0] : enmode H-byte of sig33 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe3);//cbe4[7:0] : enmode H-byte of sig34 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe4);//cbe5[7:0] : enmode H-byte of sig35 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe5);//cbe6[7:0] : enmode H-byte of sig36 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe6);//cbe7[7:0] : enmode H-byte of sig37 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe7);//cbe8[7:0] : enmode H-byte of sig38 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe8);//cbe9[7:0] : enmode H-byte of sig39 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcbe9);//cbea[7:0] : enmode H-byte of sig40 (pwrof_0, pwrof_1, norm, pwron_4 )
	Otm8009a_Write_Data(0x00);

	// cc8x   
	Otm8009a_Write_Command(0xcc80);//cc81[7:0] : reg setting for signal01 selection with u2d mode   
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc81);//cc82[7:0] : reg setting for signal02 selection with u2d mode 
	Otm8009a_Write_Data(0x26); 
	Otm8009a_Write_Command(0xcc82);//cc83[7:0] : reg setting for signal03 selection with u2d mode 
	Otm8009a_Write_Data(0x09); 
	Otm8009a_Write_Command(0xcc83);//cc84[7:0] : reg setting for signal04 selection with u2d mode 
	Otm8009a_Write_Data(0x0B); 
	Otm8009a_Write_Command(0xcc84);//cc85[7:0] : reg setting for signal05 selection with u2d mode 
	Otm8009a_Write_Data(0x01); 
	Otm8009a_Write_Command(0xcc85);//cc86[7:0] : reg setting for signal06 selection with u2d mode 
	Otm8009a_Write_Data(0x25); 
	Otm8009a_Write_Command(0xcc86);//cc87[7:0] : reg setting for signal07 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc87);//cc88[7:0] : reg setting for signal08 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc88);//cc89[7:0] : reg setting for signal09 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc89);//cc8a[7:0] : reg setting for signal10 selection with u2d mode 
	Otm8009a_Write_Data(0x00);  

	// cc9x   
	Otm8009a_Write_Command(0xcc90);//cc91[7:0] : reg setting for signal11 selection with u2d mode   
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc91);//cc92[7:0] : reg setting for signal12 selection with u2d mode
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc92);//cc93[7:0] : reg setting for signal13 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc93);//cc94[7:0] : reg setting for signal14 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc94);//cc95[7:0] : reg setting for signal15 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc95);//cc96[7:0] : reg setting for signal16 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc96);//cc97[7:0] : reg setting for signal17 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc97);//cc98[7:0] : reg setting for signal18 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc98);//cc99[7:0] : reg setting for signal19 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc99);//cc9a[7:0] : reg setting for signal20 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc9a);//cc9b[7:0] : reg setting for signal21 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcc9b);//cc9c[7:0] : reg setting for signal22 selection with u2d mode 
	Otm8009a_Write_Data(0x26); 
	Otm8009a_Write_Command(0xcc9c);//cc9d[7:0] : reg setting for signal23 selection with u2d mode 
	Otm8009a_Write_Data(0x0A); 
	Otm8009a_Write_Command(0xcc9d);//cc9e[7:0] : reg setting for signal24 selection with u2d mode 
	Otm8009a_Write_Data(0x0C); 
	Otm8009a_Write_Command(0xcc9e);//cc9f[7:0] : reg setting for signal25 selection with u2d mode 
	Otm8009a_Write_Data(0x02);  
	// ccax   
	Otm8009a_Write_Command(0xcca0);//cca1[7:0] : reg setting for signal26 selection with u2d mode   
	Otm8009a_Write_Data(0x25); 
	Otm8009a_Write_Command(0xcca1);//cca2[7:0] : reg setting for signal27 selection with u2d mode
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca2);//cca3[7:0] : reg setting for signal28 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca3);//cca4[7:0] : reg setting for signal29 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca4);//cca5[7:0] : reg setting for signal20 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca5);//cca6[7:0] : reg setting for signal31 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca6);//cca7[7:0] : reg setting for signal32 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca7);//cca8[7:0] : reg setting for signal33 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca8);//cca9[7:0] : reg setting for signal34 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 
	Otm8009a_Write_Command(0xcca9);//ccaa[7:0] : reg setting for signal35 selection with u2d mode 
	Otm8009a_Write_Data(0x00); 

    Otm8009a_Write_Command(0x3A00);//ccaa[7:0] : reg setting for signal35 selection with u2d mode 
	Otm8009a_Write_Data(0x55);//0x55
		
	Otm8009a_Write_Command(0x1100);
}


static void Otm8009a_Write_Data(uint16_t value)
{
    OTM8009A_RS_SET;
    #if OTM8009A_RGB_8BIT
    Otm8009a_Write(data<<8);
    #else
    Otm8009a_Write(value);
    #endif

}

void Otm8009a_Write_Command(uint16_t value)
{
    OTM8009A_RS_CLR;
    #if OTM8009A_RGB_8BIT
    Otm8009a_Write(data<<8);
    #else
    Otm8009a_Write(value);
    #endif
}



static void Otm8009a_Write(uint16_t value)
{
    OTM8009A_CS_CLR;  
	OTM8009A_DATA_OUT(value);
	OTM8009A_WR_CLR; 
	OTM8009A_WR_SET; 
	OTM8009A_CS_SET;
}

void Otm8009a_Reset_Clr(void)
{
    OTM8009A_RST_CLR;
}

void Otm8009a_Reset_Set(void)
{
	OTM8009A_RST_SET;
}

static void Otm8009a_direction(uint8_t direction)
{ 
	lcddev.setxcmd=0x2A00;
	lcddev.setycmd=0x2B00;
	lcddev.wramcmd=0x2C00;
	lcddev.rramcmd=0x2E00;
	switch(direction){		  
		case 0:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;		
			Otm8009a_WriteReg(0x3600,0x00);
		break;
		case 1:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			Otm8009a_WriteReg(0x3600,(1<<5)|(1<<6));
		break;
		case 2:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;	
			Otm8009a_WriteReg(0x3600,(1<<7)|(1<<6));
		break;
		case 3:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			Otm8009a_WriteReg(0x3600,(1<<7)|(1<<5));
		break;	
		default:break;
	}	
}	 

static void Otm8009a_Clear(uint16_t Color)
{
    unsigned int i;//,m;  
	Otm8009a_SetWindows(0,0,lcddev.width-1,lcddev.height-1);   
	for(i=0;i<lcddev.height*lcddev.width;i++)
	{
 //   for(m=0;m<lcddev.width;m++)
  //  {	
			Otm8009a_Write_Data(Color);
	//	}
	}
} 

static void Otm8009a_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{	
	Otm8009a_Write_Command(lcddev.setxcmd);Otm8009a_Write_Data(xStar>>8);  
	Otm8009a_Write_Command(lcddev.setxcmd+1);Otm8009a_Write_Data(xStar&0XFF);	  
	Otm8009a_Write_Command(lcddev.setxcmd+2);Otm8009a_Write_Data(xEnd>>8);   
	Otm8009a_Write_Command(lcddev.setxcmd+3);Otm8009a_Write_Data(xEnd&0XFF);   
	Otm8009a_Write_Command(lcddev.setycmd);Otm8009a_Write_Data(yStar>>8);   
	Otm8009a_Write_Command(lcddev.setycmd+1);Otm8009a_Write_Data(yStar&0XFF);  
	Otm8009a_Write_Command(lcddev.setycmd+2);Otm8009a_Write_Data(yEnd>>8);   
	Otm8009a_Write_Command(lcddev.setycmd+3);Otm8009a_Write_Data(yEnd&0XFF); 

	Otm8009a_WriteRAM_Prepare();	//��ʼд��GRAM			
}   

static void Otm8009a_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{	
	Otm8009a_Write_Command(LCD_Reg);  
	Otm8009a_Write_Data(LCD_RegValue);	    		 
}	

static void Otm8009a_WriteRAM_Prepare(void)
{
	Otm8009a_Write_Command(lcddev.wramcmd);
}

void Otm8009a_Set_Direction_and_Clear(uint8_t direction, uint16_t color)
{
    Otm8009a_direction(direction);
    Otm8009a_Set_Led();
    Otm8009a_Clear(color);
}

void Otm8009a_Set_Led(void)
{
    LCD_LED=1;
}

void Otm8009a_DrawPixel(uint16_t sx, uint16_t sy, uint16_t color)
{
	Otm8009a_SetPixelPosition(sx, sy);
	Otm8009a_Write_Data(color);
}

static void Otm8009a_SetPixelPosition(uint16_t sx, uint16_t sy)
{
	Otm8009a_SetWindows(sx, sy, sx, sy);	
}

void Otm8009a_RainbowTest(void)
{
	uint8_t num;
	uint16_t x_start;
	uint16_t x_end;
	uint16_t y_start = 0;
	uint16_t y_end = 0;

	for (num = 0; num < 7; num++)
	{
		x_start = 0;
		x_end = 480;
		y_start = num * 115;
		y_end = y_start + 115;
		if (y_end >= 800)
		{
			y_end = 799;
		}
		Otm8009a_FillBlock(x_start, x_end, y_start, y_end, rainbow_color[num]);
	} 
}

void Otm8009a_FillBlock(uint16_t xstart, uint16_t xend, uint16_t ystart, uint16_t yend, uint16_t color)
{
	uint16_t xlength = (xend - xstart) + 1;
	uint16_t ylength = (yend - ystart) + 1;
	
	uint16_t xpos = 0;
	uint16_t ypos = 0;
	
	Otm8009a_SetWindows(xstart, ystart, xend, yend);

	for (xpos = 0; xpos < xlength; xpos++)
	{
		for (ypos = 0; ypos < ylength; ypos++)
		{
			Otm8009a_Write_Data(color);
		}
	}
}


