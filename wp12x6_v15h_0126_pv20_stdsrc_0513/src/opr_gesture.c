/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "opr_gesture.h"
#include "com_global_type.h"



#define meta_delta_x     8
#define meta_delta_y     8
#define NormDx			32
#define NormDy			32
#define NormP			16
#define PointMaxNum		150



s8 feature_point_is_less;
s16 sample_point_count;
s8 result_x[NormP],result_y[NormP];

s16 max_x,min_x,max_y,min_y;
s16 data_sample_x[PointMaxNum],data_sample_y[PointMaxNum];
s16 start_x;
s16 start_y;
s8  GestureMatch_Run_Flag;
s8  match_status;

static void fetch_object_sample(s16 curx,s16 cury);
static void fetch_object_sample_2level(s16 *pdata_sample_x,s16 *pdata_sample_y);
static void sampledata_to_standardizing(s16 *pdata_sample_x,s16 *pdata_sample_y);
static void sampledata_to_normalizing(s16 *pdata_sample_x,s16 *pdata_sample_y);
static void feature_to_transform();
static void feature_to_match();

void GestureVariableInitialFunction(void)
{
	sample_point_count = 0x00;
	start_x = 0x00;
	start_y = 0x00;
	GestureMatch_Run_Flag = 0x00;
	match_status = -1;

}


s8 GestureMatchProcess(u8 ucCurrentPointNum,u8 ucLastPointNum,s16 curx,s16 cury)
{
    if (ucCurrentPointNum == 1)
    {
        if (ucLastPointNum == 0)
        {
            GestureMatch_Run_Flag = 1;
            match_status=0;
            //start get point datas
        }
    }
    else if (GestureMatch_Run_Flag > 0)
    {
        if (ucCurrentPointNum == 0)
        {
            GestureMatch_Run_Flag = 4;
            //finish get point datas
        }
        else
        {
            sample_point_count=0;
            GestureMatch_Run_Flag = 0;
            //give up the match process.
        }
    }
    //else
    //{
        //GestureMatch_Run_Flag = 0;
        //none done;
    //}

//	printf("current:%2d  match:%2d  Run:%2d  sample:%2d  \n\r",ucCurrentPointNum,match_status,GestureMatch_Run_Flag,sample_point_count);

    if(GestureMatch_Run_Flag > 0)
    {
        //start match
        fetch_object_sample(curx,cury);
        return match_status;
    }
    else
    {
        return match_status;
        //match_status=-1;
        //return 0;
    }
}

static void fetch_object_sample(s16 curx,s16 cury)
{
    if(GestureMatch_Run_Flag==1)
    {
        //init parameters
        start_x = curx;
        start_y = cury;
        data_sample_x[0] = start_x;
        data_sample_y[0] = start_y;
        max_x = start_x;
        min_x = start_x;
        max_y = start_y;
        min_y = start_y;
        sample_point_count = 1;
        GestureMatch_Run_Flag=2;//continue get datas...
    }
    else if(GestureMatch_Run_Flag==2)
    {
        s16 cur_dx,cur_dy;
        if(curx >= start_x) 
        {
            cur_dx = curx - start_x;
        }
        else
        {
            cur_dx = start_x - curx;
        }
    
        if(cury >= start_y) 
        {
            cur_dy = cury - start_y;
        }
        else
        {
            cur_dy = start_y - cury;
        }
    
        if((cur_dx > meta_delta_x) || (cur_dy > meta_delta_y))
        {
            start_x = curx;
            start_y = cury;
            data_sample_x[sample_point_count] = start_x;
            data_sample_y[sample_point_count] = start_y;
            //printk(KERN_WARNING "data_sample_x is %d %d  ",index,data_sample_x[index]);
            
            if(curx > max_x)
            max_x = curx;
            else if (curx <= min_x)
            min_x = curx;
            
            if(cury > max_y)
            max_y = cury;
            else if (cury <= min_y)
            min_y = cury;
            
            sample_point_count++;
        }
        if(sample_point_count >= PointMaxNum)
        {
            //break;
            GestureMatch_Run_Flag=4;
        }
    }
    
    if(GestureMatch_Run_Flag==4)
    {
        if(sample_point_count>4)
        {
            char wide_ratio;
            {
                s16 widerx;
                s16 widery;
                widerx=(max_x-min_x);
                widery=(max_y-min_y);
                if(widerx>=widery)
                {
                    wide_ratio=widerx/widery;
                }
                else
                {
                    wide_ratio=widery/widerx;
                }
            }
        
            if(wide_ratio<=5)
            {
                sampledata_to_standardizing(data_sample_x,data_sample_y);
                fetch_object_sample_2level(data_sample_x,data_sample_y);
                sampledata_to_normalizing(data_sample_x,data_sample_y);
                feature_to_transform();
            }
            else
            {
                //printk(KERN_WARNING "big length-width ratio");
                match_status=125;
            }
        }
        else
        {
             //printk(KERN_WARNING "There is less featrue point ");
             match_status=127;
        }
        GestureMatch_Run_Flag=0;
        sample_point_count=0;
    }
}

