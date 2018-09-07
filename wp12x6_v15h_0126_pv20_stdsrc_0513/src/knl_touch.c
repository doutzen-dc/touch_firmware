/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include "com_global.h"

void knl_touch_update_self_baseline(U16 *s_raw, U16 *s_base, U8 data_num, S8 quicktrace, S8 tracestep)
{
	U16 idx;
	S16 sig_temp;

	sig_temp = 0x0000;

	for (idx = 0; idx < data_num ; idx++)
	{
		sig_temp = ( (S16)(*(s_raw+idx)) - (*(s_base+idx)) );

		if (sig_temp < (-quicktrace))	// quick follow
		{
			(*(s_base+idx)) -= quicktrace;
		}
		else if (sig_temp > tracestep)
		{
			(*(s_base+idx)) += tracestep;		// step up
		}
		else if (sig_temp < (-tracestep))
		{
			(*(s_base+idx)) -= tracestep;
		}		
	}
}



void knl_touch_update_mutual_baseline(U16 *m_raw, U16 *m_base, U16 node, S8 tracestep)
{
	U16 idx;
	U8 idx_tx,idx_rx;
	S16 sig_temp;
	U8 rxtmp,txtmp,totalrx;

	sig_temp = 0x0000;
	txtmp = TxViewAreaPortNum;
	rxtmp = RxViewAreaPortNum;
	totalrx = RxNum;

	for ( idx_tx = 0 ; idx_tx < txtmp ; idx_tx++ )
	{
		for ( idx_rx = 0 ; idx_rx < rxtmp ; idx_rx++ )
		{
			idx = idx_tx * totalrx + idx_rx;

			sig_temp = ( (S16)(*(m_raw+idx)) - (*(m_base+idx)) );
			if (sig_temp > tracestep)
			{
				(*(m_base+idx)) += tracestep;		// step up
			}
			else if (sig_temp < (-tracestep))
			{
				(*(m_base+idx)) -= tracestep;
			}
		}
	}
}


void knl_touch_area_merge(RANGE_BUFFER *p_dst,RX_RANGE *p_inform,U8 curr_tx)
{
    U8 i,j;
    U8 match_cnt,match_idx[7];
                                   
    if(((*p_dst).SetFlag[0] == (*p_dst).ClosedFlag[0]) && ((*p_dst).SetFlag[1] == (*p_dst).ClosedFlag[1])) //no range queues are set or all range queues are closed
    {
        //find empty range queue and set it
        for(i=0; i<(*p_inform).Num; i++)
        {
            for(j=0; j<KNL_MAX_POINT_NUM; j++)
            {
                if(((*p_dst).SetFlag[j>>0x04] & (0x0001 << (j&0x0F))) == 0)
                {
                    (*p_dst).TxStart[j] = curr_tx;
                    (*p_dst).TxEnd[j] = curr_tx;
                    (*p_dst).RxStart[j] = (*p_inform).Start[i];
                    (*p_dst).RxEnd[j] = (*p_inform).End[i];
                    (*p_dst).SetFlag[j>>0x04] |= (0x0001 << (j&0x0F));    
                    break;
                }
            }
        }
    }
    else
    {
        for(i=0; i<(*p_inform).Num; i++)
        {
            match_cnt = 0x00;

            for(j=0; j<KNL_MAX_POINT_NUM; j++)
            {
                if(((*p_dst).SetFlag[j>>0x04]^(*p_dst).ClosedFlag[j>>0x04]) & (0x0001 << (j&0x0F)))
                {                                   
                    if(((*p_inform).Start[i] <= (*p_dst).RxEnd[j]) && ((*p_inform).End[i] >= (*p_dst).RxStart[j]))
                    {
                        if((*p_inform).Start[i] < (*p_dst).RxStart[j])
                            (*p_dst).RxStart[j] = (*p_inform).Start[i]; 
                                    
                        if((*p_inform).End[i] > (*p_dst).RxEnd[j])
                            (*p_dst).RxEnd[j] = (*p_inform).End[i];
                                                                                
                        (*p_dst).TxEnd[j] = curr_tx;
                        
                        if(match_cnt < 0x07)
                        {        
                            match_idx[match_cnt] = j;
                            match_cnt++;   
                        }        
                    }    
                }
            } 
                    
            if(match_cnt == 0x00)   //no match, open new RangeBuffer
            {
                for(j=0; j<KNL_MAX_POINT_NUM; j++)
                {
                    if(((*p_dst).SetFlag[j>>0x04] & (0x0001 << (j&0x0F))) == 0)
                    {
                        (*p_dst).TxStart[j] = curr_tx;
                        (*p_dst).TxEnd[j] = curr_tx;
                        (*p_dst).RxStart[j] = (*p_inform).Start[i];
                        (*p_dst).RxEnd[j] = (*p_inform).End[i];
                        (*p_dst).SetFlag[j>>0x04] |= (0x0001 << (j&0x0F));
                        break;                                
                    }
                }
            }
            else if((match_cnt > 0x01) && (match_cnt < 0x07))
            {
                //get rx_boundary  			
                for(j=1; j<match_cnt; j++)
                {
                    if((*p_dst).RxStart[match_idx[j]] < (*p_dst).RxStart[match_idx[0]])
                        (*p_dst).RxStart[match_idx[0]] = (*p_dst).RxStart[match_idx[j]];
                            
                    if((*p_dst).RxEnd[match_idx[j]] > (*p_dst).RxEnd[match_idx[0]])
                        (*p_dst).RxEnd[match_idx[0]] = (*p_dst).RxEnd[match_idx[j]];
                                
                    if((*p_dst).TxStart[match_idx[j]] < (*p_dst).TxStart[match_idx[0]])
                        (*p_dst).TxStart[match_idx[0]] = (*p_dst).TxStart[match_idx[j]];
                                                            
                    (*p_dst).SetFlag[match_idx[j]>>0x04] &= ~(0x0001 << (match_idx[j]&0x0F));  
                }
                       
                (*p_dst).TxEnd[match_idx[0]] = curr_tx;      
            }                   
        }

        for(i=0; i<KNL_MAX_POINT_NUM; i++)
        {
            if(((*p_dst).SetFlag[i>>0x04]^(*p_dst).ClosedFlag[i>>0x04]) & (0x0001 << (i&0x0F)))
            {
                if((*p_dst).TxEnd[i] != curr_tx)    //closed
                {
                    (*p_dst).ClosedFlag[i>>0x04] |= (0x0001 << (i&0x0F));
                }    
            }
        }
    }      
} 

