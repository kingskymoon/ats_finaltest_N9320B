#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include <string.h>
#include <stdlib.h>
#include "demo.h"
//////////////////////////
/*********************************
**
**  VISA-IO-无格式
**
*********************************/
//#define USE_TDS2012  //
#define USE_DSO5012  //

#include <visa.h>
#include <stdio.h>
//#include "visaudpcom.h"

//#define RESOURCE "TCPIP0::192.168.10.108::inst0::INSTR"
#define RESOURCE "TCPIP0::192.168.15.2::inst0::INSTR"
ViStatus   status;	  //定义状态变量
ViSession  defRM;	  //定义系统资源会话变量
ViSession  instr;	  //定义仪器会话变量


int cnt; //定义计数器变量
char inbuf[12000];  //定义输入缓冲区
char outbuf[128]; //定义输出缓冲区
ViSession defaultRM;	//定义系统默认资源
ViSession vi;	//定义仪器会话
void Initialize(void);


//函数声明
int fuc_rst(void);//复位声明
int fuc_idn(void);//显示信息声明
int fuc_freq(void);
int fuc_ampl(void);
int fuc_setfrecent(long long int centfre); //中心频率 
int fuc_setfrefirst(long long int frefirst);//起始频率
int fuc_setfrelast(long long int frelast);//截至频率
int fuc_setfrestep(long long int frestep);//步进频率
int fuc_setfreoffset(long long int freoffset);//偏置频率


int fuc_setreflevel(int ref);//参考电压 
int fuc_setattenuation(int att);//幅度衰减
int fuc_setscale(int scale);//刻度 
int fuc_extampgain(int extampgain);//外部放大器校正

void wtygraph(void);//图像处理

//变量
char idbuf[100]= {0};
char command[100]; 
int mod_freq = 0;
int para_freq = 0;
int mod_ampl = 0;
int para_ampl = 0;
int status_frestep = 0;//步进频率设置0自动，1手动
int status_signaltrack = 0; //信号跟踪0关，1开
int status_attenuation = 0;//衰减0自动，1手动
int status_scaletype = 0; //刻度类型0对数，1线性
int status_intpreamp = 0; //前置放大0关，1开
int status_attenuationlimit = 1;//0关1开
int status_markermode=1;//1为normal ，0为delta
int status_markerstate=0;//0为关，1为开
int status_frecntmakreso=1;//频率分辨率1自动
int status_frecntmark=0;//频率计数器0为关闭
int status_phasenoise=0;//相位噪声0关闭
int status_optphanoi=0;//优化噪声
int status_markertrace=1;//标记轨迹
int status_markertable=0;//标记列表
int status_spanpair=0;//0扫宽，1中心频率
double fre_value = 0;//储存面板上的频率值 
int fre_unit = 0;//储存面板上频率单位
long long int fre_real=0; //真实频率值= fre_value*fre_unit 
double ampl_value=0;//储存面板上幅度值
double marker_value=0;//储存面板上marker值 
int ampl_unit =0;
int marker_unit =0;
int xdata[2500];
double ydata[2500];
char tracedata[12000];
char tracedata2[1000][15];
double x_marker=0;
double y_marker=0;
int x_graph=0;
int y_graph=0;
int num=0;
double fre1=0;
double fre2=0;
double fre3=0;
char sfre1[30];
char sfre2[30];
char sfre3[30];
char sx_marker[30];
char sy_marker[30];
int markerknob=0;



////////////////////////4.20新增
static int panelHandle;
int continuous = 0;
int mode = 0;
////////////////////////4.27新增
#define DISPDATALEN 461
int XData[DISPDATALEN];	
int YData[DISPDATALEN];	
int istart;//偏移单位偏移量的个数，设定单位偏移量为3.14/180；
///////////////////////////
int main (int argc, char *argv[])
{
	for(int i=0;i<2500;i++)
		xdata[i]=i;
    istart = 0;
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "demo.uir", PANEL)) <  0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}

int CVICALLBACK main_callback (int panel, int event, void *callbackData,
								 int eventData1, int eventData2)
  {
	  switch (event)
	  {
		  case EVENT_GOT_FOCUS:

			  break;
		  case EVENT_LOST_FOCUS:

			  break;
		  case EVENT_CLOSE:
			   QuitUserInterface(0);
			  break;
	  }
	  return 0;
  }


