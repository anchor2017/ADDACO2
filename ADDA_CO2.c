#include <reg52.h>
#include <intrins.h>
#define uint unsigned int
#define uchar unsigned char

void delayMs(uint z)
{
	uint x, y;
	for(x=z;x>0;x--)
		for(y=114;y>0;y--);
}

/*------------------------------------------LCD1602--------------------------------------------*/
//LCD1602���Ŷ���
sbit RS=P3^5;
sbit RW=P3^6;
sbit EN=P3^4;

//LCDҺ��æ״̬�жϣ�Ҳ���Ƕ�д���STA7λ��״̬
void LCD_readBusy()
{
	uchar busy;
	P0=0xff;	//��λ����
	RS=0;
	RW=1;
	do
	{
		EN=1;
		busy=P0;
		EN=0;
	}while(busy&0x80);
}

//LCDдָ��
void LCD_writeCmd(uchar cmd)
{
	LCD_readBusy();
	
	RS=0;
	RW=0;
	P0=cmd;
	EN=1;
	EN=0;
}

//LCDд����
void LCD_writeDat(uchar dat)
{
	LCD_readBusy();
	RS=1;
	RW=0;
	P0=dat;
	EN=1;
	EN=0;
}

//LCDҺ����ʼ��
void LCD_init()
{
	LCD_writeCmd(0x38);  //16*2��ʾ��5*7����8λ���ݽӿ�
	LCD_writeCmd(0x0c);  //����ʾ������ʾ��꣬��겻��ʾ
	LCD_writeCmd(0x06);  //��д���һ���ַ����ַָ���1�ҹ���1
	LCD_writeCmd(0x01);  //����ָ��
}


//LCDд��һ���ַ�
void LCD_writeChar(uchar x, uchar y, uchar dat)
{
	if(y)			 //y=1 �ڶ���
		x |= 0x40;	
					//y=0 ��һ��
	x |= 0x80;
	
	LCD_writeCmd(x);	  //0x80Ҳ����80H ����ָ�����õ�ָ��
	LCD_writeDat(dat);						
}

//LCDд�������ֽ� uint
void LCD_writeTwoChar(uchar x, uchar y, uint dat)
{
	if(y)			 //y=1 �ڶ���
		x |= 0x40;	 
					//y=0 ��һ��
	x |= 0x80;
	
	LCD_writeCmd(x);	  //0x80Ҳ����80H ����ָ�����õ�ָ��
	LCD_writeDat(dat);						
}


//LCDд��һ���ַ���    �Ա�������ʾx��(0-1)����ʾy��(0-39)����ʾ�ַ���
//����ָ���ַ����67H��0110 0111,��80H��1000 0000�����Կ����ð�λ��򵥵�ʵ����ӣ�ֱ����+��Ӧ��Ҳ����
void LCD_writeString(uchar x, uchar y, uchar *string)
{
	if(y)			 //y=1 �ڶ���
		x |= 0x40;	 

	x |= 0x80;
	
	LCD_writeCmd(x);	  //0x80Ҳ����80H ����ָ�����õ�ָ��
	
	/*LCD_writeDat(*string);*/

	while(*string != '\0')
	{
		LCD_writeDat(*string++);	//ָ��ָ����׵�ַ��Ҳ���ǵ�һ���ַ�������ָ���ַ���ƣ���֮����ַ�	
	}
}
/*------------------------------------------LCD1602----------------------------------------*/

/*-------------------------------------I2C��PCF8591-----------------------------------*/
//I2C���ų�ʼ��
#define PCF8591addr 0x90
sbit SDA=P2^0;  //��������
sbit SCL=P2^1;  //ʱ������

//5us��ʱ����
void delay_5us()
{
	_nop_();
}

//I2C��ʼ��
void I2C_init()	
{
	SDA = 1;
	_nop_();
	SCL = 1;
	_nop_();
}

//I2C��ʼ�ź�
void I2C_Start()  
{
	SCL = 1;
	_nop_();
	SDA = 1;
	delay_5us();
	SDA = 0;
	delay_5us();
}

//I2C��ֹ�ź�
void I2C_Stop()
{
	SDA = 0;
	_nop_();
	SCL = 1;
	delay_5us();
	SDA = 1;
	delay_5us();
}

//��������Ӧ��
void Master_ACK(bit i)		
{
	SCL = 0; // ����ʱ����������SDA���������ϵ����ݱ仯
	_nop_(); // �������ȶ�
	if (i)	 //���i = 1 ��ô������������ ��ʾ����Ӧ��
	{
		SDA = 0;
	}
	else	 
	{
		SDA = 1;	 //���ͷ�Ӧ��
	}
	_nop_();//�������ȶ�
	SCL = 1;//����ʱ������ �ôӻ���SDA���϶��� ������Ӧ���ź�
	delay_5us();
	SCL = 0;//����ʱ�����ߣ� ռ�����߼���ͨ��
	_nop_();
	SDA = 1;//�ͷ�SDA�������ߡ�
	_nop_();
}