static void fetch_object_sample_2level(s16 *pdata_sample_x,s16 *pdata_sample_y)
{
    s16 i;
    s16 index;
    
    s8 curx;
    s8 cury;
    s8 cur_dx;
    s8 cur_dy;
    s8 start_x;
    s8 start_y;
    s8 insert_flag;
    s16 *pdata_out_x,*pdata_out_y;
    
    if(sample_point_count<=50)
    {
        insert_flag=1;
        pdata_out_x=pdata_sample_x+sample_point_count;
        pdata_out_y=pdata_sample_y+sample_point_count;
    }
    else
    {
        insert_flag=0;
        pdata_out_x=pdata_sample_x;
        pdata_out_y=pdata_sample_y;
    }
    
    start_x = (s8)pdata_sample_x[0];
    start_y = (s8)pdata_sample_y[0];
    index = 1;
    
    for(i = 1;i < sample_point_count;i++)
    {
        curx = (s8)pdata_sample_x[i];
        cury = (s8)pdata_sample_y[i];
    
        cur_dx = curx - start_x;
        
        if(cur_dx < 0) 
        {
            cur_dx = -cur_dx;
        }
    
        cur_dy = cury - start_y;
        if(cur_dy < 0) 
        {
            cur_dy = -cur_dy;
        }
    
        if((cur_dx >= 2) || (cur_dy >= 2))
        {
            if(insert_flag>0)
            {
                if((index-i+(sample_point_count<<1))<(PointMaxNum-4))
                {
                    if((cur_dx >= 8) || (cur_dy >= 8))
                    {
                        index=index+1;
                        pdata_out_x[index] = (start_x+curx)>>1;
                        pdata_out_y[index] = (start_y+cury)>>1;
                        pdata_out_x[index-1] = (start_x+pdata_out_x[index])>>1;
                        pdata_out_y[index-1] = (start_y+pdata_out_y[index])>>1;
                        pdata_out_x[index+1] = (pdata_out_x[index]+curx)>>1;
                        pdata_out_y[index+1] = (pdata_out_y[index]+cury)>>1;
                        index=index+2;
                    }
                    else if((cur_dx >= 4) || (cur_dy >= 4))
                    {
                        pdata_out_x[index] = (s16)((start_x+curx)>>1);
                        pdata_out_y[index] = (s16)((start_y+cury)>>1);
                        index++;
                    }
                }
                else
                {
                    insert_flag=-1;
                }
            }
            start_x = curx;
            start_y = cury;
            pdata_out_x[index] = (s16)curx;
            pdata_out_y[index] = (s16)cury;
            index++;
        }
    }
    sample_point_count = index;
    
    if(insert_flag!=0)
    {
        for(i = 1;i < sample_point_count;i++)
        {
            pdata_sample_x[i] = pdata_out_x[i];
            pdata_sample_y[i] = pdata_out_y[i];
        }
    }
}

static void sampledata_to_standardizing(s16 *pdata_sample_x,s16 *pdata_sample_y)
{
    s16 i = 0;
    s16 betaDx = 0;
    s16 betaDy = 0;
    
    betaDx = max_x - min_x;
    if(betaDx <= 0)
    {
        betaDx = 1;
    }
    
    betaDy = max_y - min_y;
    if(betaDy <= 0)
    {
       betaDy = 1;
    }
    
    for(i = 0;i < sample_point_count;i++)
    {
        pdata_sample_x[i] = ((pdata_sample_x[i] - min_x)<<5)/betaDx;
        pdata_sample_y[i] = ((pdata_sample_y[i] - min_y)<<5)/betaDy;
    }
}