U8 knl_touch_get_peak_number(S16 threshold,S16 *p_sig,S16 *p_local_max,U8 tx_low,U8 tx_high,U8 rx_low,U8 rx_high)
{
    U8 tx_idx,rx_idx,count,totalrx;
    U8 status,t_count,p_count;
    U16 tx_start_idx;
    vS16 sig_target;
    
    p_count = 0x00;
    status = 0x00;
    totalrx = RxNum;
    
    for(tx_idx=tx_low;tx_idx<=tx_high;tx_idx++)
    {
        tx_start_idx = (tx_idx*totalrx);
        
        for(rx_idx=rx_low;rx_idx<=rx_high;rx_idx++)
        {
            sig_target = (*(p_sig+tx_start_idx+rx_idx));
                       
            if(sig_target > (*p_local_max))
            {
                (*p_local_max) = sig_target;
            }
      
            if((sig_target < threshold) || (status & 0x08))
            {
                status &= ~0x08;
                continue;
            }
            
            status &= 0x0F;
            t_count = 0x00;
               
            if(tx_idx == tx_low)
            {
                status |= KNL_MUTUAL_TX_LITTLE_BOUNDARY; 
                t_count++;
            }
            else if(tx_idx == tx_high)
            {
                status |= KNL_MUTUAL_TX_LARGE_BOUNDARY;
                t_count++;
            }
            
            if(rx_idx == rx_low)
            {
                status |= KNL_MUTUAL_RX_LITTLE_BOUNDARY;
                t_count++;
            }
            else if(rx_idx == rx_high)
            {
                status |= KNL_MUTUAL_RX_LARGE_BOUNDARY;
                t_count++;
            }
            
            if(t_count == 0x00)
            {
                t_count = 0x08;
            }
            else if(t_count == 0x01)
            {
                t_count = 0x05;
            }
            else if(t_count == 0x02)
            {
                t_count = 0x03;
            }
                        
            count = 0x00;
            
            if((status & (KNL_MUTUAL_TX_LITTLE_BOUNDARY+KNL_MUTUAL_RX_LITTLE_BOUNDARY)) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx-totalrx-1)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx-totalrx-1)))
                        (*(p_sig+tx_start_idx+rx_idx-totalrx-1))--;
                }      
            }
            
            if((status & KNL_MUTUAL_TX_LITTLE_BOUNDARY) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx-totalrx)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx-totalrx)))
                        (*(p_sig+tx_start_idx+rx_idx-totalrx))--;
                }
   
            }
            
            if((status & (KNL_MUTUAL_TX_LITTLE_BOUNDARY+KNL_MUTUAL_RX_LARGE_BOUNDARY)) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx-totalrx+1)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx-totalrx+1)))
                        (*(p_sig+tx_start_idx+rx_idx-totalrx+1))--;                    
                }
            }
            
            if((status & KNL_MUTUAL_RX_LITTLE_BOUNDARY) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx-1)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx-1)))
                        (*(p_sig+tx_start_idx+rx_idx-1))--;
                } 
            }
            
            if((status & KNL_MUTUAL_RX_LARGE_BOUNDARY) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx+1)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx+1)))
                        (*(p_sig+tx_start_idx+rx_idx+1))--;                    
                }
            }
            
            if((status & (KNL_MUTUAL_TX_LARGE_BOUNDARY+KNL_MUTUAL_RX_LITTLE_BOUNDARY)) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx+totalrx-1)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx+totalrx-1)))
                        (*(p_sig+tx_start_idx+rx_idx+totalrx-1))--;           
                } 
            }
            
            if((status & KNL_MUTUAL_TX_LARGE_BOUNDARY) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx+totalrx)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx+totalrx)))
                        (*(p_sig+tx_start_idx+rx_idx+totalrx))--;
                }
            }
            
            if((status & (KNL_MUTUAL_TX_LARGE_BOUNDARY+KNL_MUTUAL_RX_LARGE_BOUNDARY)) == 0x00)
            {
                if(sig_target >= (*(p_sig+tx_start_idx+rx_idx+totalrx+1)))
                {
                    count++;
                    
                    if(sig_target == (*(p_sig+tx_start_idx+rx_idx+totalrx+1)))
                        (*(p_sig+tx_start_idx+rx_idx+totalrx+1))--;
                }
            }
            
            
            if(count == t_count)
            {
                p_count++;
                
                if((status & KNL_MUTUAL_RX_LARGE_BOUNDARY) == 0x00)
                    status |= 0x08;
            }
               
        }
    }
        
    return p_count;
}

U8 knl_touch_set_rx_range(RX_RANGE *p_inform,S16 *p_sig,U16 start_idx,S16 threshold,U8 start,U8 end)
{

    U8 idx,state,num;

    state = 0x00;
    num = 0x00;

    for(idx=start; idx<=end; idx++)
    {
        switch(state)
        {         
            case 0x00:
            if((*(p_sig+start_idx+idx)) >= threshold)
            {
                (*p_inform).Start[num] = idx;
                (*p_inform).End[num] = idx;
                state = 0x01;

                if(idx == end)
                    num++;
            }
            break;

            case 0x01:
            if((*(p_sig+start_idx+idx)) >= threshold)
            {
                (*p_inform).End[num] = idx;

                if(idx == end)
                    num++;
            }
			
            else
            {
                num++;
                state = 0x00;
            }
            break;
        }
    }

    return num;

}



void knl_touch_get_pre_point_range(RANGE_BUFFER *p_out,RX_RANGE *p_inform,S16 *p_sig,S16 threshold)
{
    U8 tx_idx,totalrx;
    U16 tx_start_idx;

    (*p_out).SetFlag[0] = 0x0000;
    (*p_out).SetFlag[1] = 0x0000;
    (*p_out).ClosedFlag[0] = 0x0000;
    (*p_out).ClosedFlag[1] = 0x0000;
	totalrx = RxNum;

    for(tx_idx = 0x00; tx_idx<TxViewAreaPortNum; tx_idx++)
    {
        //Search Range: assigned Tx and total Rx
        tx_start_idx = (tx_idx*totalrx);
        (*p_inform).Num = knl_touch_set_rx_range(p_inform,p_sig,tx_start_idx,threshold,0x00,RxViewAreaMaxPort);//Rx Peak Number
            
        if((*p_inform).Num != 0x00)
        {
            knl_touch_area_merge(p_out,p_inform,tx_idx);             
        }
        else
        {
            //close all current point
            (*p_out).ClosedFlag[0] = (*p_out).SetFlag[0];
            (*p_out).ClosedFlag[1] = (*p_out).SetFlag[1];
        }

        if(tx_idx == TxViewAreaMaxPort)
        {
            (*p_out).ClosedFlag[0] = (*p_out).SetFlag[0];
            (*p_out).ClosedFlag[1] = (*p_out).SetFlag[1];
        }    
    }
}