///////////////////////////////////////////////////////////////////////////
//功能函数
//更改command后调用
void wtyprint(void)
{
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
		viPrintf(vi,command); 
	viClose(vi);
	viClose(defaultRM);	
}
//原始数据tracedata，分割后数据tracedata2[1000][15];最终数据ydata[2500]
void wtygraph(void)
{
	//char temp[12000];
	//strcpy(temp,tracedata);
	char delims[] = ",";
	char *tempresult = NULL;
	tempresult = strtok(tracedata,delims);
	int ii=0;
    int jj=0;
	while( tempresult != NULL)
    {
        //printf("%s\n",tempresult );
        sprintf(tracedata2[ii++],"%s",tempresult );
        tempresult = strtok( NULL, delims );
    }
	for(jj=0;jj<ii;jj++)
    {
        //printf("%s\n",tracedata2[j]);
        ydata[jj]=atof(tracedata2[jj]);
        //printf("%f\n",ydata2[j]);
    }
	
}
//初始化测量操作，不一定能用
void Initialize(void)
{	 
	//viPrintf(vi,":AUTOSCALE\n");
	viPrintf(vi,":CALCulate:AUTOtune\n"); 
}
 //619新增
int fuc_rst(void)	  
{
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
		viPrintf(vi,"*RST\n");
    viClose(vi);
	viClose(defaultRM);
	return 0;
}
int fuc_idn(void)	  
{
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	viQueryf(vi, "*IDN?\n", "%t", &idbuf);
		//viPrintf(vi,"*IDN?\n");
		//viScanf(vi, "%t", &idbuf);
    viClose(vi);
	viClose(defaultRM);
	return 0;
}
int fuc_freq(void)//想写成真正的菜单还没有实现
{
	//if(mod%1000)
	//假设有四层菜单，每层对应7个按键，char 7个字符
	
	return 0;
}
int fuc_ampl(void)
{
	return 0;
}

int fuc_setfrecent(long long int centfre) //中心频率 
{
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,"SENS:FREQ:CENT %lld\n",centfre);
	viPrintf(vi,command);
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",centfre);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
}

int fuc_setfrefirst(long long int frefirst)//起始频率
{
    viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,"SENS:FREQ:STAR %lld\n",frefirst);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frefirst);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
}

int fuc_setfrelast(long long int frelast)//截至频率 
{
    viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,"SENS:FREQ:STOP %lld\n",frelast);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frelast);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
}