static void sampledata_to_normalizing(s16 *pdata_sample_x,s16 *pdata_sample_y)
{
    s16 i,i0;
    s8 j,j0;
    s8 curv;
    s8 overlay;
    
    if(sample_point_count >= NormP)
    {
        feature_point_is_less = 0;
        
        for(i0 = 2;i0 <= sample_point_count;i0++)
        {
            j0 = (i0 <<4)/sample_point_count;//normalize to 16point
            if(j0 < 2) j0 = 2;
            i=i0-1;
            j=j0-1;
            
            curv = (s8)pdata_sample_x[i];
            overlay = (s8)pdata_sample_x[j];
            
            if(overlay == 0)
            {
                pdata_sample_x[j] = (s16)curv;
                if(i != j) pdata_sample_x[i] = 0;
            }
            else
            {
                if(i != j)
                {
                    pdata_sample_x[j] = (s16)((curv + overlay)>>1);
                    pdata_sample_x[i] = 0;
                }
            }
            
            curv = (s8)pdata_sample_y[i];
            overlay = (s8)pdata_sample_y[j];
            
            if(overlay == 0)
            {
                pdata_sample_y[j] = (s16)curv;
                if(i != j) pdata_sample_y[i] = 0;
            }
            else
            {
                if(i != j)
                {
                    pdata_sample_y[j] =(s16)((curv + overlay)>>1);
                    pdata_sample_y[i] = 0;
                }
            }
        }
        
        for(i = 0;i < NormP;i++)
        {
            //char CROM rx[NormP]={0,15,21,24,23,17,11,5,1,1,2,8,15,22,28,31};
            //char CROM ry[NormP]={25,16,11,5,0,1,3,8,15,21,27,30,31,29,24,21};
            //result_x[i] = (s8)rx[i];
            //result_y[i] = (s8)ry[i];
            result_x[i] = (s8)pdata_sample_x[i];
            result_y[i] = (s8)pdata_sample_y[i];

			//printf("[%2d]:[%3d,%3d] ",i,result_x[i],result_y[i]);
        }
    }
    else
    {
        feature_point_is_less = 1;
    }
}