void knl_touch_get_post_point_range(RANGE_BUFFER *p_out,RANGE_BUFFER *p_in,RX_RANGE *p_inform,S16 *p_sig,S16 peak_th,U8 area_th)
{
    U8 idx,area_count;
    U8 tx_idx,rx_idx,peak_num,totalrx;
    U16 tx_start_idx;
	U8 mergeratio_h,mergeratio_l;
#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING
	U8 BigNodeTHR;
	BigNodeTHR = FwSetting.BigThumbNodeTHR;
#endif
	mergeratio_h = (FwSetting.AreaMergeRatioBit & 0xF0) >> 4;
	mergeratio_l = (FwSetting.AreaMergeRatioBit & 0x0F);
	totalrx = RxNum;

    (*p_out).SetFlag[0] = 0x0000;
    (*p_out).SetFlag[1] = 0x0000; 
    (*p_out).ClosedFlag[0] = 0x0000;
    (*p_out).ClosedFlag[1] = 0x0000;

    for(idx=0x00; idx<KNL_MAX_POINT_NUM; idx++)
    {
        if((*p_in).ClosedFlag[idx>>0x04] & (0x0001 << (idx&0x0F)))
        { 
             //re-sharp the point_range
            area_count = 0x00;
            peak_num = 0x00;
            //get local maximun delta
            for(tx_idx=(*p_in).TxStart[idx]; tx_idx<=(*p_in).TxEnd[idx]; tx_idx++)
            {
                tx_start_idx = (tx_idx*totalrx);
                
                for(rx_idx=(*p_in).RxStart[idx]; rx_idx<=(*p_in).RxEnd[idx]; rx_idx++)
                {                                
                    if((*(p_sig+tx_start_idx+rx_idx)) > FwSetting.MutualPalmTh)
                        area_count++;
                }                    
            }
#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING
			if (area_count >= BigNodeTHR)
			{
				BigThumb = 1;		// switch to "BigThumb" mode
			}
#endif
	   
            if((area_count > area_th) || ((((*p_in).TxEnd[idx]-(*p_in).TxStart[idx]+1)>FwSetting.PalmTxRange)&&(((*p_in).RxEnd[idx]-(*p_in).RxStart[idx]+1)>FwSetting.PalmRxRange)) )
                continue;
            
            SigLocalMax = 0x0000;
            peak_num = knl_touch_get_peak_number(peak_th,p_sig,(&SigLocalMax),(*p_in).TxStart[idx],(*p_in).TxEnd[idx],(*p_in).RxStart[idx],(*p_in).RxEnd[idx]);

            if(peak_num == 0x01)
            {
                for(tx_idx=0x00;tx_idx<KNL_MAX_POINT_NUM;tx_idx++)
                {
                    if(((*p_out).ClosedFlag[tx_idx>>0x04] & (0x0001 << (tx_idx&0x0F))) == 0x0000)
                    {
                        (*p_out).TxStart[tx_idx] = (*p_in).TxStart[idx];
                        (*p_out).TxEnd[tx_idx] = (*p_in).TxEnd[idx];
                        (*p_out).RxStart[tx_idx] = (*p_in).RxStart[idx];
                        (*p_out).RxEnd[tx_idx] = (*p_in).RxEnd[idx]; 
                        (*p_out).SetFlag[tx_idx>>0x04] |=  (0x0001 << (tx_idx&0x0F));   
                        (*p_out).ClosedFlag[0] = (*p_out).SetFlag[0];
                        (*p_out).ClosedFlag[1] = (*p_out).SetFlag[1];
                        break;
                    }
                }
            }
            else if(peak_num > 0x01)
            {                              				
                for(tx_idx=(*p_in).TxStart[idx]; tx_idx<=(*p_in).TxEnd[idx]; tx_idx++)
                {
                    tx_start_idx = (tx_idx*totalrx);
                    
                    (*p_inform).Num = knl_touch_set_rx_range(p_inform,p_sig,tx_start_idx,((SigLocalMax * mergeratio_h)/mergeratio_l),(*p_in).RxStart[idx],(*p_in).RxEnd[idx]);                 
                    
                    if((*p_inform).Num != 0x00)
                    {                                 
                        knl_touch_area_merge(p_out,p_inform,tx_idx);                     
                    }
                    else
                    {
                        (*p_out).ClosedFlag[0] = (*p_out).SetFlag[0];
                        (*p_out).ClosedFlag[1] = (*p_out).SetFlag[1];
                    }
                        
                    if(tx_idx == (*p_in).TxEnd[idx]) 
                    {
                        (*p_out).ClosedFlag[0] = (*p_out).SetFlag[0];
                        (*p_out).ClosedFlag[1] = (*p_out).SetFlag[1];
                    }   
                }  
            }          
        }                                
    }
}



U8 knl_touch_mutual_set_signal(RANGE_BUFFER *p_in,S16 peak_th,S16 *p_sig_src,S16 *p_tx_dst,S16 *p_rx_dst,U8 index,U8 *tx_num,U8 *rx_num,S16 min_th)
{
    U8 tx_idx,rx_idx,rx_peak,tx_peak,totalrx;
    U16 tx_start_idx;
    S16 local_max;
    
    local_max = 0x0000;
    tx_peak = 0x00;
    rx_peak = 0x00;
	totalrx = RxNum;

    for(tx_idx=(*p_in).TxStart[index]; tx_idx<=(*p_in).TxEnd[index]; tx_idx++)
    {
        tx_start_idx = (tx_idx*totalrx);
        
        for(rx_idx=(*p_in).RxStart[index]; rx_idx<=(*p_in).RxEnd[index]; rx_idx++)
        {
            if((*(p_sig_src+tx_start_idx+rx_idx)) > local_max)
            {
                local_max = (*(p_sig_src+tx_start_idx+rx_idx));
                tx_peak = tx_idx;
                rx_peak = rx_idx;
            }
        }
    }
    
//    if((tx_peak == 0x00) || (tx_peak == TxViewAreaMaxPort) || (rx_peak == 0x00) || (rx_peak == RxViewAreaMaxPort))
//        peak_th <<= 0x01;
    
    if(local_max < peak_th)
        return 0xFF;
    
    //expand tx range if needed
    if((tx_peak == (*p_in).TxStart[index]) && ((*p_in).TxStart[index] != 0x00))
        (*p_in).TxStart[index]--;  
    
    if((tx_peak == (*p_in).TxEnd[index]) && ((*p_in).TxEnd[index] != TxViewAreaMaxPort))
        (*p_in).TxEnd[index]++; 
    
    //expand rx range if needed
    if((rx_peak == (*p_in).RxStart[index]) && ((*p_in).RxStart[index] != 0x00))
        (*p_in).RxStart[index]--;  
    
    if((rx_peak == (*p_in).RxEnd[index]) && ((*p_in).RxEnd[index] != RxViewAreaMaxPort))
        (*p_in).RxEnd[index]++; 
    
    //initial signal buffer
    (*tx_num) = ((*p_in).TxEnd[index] - (*p_in).TxStart[index] + 0x01);
    (*rx_num) = ((*p_in).RxEnd[index] - (*p_in).RxStart[index] + 0x01);
    
    for(tx_idx=0x00; tx_idx<(*tx_num); tx_idx++)
    {
        (*(p_tx_dst+tx_idx)) = 0x0000;    
    }
    
    for(rx_idx = 0x00; rx_idx<(*rx_num); rx_idx++)
    {
        (*(p_rx_dst+rx_idx)) = 0x0000;    
    }

    for(tx_idx=0x00; tx_idx<(*tx_num); tx_idx++)
    {    
        tx_start_idx = (((*p_in).TxStart[index]+tx_idx)*totalrx);
        
        for(rx_idx=0x00; rx_idx<(*rx_num); rx_idx++)
        {
            if((*(p_sig_src+tx_start_idx+(*p_in).RxStart[index]+rx_idx)) > min_th)
            {
                (*(p_rx_dst+rx_idx)) += (*(p_sig_src+tx_start_idx+(*p_in).RxStart[index]+rx_idx));
                (*(p_tx_dst+tx_idx)) += (*(p_sig_src+tx_start_idx+(*p_in).RxStart[index]+rx_idx));
            }
        }
    }
    
    return index;
}