int fuc_setfrestep(long long int frestep)//步进频率
{
	//[:SENSe]:FREQuency:CENTer:STEP[:INCRement] <freq>
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,"SENS:FREQ:CENT:STEP:INCR %lld\n",frestep);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frelast);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
	
}
int fuc_setfreoffset(long long int freoffset)//偏置频率
{
	//:DISPlay:WINDow:TRACe:X[:SCALe]:OFFSet <freq>
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,":DISP:WIND:TRAC:X:SCAL:OFFS %lld\n",freoffset);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frelast);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
}
int fuc_setreflevel(int ref)//参考电压
{
	//:DISPlay:WINDow:TRACe:Y[:SCALe]:RLEVel <ampl>
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,":DISP:WIND:TRAC:Y:SCALe:RLEV %d\n",ref);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frelast);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
}
int fuc_setattenuation(int att)//幅度衰减
{
	//[:SENSe]:POWer[:RF]:ATTenuation <rel_ampl>	
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,":SENSe:POWer:RF:ATTenuation %d\n",att);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frelast);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
	
}
int fuc_setscale(int scale)//刻度
{
	//:DISPlay:WINDow:TRACe:Y[:SCALe]:PDIVision <rel_ampl>
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,":DISPlay:WINDow:TRACe:Y:SCALe:PDIVision %d\n",scale);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frelast);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
}
int fuc_extampgain(int extampgain)//外部放大器校正
{
	//[:SENSe]:CORRection:OFFSet[:MAGNitude] <rel_ampl>
	viOpenDefaultRM(&defaultRM);
	viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	viClear(vi);
	sprintf(command,":SENSe:CORRection:OFFSet:MAGNitude %d\n",extampgain);
	viPrintf(vi,command); 
	//viPrintf(vi,":SENS:FREQ:CENT %lf MHz\n",frelast);
	viClose(vi);
	viClose(defaultRM);	
	return 0;
	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////分界线  

//复位回调函数
int CVICALLBACK get_rst (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			fuc_rst();
			break;
	}
	return 0;
}
//仪器表示回调函数
int CVICALLBACK get_idn (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			fuc_idn();
			SetCtrlVal(panelHandle,PANEL_result,idbuf); 
			break;
	}
	return 0;
}
//定时器回调函数
int CVICALLBACK time_fuc (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	int i;
	float fvalue;
	float fstart;
	switch (event)
	{
		case EVENT_TIMER_TICK:
			 /*
			SetCtrlAttribute(panelHandle, PANEL_GRAPH,ATTR_REFRESH_GRAPH,0);
			DeleteGraphPlot (panelHandle, PANEL_GRAPH, -1, VAL_DELAYED_DRAW); //清屏      
			fstart =istart*3.14/30;
			for(i=0;i<DISPDATALEN;i++)
			{
				fvalue = i;
				fvalue = fvalue/100;
				ydata[i]=100+80*sin(fvalue+fstart);
				PlotPoint (panelHandle, PANEL_GRAPH, xdata[i],ydata[i], VAL_SMALL_SOLID_SQUARE, VAL_RED);
			}
			RefreshGraph( panelHandle, PANEL_GRAPH); 
			istart++;
			*/
			
			
			//:TRAC:DATA? TRACE1
			strcpy(command, ":TRAC:DATA? TRACE1\n");
			viOpenDefaultRM(&defaultRM);
			viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
			viClear(vi);
				//viPrintf(vi,command);
				//viScanf(vi,"%c",tracedata);
			    //viQueryf(vi,command,"%c",&tracedata);
			    viQueryf(vi,command,"%t",&tracedata); 
			viClose(vi);
			viClose(defaultRM);
			
			//屏幕信息
			//[:SENSe]:FREQuency:STARt?
			//[:SENSe]:FREQuency:CENTer?
			//[:SENSe]:FREQuency:STOP?
			//double fre1=0;double fre2=0;double fre3=0;char sfre1[30];char sfre2[30];char sfre3[30];
			
			
				viOpenDefaultRM(&defaultRM);
	            viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	            viClear(vi);
				
					viQueryf(vi, ":SENSe:FREQuency:STARt?\n", "%t", &sfre1);
				    //sprintf(sfre1,"%lf\n",fre1); 
					SetCtrlVal(panelHandle,PANEL_fre1,sfre1);
					
					viQueryf(vi, ":SENSe:FREQuency:CENTer?\n", "%t", &sfre2);
				    // sprintf(sfre2,"%lf\n",fre2); 
					SetCtrlVal(panelHandle,PANEL_fre2,sfre2);
					
					viQueryf(vi, ":SENSe:FREQuency:STOP?\n", "%t", &sfre3);
				    //sprintf(sfre3,"%lf\n",fre3); 
					SetCtrlVal(panelHandle,PANEL_fre3,sfre3);
			
			    viClose(vi);
	            viClose(defaultRM);
			
			 
			
			////数据数据处理
			wtygraph(); 
			SetCtrlAttribute(panelHandle, PANEL_GRAPH,ATTR_REFRESH_GRAPH,0);
			DeleteGraphPlot (panelHandle, PANEL_GRAPH, -1, VAL_DELAYED_DRAW); //清屏
			for(i=0;i<DISPDATALEN;i++)
			{
				PlotPoint (panelHandle, PANEL_GRAPH, xdata[i],ydata[i], VAL_SMALL_SOLID_SQUARE, VAL_RED);
			}
			
			
			if(status_markerstate)//marker菜单打开
			{
				//:CALCulate:MARKer[1]|2|3|4:Y?
				//:CALCulate:MARKer[n]:X?
			    viOpenDefaultRM(&defaultRM);
	            viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
	            viClear(vi);
					viQueryf(vi, ":CALCulate:MARKer1:Y?\n", "%lf", &y_marker);
				    
				
					viQueryf(vi, ":CALCulate:MARKer1:X?\n", "%lf", &x_marker); 

				
					//映射
					GetCtrlVal(panelHandle, PANEL_markerknob, &markerknob);
					fre3=atof(sfre3);
					fre1=atof(sfre1);
					x_graph=(x_marker-fre1)/(fre3-fre1)*DISPDATALEN + markerknob;
					y_graph=ydata[x_graph];
					PlotPoint (panelHandle, PANEL_GRAPH, x_graph,y_graph+2, VAL_SMALL_SOLID_SQUARE, VAL_GREEN);
					
					x_marker= x_graph*(fre3-fre1)/DISPDATALEN+ fre1;
					sprintf(sx_marker,"%lf\n",x_marker); 
					SetCtrlVal(panelHandle,PANEL_x_marker,sx_marker);
					sprintf(sy_marker,"%d\n",y_graph); 
					SetCtrlVal(panelHandle,PANEL_y_marker,sy_marker);
					
				viClose(vi);
	            viClose(defaultRM);
			}
			
			RefreshGraph( panelHandle, PANEL_GRAPH);

			
			
			break;
	}
	return 0;
}
//测试按钮回调函数
/*
int CVICALLBACK okcallback (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{ char result2[100];
  double result4;
  long long int suibian=0;
  int unit=0;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_fre_value, &fre_value);
			GetCtrlVal(panelHandle, PANEL_RING_freunit, &unit); 
			suibian=result4*unit;
			//sprintf(command,"unit %d\n",unit);
			sprintf(command,"SENS:FREQ:STOP %lld\n",suibian);
			SetCtrlVal(panelHandle,PANEL_result3,command);
			break;
	}
	return 0;
}
*/
//设置中心频率回调函数
int CVICALLBACK setfrecent (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	//double fre_value = 0;//储存面板上的频率值 
	//int fre_unit = 0;//储存面板上频率单位
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_fre_value, &fre_value);
			GetCtrlVal(panelHandle, PANEL_fre_unit, &fre_unit);
			fre_real =fre_value * fre_unit;
			fuc_setfrecent(fre_real); 
			break;
	}
	return 0;
}
//设置起始频率回调函数
int CVICALLBACK setfrefirst (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		    GetCtrlVal(panelHandle, PANEL_fre_value, &fre_value);
			GetCtrlVal(panelHandle, PANEL_fre_unit, &fre_unit);
			fre_real =fre_value * fre_unit;
			fuc_setfrefirst(fre_real);
			break;
	}
	return 0;
}
//设置截至频率回调函数 
int CVICALLBACK setfrelast (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		    GetCtrlVal(panelHandle, PANEL_fre_value, &fre_value);
			GetCtrlVal(panelHandle, PANEL_fre_unit, &fre_unit);
			fre_real =fre_value * fre_unit;
			fuc_setfrelast(fre_real); 
			break;
	}
	return 0;
}
//频率步进回调函数
int CVICALLBACK setfrestep (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	//[:SENSe]:FREQuency:CENTer:STEP:AUTO OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_frestep==0) //现在是自动模式用户想改为手动
			{
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":SENSe:FREQ:CENT:STEP:AUTO OFF\n");
				viClose(vi);
				viClose(defaultRM);
				
				GetCtrlVal(panelHandle, PANEL_fre_value, &fre_value);
				GetCtrlVal(panelHandle, PANEL_fre_unit, &fre_unit);
				fre_real =fre_value * fre_unit;
				fuc_setfrestep(fre_real);
				
				SetCtrlVal(panelHandle,PANEL_LED_frestep1,0);
				SetCtrlVal(panelHandle,PANEL_LED_frestep2,1);
				status_frestep=1;
			}
			else//手动改为自动
			{
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":SENSe:FREQ:CENT:STEP:AUTO ON\n");
				viClose(vi);
				viClose(defaultRM);
				
				SetCtrlVal(panelHandle,PANEL_LED_frestep1,1);
				SetCtrlVal(panelHandle,PANEL_LED_frestep2,0);
				status_frestep=0;
			}
			break;
	}
	return 0;
}
//频率偏置回调
int CVICALLBACK setfreoffset (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			 	GetCtrlVal(panelHandle, PANEL_fre_value, &fre_value);
				GetCtrlVal(panelHandle, PANEL_fre_unit, &fre_unit);
				fre_real =fre_value * fre_unit;
				fuc_setfreoffset(fre_real);
			break;
	}
	return 0;
}
//信号跟踪回调
int CVICALLBACK signaltrack (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:TRCKing[:STATe] OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_signaltrack==0) //目前状态是关，打开
			{   
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":CALCulate:MARKer1:TRCKing:STATe ON\n");
				viClose(vi);
				viClose(defaultRM);
				
			    SetCtrlVal(panelHandle,PANEL_LED_signaltrack1,1);
				SetCtrlVal(panelHandle,PANEL_LED_signaltrack2,0);
				status_signaltrack=1;
			}
			else
			{   
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":CALCulate:MARKer1:TRCKing:STATe OFF\n");
				viClose(vi);
				viClose(defaultRM);
				 
				SetCtrlVal(panelHandle,PANEL_LED_signaltrack1,0);
				SetCtrlVal(panelHandle,PANEL_LED_signaltrack2,1);
				status_signaltrack=0;
			}
			break;
	}
	return 0;
}