//���ӻ�Ӧ��
bit Test_ACK()
{
	SCL = 1;
	delay_5us();
	if (SDA)	//�ӻ����ͷ�Ӧ���ź�
	{
		SCL = 0;
		_nop_();
		I2C_Stop();
		return(0);
	}
	else		//�ӻ�Ӧ��
	{
		SCL = 0;
		_nop_();
		return(1);
	}
}

//����һ���ֽ�
void I2C_send_byte(uchar byte)
{
	uchar i;
	for(i = 0 ; i < 8 ; i++) //�������ݸ�E2PROM���ȴ����λ��ʼ��
	{
		SCL = 0;
		_nop_();				
		if (byte & 0x80)  //1000 0000
		{ 
			_nop_();				   
			SDA = 1;	
		}
		else
		{				
			SDA = 0;
			_nop_();
		}
		SCL = 1;
		_nop_();
		byte <<= 1;	// 0101 0100B 
	}
	SCL = 0;
	_nop_();
	_nop_();
	SDA = 1;
}


//I2C ��һ�ֽ�
uchar I2C_read_byte()
{
	uchar dat,i;
	SCL = 0;
	_nop_();
	SDA = 1;   //������������
	_nop_();
	for(i = 0 ; i < 8 ; i++)
	{
		SCL = 1;
		_nop_();
		if (SDA)			    
		{
			 dat |= 0x01; //0000 0001 ��λ��ǰ��λͬdat, ���һλΪ1
		}
		else
		{
			dat &=  0xfe;	//1111 1110	 ��λ�룬ǰ��λͬdat, ���һλΪ0
		}
		_nop_();
		SCL = 0 ;
		_nop_();
		if(i < 7)
		{
			dat = dat << 1;	 //�ߵ�λ�������ӵ�λ���Ƶ���λ
		}
	}
	return(dat);
}

//I2C�����������ݣ�ÿһ��д�����ݶ���Ҫ���ӻ�Ӧ��
//ÿ��ʹ���������ֻ��д��һ�ֽڵ�����
bit I2C_transData(uchar ADDR, DAT)	//�ȷ���E2PROM��ַ���ٷ�������
{								 
	I2C_Start();					//������ʼ�ź�
	I2C_send_byte(PCF8591addr+0); //д��������ַ
	if (!Test_ACK())			   //���ӻ�Ӧ�𣬾����Ƿ������������
	{
		return(0);
	}
	
	I2C_send_byte(ADDR); 		   //����д���׵�ַ���ɱ�д�߾���
	if (!Test_ACK())			   //���ӻ�Ӧ��
	{
		return(0);
	}
	
	I2C_send_byte(DAT);			 //����һ�ֽڵ�����	 
	if (!Test_ACK())		   //���ӻ�Ӧ��
	{
		return(0);
	}
	I2C_Stop();				   //I2C��ֹ�ź�
	return(1);	
}

//I2C������������
//������ȡE2PROM���ݣ�ÿ��ֻ�ܴ�E2PROM���ض���ַ��ȡһ�ֽ����ݣ������ظ�����
uchar I2C_receData(uchar ADDR)	 
{
	uchar DAT;
	I2C_Start();					 //��ʼ�ź�
	I2C_send_byte(PCF8591addr+0);	 //����������ַ����������
	if (!Test_ACK())
	{
		return(0);
	}
	
	I2C_send_byte(ADDR);
	Master_ACK(0);		//����������Ӧ��

	//------------------
	I2C_Start();
	I2C_send_byte(PCF8591addr+1); 	//����������ַ����������
	if (!Test_ACK())
	{
		return(0);
	}

	DAT = I2C_read_byte();	//������ȡ��һ�ֽ�֮��
	Master_ACK(0);		   //����������Ӧ��

	I2C_Stop();
	return(DAT);	
}
/*-------------------------------------I2C��PCF8591-----------------------------------*/


void main()
{
	uchar i;
	uchar Pvalue;
	uchar Pstring[]={"CO2:"};
	uchar Pchar[]={0, 0, 0};
	LCD_init();
	I2C_init();
	LCD_writeString(2, 1, Pstring);
	while(1)
	{
		Pvalue=I2C_receData(0x43);
		//ģ������ʹ��λ��1�����ߵ������룬�Զ�������־λ��0�������������Ҫ��1��������ͨ��3	
		delayMs(2000);				

		Pchar[0]=Pvalue/100;
		Pchar[1]=Pvalue%100/10;
		Pchar[2]=Pvalue%10;

		LCD_writeString(6, 1, " ");
		for(i=1;i<3;i++)
		{	
			LCD_writeDat('0'+Pchar[i]);
		}
	}
}