#if SELF_ASSISTANCE
//knl_touch_self_set_signal((&SigFrame.Msig.Mutual[0]),SelfPeakTx[0],SelfPeakRx[0],(&TxSigBuffer[0]),(&RxSigBuffer[0]),SelfTxAddrNum,SelfRxAddrNum);//
void knl_touch_self_set_signal(S16 *p_sig_src,U8 txpeak, U8 rxpeak, S16 *p_tx_dst,S16 *p_rx_dst,U8 tx_num,U8 rx_num)
{
	U8 txidx,rxidx,totalrx;
	U16 tx_start_node;
	totalrx = RxNum;

	for(txidx=0x00; txidx<tx_num; txidx++)
	{
		(*(p_tx_dst+txidx)) = 0x0000;    
	}

	for(rxidx = 0x00; rxidx<rx_num; rxidx++)
	{
		(*(p_rx_dst+rxidx)) = 0x0000;    
	}

	for ( txidx = 0 ; txidx < tx_num ; txidx++ )
	{
		tx_start_node = ((PointRange.TxStart[0] + txidx) * totalrx);
//		printf("tx_start:%3d  mutu-d:%4d  \n\r",tx_start_node,mutemp);
		
		for ( rxidx = 0 ; rxidx < rx_num ; rxidx++ )
		{
            (*(p_rx_dst+rxidx)) += (*(p_sig_src+tx_start_node+PointRange.RxStart[0]+rxidx));
            (*(p_tx_dst+txidx)) += (*(p_sig_src+tx_start_node+PointRange.RxStart[0]+rxidx));//Peak delta
//				printf("[%3d]:%3d = %4d  \n\r ",rxidx,(tx_start_node+PointRange.RxStart[0]+rxidx),(*(p_sig_src+tx_start_node+PointRange.RxStart[0]+rxidx)) );
		}
	}
//		printf("MU:%4d node:%3d  ---  PR_S:[%2d,%2d]   tx:[0]%3d,tx:[1]%3d,tx:[2]%3d  rx[0]:%3d,rx[1]:%3d,rx[2]:%3d  \n\r",mutemp,pknode,PointRange.TxStart[0],PointRange.RxStart[0],(*(p_tx_dst+0)),(*(p_tx_dst+1)),(*(p_tx_dst+2)),(*(p_rx_dst+0)),(*(p_rx_dst+1)),(*(p_rx_dst+2)));
}
#endif


U8 knl_touch_point_merge(POINT_LIST *point_src,U8 min_dtxdrx)
{
    U8 idx0,idx1,pass_flag;
    S16 tx_diff,rx_diff;
    
    idx0 = 0x00;
  
    while(1)
    {
        pass_flag = 0x80;
        //get compare source index
        if((*point_src).CurrSetFlag[(idx0>>0x04)] & (0x0001 << (idx0&0x0F)))
        {
            for(idx1=(idx0+0x01); idx1<KNL_MAX_POINT_NUM; idx1++)
            {
                if((*point_src).CurrSetFlag[(idx1>>0x04)] & (0x0001<<idx1))
                {
                    tx_diff = ((*point_src).CurrPos[TX_IDX][idx0] - (*point_src).CurrPos[TX_IDX][idx1]);
                    
                    if(tx_diff < 0x0000)
                        tx_diff = (0x0000 - tx_diff);
                    
                    rx_diff = ((*point_src).CurrPos[RX_IDX][idx0] - (*point_src).CurrPos[RX_IDX][idx1]);
                    
                    if(rx_diff < 0x0000)
                        rx_diff = (0x0000 - rx_diff); 
                    
                    if((tx_diff+rx_diff) < min_dtxdrx)
                    {
                        (*point_src).CurrPos[TX_IDX][idx0] = (((*point_src).CurrPos[TX_IDX][idx0] + (*point_src).CurrPos[TX_IDX][idx1]) >> 0x01); 
                        (*point_src).CurrPos[RX_IDX][idx0] = (((*point_src).CurrPos[RX_IDX][idx0] + (*point_src).CurrPos[RX_IDX][idx1]) >> 0x01);
                        (*point_src).CurrSetFlag[idx1>>0x04] &= ~(0x0001<<(idx1&0x0F));
                        pass_flag = 0xFF;                           
                    }               
                }
            }
        }
      
        if(pass_flag == 0x80)
        {
            idx0++;
            
            if(idx0 == KNL_MAX_POINT_NUM)
                break;
        }
    }
    
    idx0 = 0x00;
    
    for(idx1=0x00; idx1<KNL_MAX_POINT_NUM; idx1++)
    {
        if((*point_src).CurrSetFlag[idx1>>0x04] & (0x0001<<(idx1&0x0F)))
        {
            idx0++;
        }
    }
   return idx0;
}
//
knl_touch_cal_position(TxViewAreaMaxPort,PointRange.TxStart[0],SelfTxAddrNum,FwSetting.TxInterpolationNum,(&TxSigBuffer[0]),TX_IDX);
U16 knl_touch_cal_position(U8 max_port,U8 s_port,U8 range,U8 weight,S16 *sig_ptr, U8 axis)
{
    U8 i;
    U16 sig_sum;
    U32 sig_axis_sum;
    U16 pos;
	S16 sigtemp;

    sig_sum = 0x0000;
    sig_axis_sum = 0x00000000;

    for(i=0; i < range; i++)
    {
#if 1
		sigtemp = (*(sig_ptr+i));//Delta
		if (sigtemp < 0)
			sigtemp = 0;
        if(sigtemp > 0)
        {
            sigtemp >>= DELTA_DENOISE_BIT; 
        }
		sig_sum += sigtemp;
		sig_axis_sum += (sigtemp * i);
#else
		sig_sum += (U16)(*(sig_ptr+i));
		sig_axis_sum += (U16)((*(sig_ptr+i))*i);
#endif
    }
    
    if(sig_sum == 0x0000)
    {
        return 0xFFFF;
    }
    pos = ((sig_axis_sum * weight)/sig_sum);
    pos += (s_port * weight);//weight is equal to 64,sport is equal to TX channel number

#if !WHOLE_VA_EXTEND
	pos += (weight/2);					// because of weight-sum rule

	#if SET__DISABLE__TO_GET_SHORTAGE	// disable to gain the default shortage
	if (axis == TX_IDX)
	{
		if (pos < TxMinEdgeExtendCH)
		{
			pos = (((pos-FwSetting.TxStartShortage) * TxScalingNumrator)/TxStartScalingDeno);
		}
		else if (pos >= TxMaxEdgeExtendCH)
		{
			pos = TxMaxEdgeExtendCH + ((pos - TxMaxEdgeExtendCH)*TxScalingNumrator/TxEndScalingDeno);
		}
	}
	else if (axis == RX_IDX)
	{
		if (pos < RxMinEdgeExtendCH)
		{
			pos = (((pos-FwSetting.RxStartShortage) * RxScalingNumrator)/RxStartScalingDeno);
		}
		else if (pos >= RxMaxEdgeExtendCH)
		{
			pos = RxMaxEdgeExtendCH + ((pos - RxMaxEdgeExtendCH)*RxScalingNumrator/RxEndScalingDeno);
		}
	}
	#endif
#endif

    return pos;
}