//参考电压回调函数
int CVICALLBACK setreflevel (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	int reflevel=0; 
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_ampl_value, &ampl_value);
			reflevel=ampl_value;
			fuc_setreflevel(reflevel);
			sprintf(command,":DISP:WIND:TRAC:Y:SCAL:RLEV %d\n",reflevel);
			//SetCtrlVal(panelHandle,PANEL_result3,command);
			break;
	}
	return 0;
}
//幅度衰减回调函数
int CVICALLBACK setattenuation (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	int attenuation=0; 
	//[:SENSe]:POWer[:RF]:ATTenuation:AUTO OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_attenuation==0)//现在是0，自动想要改成手动
			{   
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":SENSe:POWer:RF:ATTenuation:AUTO OFF\n");
				viClose(vi);
				viClose(defaultRM);
				
				GetCtrlVal(panelHandle, PANEL_ampl_value, &ampl_value);
				attenuation = ampl_value;
				fuc_setattenuation(attenuation);
			    
				SetCtrlVal(panelHandle,PANEL_LED_attenuation1,0);
				SetCtrlVal(panelHandle,PANEL_LED_attenuation2,1);
				status_attenuation=1;
			
			}
			else
			{   
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":SENSe:POWer:RF:ATTenuation:AUTO ON\n");
				viClose(vi);
				viClose(defaultRM);
				
				SetCtrlVal(panelHandle,PANEL_LED_attenuation1,1);
				SetCtrlVal(panelHandle,PANEL_LED_attenuation2,0);
				status_attenuation=0;
			}

			break;
	}
	return 0;
}
//刻度格回调
int CVICALLBACK setscale (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{ 
	int scale=0;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_ampl_value, &ampl_value);
			scale=ampl_value;
			fuc_setscale(scale);
			break;
	}
	return 0;
}