static void feature_to_transform()
{
    s8 startx,starty;
    s8 i = 0;

    if(!feature_point_is_less)
    {
        startx=result_x[0];
        starty=result_y[0];
        result_x[0]=0;
        result_y[0]=0;
        for(i = 1;i < NormP;i++)
        {
            result_x[i]=(result_x[i]-startx)<<1;
            result_y[i]=(result_y[i]-starty)<<1;
        }
        feature_to_match();
    }
    else
    {
        //printk(KERN_WARNING "There is less featrue point ");
        match_status=127;
    }
}

 
static void feature_to_match()
{
    s8 i;
    s8 j;
    s16 rt;
    s8 r1,r2,r,rm;
    s16 sx1,sx2;
    s16 sx3,sx4,sx5;
    s16 sx6,sx7,sx8;
    s16 sy1,sy2;	
    s16 sy3,sy4,sy5;
    s16 sy6,sy7,sy8;
    s8 maxi;
    s8 maxr;
    s16 sumdiff_x,sumdiff_y;
    s16 max_dx,max_dy,max_d;
    s8 *pe;
    s8 *pe2;

    s16 tx,ty;
    s16 ex,rx;
	s16 ry,ey;
			
	maxi = 0x00;
	maxr = 0x00;
    
    for(i = 0;i < (FwSetting.GestureWritingStyle);i++)
    {
        sumdiff_x = 0;
        sumdiff_y = 0;
        max_dx = 0;
        max_dy = 0;
        max_d=0;
        pe=(s8 *)(gs_simulation[i << 1]);
        pe2=(s8 *)(gs_simulation[(i << 1)+1]);
        for(j = 1;j < NormP;j++)
        {
//            s16 tx,ty;
            tx=abs(pe[j]-result_x[j]);
            if(tx>max_dx)
            {
                max_dx=tx;
            }
            ty=abs(pe2[j]-result_y[j]);
            if(ty>max_dy)
            {
               max_dy=ty;
            }
            sumdiff_x=sumdiff_x+tx;
            sumdiff_y=sumdiff_y+ty;
            ty=tx+ty;
            if(ty>max_d)
            {
                max_d=ty;
            }
        }
        if((sumdiff_x>384)||(sumdiff_y>384))
        {
            continue;
        }
        if((max_dx>=45)||(max_dy>=45))
        {
            continue;
        }
        rm=0;
        if(max_d>=40)
        {
            rm=((max_d-40)<<3)/(48-40);
            //continue;
        }
        
        sx1 = 0;
        sx2 = 0;
        sx3 = 0;
        sx4 = 0;
        sx5 = 0;
        pe=(s8 *)(gs_simulation[i << 1]);
        for(j = 1;j < NormP;j++)
        {
//        s16 ex,rx;
        ex=(s16)pe[j];
        rx=(s16)result_x[j];
        sx1 += rx;
        sx2 += ex;
        sx3 +=(rx * rx)>>4;
        sx4 += (ex * ex)>>4;
        sx5 += (rx * ex)>>4;
        }
        sx1 = sx1>>4;
        sx2 = sx2>>4;
        sx6 = sx5 - (sx1*sx2);
        if(sx6<=0)
        {
            continue;
        }
        sx7 = sx3 - (sx1*sx1);
        sx8 = sx4 - (sx2*sx2);
        if((sx7 == 0)||(sx8 == 0))
        {
            r1 = 100;
        }
        else
        {
            sx6=sx6>>4;
            rt =sx6*25;
            rt=(s16)((s32)rt/(sx7>>4));
            rt=rt*sx6;
            r1=(s8)((s32)rt/(sx8>>6));
        }
        if(r1<56)
        {
            continue;
        }
        
        sy1 = 0;
        sy2 = 0;
        sy3 = 0;
        sy4 = 0;
        sy5 = 0;
        pe=(s8 *)(gs_simulation[(i << 1)+1]);
        for(j = 1;j < NormP;j++)
        {
//            s16 ry,ey;
            ey=(s16)pe[j];
            ry=(s16)result_y[j];
            sy1 += ry;
            sy2 += ey;
            sy3 +=(ry * ry)>>4;
            sy4 +=(ey * ey)>>4;
            sy5 +=(ry * ey)>>4;
        }
        sy1 = sy1>>4;
        sy2 = sy2>>4;
        sy6 = sy5 - (sy1*sy2);
        if(sy6<=0)
        {
            continue;
        }
        sy7 = sy3 - (sy1*sy1);
        sy8 = sy4 - (sy2*sy2);
        
        if((sy7 == 0)||(sy8 == 0))
        {
            r2 = 100;
        }
        else
        {
            sy6=sy6>>4;
            rt =sy6*25;
            rt=(s16)((s32)rt/(sy7>>4));
            rt=rt*sy6;
            r2=(s8)((s32)rt/(sy8>>6));
        }
        if(r2<56)
        {
            continue;
        }
        
        r = (r1>>1)+(r2>>1)-rm;
        
        //szOutText = "";
        //szOutText = szOutText + "#"  + "r: " + e_str[i] + " (" + i + ") " + r + "\r\n";	
        //Report.WriteStringIntoReport(szOutText,0);
        //printk(KERN_WARNING "r: %c(%d), r  is %d\n",e_str[i],i,r);
        //Toast.makeText(getApplicationContext(), "r:" + e_str[i] + "(" + i + ")" + r,
        //Toast.LENGTH_SHORT).show();
        if(r>maxr)
        {
            maxr=r;
            maxi=i;
        }
    }
    
    
    if((maxr >= 70)&&(gs_character[maxi]!=0xff))
    {
        //printk(KERN_WARNING "There max match char is %c(%d), r  is %d\n",e_str[maxi],maxi,maxr);
        match_status=maxi+1;
        //break;
    }
    else
    {
        //printk(KERN_WARNING "There input char is not match ");
        match_status=126;
    }
}