void knl_touch_position_scaling(POINT_LIST *p_in,U8 tx_port_num,U8 rx_port_num)
{
    U8 idx;
    U16 bit_n;
    U32 tmp_position;
	U16 txscaling,rxscaling;

	txscaling = TxScalingParameter;
	rxscaling = RxScalingParameter;
   
    for(idx=0x00; idx<KNL_MAX_POINT_NUM; idx++)
    {
        bit_n = (0x0001 << (idx&0x0F));
        
        if((*p_in).CurrSetFlag[idx>>0x04] & bit_n)
        {
            //tx
            tmp_position = (*p_in).CurrPos[TX_IDX][idx];
            tmp_position *= txscaling;
            
            if(tmp_position & 0x300)
                tmp_position += 0x400;
                
            (*p_in).CurrPos[TX_IDX][idx] = ((U16)(tmp_position >>= 0x0A));  

            //rx                  
            tmp_position = (*p_in).CurrPos[RX_IDX][idx]; 
            tmp_position *= rxscaling;
            
            if(tmp_position & 0x300)
                tmp_position += 0x400;
                
            (*p_in).CurrPos[RX_IDX][idx] = ((U16)(tmp_position >>= 0x0A)); 
        }
        
    }
}

void knl_touch_position_remapping(POINT_LIST *p_in)
{
    U8 idx;
    U16 bit_n;
	
    for(idx=0x00; idx<KNL_MAX_POINT_NUM; idx++)
    {
		bit_n = (0x0001 << (idx&0x0F));
		if((*p_in).CurrSetFlag[idx>>0x04] & bit_n)
		{
			if(FwSetting.PosMappingFunction & bTX_POSITION_REVERSE)//Tx REVERSE Y pos
				(*p_in).CurrPos[TX_IDX][idx] = (FwSetting.TxTargetResolution - (*p_in).CurrPos[TX_IDX][idx] - 1);

			if(FwSetting.PosMappingFunction & bRX_POSITION_REVERSE)
				(*p_in).CurrPos[RX_IDX][idx] = (FwSetting.RxTargetResolution - (*p_in).CurrPos[RX_IDX][idx] - 1);
		}
    }
}


U16 knl_touch_get_distance(vS16 diff_x, vS16 diff_y)
{

    if(diff_x < 0x0000)
    {
        diff_x = (0x0000 - diff_x);
    }

    if(diff_y < 0x0000)
    {
        diff_y = (0x0000 - diff_y);    
    }
    
    return (diff_x + diff_y);
    
}