//刻度类型回调函数
int CVICALLBACK setscaletype (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	//:DISPlay:WINDow:TRACe:Y[:SCALe]:SPACing LINear|LOGarithmic
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_scaletype==0) //现在状态是对数，要改成线性
			{
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":DISPlay:WINDow:TRACe:Y:SCALe:SPACing LINear\n");
				viClose(vi);
				viClose(defaultRM);
				SetCtrlVal(panelHandle,PANEL_LED_scaletype1,0);
				SetCtrlVal(panelHandle,PANEL_LED_scaletype2,1);
				status_scaletype=1;	
				SetCtrlAttribute(panelHandle, PANEL_butt_scale,ATTR_DIMMED,1);
			}
			else
			{
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":DISPlay:WINDow:TRACe:Y:SCALe:SPACing LOGarithmic\n");
				viClose(vi);
				viClose(defaultRM);
				SetCtrlVal(panelHandle,PANEL_LED_scaletype1,1);
				SetCtrlVal(panelHandle,PANEL_LED_scaletype2,0);
				status_scaletype=0;
				SetCtrlAttribute(panelHandle, PANEL_butt_scale,ATTR_DIMMED,0);  
			}

			break;
	}
	return 0;
}

int CVICALLBACK setintpreamp (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	//[:SENSe]:POWer[:RF]:GAIN[:STATe] OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_intpreamp==0)
			{
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":SENSe:POWer:RF:GAIN:STATe ON\n");
				viClose(vi);
				viClose(defaultRM);
				SetCtrlVal(panelHandle,PANEL_LED_intpreamp1,1);
				SetCtrlVal(panelHandle,PANEL_LED_intpreamp2,0);
				status_intpreamp=1;
			}
			else
			{
				viOpenDefaultRM(&defaultRM);
				viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
				viClear(vi);
					viPrintf(vi,":SENSe:POWer:RF:GAIN:STATe OFF\n");
				viClose(vi);
				viClose(defaultRM);
				SetCtrlVal(panelHandle,PANEL_LED_intpreamp1,0);
				SetCtrlVal(panelHandle,PANEL_LED_intpreamp2,1);
				status_intpreamp=0;
			}
			break;
	}
	return 0;
}