void knl_touch_points_tracking(POINT_LIST *p_list,TRACKING_BUFFER *p_track_buff,U16 min_distance,U8 point_num)
{
    U8 k,temp_c,temp_p,match_cnt,index0,index1;
    U16 j,mask_p,match_distance;
    S16 tx_offset,rx_offset;
    U32 i,mask_c,temp;

    match_distance = min_distance;
    //if there was no fingers before, send current finger position directly 
    if((*p_list).SetFlag == 0x0000)
    {
        index0 = 0x00;
        
        for(index1=0x00; index1<KNL_MAX_FINGER_NUM; index1++)
        {
            if((*p_list).CurrSetFlag[index1>>0x04] & (0x0001 << (index1&0x0F)))
            {
                (*p_list).QueuePos[RX_IDX][index0] = (*p_list).CurrPos[RX_IDX][index1]; 
                (*p_list).QueuePos[TX_IDX][index0] = (*p_list).CurrPos[TX_IDX][index1];
                (*p_list).SetFlag |= (0x0001 << index0);
                index0++;
            }
        }
        (*p_list).LegalFlag = (*p_list).SetFlag;
        return;
    }

    temp_c = 0x00;
    temp_p = 0x00;
    
	//calculate the couple number
    for(index0=0x00; index0<KNL_MAX_POINT_NUM; index0++)
    {
        if((*p_list).CurrSetFlag[index0>>0x04] & (0x0001 << (index0&0x0F)))
        {
            temp_p = 0x00;
            
            for(index1=0x00; index1<KNL_MAX_FINGER_NUM; index1++)
            {
                if((*p_list).SetFlag & (0x0001 << index1))
                {
                    (*(p_track_buff+temp_c)).P0.Curr = index0;
                    (*(p_track_buff+temp_c)).P0.Pre = index1;

                    tx_offset = ((*p_list).CurrPos[TX_IDX][index0]-(*p_list).QueuePos[TX_IDX][index1]);
                    rx_offset = ((*p_list).CurrPos[RX_IDX][index0]-(*p_list).QueuePos[RX_IDX][index1]);

                    (*(p_track_buff+temp_c)).P0.Distance = knl_touch_get_distance(rx_offset,tx_offset);
                    temp_c++;   //calculate total set number
                    temp_p++;   //calculate TempPosSet number, the maximum value of the temp_p is equal to KNL_MAX_FINGER_NUM
                }         
            }
        }
    } 
	

    if(temp_c == 0x01)  //if total number == 1
    {
        if(PointStartMove & (0x0001<<(*p_track_buff).P0.Pre))
        {
            match_distance = 0x7FFF;
        }
        else
        {
            match_distance = min_distance;
        }
        
        if((*p_track_buff).P0.Distance < match_distance)
        {   
            (*p_list).QueuePos[RX_IDX][(*p_track_buff).P0.Pre] = (*p_list).CurrPos[RX_IDX][(*p_track_buff).P0.Curr];
            (*p_list).QueuePos[TX_IDX][(*p_track_buff).P0.Pre] = (*p_list).CurrPos[TX_IDX][(*p_track_buff).P0.Curr];
            (*p_list).LegalFlag |= (0x0001<<(*p_track_buff).P0.Pre);	
        }
        else
        {	
            for(index0=0x00; index0<KNL_MAX_FINGER_NUM; index0++)
            {
                j = (0x0001<<index0);
                
                if(((*p_list).SetFlag & j) == 0x0000)
                {
                    (*p_list).QueuePos[RX_IDX][index0] = (*p_list).CurrPos[RX_IDX][(*p_track_buff).P0.Curr];
                    (*p_list).QueuePos[TX_IDX][index0] = (*p_list).CurrPos[TX_IDX][(*p_track_buff).P0.Curr];
                    (*p_list).LegalFlag |= j;
                    (*p_list).SetFlag |= j;
                    break;                    
                }
            }   
        }
    }
    else
    {
        if(point_num < temp_p)
        {
            match_cnt = point_num;
        }
        else
        {
            match_cnt = temp_p;
        }
  
        temp_p = 0x00;
        mask_c = 0xFFFFFFFF;
        mask_p = (*p_list).SetFlag;
        
        index0 = 0x00;
        index1 = 0x00;
        
        k = 0x00;
     
        while(1)
        {
            for(; k<temp_c; k++)
            {
                // let min distance on top
                if((*(p_track_buff+k)).P0.Distance < (*(p_track_buff+index1)).P0.Distance)
                {
                    index1 = k;
                }
            }
            //swap data
            if(index0 != index1)
            {
                temp = (*(p_track_buff+index1)).PointData;
                (*(p_track_buff+index1)).PointData = (*(p_track_buff+index0)).PointData;
                (*(p_track_buff+index0)).PointData = temp;
            }

            i = (0x00000001 << ((*(p_track_buff+index0)).P0.Curr));
            j = (0x0001 << ((*(p_track_buff+index0)).P0.Pre));
            
            if(PointStartMove & j)
            {
                match_distance = 0x7FFF;
            }
            else
            {
                match_distance = min_distance;
            }

            if((*(p_track_buff+index0)).P0.Distance < match_distance)
            {        
                if((mask_c & i) && (mask_p & j))	//matched the minimum distance
                {                 
                    mask_c &= ~i;
                    mask_p &= ~j;
                    (*p_list).LegalFlag |= j;                                                       
                    (*p_list).QueuePos[RX_IDX][(*(p_track_buff+index0)).P0.Pre] = (*p_list).CurrPos[RX_IDX][(*(p_track_buff+index0)).P0.Curr];
                    (*p_list).QueuePos[TX_IDX][(*(p_track_buff+index0)).P0.Pre] = (*p_list).CurrPos[TX_IDX][(*(p_track_buff+index0)).P0.Curr];       
                                
                    temp_p++; 						
                }
            }
            else
            {
                break;  //the minimum distance is larger than threshold, other point set to empty queue
            }

            
            if(temp_p == match_cnt)
                break;
                                             
            // from top + 1 search
            index0++;

            index1 = index0;
            k = index0;
        }
       
        // no match case
        if((point_num > match_cnt) || (temp_p != match_cnt))
        {
            for(k = 0; k<point_num; k++)
            {
                i = (0x00000001 << k);

                if(mask_c & i)
                {
                    //find no use pre
                    for(index0=0; index0<KNL_MAX_FINGER_NUM; index0++)
                    {
                        j = (0x0001 << index0);
                        
                        if(((*p_list).SetFlag & j) == 0x0000)
                        {
                            mask_c &= ~i;
                            (*p_list).QueuePos[RX_IDX][index0] = (*p_list).CurrPos[RX_IDX][k];
                            (*p_list).QueuePos[TX_IDX][index0] = (*p_list).CurrPos[TX_IDX][k];
                            (*p_list).SetFlag |= j;
                            (*p_list).LegalFlag |= j;                             
                            break;
                        }
                    }
                }
            }
        }
    }
}


void knl_touch_rotate_queue(POINT_OUTPUT *p_out, POINT_LIST *p_in)
{
    U8 idx;
    U16 bit_n;
	U8 RQ_Latency;

	RQ_Latency = FwSetting.RotateQueueLatencyFrame;
    
    (*p_out).PointNum = 0x00;
    
    for(idx=0x00; idx<KNL_MAX_FINGER_NUM; idx++)
    {
        bit_n = (0x0001 << idx);
        
        if((*p_in).LegalFlag & bit_n)
        {
            if( RotateQueue.RoQueue[idx] < RQ_Latency )
            {
                RotateQueue.RoQueue[idx]++;
                RotateQueue.TouchInfo &= ~bit_n;
                (*p_out).TouchInfo &= ~bit_n;
			}
			else	//( > ROTATE_QUEUE_LATANCY)
			{
				RotateQueue.RoQueue[idx] = 99;
				(*p_out).Position[TX_IDX][idx] = (*p_in).QueuePos[TX_IDX][idx];
				(*p_out).Position[RX_IDX][idx] = (*p_in).QueuePos[RX_IDX][idx];
				(*p_out).TouchInfo |= bit_n;
				(*p_out).PointNum++;
				RotateQueue.TouchInfo |= bit_n;
			}
        }
        else
        {
			RotateQueue.RoQueue[idx] = 0x00;
			RotateQueue.TouchInfo &= ~bit_n;
			(*p_out).TouchInfo &= ~bit_n;
			(*p_in).SetFlag &= ~bit_n;
        }   
    }
}


U16 wp_abs(U16 a0,U16 a1)
{
    U16 temp;
    
    if(a0 >= a1)
    {
        temp = (a0 - a1);
    }
    else
    {
        temp = (a1 - a0);
    }

    return temp;
}


void knl_touch_get_increment(U16 current, U16 *filter_pos, U16 alpha)
{
	U16 distance;

	distance = wp_abs( current,(*filter_pos) );
	distance = ( (U16)( ((U32)distance*alpha )>>8) );

	if (current > (*filter_pos))
		(*filter_pos) += distance;
	else
		(*filter_pos) -= distance;
}


//##############################################