int CVICALLBACK set_yaxis (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	//:UNIT:POWer DBM|DBMV|DBUV|DBUA|V|W|A
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_ampl_unit, &ampl_unit);
			if(ampl_unit==0)
				strcpy(command, ":UNIT:POWer DBM\n");
			else if(ampl_unit==1)
				strcpy(command, ":UNIT:POWer DBMV\n");
			else if(ampl_unit==2)
				strcpy(command, ":UNIT:POWer DBUV\n");
			else if(ampl_unit==3)
				strcpy(command, ":UNIT:POWer DBUA\n");
			else if(ampl_unit==4)
				strcpy(command, ":UNIT:POWer W\n");
			else if(ampl_unit==5)
				strcpy(command, ":UNIT:POWer V\n");
			else if(ampl_unit==6)
				strcpy(command, ":UNIT:POWer A\n");
			
			//SetCtrlVal(panelHandle,PANEL_result3,command);
			viOpenDefaultRM(&defaultRM);
			viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
			viClear(vi);
				viPrintf(vi,command);
			viClose(vi);
			viClose(defaultRM);
			break;
	}
	return 0;
}
//两个按键回调
int CVICALLBACK tracebegin (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		SetCtrlAttribute(panelHandle, PANEL_TIMER1,ATTR_ENABLED,1);    //使定时器有效     
		SetCtrlAttribute(panelHandle, PANEL_butt_tracestop,ATTR_DIMMED,0);	   
		SetCtrlAttribute(panelHandle, PANEL_butt_tracebegin,ATTR_DIMMED,1);
			break;
	}
	return 0;
}

int CVICALLBACK tracestop (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		SetCtrlAttribute(panelHandle, PANEL_TIMER1,ATTR_ENABLED,0);    //使定时器无效      
		SetCtrlAttribute(panelHandle, PANEL_butt_tracestop,ATTR_DIMMED,1);	   
		SetCtrlAttribute(panelHandle, PANEL_butt_tracebegin,ATTR_DIMMED,0);
			break;
	}
	return 0;
}
//外部放大器校正回调函数
int CVICALLBACK extampgain (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_ampl_value, &ampl_value);
			fuc_extampgain(ampl_value);
			break;
	}
	return 0;
}

int CVICALLBACK attenuationlimit (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	//[:SENSe]:POWer[:RF]:ATTLimit:STATe OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_attenuationlimit==1) //默认是开
			{
				strcpy(command, ":SENSe:POWer:RF:ATTLimit:STATe OFF\n");
				SetCtrlVal(panelHandle,PANEL_LED_attenuationlimit1,0);
				SetCtrlVal(panelHandle,PANEL_LED_attenuationlimit2,1);
				status_attenuationlimit=0;
			}
			else
			{
				strcpy(command, ":SENSe:POWer:RF:ATTLimit:STATe ON\n");
				SetCtrlVal(panelHandle,PANEL_LED_attenuationlimit1,1);
				SetCtrlVal(panelHandle,PANEL_LED_attenuationlimit2,0);
				status_attenuationlimit=1;
			}
			viOpenDefaultRM(&defaultRM);
			viOpen(defaultRM,RESOURCE,VI_NULL,VI_NULL,&vi);
			viClear(vi);
				viPrintf(vi,command);
			viClose(vi);
			viClose(defaultRM);
				
			break;
	}
	return 0;
}
///////////////////////////////////////////////////////////////marker(621改结构不区分回调函数和功能函数，采用wtyprint）
//marker 模式修改
int CVICALLBACK markermode (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:MODE POSition|DELTa
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_markermode==1)  //现在是normal模式
			{
				strcpy(command, ":CALCulate:MARKer1:MODE DELTa\n");
				status_markermode=0;
			}
			else
			{
				strcpy(command, ":CALCulate:MARKer1:MODE POSition\n");
				status_markermode=1;
			}
			wtyprint();
			//SetCtrlVal(panelHandle,PANEL_result3,command);
			break;
	}
	return 0;
}