void knl_touch_stepless_pointfilter(POINT_OUTPUT *p_out, FORCE_POINT_FILTER *point_filter)
{
	U8 idx;
	U16 bit_n,fast,slow;
	U16 tx_pos,rx_pos;
	U16 DistanceOrgFilter,DistanceOrgLastFinger;
	U16 alpha,jitter;

	jitter = FwSetting.JitterFrozen << 4;
	fast = FwSetting.SteplessFastFollowFilter;
	slow = FwSetting.SteplessSlowFollowFilter;
	
	for(idx=0x00; idx<KNL_MAX_FINGER_NUM; idx++)
	{
    	DistanceOrgLastFinger = 0x0000;
    	DistanceOrgFilter = 0x0000;
    	alpha = 0x0000;   
	    
		bit_n = (0x0001 << idx);
		
		if((*p_out).TouchInfo & bit_n)	// finger on
		{
			tx_pos = (*p_out).Position[TX_IDX][idx];
			rx_pos = (*p_out).Position[RX_IDX][idx];
			tx_pos <<= 4;
			rx_pos <<= 4;
			
			if( (*point_filter).PointEnterCounter[idx] >= 0x01)
			{
			    DistanceOrgFilter = wp_abs( (*point_filter).FilteredPosition[TX_IDX][idx],tx_pos ) + wp_abs((*point_filter).FilteredPosition[RX_IDX][idx],rx_pos);
    	    	DistanceOrgLastFinger = wp_abs( (*point_filter).LastFramePosition[TX_IDX][idx],tx_pos ) + wp_abs((*point_filter).LastFramePosition[RX_IDX][idx],rx_pos);
				(*point_filter).PointFrozen[idx] = 0x0;

				if (DistanceOrgFilter > RV_G_THDIFF)
				{
					alpha = fast;
				}
                else if (DistanceOrgFilter < jitter)
                {
                    (*point_filter).PointFrozen[idx] = 0x1;
                }

				else
				{
					alpha = ( ((U32)DistanceOrgFilter<<8) )/RV_G_THDIFF;
					if (alpha < slow)
					{
						alpha = slow;
					}
				}

				if (alpha > fast)
				{
					alpha = fast;
				}
				
				if ( DistanceOrgLastFinger < ((U16)(*point_filter).DistanceofLastFrame[idx]) )
				{
					alpha = (*point_filter).AlphaofLastFrame[idx];
				}
				else
				{
					(*point_filter).AlphaofLastFrame[idx] = alpha;
				}

				if((*point_filter).DistanceofLastFrame[idx] > (FwSetting.TappingDistance<<0x03))
				{
				    PointStartMove |= bit_n;    
				}

				(*point_filter).DistanceofLastFrame[idx] = DistanceOrgLastFinger;
				
                if ((*point_filter).PointFrozen[idx] == 0x1)
                {
                    // do not move
                }
                else
				{
					knl_touch_get_increment(tx_pos, (&point_filter->FilteredPosition[TX_IDX][idx]), alpha);
					knl_touch_get_increment(rx_pos, (&point_filter->FilteredPosition[RX_IDX][idx]), alpha);								
					(*point_filter).LastFramePosition[TX_IDX][idx] = tx_pos;
					(*point_filter).LastFramePosition[RX_IDX][idx] = rx_pos;
				}
				tx_pos = (*point_filter).FilteredPosition[TX_IDX][idx];
				rx_pos = (*point_filter).FilteredPosition[RX_IDX][idx];
			}

			if( (*point_filter).PointEnterCounter[idx] == 0x00)
			{
				(*point_filter).PointEnterCounter[idx] = 0x01;
				(*point_filter).LastFramePosition[TX_IDX][idx] = tx_pos;
				(*point_filter).LastFramePosition[RX_IDX][idx] = rx_pos;
				(*point_filter).FilteredPosition[TX_IDX][idx] = tx_pos;
				(*point_filter).FilteredPosition[RX_IDX][idx] = rx_pos;
			}

			// restore
			tx_pos >>= 4;
			rx_pos >>= 4;
			(*p_out).Position[TX_IDX][idx] = tx_pos;
			(*p_out).Position[RX_IDX][idx] = rx_pos;
		}
		else							// finger leave
		{
			(*point_filter).LastFramePosition[TX_IDX][idx] = 0xFFFF;
			(*point_filter).LastFramePosition[RX_IDX][idx] = 0xFFFF;
			(*point_filter).DistanceofLastFrame[idx] = 0x0000;
			(*point_filter).PointEnterCounter[idx] = 0x00;
			(*point_filter).AlphaofLastFrame[idx]= fast;
			(*point_filter).PointFrozen[idx] = 0x0;
			PointStartMove &= ~bit_n;
		}		
	}
}

#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING

void knl_touch_space_pointfilter(POINT_OUTPUT *p_out)
{
    U8 i;
    U8 j;
    U8 k;
    U16 bit_n;
    U16 tmp_tx;
    U16 tmp_rx;

//    static U8 FingerFrameCounter[KNL_MAX_FINGER_NUM];
//    static POINT_OUTPUT FingersPool[AVERAGE_LARGE_FRAME];


    for(i = 0; i < KNL_MAX_FINGER_NUM; i++)
    {
        bit_n = (0x0001<<i);

		if (((*p_out).TouchInfo & bit_n) && (BigThumb == 1) && (BigThumbFirstIn[i] == 0))
		{
			for(k = 0; k < AVERAGE_LARGE_FRAME; k++)		// copy to queue buffer
			{
				FingersPool[k].Position[TX_IDX][i] = p_out->Position[TX_IDX][i];
				FingersPool[k].Position[RX_IDX][i] = p_out->Position[RX_IDX][i];
			}
			BigThumbFirstIn[i] = 0x01;
		}
		else if (BigThumb == 0x00)
		{
			BigThumbFirstIn[i] = 0;
		}

        if (((*p_out).TouchInfo & bit_n) && (BigThumbFirstIn[i] == 1))
        {
            if(FingerFrameCounter[i] == 0)
            {
                for(j = 0 ; j < AVERAGE_LARGE_FRAME; j++)
                {
                    FingersPool[j].Position[TX_IDX][i] = p_out->Position[TX_IDX][i];
                    FingersPool[j].Position[RX_IDX][i] = p_out->Position[RX_IDX][i];
                }
            }
            else
            {
                FingersPool[FingerFrameCounter[i]%AVERAGE_LARGE_FRAME].Position[TX_IDX][i] = p_out->Position[TX_IDX][i];
                FingersPool[FingerFrameCounter[i]%AVERAGE_LARGE_FRAME].Position[RX_IDX][i] = p_out->Position[RX_IDX][i];

                tmp_tx = 0;
                tmp_rx = 0;
                for(k = 0; k < AVERAGE_LARGE_FRAME; k++)
                {
                    tmp_tx = (tmp_tx + FingersPool[k].Position[TX_IDX][i]);
                    tmp_rx = (tmp_rx + FingersPool[k].Position[RX_IDX][i]);
                }
				
                p_out->Position[TX_IDX][i] = (tmp_tx/AVERAGE_LARGE_FRAME);
                p_out->Position[RX_IDX][i] = (tmp_rx/AVERAGE_LARGE_FRAME);
            }

			FingerFrameCounter[i]++;
            if(FingerFrameCounter[i] == (AVERAGE_LARGE_FRAME + 1))
	        {
    	        FingerFrameCounter[i] = 1;
        	}
        }
        else
        {
            FingerFrameCounter[i] = 0;
        }
    }
}
#endif

// ####### M A I N.c Re-direct function #######
void _COORD_XY_LINEARITY_SMOOTHING(void)
{
	knl_touch_rotate_queue((&PointOutput),(&PointList));
	knl_touch_stepless_pointfilter((&PointOutput),(&enForcePointFilter));
#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING
	knl_touch_space_pointfilter((&PointOutput));
#endif

#if GESTURE_REMOTE_CONTROL
	knl_touch_finger_gesture((&PointOutput));
#endif
}

U8 _COORD_XY_ADDRESSING(U8 mflag, U8 sflag)
{
	U8 point_num,idx;
	S16	UsedFingerThreshold;

	UsedFingerThreshold = (PrePointNum == 0)? FwSetting.MutualFingerFirstTHR : MutualDynamicThr;
	point_num = 0x00;
	
	if ((mflag != 0) && (DoSelfReport == 0))//if touch finger not 1
	{
		knl_touch_get_pre_point_range((&PreRangeBuffer),(&RxRangeRecord),(&SigFrame.Msig.Mutual[0]),UsedFingerThreshold);
		knl_touch_get_post_point_range((&PointRange),(&PreRangeBuffer),(&RxRangeRecord),(&SigFrame.Msig.Mutual[0]),UsedFingerThreshold, FwSetting.PalmAreaNum);

		for(idx=0; idx<KNL_MAX_POINT_NUM; idx++)
		{						
			if(PointRange.ClosedFlag[idx>>0x04] & (0x0001 << (idx&0x0F)))
			{
				if(knl_touch_mutual_set_signal((&PointRange),UsedFingerThreshold,(&SigFrame.Msig.Mutual[0]),(&TxSigBuffer[0]),(&RxSigBuffer[0]),idx,(&TxPortNum),(&RxPortNum),FwSetting.MutualAreaTh) != 0xFF)
				{
					PointList.CurrPos[TX_IDX][point_num] = knl_touch_cal_position(TxViewAreaMaxPort,PointRange.TxStart[idx],TxPortNum,FwSetting.TxInterpolationNum,(&TxSigBuffer[0]),TX_IDX);
					PointList.CurrPos[RX_IDX][point_num] = knl_touch_cal_position(RxViewAreaMaxPort,PointRange.RxStart[idx],RxPortNum,FwSetting.RxInterpolationNum,(&RxSigBuffer[0]),RX_IDX);
					if((PointList.CurrPos[TX_IDX][point_num] != 0xFFFF) && (PointList.CurrPos[RX_IDX][point_num] != 0xFFFF))
					{
						PointList.CurrSetFlag[point_num>>0x04] |= (0x0001 << (point_num&0x0F));                            
						point_num++;
					}
				}                                                       										
			}
		}
		point_num = knl_touch_point_merge((&PointList),FwSetting.MinMergeDTxDRx);
	}
#if SELF_MODE_REPORT_FINGER
	else if (DoSelfReport)		// SELF Report only one finger touch TP
	{
		point_num = 0x00;
		PointRange.SetFlag[0] = 0x01;
		PointRange.SetFlag[1] = 0x00;
		PointRange.ClosedFlag[0] = 0x0001;
		PointRange.ClosedFlag[1] = 0x0000;

		if(SelfPeakTx[0] == 0)//Frirst Tx channel
		{
			PointRange.TxStart[0] = (SelfPeakTx[0]);
			PointRange.TxEnd[0] = (SelfPeakTx[0]+1);
			SelfTxAddrNum = 2;
		}
		else if(SelfPeakTx[0] == TxViewAreaMaxPort)//Last Tx channel
		{
			PointRange.TxStart[0] = (SelfPeakTx[0]-1);
			PointRange.TxEnd[0] = (SelfPeakTx[0]);
			SelfTxAddrNum = 2;
		}
		else if ( (SelfPeakTx[0] > 0) && (SelfPeakTx[0] < TxViewAreaMaxPort))//Center Tx channel
		{
			PointRange.TxStart[0] = (SelfPeakTx[0]-1);
			PointRange.TxEnd[0] = (SelfPeakTx[0]+1);
			SelfTxAddrNum = 3;
		}
		

		if(SelfPeakRx[0] == 0)//First Rx channel 
		{
			PointRange.RxStart[0] = (SelfPeakRx[0]);
			PointRange.RxEnd[0] = (SelfPeakRx[0]+1);
			SelfRxAddrNum = 2;
		}
		else if(SelfPeakRx[0] == RxViewAreaMaxPort)//Last Rx channel
		{
			PointRange.RxStart[0] = (SelfPeakRx[0]-1);
			PointRange.RxEnd[0] = (SelfPeakRx[0]);
			SelfRxAddrNum = 2;
		}
		else if ( (SelfPeakRx[0] > 0) && (SelfPeakRx[0] < RxViewAreaMaxPort) )//Center Rx channel
		{
			PointRange.RxStart[0] = (SelfPeakRx[0]-1);
			PointRange.RxEnd[0] = (SelfPeakRx[0]+1);
			SelfRxAddrNum = 3;
		}
		
		knl_touch_self_set_signal((&SigFrame.Msig.Mutual[0]),SelfPeakTx[0],SelfPeakRx[0],(&TxSigBuffer[0]),(&RxSigBuffer[0]),SelfTxAddrNum,SelfRxAddrNum);//
		//calc peak range delta
		PointList.CurrPos[TX_IDX][0] = knl_touch_cal_position(TxViewAreaMaxPort,PointRange.TxStart[0],SelfTxAddrNum,FwSetting.TxInterpolationNum,(&TxSigBuffer[0]),TX_IDX);
		PointList.CurrPos[RX_IDX][0] = knl_touch_cal_position(RxViewAreaMaxPort,PointRange.RxStart[0],SelfRxAddrNum,FwSetting.RxInterpolationNum,(&RxSigBuffer[0]),RX_IDX);
		if((PointList.CurrPos[TX_IDX][0] != 0xFFFF) && (PointList.CurrPos[RX_IDX][0] != 0xFFFF))
		{
			PointList.CurrSetFlag[point_num>>0x04] |= (0x0001 << (point_num&0x0F));                            
			point_num = 0x01;
		}
	}
#endif
	if(point_num > 0x00)
	{
	#if WHOLE_VA_EXTEND
		knl_touch_position_scaling((&PointList),TxViewAreaPortNum,RxViewAreaPortNum);
	#endif
		knl_touch_position_remapping((&PointList));
		knl_touch_points_tracking((&PointList),(&PointTrackingBuffer[0]), FwSetting.TappingDistance, point_num);
	}
#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING
	else
	{
		BigThumb = 0x00;		// wait no finger on, release!
	}
#endif

	return point_num;
}

void _SELF___UPDATE_BASELINE(U8 s_flag)
{
#if SELF_ASSISTANCE

	if (s_flag == 0)
	{
		knl_touch_update_self_baseline((&SelfRawDataBuffer[SelfReady][0]),(&BaseFrame.Sbase.Self[0]),SelfActualDataNum, FwSetting.SelfBaseTraceQuickStep, FwSetting.SelfBaseTraceStep);
		SelfBaseAbnormalCounter = 0;
	}
#endif
}


void _MUTUAL_UPDATE_BASELINE(U8 m_flag)
{
	if (m_flag == 0)
	{
		knl_touch_update_mutual_baseline((&MutualRawDataBuffer[MutualReady][0]),(&BaseFrame.Mbase.Mutual[0]),MutualActualDataNum,FwSetting.MutualBaseTraceStep);
		MutualBaseAbnormalCounter = 0;
	}
}
// ####### M A I N.c Re-direct function #######