//全部关闭
int CVICALLBACK alloff (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer:AOFF
	switch (event)
	{
		case EVENT_COMMIT:
			strcpy(command,":CALCulate:MARKer:AOFF\n");
			wtyprint(); 
			break;
	}
	return 0;
}
//关
int CVICALLBACK markerstate (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:STATe OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_markerstate==0)
			{
				strcpy(command,":CALCulate:MARKer1:STATe ON\n");
				status_markerstate=1;
				SetCtrlAttribute(panelHandle, PANEL_x_marker,ATTR_DIMMED,0);
				SetCtrlAttribute(panelHandle, PANEL_y_marker,ATTR_DIMMED,0); 
			}
			else
			{
				strcpy(command,":CALCulate:MARKer1:STATe OFF\n");
				status_markerstate=0;
				SetCtrlAttribute(panelHandle, PANEL_x_marker,ATTR_DIMMED,1);
				SetCtrlAttribute(panelHandle, PANEL_y_marker,ATTR_DIMMED,1); 
			}
			wtyprint();
			//SetCtrlVal(panelHandle,PANEL_result3,command);				   
			break;
	}
	return 0;
}

//频率分辨率
int CVICALLBACK frecntmakreso (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer:FCOunt:RESolution:AUTO OFF|ON|0|1
	//:CALCulate:MARKer:FCOunt:RESolution <real>
	int frecntmakreso=0; 
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_frecntmakreso==1) //目前是自动，改为手动
			{
				strcpy(command,":CALCulate:MARKer:FCOunt:RESolution:AUTO OFF\n");
				wtyprint();
				GetCtrlVal(panelHandle, PANEL_marker_value, &marker_value);
				frecntmakreso=marker_value;
				sprintf(command,":CALCulate:MARKer:FCOunt:RESolution %d\n",frecntmakreso);
				wtyprint();
				status_frecntmakreso=0;
			}
			else//目前是手动，改为自动
			{
				strcpy(command,":CALCulate:MARKer:FCOunt:RESolution:AUTO ON\n");
				wtyprint();
				status_frecntmakreso=1;
			}
			break;
	}
	return 0;
}
//频率计数器
int CVICALLBACK frecntmark (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:FCOunt[:STATe] OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_frecntmark==0)
			{
				strcpy(command,":CALCulate:MARKer:FCOunt:STATe ON\n");
				status_frecntmark=1;
			}
			else
			{
				strcpy(command,":CALCulate:MARKer:FCOunt:STATe OFF\n");
				status_frecntmark=0;
			}
			wtyprint();
			break;
	}
	return 0;
}
//相位噪声
int CVICALLBACK phasenoise (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:PHNoise:[STATe] ON|OFF|1|0
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_phasenoise==0)
			{
				strcpy(command,":CALCulate:MARKer1:PHNoise:STATe ON\n");
				status_phasenoise=1;
			}
			else
			{
				strcpy(command,":CALCulate:MARKer1:PHNoise:STATe OFF\n");
				status_phasenoise=0;
			}
			wtyprint();
			break;
	}
	return 0;
}
//相位噪声偏置手动
int CVICALLBACK noioffman (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer:PHNoise:OFFSet:FREQuency <freq>
	int noioffman=0;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_marker_value, &marker_value);
			noioffman=marker_value;
			sprintf(command,":CALCulate:MARKer:PHNoise:OFFSet:FREQuency %d\n",noioffman); 
			wtyprint();
			//SetCtrlVal(panelHandle,PANEL_result3,command);
			break;
	}
	return 0;
}
//相位噪声偏置
int CVICALLBACK noioffset (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
    //:CALCulate:MARKer:PHNoise:OFFSet 1kHz|-1kHz|10kHz|-10kHz|20kHz|-20kHz|30kHz|-30kHz|50kHz|-50kHz|100kHz|-100kHz|1MHz|-1MHz
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_marker_unit, &marker_unit);
			if(marker_unit==0)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet 1kHz\n");
			else if(marker_unit==1)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet -1kHz\n");
			else if(marker_unit==2)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet 10kHz\n");
			else if(marker_unit==3)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet -10kHz\n");
			else if(marker_unit==4)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet 20kHz\n");
			else if(marker_unit==5)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet -20kHz\n");
			else if(marker_unit==6)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet 30kHz\n");
			else if(marker_unit==7)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet -30kHz\n");
			else if(marker_unit==8)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet 50kHz\n");
			else if(marker_unit==9)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet -50kHz\n");
			else if(marker_unit==10)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet 100kHz\n");
			else if(marker_unit==11)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet -100kHz\n");
			else if(marker_unit==12)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet 1MkHz\n");
			else if(marker_unit==13)
				strcpy(command,":CALCulate:MARKer:PHNoise:OFFSet -1MkHz\n");
			wtyprint();
			//SetCtrlVal(panelHandle,PANEL_result3,command);
			
			break;
	}
	return 0;
}
//优化相位噪声
int CVICALLBACK optphanoi (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer:PHNoise:OPTimize ON|OFF|1|0
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_optphanoi==0)
			{
				strcpy(command,":CALCulate:MARKer:PHNoise:OPTimize ON\n");
				status_optphanoi=1;
			}
			else
			{
				strcpy(command,":CALCulate:MARKer:PHNoise:OPTimize OFF\n");
				status_optphanoi=0;
			}
			wtyprint();
			//SetCtrlVal(panelHandle,PANEL_result3,command);
			break;
	}
	return 0;
}
//标记轨迹
int CVICALLBACK markertrace (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:TRACe:AUTO OFF|ON|0|1
	//:CALCulate:MARKer[n]:TRACe <integer>
	int markertraceint=0; 
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_markertrace==1)//现在是自动
			{
				strcpy(command,":CALCulate:MARKer1:TRACe:AUTO OFF\n");
				wtyprint();
				
				GetCtrlVal(panelHandle, PANEL_marker_value, &marker_value);
			    markertraceint=marker_value;
				sprintf(command,":CALCulate:MARKer[n]:TRACe %d\n",markertraceint); 
				wtyprint();
				
				status_markertrace=0;
			}
			else
			{
				strcpy(command,":CALCulate:MARKer1:TRACe:AUTO ON\n");
				wtyprint(); 
				status_markertrace=1;
			}
			break;
	}
	return 0;
}
//标记列表
int CVICALLBACK markertable (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer:TABLe:STATe OFF|ON|0|1
	switch (event)
	{
		case EVENT_COMMIT:
			if(status_markertable==0)
			{
				strcpy(command,":CALCulate:MARKer:TABLe:STATe ON\n");
				status_markertable=1;
			}
			else
			{
				strcpy(command,":CALCulate:MARKer:TABLe:STATe OFF\n");
				status_markertable=0;
			}
			wtyprint();
			break;
	}
	return 0;
}
//扫宽对
int CVICALLBACK spanpair (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:X:SPAN <param>
	//:CALCulate:MARKer[n]:X:CENTer <param>
	int spanpair=0;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_marker_value, &marker_value);
			spanpair=marker_value;
			if(status_spanpair==0)//现在是扫宽改变成中心
			{
				sprintf(command,":CALCulate:MARKer1:X:CENTer %d\n",spanpair);
				status_spanpair=1;
			}
			else
			{
				sprintf(command,":CALCulate:MARKer1:X:SPAN %d\n",spanpair);
				status_spanpair=0; 
			}
			wtyprint();
			break;
	}
	return 0;
}
//delta对
int CVICALLBACK deltapair (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	//:CALCulate:MARKer[n]:X:REFerence <param>
	int deltapair=0; 
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle, PANEL_marker_value, &marker_value);
			deltapair=marker_value;
			sprintf(command,":CALCulate:MARKer[n]:X:REFerence %d\n",deltapair);
			wtyprint();
			break;
	}
	return 0;
}

int CVICALLBACK autotune (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	//viPrintf(vi,":CALCulate:AUTOtune\n");
	switch (event)
	{
		case EVENT_COMMIT:
		strcpy(command,":CALCulate:AUTOtune\n") ;
		wtyprint();
			break;
	}
	return 0;
}
