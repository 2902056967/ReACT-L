#ifndef __Cross__
#include "cross.h"
#endif

#include <map>
#include <string>
#include <queue>


TCross::TCross( int N )
{
  fMaxNumOfABcycle = 2000; /* Set an appropriate value (2000 is usually enough) */

  fN = N;
  tBestTmp.Define( fN );

  near_data = new int* [ fN ];
  for ( int j = 0; j < fN; ++j ) 
    near_data[j] = new int [ 5 ];

  fABcycle = new int* [ fMaxNumOfABcycle ];
  for ( int j = 0; j < fMaxNumOfABcycle; ++j ) 
    fABcycle[j] = new int [ 2*fN + 4 ];

  koritsu = new int [ fN ];
  bunki = new int [ fN ];
  kori_inv = new int [ fN ];
  bun_inv = new int [ fN ];
  check_koritsu = new int [ fN ];
  fRoute = new int [ 2*fN + 1 ];
  fPermu = new int [ fMaxNumOfABcycle ];

  fC = new int [ 2*fN+4 ];
  fJun = new int[ fN+ 1 ];
  fOrd1 = new int [ fN ];
  fOrd2 = new int [ fN ];

  // Speed Up Start
  fOrder = new int [ fN ];
  fInv = new int [ fN ];
  fSegment = new int* [ fN ];
  for ( int j = 0; j < fN; ++j ) 
    fSegment[ j ] = new int [ 2 ];
  fSegUnit = new int [ fN ]; 
  fSegPosiList = new int[ fN ];
  LinkAPosi = new int [ fN ];
  LinkBPosi = new int* [ fN ];
  for ( int j = 0; j < fN; ++j ) 
    LinkBPosi[ j ] = new int [ 2 ];
  fPosiSeg = new int [ fN ];
  fNumOfElementInUnit = new int [ fN ]; 
  fCenterUnit = new int [ fN ]; 
  for ( int j = 0; j < fN; ++j ) 
    fCenterUnit[ j ] = 0;
  fListOfCenterUnit = new int [ fN+2 ]; 
  fSegForCenter = new int [ fN ]; 
  fGainAB = new int [ fN ]; 
  fModiEdge = new int* [ fN ]; 				 
  for ( int j = 0; j < fN; ++j ) 
    fModiEdge[ j ] = new int [ 4 ]; 				 
  fBestModiEdge = new int* [ fN ]; 				 
  for ( int j = 0; j < fN; ++j ) 
    fBestModiEdge[ j ] = new int [ 4 ]; 				 
  fAppliedCylce = new int [ fN ];
  fBestAppliedCylce = new int [ fN ];
  // Speed Up End

  // Block2
  fNumOfElementINAB = new int [ fMaxNumOfABcycle ];
  fInEffectNode = new int* [ fN ];
  for( int i = 0; i < fN; ++i )
    fInEffectNode[ i ] = new int [ 2 ];
  fWeight_RR = new int* [ fMaxNumOfABcycle ];
  for( int i = 0; i < fMaxNumOfABcycle; ++i )
    fWeight_RR[ i ] = new int [ fMaxNumOfABcycle ];
  fWeight_SR = new int [ fMaxNumOfABcycle ];
  fWeight_C = new int [ fMaxNumOfABcycle ];
  fUsedAB = new int [ fN ];
  fMoved_AB = new int [ fN ];
  fABcycleInEset = new int [ fMaxNumOfABcycle ];
}

TCross::~TCross()
{
  delete [] koritsu;
  delete [] bunki;
  delete [] kori_inv;
  delete [] bun_inv;
  delete [] check_koritsu;
  delete [] fRoute;
  delete [] fPermu;

  for ( int j = 0; j < fN; ++j ) 
    delete[] near_data[ j ];
  delete[] near_data;

  for ( int j = 0; j < fMaxNumOfABcycle; ++j ) 
    delete[] fABcycle[ j ];
  delete[] fABcycle;

  delete [] fC;
  delete [] fJun; 
  delete [] fOrd1; 
  delete [] fOrd2; 


  // Speed Up Start
  delete [] fOrder;
  delete [] fInv;

  for ( int j = 0; j < fN; ++j ) 
    delete[] fSegment[ j ];
  delete[] fSegment;
  delete[] fSegUnit;
  delete [] fSegPosiList;
  delete [] LinkAPosi;
  for ( int j = 0; j < fN; ++j ) 
    delete[] LinkBPosi[ j ];
  delete [] LinkBPosi;
  delete [] fPosiSeg;
  delete [] fNumOfElementInUnit; 
  delete [] fCenterUnit;
  delete [] fListOfCenterUnit;
  delete [] fSegForCenter;
  delete [] fGainAB;

  for ( int j = 0; j < fN; ++j ) 
    delete[] fModiEdge[ j ];
  delete [] fModiEdge;
  for ( int j = 0; j < fN; ++j ) 
    delete[] fBestModiEdge[ j ];
  delete [] fBestModiEdge;
  delete [] fAppliedCylce;
  delete [] fBestAppliedCylce;
  // Speed Up End
  

  // Block2
  delete [] fNumOfElementINAB;
  for ( int j = 0; j < fN; ++j ) 
    delete [] fInEffectNode[ j ];
  delete [] fInEffectNode;
  for( int i = 0; i < fMaxNumOfABcycle; ++i )
    delete [] fWeight_RR[ i ];
  delete [] fWeight_SR;
  delete [] fWeight_C;
  delete [] fUsedAB;
  delete [] fMoved_AB;
  delete [] fABcycleInEset;
}

void TCross::SetParents( const TIndi& tPa1, const TIndi& tPa2, int flagC[ 10 ], int numOfKids )
{
  this->SetABcycle( tPa1, tPa2, flagC, numOfKids ); 

  fDis_AB = 0;   

  int curr, next, st, pre;
  st = 0;
  curr=-1;
  next = st;
  for( int i = 0; i < fN; ++i )
  {
    pre=curr;
    curr=next;
    if( tPa1.fLink[curr][0] != pre ) 
      next = tPa1.fLink[ curr ][ 0 ];
    else 
      next=tPa1.fLink[curr][1];
    
    if( tPa2.fLink[ curr ][ 0 ] != next && tPa2.fLink[ curr ][ 1 ] != next )  
      ++fDis_AB; 

    fOrder[ i ] = curr;
    fInv[ curr ] = i;
  }

  // 使用更健壮的检查方式，而不是直接断言
  if (next != st) {
    printf("[WARNING] Tour integrity check in SetParents failed: next (%d) != st (%d)\n", next, st);
    printf("[WARNING] This might indicate a problem with the parent tour structure.\n");
    printf("[WARNING] Attempting to continue execution...\n");
  }

  if( flagC[ 1 ] == 2 ){           /* Block2 */
    fTmax = 10;                    /* Block2 */
    fMaxStag = 20;                 /* Block2 (1:Greedy LS, 20:Tabu Search) */
    this->SetWeight( tPa1, tPa2 ); /* Block2 */
  }
}


void TCross::DoIt( TIndi& tKid, const TIndi& tPa1, const TIndi& tPa2, int numOfKids, int flagP, int flagC[ 10 ], int **fEdgeFreq, TFeatureExtractor* featureExtractor, const pybind11::object& get_action_mask_func, const pybind11::object& store_experience_func )
{
  int Num;     
  int jnum, centerAB; 
  int gain;
  int BestGain;  
  double pointMax, point;
  double DLoss;

  fEvalType = flagC[ 0 ];              /* 1:Greedy, 2:---, 3:Distance, 4:Entropy */
  fEsetType = flagC[ 1 ];              /* 1:Single-AB, 2:Block2 */

  assert( fEvalType == 1 || fEvalType == 3 || fEvalType == 4 );
  assert( fEsetType == 1 || fEsetType == 2 );

  if ( numOfKids <= fNumOfABcycle ) 
    Num = numOfKids;
  else 
    Num = fNumOfABcycle;

  if( fEsetType == 1 ){         /* Single-AB */
    tRand->Permutation( fPermu, fNumOfABcycle, fNumOfABcycle ); 
  }
  
  // 这是一个临时的布尔值，用于模拟和测试RL决策路径。
  // 在未来的任务中，我们将通过策略注入机制来动态控制它。
  bool use_rl_path = true;

  if (use_rl_path) {
      // --- RL 决策路径 ---
      namespace py = pybind11; // a shorthand

      // 1. 提取所有 AB 环的特征并打包成 Python 列表
      py::list all_features_list;
      if (fNumOfABcycle > 0) {
          for (int i = 0; i < fNumOfABcycle; ++i) {
              std::vector<double> features = featureExtractor->ExtractFeatures(fABcycle[i], tPa1, tPa2);
              py::list single_feature_list;
              for(const auto& val : features) {
                  single_feature_list.append(val);
  }
              all_features_list.append(single_feature_list);
          }
      }

      // 2. 调用 Python 函数
      py::list action_mask_py = get_action_mask_func(all_features_list);
      
      // 存储动作的log_probs，用于后续传递给store_experience
      py::list last_log_probs = py::list(); // 存储log_probs的空列表

      // 3. 将返回的 Python 列表转换为 C++ 向量
      std::vector<int> action_mask;
      for (py::handle obj : action_mask_py) {
          action_mask.push_back(obj.cast<int>());
    }

      // 4. 新的验证步骤：打印从 Python 收到的决策
      printf("--- [C++ DEBUG] Received action mask from Python: [");
      for (size_t i = 0; i < action_mask.size(); ++i) {
          printf("%d%s", action_mask[i], (i == action_mask.size() - 1) ? "" : ", ");
      }
      printf("]\n\n");
      
      // 5. 根据 Python 的决策来构建 E-set
    fNumOfABcycleInEset = 0;
      for (int i = 0; i < fNumOfABcycle; ++i) {
          if (i < action_mask.size() && action_mask[i] == 1) {
              fABcycleInEset[fNumOfABcycleInEset++] = i;
    }
      }

      // --- 步骤 b (核心重构)：在临时副本上预演并进行决定性验证 ---
      int** temp_link = new int*[fN];
      for (int i = 0; i < fN; ++i) {
          temp_link[i] = new int[2];
          temp_link[i][0] = tPa1.fLink[i][0];
          temp_link[i][1] = tPa1.fLink[i][1];
      }

      // 在副本上应用所有选择的 AB 环的边交换
      int total_gain = 0;
      for (int i = 0; i < fNumOfABcycle; ++i) {
          if (i < action_mask.size() && action_mask[i] == 1) {
              const int* cycle_data = fABcycle[i];
              int vertex_count = cycle_data[0];

              for (int j = 0; j < vertex_count / 2; ++j) {
                  int r1 = cycle_data[2 + 2 * j];
                  int r2 = cycle_data[3 + 2 * j];
                  int b1 = cycle_data[1 + 2 * j];
                  int b2 = cycle_data[4 + 2 * j];

                  // 更新 r1 的邻居
                  if (temp_link[r1][0] == r2) temp_link[r1][0] = b1;
                  else if (temp_link[r1][1] == r2) temp_link[r1][1] = b1;

                  // 更新 r2 的邻居
                  if (temp_link[r2][0] == r1) temp_link[r2][0] = b2;
                  else if (temp_link[r2][1] == r1) temp_link[r2][1] = b2;
	  }
              total_gain += fGainAB[i];
	}
      }
      
      bool is_structure_valid = this->IsValidLinkStructure(temp_link);

      // --- 步骤 c：根据预验证结果，提交或拒绝 ---
      if (is_structure_valid) {
          printf("[INFO] RL decision PASSED pre-verification. Applying to offspring.\n");
          // 验证通过，一次性将最终的图结构应用到真实的 tKid 上
          for (int i = 0; i < fN; ++i) {
              tKid.fLink[i][0] = temp_link[i][0];
              tKid.fLink[i][1] = temp_link[i][1];
          }
          
          // 执行后续的修复与评估
          this->MakeUnit();                                   
          this->MakeCompleteSol(tKid);
          
          // 计算最终增益和奖励
          int final_gain = tPa1.fEvaluationValue - tKid.fEvaluationValue;
          printf("[INFO] Valid offspring generated with total gain: %d\n", final_gain);
          
          // 计算奖励并存储经验
          int best_parent_eval = (tPa1.fEvaluationValue < tPa2.fEvaluationValue) ? tPa1.fEvaluationValue : tPa2.fEvaluationValue;
          double reward = static_cast<double>(best_parent_eval - tKid.fEvaluationValue) / best_parent_eval;
          
          printf("[ReACT-L REWARD] Base Reward Calculated: %.4f\n", reward);
          
          // 创建奖励列表
          py::list rewards_list = py::list();
          for (int i = 0; i < action_mask.size(); i++) {
              rewards_list.append(reward);
          }
          
          // 调用Python函数存储经验
          bool done = false; // 假设这不是一个终止状态
          store_experience_func(rewards_list, done);
          
          printf("[ReACT-L INFO] Stored %d experiences with reward: %.4f\n", action_mask.size(), reward);
      } else {
          // 预验证失败，拒绝这个决策，子代回退到父代A
          printf("[WARNING] RL decision FAILED pre-verification. Rejecting and reverting to Parent A.\n");
          tKid = tPa1;
          // 不存储任何经验，或存储一个带惩罚的经验
      }

      // 释放临时副本的内存
      for (int i = 0; i < fN; ++i) {
          delete[] temp_link[i];
      }
      delete[] temp_link;

      return; // DoIt 方法的 RL 路径到此结束
  } else {
      // --- 传统启发式路径 ---
      if( fEsetType == 1 ){         /* Single-AB */
        /* Step 3, 4, 5, and 6 of EAX */
        tKid = tPa1;
        BestGain = 0;

        for( int j = 0; j < Num; ++j ){
          jnum = fPermu[ j ];
          /* Step 4 and 5 of EAX */
          this->ChangeSol( tKid, jnum, 1 );  
          this->MakeCompleteSol( tKid );
          eval->DoIt( tKid );  // 评估器会直接更新tKid.fEvaluationValue
          gain = tPa1.fEvaluationValue - tKid.fEvaluationValue;
          this->BackToPa1( tKid );
    
          /* Step 6 of EAX */
          if( flagP == 1 ){   /* 1:Preserve the single best offspring */
            if( gain > BestGain ){
      BestGain = gain;        
              this->ChangeSol( tKid, jnum, 1 );
              this->MakeCompleteSol( tKid );
              eval->DoIt( tKid );  // 评估器会直接更新tKid.fEvaluationValue
              this->IncrementEdgeFreq( fEdgeFreq ); 
              this->BackToPa1( tKid );
            }
          }
          else if( flagP == 2 ){  /* 2:Preserve all the improving offspring (modified version) */
            if( gain > 0 ){
              this->ChangeSol( tKid, jnum, 1 );
              this->MakeCompleteSol( tKid );
              eval->DoIt( tKid );  // 评估器会直接更新tKid.fEvaluationValue
              tBestTmp = tKid;
              this->IncrementEdgeFreq( fEdgeFreq ); 
              this->BackToPa1( tKid );
              
              pointMax = -1;
              if( fEvalType == 1 ){
                tKid = tBestTmp;
                goto move_to_next_iteration;
              }
              if( fEvalType == 3 )
                pointMax = double( gain );
              else if( fEvalType == 4 ){
                DLoss = double( this->Cal_ENT_Loss( fEdgeFreq ) );
                if( DLoss <= 0.0 )
                  DLoss = 0.00000001;
                pointMax = gain / DLoss;
              }
              
              jnum = fPermu[ j ];
              ++j;
              if( j == fNumOfABcycle )
                break;
              
              while( j < fNumOfABcycle ){
                jnum = fPermu[ j ];
                this->ChangeSol( tKid, jnum, 1 );
                this->MakeCompleteSol( tKid );
                eval->DoIt( tKid );  // 评估器会直接更新tKid.fEvaluationValue
                gain = tPa1.fEvaluationValue - tKid.fEvaluationValue;
                if( gain <= 0 ){
                  this->BackToPa1( tKid );
                  ++j;
                  continue;
                }
                
                if( fEvalType == 1 ){
                  this->IncrementEdgeFreq( fEdgeFreq );
                  tBestTmp = tKid;
                  this->BackToPa1( tKid );
                  ++j;
                  continue;
                }
                else if( fEvalType == 3 )
                  point = double( gain );
                else if( fEvalType == 4 ){
                  DLoss = double( this->Cal_ENT_Loss( fEdgeFreq ) );
                  if( DLoss <= 0.0 )
                    DLoss = 0.00000001;
                  point = gain / DLoss;
                }
                
                if( point > pointMax ){
                  this->IncrementEdgeFreq( fEdgeFreq );
                  pointMax = point;
                  tBestTmp = tKid;
    }

    this->BackToPa1( tKid ); 
                ++j;
              }
              
              tKid = tBestTmp;
              break;
            }
          }
  }

        if( BestGain > 0 ){
          this->ChangeSol( tKid, fPermu[ 0 ], 1 );
          this->MakeCompleteSol( tKid );
    this->GoToBest( tKid ); 
        }
      move_to_next_iteration:;
      }
      else if( fEsetType == 2 ){  /* Block2 */
        fNumOfAppliedCycle = 0;
        tKid = tPa1;
        centerAB = 0;
        tBestTmp = tPa1;
        tBestTmp.fEvaluationValue = tPa1.fEvaluationValue;
        
        this->Search_Eset( centerAB );   /* Block2 */
      
        // --- 使用预验证逻辑 ---
        int** temp_link = new int*[fN];
        for (int i = 0; i < fN; ++i) {
            temp_link[i] = new int[2];
            temp_link[i][0] = tPa1.fLink[i][0];
            temp_link[i][1] = tPa1.fLink[i][1];
        }

        // 在临时副本上应用所有选择的AB环的边交换
        for (int s = 0; s < fNumOfABcycleInEset; ++s) {
            const int* cycle_data = fABcycle[fABcycleInEset[s]];
            int vertex_count = cycle_data[0];

            for (int j = 0; j < vertex_count / 2; ++j) {
                int r1 = cycle_data[2 + 2 * j];
                int r2 = cycle_data[3 + 2 * j];
                int b1 = cycle_data[1 + 2 * j];
                int b2 = cycle_data[4 + 2 * j];

                // 更新 r1 的邻居
                if (temp_link[r1][0] == r2) temp_link[r1][0] = b1;
                else if (temp_link[r1][1] == r2) temp_link[r1][1] = b1;

                // 更新 r2 的邻居
                if (temp_link[r2][0] == r1) temp_link[r2][0] = b2;
                else if (temp_link[r2][1] == r1) temp_link[r2][1] = b2;
            }
        }
        
        bool is_structure_valid = this->IsValidLinkStructure(temp_link);
        
        if (is_structure_valid) {
            printf("[INFO] Block2 decision PASSED pre-verification. Applying to offspring.\n");
            // 验证通过，一次性将最终的图结构应用到真实的 tKid 上
            for (int i = 0; i < fN; ++i) {
                tKid.fLink[i][0] = temp_link[i][0];
                tKid.fLink[i][1] = temp_link[i][1];
            }
            
            // 执行后续的修复与评估
            this->MakeCompleteSol(tKid);
            eval->DoIt(tKid);
            if (tKid.fEvaluationValue < tBestTmp.fEvaluationValue) {
                tBestTmp = tKid;
                this->IncrementEdgeFreq(fEdgeFreq);
            }
            tKid = tBestTmp;
        } else {
            // 预验证失败，拒绝这个决策，子代回退到父代A
            printf("[WARNING] Block2 decision FAILED pre-verification. Reverting to Parent A.\n");
            tKid = tPa1;
        }
        
        // 释放临时副本的内存
        for (int i = 0; i < fN; ++i) {
            delete[] temp_link[i];
        }
        delete[] temp_link;
      }
  }

  fNumOfGeneratedCh = 1;
}


void TCross::SetABcycle( const TIndi& tPa1, const TIndi& tPa2, int flagC[ 10 ], int numOfKids )
{
  bunki_many=0; koritsu_many=0;
  for( int j = 0; j < fN ; ++j )
  {
    near_data[j][1]=tPa1.fLink[j][0];
    near_data[j][3]=tPa1.fLink[j][1];

    near_data[j][0] = 2;
    
    koritsu[koritsu_many]=j;
    koritsu_many++;

    near_data[j][2]=tPa2.fLink[j][0];
    near_data[j][4]=tPa2.fLink[j][1];
  }
  for(int j = 0; j < fN; ++j ) 
  {
    check_koritsu[j]=-1;
    kori_inv[koritsu[j]]=j;
  }

  /**************************************************/

  fNumOfABcycle=0; 
  flag_st=1;                   
  while(koritsu_many!=0)
  {                                                               
    if(flag_st==1)          
    {
      fPosiCurr=0;
      r=rand()%koritsu_many;
      st=koritsu[r];    
      check_koritsu[st]=fPosiCurr;
      fRoute[fPosiCurr]=st;
      ci=st;
      pr_type=2;
    }
    else if(flag_st==0)    
    {
      ci=fRoute[fPosiCurr];   
    }
                        
    flag_circle=0;
    while(flag_circle==0)
    {
      fPosiCurr++;
      pr=ci;
      
      switch(pr_type)
      {
      case 1:                 
	ci=near_data[pr][fPosiCurr%2+1];
	break;
      case 2:   
	r=rand()%2;
	ci=near_data[pr][fPosiCurr%2+1+2*r];
	if(r==0) this->Swap(near_data[pr][fPosiCurr%2+1],near_data[pr][fPosiCurr%2+3]);
	break;
      case 3:   
	ci=near_data[pr][fPosiCurr%2+3];
      }

      fRoute[fPosiCurr]=ci;
      
      if(near_data[ci][0]==2) 
      {   
	if(ci==st)            
	{        
	  if(check_koritsu[st]==0) 
	  {        
	    if((fPosiCurr-check_koritsu[st])%2==0)  
	    {                  
	      if(near_data[st][fPosiCurr%2+1]==pr)
	      {
		this->Swap(near_data[ci][fPosiCurr%2+1],near_data[ci][fPosiCurr%2+3]); 
	      }
	      st_appear = 1;
	      this->FormABcycle();
	      if( flagC[ 1 ] == 1 && fNumOfABcycle == numOfKids ) goto LLL;
	      if( fNumOfABcycle == fMaxNumOfABcycle ) goto LLL;

	      flag_st=0;
	      flag_circle=1;
	      pr_type=1; 
	    }
	    else
	    {
	      this->Swap(near_data[ci][fPosiCurr%2+1],near_data[ci][fPosiCurr%2+3]); 
	      pr_type=2;
	    }
	    check_koritsu[st]=fPosiCurr;
	  } 
	  else                     
	  {         
	    st_appear = 2;
	    this->FormABcycle();
	    if( flagC[ 1 ] == 1 && fNumOfABcycle == numOfKids ) goto LLL;
	    if( fNumOfABcycle == fMaxNumOfABcycle ) goto LLL;

	    flag_st=1;
	    flag_circle=1;
	  }
	}
	else if(check_koritsu[ci]==-1) 
	{
	  check_koritsu[ci]=fPosiCurr;
	  if(near_data[ci][fPosiCurr%2+1]==pr)
	  {
	    this->Swap(near_data[ci][fPosiCurr%2+1],near_data[ci][fPosiCurr%2+3]); 
	  }
	  pr_type=2;
	}
	else if(check_koritsu[ci]>0)   
	{
	  this->Swap(near_data[ci][fPosiCurr%2+1],near_data[ci][fPosiCurr%2+3]); 
	  if((fPosiCurr-check_koritsu[ci])%2==0)  
	  {
	    st_appear = 1;
	    this->FormABcycle();
	    if( flagC[ 1 ] == 1 && fNumOfABcycle == numOfKids ) goto LLL;
	      if( fNumOfABcycle == fMaxNumOfABcycle ) goto LLL;
	      
	    flag_st=0;
	    flag_circle=1;
	    pr_type=1;
	  }
	  else
	  {
	    this->Swap(near_data[ci][(fPosiCurr+1)%2+1],near_data[ci][(fPosiCurr+1)%2+3]); 
	    pr_type=3;
	  }  
	}
      }
      else if(near_data[ci][0]==1)    
      {
	if(ci==st)                    
        {
	  st_appear = 1;
	  this->FormABcycle();
	  if( flagC[ 1 ] == 1 && fNumOfABcycle == numOfKids ) goto LLL;
	  if( fNumOfABcycle == fMaxNumOfABcycle ) goto LLL;

	  flag_st=1;
	  flag_circle=1;
	}
	else pr_type=1;
      }
    }
  }
                                       
  while(bunki_many!=0)
  {            
    fPosiCurr=0;   
    r=rand()%bunki_many;
    st=bunki[r];
    fRoute[fPosiCurr]=st;
    ci=st;
    
    flag_circle=0;
    while(flag_circle==0)
    { 
      pr=ci; 
      fPosiCurr++;
      ci=near_data[pr][fPosiCurr%2+1]; 
      fRoute[fPosiCurr]=ci;
      if(ci==st)                       
      {
	st_appear = 1;
	this->FormABcycle();
	if( flagC[ 1 ] == 1 && fNumOfABcycle == numOfKids ) goto LLL;
	if( fNumOfABcycle == fMaxNumOfABcycle ) goto LLL;
	
	flag_circle=1;
      }
    }
  }

LLL: ;

  if( fNumOfABcycle == fMaxNumOfABcycle ){
    printf( "fMaxNumOfABcycle(%d) must be increased\n", fMaxNumOfABcycle );
    exit( 1 );
  }
}


void TCross::FormABcycle()
{
  int j;
  int st_count;
  int edge_type;
  int st,ci, stock;
  int cem;                   
  int diff;
 
  if(fPosiCurr%2==0) edge_type=1; 
  else edge_type=2;               
  st=fRoute[fPosiCurr];
  cem=0;
  fC[cem]=st;    

  st_count=0;
  while(1)
  {
    cem++;
    fPosiCurr--;
    ci=fRoute[fPosiCurr];
    if(near_data[ci][0]==2)
    {
      koritsu[kori_inv[ci]]=koritsu[koritsu_many-1];
      kori_inv[koritsu[koritsu_many-1]]=kori_inv[ci];
      koritsu_many--;
      bunki[bunki_many]=ci;
      bun_inv[ci]=bunki_many;
      bunki_many++;
    }
    else if(near_data[ci][0]==1)
    {
      bunki[bun_inv[ci]]=bunki[bunki_many-1];
      bun_inv[bunki[bunki_many-1]]=bun_inv[ci];
      bunki_many--;
    }
             
    near_data[ci][0]--;
    if(ci==st) st_count++;
    if(st_count==st_appear) break;
    fC[cem]=ci;  
  }

  if(cem==2)
    return;

  fABcycle[fNumOfABcycle][0]=cem;    

  if(edge_type==2)
  {
    stock=fC[0];
    for( int j=0;j<cem-1;j++) fC[j]=fC[j+1];
    fC[cem-1]=stock;
  }
  
  for( int j=0;j<cem;j++) 
    fABcycle[fNumOfABcycle][j+2]=fC[j];
  fABcycle[fNumOfABcycle][1]=fC[cem-1];
  fABcycle[fNumOfABcycle][cem+2]=fC[0];
  fABcycle[fNumOfABcycle][cem+3]=fC[1];

  fC[ cem ] = fC[ 0 ]; 
  fC[ cem+1 ] = fC[ 1 ]; 
  diff = 0;
  for( j = 0; j < cem/2; ++j ) 
  {
    diff = diff + eval->fEdgeDis[fC[2*j]][fC[1+2*j]]
                - eval->fEdgeDis[fC[1+2*j]][fC[2+2*j]];
  }
  fGainAB[fNumOfABcycle] = diff;
  ++fNumOfABcycle;
}


void TCross::Swap(int &a,int &b)
{
  int s;
  s=a;
  a=b;
  b=s;
}


void TCross::ChangeSol( TIndi& tKid, int ABnum, int type )
{
  printf("[DEBUG] Applying AB cycle %d...\n", ABnum);
  
  int j;
  int cem,r1,r2,b1,b2;
  int po_r1, po_r2, po_b1, po_b2; 

  cem=fABcycle[ABnum][0];  
  fC[0]=fABcycle[ABnum][0];

  if(type==2)   
  {
    for(j=0;j<cem+3;j++) fC[cem+3-j]=fABcycle[ABnum][j+1];
  }
  else for(j=1;j<=cem+3;j++) fC[j]=fABcycle[ABnum][j];

  for(j=0;j<cem/2;j++)
  {                           
    r1=fC[2+2*j];r2=fC[3+2*j];
    b1=fC[1+2*j];b2=fC[4+2*j];

    if(tKid.fLink[r1][0]==r2)
      tKid.fLink[r1][0]=b1;
    else 
      tKid.fLink[r1][1]=b1;
    if(tKid.fLink[r2][0]==r1) 
      tKid.fLink[r2][0]=b2;
    else
      tKid.fLink[r2][1]=b2;   

    po_r1 = fInv[ r1 ]; 
    po_r2 = fInv[ r2 ]; 
    po_b1 = fInv[ b1 ]; 
    po_b2 = fInv[ b2 ]; 
    
    if( po_r1 == 0 && po_r2 == fN-1 )
      fSegPosiList[ fNumOfSPL++ ] = po_r1;
    else if( po_r1 == fN-1 && po_r2 == 0 )
      fSegPosiList[ fNumOfSPL++ ] = po_r2;
    else if( po_r1 < po_r2 )
      fSegPosiList[ fNumOfSPL++ ] = po_r2;
    else if( po_r2 < po_r1 )
      fSegPosiList[ fNumOfSPL++ ] = po_r1;
    else
      assert( 1 == 2 );
    
    LinkBPosi[ po_r1 ][ 1 ] = LinkBPosi[ po_r1 ][ 0 ];
    LinkBPosi[ po_r2 ][ 1 ] = LinkBPosi[ po_r2 ][ 0 ];
    LinkBPosi[ po_r1 ][ 0 ] = po_b1; 
    LinkBPosi[ po_r2 ][ 0 ] = po_b2; 
  }
}


void TCross::MakeCompleteSol( TIndi& tKid )
{
  int j,j1,j2,j3;
  int st,ci,pre,curr,next,a,b,c,d,aa,bb,a1,b1;
  int city_many;
  int remain_unit_many;
  int ucm;
  int unit_num;
  int min_unit_city; 
  int near_num;
  int unit_many;               
  int center_un;               
  int select_un;               
  int diff,max_diff;
  int count;      
  int nearMax;

  fGainModi = 0;         

  while( fNumOfUnit != 1 )
  {    
    min_unit_city = fN + 12345;
    for( int u = 0; u < fNumOfUnit; ++u ) 
    {
      if( fNumOfElementInUnit[ u ] < min_unit_city )
      {
	center_un = u;
        min_unit_city = fNumOfElementInUnit[ u ];
      }
    }  

    st = -1;
    fNumOfSegForCenter = 0;   
    for( int s = 0; s < fNumOfSeg; ++s ){
      if( fSegUnit[ s ] == center_un ){
	int posi = fSegment[ s ][ 0 ];
	st = fOrder[ posi ];    
	fSegForCenter[  fNumOfSegForCenter++ ] = s; 
      }
    } 
    assert( st != -1 );

    curr = -1;
    next = st;
    fNumOfElementInCU = 0;
    while(1){ 
      pre = curr;
      curr = next;
      fCenterUnit[ curr ] = 1;     
      fListOfCenterUnit[ fNumOfElementInCU ] = curr;
      ++fNumOfElementInCU;

      if( tKid.fLink[ curr ][ 0 ] != pre )
	next = tKid.fLink[ curr ][ 0 ];
      else 
	next = tKid.fLink[ curr ][ 1 ]; 

      if( next == st ) break;
    }       
    fListOfCenterUnit[ fNumOfElementInCU ] = fListOfCenterUnit[ 0 ];
    fListOfCenterUnit[ fNumOfElementInCU+1 ] = fListOfCenterUnit[ 1 ];

    assert( fNumOfElementInCU == fNumOfElementInUnit[ center_un ] );

    max_diff = -999999999;
    a1 = -1; b1 = -1;
    nearMax = 10; /* N_near (see Step 5.3 in Section 2.2 of the Online Supplement) */
    /* nearMax must be smaller than or equal to eva->fNearNumMax (kopt.cpp ) */

  RESTART:;
    for( int s = 1; s <= fNumOfElementInCU; ++s )  
    { 
      a = fListOfCenterUnit[ s ];

      for( near_num = 1; near_num <= nearMax; ++near_num )   
      {
	c = eval->fNearCity[ a ][ near_num ];
	if( fCenterUnit[ c ] == 0 )   
	{
	  for( j1 = 0; j1 < 2; ++j1 )
	  {
	    b = fListOfCenterUnit[ s-1+2*j1 ];
            for( j2 = 0; j2 < 2; ++j2 )
	    {
	      d = tKid.fLink[ c ][ j2 ];
	      diff = eval->fEdgeDis[a][b] + eval->fEdgeDis[c][d] -
                     eval->fEdgeDis[a][c] - eval->fEdgeDis[b][d];
	      if( diff > max_diff ) 
	      { 
	        aa = a; bb = b; a1 = c; b1 = d;
	        max_diff = diff;
	      }
	      diff = eval->fEdgeDis[a][b] + eval->fEdgeDis[d][c] - 
		     eval->fEdgeDis[a][d] - eval->fEdgeDis[b][c];
	      if( diff > max_diff ) 
	      {
	        aa = a; bb = b; a1 = d; b1 = c;
	        max_diff = diff;
	      } 
	    }
	  }
	}
      }
    }

    if( a1 == -1 && nearMax == 10 ){  /* This value must also be changed if nearMax is chenged above */
      nearMax = 50;
      goto RESTART;
    }    
    else if( a1 == -1 && nearMax == 50  )
    {       
      int r = rand() % ( fNumOfElementInCU - 1 );
      a = fListOfCenterUnit[ r ];
      b = fListOfCenterUnit[ r+1 ];
      for( j = 0; j < fN; ++j )
      {
	if( fCenterUnit[ j ] == 0 )
        {
	  aa = a; bb = b;
	  a1 = j;
	  b1 = tKid.fLink[ j ][ 0 ];
	  break;
	}
      }
      max_diff = eval->fEdgeDis[aa][bb] + eval->fEdgeDis[a1][b1] -
         	 eval->fEdgeDis[a][a1] - eval->fEdgeDis[b][b1];
    }  

    if( tKid.fLink[aa][0] == bb ) tKid.fLink[aa][0]=a1;
    else tKid.fLink[aa][1] = a1;
    if( tKid.fLink[bb][0] == aa ) tKid.fLink[bb][0] = b1;
    else tKid.fLink[bb][1] = b1;   
    if( tKid.fLink[a1][0] == b1 ) tKid.fLink[a1][0] = aa;
    else tKid.fLink[a1][1] = aa;
    if( tKid.fLink[b1][0] == a1 ) tKid.fLink[b1][0] = bb;
    else tKid.fLink[b1][1] = bb; 


    fModiEdge[ fNumOfModiEdge ][ 0 ] = aa;
    fModiEdge[ fNumOfModiEdge ][ 1 ] = bb;
    fModiEdge[ fNumOfModiEdge ][ 2 ] = a1;
    fModiEdge[ fNumOfModiEdge ][ 3 ] = b1;
    ++fNumOfModiEdge;


    fGainModi += max_diff;
    

    int posi_a1 = fInv[ a1 ];  
    select_un = -1;
    for( int s = 0; s < fNumOfSeg; ++s ){
      if( fSegment[ s ][ 0 ] <= posi_a1 && posi_a1 <=  fSegment[ s ][ 1 ] ){
	select_un = fSegUnit[ s ];       
	break;
      }
    } 
    assert( select_un != -1 );

    for( int s = 0; s < fNumOfSeg; ++s ){
      if( fSegUnit[ s ] == select_un )
	fSegUnit[ s ] = center_un;
    }
    fNumOfElementInUnit[ center_un ] += fNumOfElementInUnit[ select_un ];
    
    for( int s = 0; s < fNumOfSeg; ++s ){
      if( fSegUnit[ s ] == fNumOfUnit - 1 )
	fSegUnit[ s ] = select_un;
    }
    fNumOfElementInUnit[ select_un ] = fNumOfElementInUnit[ fNumOfUnit - 1 ];
    --fNumOfUnit;

    for( int s = 0; s < fNumOfElementInCU; ++s ){
      c = fListOfCenterUnit[ s ];
      fCenterUnit[ c ] = 0;
    }
  }
}  


void TCross::MakeUnit()                    
{
  printf("[DEBUG] Entering MakeUnit...\n");
  
  int flag = 1; 
  for( int s = 0; s < fNumOfSPL; ++s ){
    if( fSegPosiList[ s ] == 0 ){
      flag = 0;
      break;
    }
  }
  if( flag == 1 ) 
  {
    fSegPosiList[ fNumOfSPL++ ] = 0;

    LinkBPosi[ fN-1 ][ 1 ]  = LinkBPosi[ fN-1 ][ 0 ];
    LinkBPosi[ 0 ][ 1 ] = LinkBPosi[ 0 ][ 0 ];
    LinkBPosi[ fN-1 ][ 0 ] = 0; 
    LinkBPosi[ 0 ][ 0 ] = fN-1;

  }

  tSort->Sort( fSegPosiList, fNumOfSPL );     


  fNumOfSeg = fNumOfSPL;
  for( int s = 0; s < fNumOfSeg-1; ++s ){
    fSegment[ s ][ 0 ] = fSegPosiList[ s ];
    fSegment[ s ][ 1 ] = fSegPosiList[ s+1 ]-1;
  }

  fSegment[ fNumOfSeg-1 ][ 0 ] = fSegPosiList[ fNumOfSeg-1 ];
  fSegment[ fNumOfSeg-1 ][ 1 ] = fN - 1;


  for( int s = 0; s < fNumOfSeg; ++s ){
    LinkAPosi[ fSegment[ s ][ 0 ] ] = fSegment[ s ][ 1 ];
    LinkAPosi[ fSegment[ s ][ 1 ] ] = fSegment[ s ][ 0 ];
    fPosiSeg[ fSegment[ s ][ 0 ] ] = s;
    fPosiSeg[ fSegment[ s ][ 1 ] ] = s;
  }

  for( int s = 0; s < fNumOfSeg; ++s )
    fSegUnit[ s ] = -1;
  fNumOfUnit = 0; 

  int p_st, p1, p2, p_next, p_pre; 
  int segNum;
  int loop_count = 0;

  while(1)
  {
    printf("[DEBUG] MakeUnit main loop, iteration: %d, current units: %d\n", loop_count, fNumOfUnit);
    
    // 添加熔断机制
    if (loop_count > fN * 2) {
      printf("[FATAL ERROR] MakeUnit loop count exceeded threshold. Forcing exit.\n");
      exit(1);
    }
    loop_count++;
    
    flag = 0;
    for( int s = 0; s < fNumOfSeg; ++s ){
      if( fSegUnit[ s ] == -1 ){
	p_st = fSegment[ s ][ 0 ]; 
	p_pre = -1;
	p1 = p_st;
	flag = 1;
	break;
      }
    }
    if( flag == 0 )
      break;
    
    while(1)
    {
      segNum = fPosiSeg[ p1 ];
      fSegUnit[ segNum ] = fNumOfUnit;

      p2 = LinkAPosi[ p1 ];
      p_next = LinkBPosi[ p2 ][ 0 ];
      if( p1 == p2 ){
	if( p_next == p_pre )
	  p_next = LinkBPosi[ p2 ][ 1 ];
      } 
      
      if( p_next == p_st ){
	++fNumOfUnit;
	break;
      }

      p_pre = p2;
      p1 = p_next;
    }
  }
  

  for( int s = 0; s < fNumOfUnit; ++s )
    fNumOfElementInUnit[ s ] = 0; 
  
  int unitNum = -1;
  int tmpNumOfSeg = -1;
  for( int s = 0; s < fNumOfSeg; ++s ){
    if( fSegUnit[ s ] != unitNum ){
      ++tmpNumOfSeg;
      fSegment[ tmpNumOfSeg ][ 0 ] = fSegment[ s ][ 0 ];
      fSegment[ tmpNumOfSeg ][ 1 ] = fSegment[ s ][ 1 ];
      unitNum = fSegUnit[ s ];
      fSegUnit[ tmpNumOfSeg ] = unitNum;
      fNumOfElementInUnit[ unitNum ] += 
	fSegment[ s ][ 1 ] - fSegment[ s ][ 0 ] + 1;
    }
    else
    {
      fSegment[ tmpNumOfSeg ][ 1 ] = fSegment[ s ][ 1 ];
      fNumOfElementInUnit[ unitNum ] += 
	fSegment[ s ][ 1 ] - fSegment[ s ][ 0 ] + 1;
    }
  }
  fNumOfSeg = tmpNumOfSeg + 1;  
}


void TCross::BackToPa1( TIndi& tKid )
{
  int aa, bb, a1, b1; 
  int jnum;

  for( int s = fNumOfModiEdge -1; s >= 0; --s ){ 
    aa = fModiEdge[ s ][ 0 ];
    a1 = fModiEdge[ s ][ 1 ];   // $B$3$3$rJQ99$KCm0U(B 
    bb = fModiEdge[ s ][ 2 ];   // $B$3$3$rJQ99$KCm0U(B 
    b1 = fModiEdge[ s ][ 3 ];

    if( tKid.fLink[aa][0] == bb ) tKid.fLink[aa][0] = a1;
    else tKid.fLink[aa][1] = a1;
    if( tKid.fLink[b1][0] == a1 ) tKid.fLink[b1][0] = bb;
    else tKid.fLink[b1][1] = bb; 
    if( tKid.fLink[bb][0] == aa ) tKid.fLink[bb][0] = b1;
    else tKid.fLink[bb][1] = b1;   
    if( tKid.fLink[a1][0] == b1 ) tKid.fLink[a1][0] = aa;
    else tKid.fLink[a1][1] = aa;
  }
  
  for( int s = 0; s < fNumOfAppliedCycle; ++s ){
    jnum = fAppliedCylce[ s ];
    this->ChangeSol( tKid, jnum, 2 );
  }
}

void TCross::GoToBest( TIndi& tKid )
{
  int aa, bb, a1, b1; 
  int jnum;

  for( int s = 0; s < fNumOfBestAppliedCycle; ++s ){
    jnum = fBestAppliedCylce[ s ];
    this->ChangeSol( tKid, jnum, 1 );
  }

  for( int s = 0; s < fNumOfBestModiEdge; ++s )
  { 
    aa = fBestModiEdge[ s ][ 0 ];
    bb = fBestModiEdge[ s ][ 1 ];   
    a1 = fBestModiEdge[ s ][ 2 ];   
    b1 = fBestModiEdge[ s ][ 3 ];

    if( tKid.fLink[aa][0] == bb ) tKid.fLink[aa][0]=a1;
    else tKid.fLink[aa][1] = a1;
    if( tKid.fLink[bb][0] == aa ) tKid.fLink[bb][0] = b1;
    else tKid.fLink[bb][1] = b1;   
    if( tKid.fLink[a1][0] == b1 ) tKid.fLink[a1][0] = aa;
    else tKid.fLink[a1][1] = aa;
    if( tKid.fLink[b1][0] == a1 ) tKid.fLink[b1][0] = bb;
    else tKid.fLink[b1][1] = bb; 
  }
}


void TCross::IncrementEdgeFreq( int **fEdgeFreq )
{
  int j, jnum, cem;
  int r1, r2, b1, b2;
  int aa, bb, a1;
  
  for( int s = 0; s < fNumOfBestAppliedCycle; ++s ){
    jnum = fBestAppliedCylce[ s ];
    
    cem = fABcycle[ jnum ][ 0 ];  
    fC[ 0 ] = fABcycle[ jnum ][ 0 ];

    for( j = 1; j <= cem+3; ++j ) 
      fC[ j ] = fABcycle[ jnum ][ j ];

    for( j = 0; j <cem/2; ++j )
    {                           
      r1 = fC[2+2*j]; r2 = fC[3+2*j]; 
      b1 = fC[1+2*j]; b2 = fC[4+2*j]; 

      // r1 - b1 add    
      // r1 - r2 remove
      // r2 - r1 remove
      // r2 - b2 add

      ++fEdgeFreq[ r1 ][ b1 ];
      --fEdgeFreq[ r1 ][ r2 ];
      --fEdgeFreq[ r2 ][ r1 ];
      ++fEdgeFreq[ r2 ][ b2 ];

    }
  }

  for( int s = 0; s < fNumOfBestModiEdge; ++s )
  { 
    aa = fBestModiEdge[ s ][ 0 ];
    bb = fBestModiEdge[ s ][ 1 ];   
    a1 = fBestModiEdge[ s ][ 2 ];   
    b1 = fBestModiEdge[ s ][ 3 ];

    --fEdgeFreq[ aa ][ bb ];
    --fEdgeFreq[ bb ][ aa ];
    ++fEdgeFreq[ aa ][ a1 ];
    ++fEdgeFreq[ bb ][ b1 ];
    --fEdgeFreq[ bb ][ aa ];
    --fEdgeFreq[ b1 ][ a1 ];
    ++fEdgeFreq[ a1 ][ aa ];
    ++fEdgeFreq[ b1 ][ bb ];
  }
}


int TCross::Cal_ADP_Loss( int **fEdgeFreq )
{
  int j, jnum, cem;
  int r1, r2, b1, b2;
  int aa, bb, a1;
  double DLoss; 
  double h1, h2;

  
  DLoss = 0;
  for( int s = 0; s < fNumOfAppliedCycle; ++s ){
    jnum = fAppliedCylce[ s ];
    
    cem = fABcycle[ jnum ][ 0 ];  
    fC[ 0 ] = fABcycle[ jnum ][ 0 ];

    for( j = 1; j <= cem+3; ++j ) 
      fC[ j ] = fABcycle[ jnum ][ j ];

    for( j = 0; j <cem/2; ++j )
    {                           
      r1 = fC[2+2*j]; r2 = fC[3+2*j]; 
      b1 = fC[1+2*j]; b2 = fC[4+2*j]; 

      // r1 - b1 add 
      // r1 - r2 remove
      // r2 - r1 remove
      // r2 - b2 add

      DLoss -= (fEdgeFreq[ r1 ][ r2 ]-1);
      DLoss -= (fEdgeFreq[ r2 ][ r1 ]-1);
      DLoss += fEdgeFreq[ r2 ][ b2 ];
      DLoss += fEdgeFreq[ b2 ][ r2 ];

      // Remove
      --fEdgeFreq[ r1 ][ r2 ]; 
      --fEdgeFreq[ r2 ][ r1 ]; 

      // Add
      ++fEdgeFreq[ r2 ][ b2 ]; 
      ++fEdgeFreq[ b2 ][ r2 ]; 
    }
  }


  for( int s = 0; s < fNumOfModiEdge; ++s )
  { 
    aa = fModiEdge[ s ][ 0 ];
    bb = fModiEdge[ s ][ 1 ];   
    a1 = fModiEdge[ s ][ 2 ];   
    b1 = fModiEdge[ s ][ 3 ];

    DLoss -= (fEdgeFreq[ aa ][ bb ]-1);
    DLoss -= (fEdgeFreq[ bb ][ aa ]-1);
    DLoss -= (fEdgeFreq[ a1 ][ b1 ]-1);
    DLoss -= (fEdgeFreq[ b1 ][ a1 ]-1);

    DLoss += fEdgeFreq[ aa ][ a1 ];
    DLoss += fEdgeFreq[ a1 ][ aa ];
    DLoss += fEdgeFreq[ bb ][ b1 ];
    DLoss += fEdgeFreq[ b1 ][ bb ];

    // Remove
    --fEdgeFreq[ aa ][ bb ];
    --fEdgeFreq[ bb ][ aa ];
    --fEdgeFreq[ a1 ][ b1 ];
    --fEdgeFreq[ b1 ][ a1 ];

    // Add
    ++fEdgeFreq[ aa ][ a1 ];
    ++fEdgeFreq[ a1 ][ aa ];
    ++fEdgeFreq[ bb ][ b1 ];
    ++fEdgeFreq[ b1 ][ bb ];
  }

  
  for( int s = 0; s < fNumOfAppliedCycle; ++s ){
    jnum = fAppliedCylce[ s ];
    
    cem = fABcycle[ jnum ][ 0 ];  
    fC[ 0 ] = fABcycle[ jnum ][ 0 ];

    for( j = 1; j <= cem+3; ++j ) 
      fC[ j ] = fABcycle[ jnum ][ j ];

    for( j = 0; j <cem/2; ++j )
    {                           
      r1 = fC[2+2*j]; r2 = fC[3+2*j]; 
      b1 = fC[1+2*j]; b2 = fC[4+2*j]; 

      ++fEdgeFreq[ r1 ][ r2 ]; 
      ++fEdgeFreq[ r2 ][ r1 ]; 
      --fEdgeFreq[ r2 ][ b2 ]; 
      --fEdgeFreq[ b2 ][ r2 ]; 
    }
  }

  // Modification
  for( int s = 0; s < fNumOfModiEdge; ++s )
  { 
    aa = fModiEdge[ s ][ 0 ];
    bb = fModiEdge[ s ][ 1 ];   
    a1 = fModiEdge[ s ][ 2 ];   
    b1 = fModiEdge[ s ][ 3 ];

    // Remove
    ++fEdgeFreq[ aa ][ bb ];
    ++fEdgeFreq[ bb ][ aa ];

    ++fEdgeFreq[ a1 ][ b1 ];
    ++fEdgeFreq[ b1 ][ a1 ];

    --fEdgeFreq[ aa ][ a1 ];
    --fEdgeFreq[ a1 ][ aa ];

    --fEdgeFreq[ bb ][ b1 ];
    --fEdgeFreq[ b1 ][ bb ];

  }

  return int(DLoss / 2);
}


double TCross::Cal_ENT_Loss( int **fEdgeFreq )
{
  int j, jnum, cem;
  int r1, r2, b1, b2;
  int aa, bb, a1;
  double DLoss; 
  double h1, h2;

  
  DLoss = 0;
// AB-cycle
  for( int s = 0; s < fNumOfAppliedCycle; ++s ){
    jnum = fAppliedCylce[ s ];
    
    cem = fABcycle[ jnum ][ 0 ];  
    fC[ 0 ] = fABcycle[ jnum ][ 0 ];

    for( j = 1; j <= cem+3; ++j ) 
      fC[ j ] = fABcycle[ jnum ][ j ];

    for( j = 0; j <cem/2; ++j )
    {                           
      r1 = fC[2+2*j]; r2 = fC[3+2*j]; 
      b1 = fC[1+2*j]; b2 = fC[4+2*j]; 

      // r1 - b1 add    
      // r1 - r2 remove
      // r2 - r1 remove
      // r2 - b2 add

      // Remove
      h1 = (double)( fEdgeFreq[ r1 ][ r2 ] - 1 )/(double)fNumOfPop;
      h2 = (double)( fEdgeFreq[ r1 ][ r2 ] )/(double)fNumOfPop;
      if( fEdgeFreq[ r1 ][ r2 ] - 1 != 0 )
	DLoss -= h1 * log( h1 );
      DLoss += h2 * log( h2 );
      --fEdgeFreq[ r1 ][ r2 ]; 
      --fEdgeFreq[ r2 ][ r1 ]; 

      // Add
      h1 = (double)( fEdgeFreq[ r2 ][ b2 ] + 1 )/(double)fNumOfPop;
      h2 = (double)( fEdgeFreq[ r2 ][ b2 ])/(double)fNumOfPop;
      DLoss -= h1 * log( h1 );
      if( fEdgeFreq[ r2 ][ b2 ] != 0 )
	DLoss += h2 * log( h2 );
      ++fEdgeFreq[ r2 ][ b2 ]; 
      ++fEdgeFreq[ b2 ][ r2 ]; 
    }
  }

  // Modification
  for( int s = 0; s < fNumOfModiEdge; ++s )
  { 
    aa = fModiEdge[ s ][ 0 ];
    bb = fModiEdge[ s ][ 1 ];   
    a1 = fModiEdge[ s ][ 2 ];   
    b1 = fModiEdge[ s ][ 3 ];

    // Remove
    h1 = (double)( fEdgeFreq[ aa ][ bb ] - 1 )/(double)fNumOfPop;
    h2 = (double)( fEdgeFreq[ aa ][ bb ] )/(double)fNumOfPop;
    if( fEdgeFreq[ aa ][ bb ] - 1 != 0 )
      DLoss -= h1 * log( h1 );
    DLoss += h2 * log( h2 );
    --fEdgeFreq[ aa ][ bb ];
    --fEdgeFreq[ bb ][ aa ];

    h1 = (double)( fEdgeFreq[ a1 ][ b1 ] - 1 )/(double)fNumOfPop;
    h2 = (double)( fEdgeFreq[ a1 ][ b1 ] )/(double)fNumOfPop;
    if( fEdgeFreq[ a1 ][ b1 ] - 1 != 0 )
      DLoss -= h1 * log( h1 );
    DLoss += h2 * log( h2 );
    --fEdgeFreq[ a1 ][ b1 ];
    --fEdgeFreq[ b1 ][ a1 ];

    // Add
    h1 = (double)( fEdgeFreq[ aa ][ a1 ] + 1 )/(double)fNumOfPop;
    h2 = (double)( fEdgeFreq[ aa ][ a1 ])/(double)fNumOfPop;
    DLoss -= h1 * log( h1 );
    if( fEdgeFreq[ aa ][ a1 ] != 0 )
      DLoss += h2 * log( h2 );
    ++fEdgeFreq[ aa ][ a1 ];
    ++fEdgeFreq[ a1 ][ aa ];

    h1 = (double)( fEdgeFreq[ bb ][ b1 ] + 1 )/(double)fNumOfPop;
    h2 = (double)( fEdgeFreq[ bb ][ b1 ])/(double)fNumOfPop;
    DLoss -= h1 * log( h1 );
    if( fEdgeFreq[ bb ][ b1 ] != 0 )
      DLoss += h2 * log( h2 );
    ++fEdgeFreq[ bb ][ b1 ];
    ++fEdgeFreq[ b1 ][ bb ];
  }
  DLoss = -DLoss;  

  // restore EdgeFreq
  for( int s = 0; s < fNumOfAppliedCycle; ++s ){
    jnum = fAppliedCylce[ s ];
    
    cem = fABcycle[ jnum ][ 0 ];  
    fC[ 0 ] = fABcycle[ jnum ][ 0 ];

    for( j = 1; j <= cem+3; ++j ) 
      fC[ j ] = fABcycle[ jnum ][ j ];

    for( j = 0; j <cem/2; ++j )
    {                           
      r1 = fC[2+2*j]; r2 = fC[3+2*j]; 
      b1 = fC[1+2*j]; b2 = fC[4+2*j]; 

      ++fEdgeFreq[ r1 ][ r2 ]; 
      ++fEdgeFreq[ r2 ][ r1 ]; 
      --fEdgeFreq[ r2 ][ b2 ]; 
      --fEdgeFreq[ b2 ][ r2 ]; 
    }
  }

  for( int s = 0; s < fNumOfModiEdge; ++s )
  { 
    aa = fModiEdge[ s ][ 0 ];
    bb = fModiEdge[ s ][ 1 ];   
    a1 = fModiEdge[ s ][ 2 ];   
    b1 = fModiEdge[ s ][ 3 ];

    ++fEdgeFreq[ aa ][ bb ];
    ++fEdgeFreq[ bb ][ aa ];

    ++fEdgeFreq[ a1 ][ b1 ];
    ++fEdgeFreq[ b1 ][ a1 ];

    --fEdgeFreq[ aa ][ a1 ];
    --fEdgeFreq[ a1 ][ aa ];

    --fEdgeFreq[ bb ][ b1 ];
    --fEdgeFreq[ b1 ][ bb ];

  }

  return DLoss;
}


void TCross::SetWeight( const TIndi& tPa1, const TIndi& tPa2 ) 
{
  int cem;
  int r1, r2, v1, v2, v_p;
  int AB_num;

  for( int i = 0; i < fN; ++i ){
    fInEffectNode[ i ][ 0 ] = -1;  
    fInEffectNode[ i ][ 1 ] = -1;
  }

  // Step 1:
  for( int s = 0; s < fNumOfABcycle; ++s ){
    cem = fABcycle[ s ][ 0 ];  
    for( int j = 0; j < cem/2; ++j ){
      r1 = fABcycle[ s ][ 2*j+2 ];  // red edge
      r2 = fABcycle[ s ][ 2*j+3 ]; 

      if( fInEffectNode[ r1 ][ 0 ] == -1 ) fInEffectNode[ r1 ][ 0 ] = s;
      else if ( fInEffectNode[ r1 ][ 1 ] == -1 ) fInEffectNode[ r1 ][ 1 ] = s;
      else assert( 1 == 2 );

      if( fInEffectNode[ r2 ][ 0 ] == -1 ) fInEffectNode[ r2 ][ 0 ] = s;
      else if ( fInEffectNode[ r2 ][ 1 ] == -1 ) fInEffectNode[ r2 ][ 1 ] = s;
      else assert( 1 == 2 );
    }
  }
  
  // Step 2:
  for( int i = 0; i < fN; ++i ){
    if( fInEffectNode[ i ][ 0 ] != -1 && fInEffectNode[ i ][ 1 ] == -1 ){ 
      AB_num = fInEffectNode[ i ][ 0 ];
      v1 = i;

      if( tPa1.fLink[ v1 ][ 0 ] != tPa2.fLink[ v1 ][ 0 ] && tPa1.fLink[ v1 ][ 0 ] != tPa2.fLink[ v1 ][ 1 ] )
	v_p = tPa1.fLink[ v1 ][ 0 ];
      else if( tPa1.fLink[ v1 ][ 1 ] != tPa2.fLink[ v1 ][ 0 ] && tPa1.fLink[ v1 ][ 1 ] != tPa2.fLink[ v1 ][ 1 ] )
	v_p = tPa1.fLink[ v1 ][ 1 ];
      else
	assert( 1 == 2 );

      while( 1 ){
	assert( fInEffectNode[ v1 ][ 0 ] != -1 );
	assert( fInEffectNode[ v1 ][ 1 ] == -1 );
	fInEffectNode[ v1 ][ 1 ] = AB_num;

	if( tPa1.fLink[ v1 ][ 0 ] != v_p )
	  v2 = tPa1.fLink[ v1 ][ 0 ];
	else if( tPa1.fLink[ v1 ][ 1 ] != v_p )
	  v2 = tPa1.fLink[ v1 ][ 1 ];
	else 
	  assert( 1 == 2 );

	if( fInEffectNode[ v2 ][ 0 ] == -1 )
	  fInEffectNode[ v2 ][ 0 ] = AB_num;
	else if( fInEffectNode[ v2 ][ 1 ] == -1 )
	  fInEffectNode[ v2 ][ 1 ] = AB_num;
	else 
	  assert( 1 == 2 );
	
	if( fInEffectNode[ v2 ][ 1 ] != -1 )
	  break;

	v_p = v1;
	v1 = v2;
      }
    }
  }

  // Step 3:
  assert( fNumOfABcycle < fMaxNumOfABcycle );
  for( int s1 = 0; s1 < fNumOfABcycle; ++s1 ){
    fWeight_C[ s1 ] = 0;
    for( int s2 = 0; s2 < fNumOfABcycle; ++s2 ){
      fWeight_RR[ s1 ][ s2 ] = 0;
    }
  }
  
  for( int i = 0; i < fN; ++i ){
    assert( (fInEffectNode[ i ][ 0 ] == -1 && fInEffectNode[ i ][ 1 ] == -1) ||
	    (fInEffectNode[ i ][ 0 ] != -1 && fInEffectNode[ i ][ 1 ] != -1) );

    if( fInEffectNode[ i ][ 0 ] != -1 && fInEffectNode[ i ][ 1 ] != -1 ){
      ++fWeight_RR[ fInEffectNode[ i ][ 0 ] ][ fInEffectNode[ i ][ 1 ] ];
      ++fWeight_RR[ fInEffectNode[ i ][ 1 ] ][ fInEffectNode[ i ][ 0 ] ];
    }
    if( fInEffectNode[ i ][ 0 ] != fInEffectNode[ i ][ 1 ] ){
      ++fWeight_C[ fInEffectNode[ i ][ 0 ] ];
      ++fWeight_C[ fInEffectNode[ i ][ 1 ] ];
    }
    
  }
  for( int s1 = 0; s1 < fNumOfABcycle; ++s1 )
    fWeight_RR[ s1 ][ s1 ] = 0;
  

  for( int i = 0; i < fN; ++i ){
    assert( ( fInEffectNode[ i ][ 0 ] != -1 && fInEffectNode[ i ][ 1 ] != -1 ) ||
	    ( fInEffectNode[ i ][ 0 ] == -1 && fInEffectNode[ i ][ 1 ] == -1 ) );
  }

}


int TCross::Cal_C_Naive() 
{
  int count_C;
  int tt;

  count_C = 0;

  for( int i = 0; i < fN; ++i ){
    if( fInEffectNode[ i ][ 0 ] != -1 && fInEffectNode[ i ][ 1 ] != -1 ){
      tt = 0;
      if( fUsedAB[ fInEffectNode[ i ][ 0 ] ] == 1 )
	++tt;
      if( fUsedAB[ fInEffectNode[ i ][ 1 ] ] == 1 )
	++tt;
      if( tt == 1 )
	++count_C;
    }
  }
  return count_C;
}

void TCross::Search_Eset( int centerAB ) 
{
  int nIter, stagImp;
  int delta_weight, min_delta_weight_nt;
  int flag_AddDelete, flag_AddDelete_nt;
  int selected_AB, selected_AB_nt;
  int t_max;
  int jnum;

  fNum_C = 0;  // Number of C nodes in E-set
  fNum_E = 0;  // Number of Edges in E-set 

  fNumOfUsedAB = 0;
  for( int s1 = 0; s1 < fNumOfABcycle; ++s1 ){
    fUsedAB[ s1 ] = 0;
    fWeight_SR[ s1 ] = 0;
    fMoved_AB[ s1 ] = 0;
  }

  for( int s = 0; s < fNumOfABcycleInEset; ++s )   
  {
    jnum = fABcycleInEset[ s ];
    this->Add_AB( jnum );
  }
  fBest_Num_C = fNum_C;
  fBest_Num_E = fNum_E;
  
  stagImp = 0;
  nIter = 0;
  while( 1 )
  { 
    ++nIter;

    min_delta_weight_nt = 99999999;  
    flag_AddDelete = 0;
    flag_AddDelete_nt = 0;
    for( int s1 = 0; s1 < fNumOfABcycle; ++s1 )
    {
      if( fUsedAB[ s1 ] == 0 && fWeight_SR[ s1 ] > 0 )
      {
	delta_weight = fWeight_C[ s1 ] - 2 * fWeight_SR[ s1 ];   
	if( fNum_C + delta_weight < fBest_Num_C ){
	  selected_AB = s1;
	  flag_AddDelete = 1;
	  fBest_Num_C = fNum_C + delta_weight;
	}
	if( delta_weight < min_delta_weight_nt && nIter > fMoved_AB[ s1 ] ){
	  selected_AB_nt = s1;
	  flag_AddDelete_nt = 1;
	  min_delta_weight_nt = delta_weight;
	}
      }
      else if( fUsedAB[ s1 ] == 1 && s1 != centerAB )
      {
	delta_weight = - fWeight_C[ s1 ] + 2 * fWeight_SR[ s1 ];   
	if( fNum_C + delta_weight < fBest_Num_C ){
	  selected_AB = s1;
	  flag_AddDelete = -1;
	  fBest_Num_C = fNum_C + delta_weight;
	}
	if( delta_weight < min_delta_weight_nt && nIter > fMoved_AB[ s1 ] ){
	  selected_AB_nt = s1;
	  flag_AddDelete_nt = -1;
	  min_delta_weight_nt = delta_weight;
	}
      }
    }
      
    if( flag_AddDelete != 0 ){
      if( flag_AddDelete == 1 ){
	this->Add_AB( selected_AB );
      }
      else if( flag_AddDelete == -1 )
	this->Delete_AB( selected_AB );
      
      fMoved_AB[ selected_AB ] = nIter + tRand->Integer( 1, fTmax ); 
      assert( fBest_Num_C == fNum_C );
      fBest_Num_E = fNum_E;

      fNumOfABcycleInEset = 0;
      for( int s1 = 0; s1 < fNumOfABcycle; ++s1 ){
	if( fUsedAB[ s1 ] == 1 )
	  fABcycleInEset[ fNumOfABcycleInEset++ ] = s1;
      }
      assert( fNumOfABcycleInEset == fNumOfUsedAB );      
      stagImp = 0;
    }
    else if( flag_AddDelete_nt != 0 ) {
      if( flag_AddDelete_nt == 1 ){
	this->Add_AB( selected_AB_nt );
      }
      else if( flag_AddDelete_nt == -1 )
	this->Delete_AB( selected_AB_nt );

      fMoved_AB[ selected_AB_nt ] = nIter + tRand->Integer( 1, fTmax ); 
    } 

    if( flag_AddDelete == 0 )
      ++stagImp;
    if( stagImp == fMaxStag )
      break;
  }
}


void TCross::Add_AB( int AB_num )  
{
  fNum_C += fWeight_C[ AB_num ] - 2 * fWeight_SR[ AB_num ];   
  fNum_E += fABcycle[ AB_num ][ 0 ] / 2;  

  assert( fUsedAB[ AB_num ] == 0 );
  fUsedAB[ AB_num ] = 1;
  ++fNumOfUsedAB;

  for( int s1 = 0; s1 < fNumOfABcycle; ++s1 ){
    fWeight_SR[ s1 ] += fWeight_RR[ s1 ][ AB_num ];
  }
}


void TCross::Delete_AB( int AB_num )  
{
  fNum_C -= fWeight_C[ AB_num ] - 2 * fWeight_SR[ AB_num ];   
  fNum_E -= fABcycle[ AB_num ][ 0 ] / 2;  

  assert( fUsedAB[ AB_num ] == 1 );
  fUsedAB[ AB_num ] = 0;
  --fNumOfUsedAB;

  for( int s1 = 0; s1 < fNumOfABcycle; ++s1 ){
    fWeight_SR[ s1 ] -= fWeight_RR[ s1 ][ AB_num ];
  }
}


void TCross::CheckValid( TIndi& indi )
{
  int curr, pre, next, st;
  int count;

  st = 0;
  curr = -1;
  next = st;

  count = 0;
  while(1){ 
    pre = curr;
    curr = next;
    ++count;
    if( indi.fLink[ curr ][ 0 ] != pre )
      next = indi.fLink[ curr ][ 0 ];
    else 
      next = indi.fLink[ curr ][ 1 ]; 
    
    if( next == st ) break;

    if( count > fN ){
      printf( "Invalid = %d\n", count );
      break;
    }
  }       
  if( count != fN )
      printf( "Invalid = %d\n", count );
}

bool TCross::VerifyTourIntegrity(const TIndi& indi, const std::string& stage_name) {
    bool integrity_ok = true;
    bool needs_repair = false;
    
    // 第一轮：检查链接值是否有效
    for (int i = 0; i < fN; ++i) {
        int neighbor1 = indi.fLink[i][0];
        int neighbor2 = indi.fLink[i][1];

        if (neighbor1 < 0 || neighbor1 >= fN || neighbor2 < 0 || neighbor2 >= fN || neighbor1 == neighbor2 || neighbor1 == i || neighbor2 == i) {
            printf("[DEBUG_VERIFY FAILED at %s] City %d has invalid links: [%d, %d]\n", stage_name.c_str(), i, neighbor1, neighbor2);
            integrity_ok = false;
            
            // 如果是父代B，尝试修复它
            if (stage_name == "Parent B") {
                needs_repair = true;
                printf("[DEBUG_REPAIR] Fixing invalid links for city %d\n", i);
                // 使用安全的默认值
                const_cast<TIndi&>(indi).fLink[i][0] = (i + 1) % fN;
                const_cast<TIndi&>(indi).fLink[i][1] = (i + fN - 1) % fN;
            }
        }
    }
    
    // 第二轮：检查链接的对称性
    for (int i = 0; i < fN; ++i) {
        int neighbor1 = indi.fLink[i][0];
        int neighbor2 = indi.fLink[i][1];
        
        // 确保邻居在有效范围内
        if (neighbor1 >= 0 && neighbor1 < fN) {
            bool check1 = (indi.fLink[neighbor1][0] == i || indi.fLink[neighbor1][1] == i);
            if (!check1) {
                printf("[DEBUG_VERIFY FAILED at %s] Asymmetric link detected for city %d and %d\n", stage_name.c_str(), i, neighbor1);
                integrity_ok = false;
                
                // 如果是父代B，尝试修复它
                if (stage_name == "Parent B") {
                    needs_repair = true;
                    printf("[DEBUG_REPAIR] Fixing asymmetric link between %d and %d\n", i, neighbor1);
                    if (const_cast<TIndi&>(indi).fLink[neighbor1][0] != i) {
                        const_cast<TIndi&>(indi).fLink[neighbor1][1] = i;
                    } else {
                        const_cast<TIndi&>(indi).fLink[neighbor1][0] = i;
                    }
                }
            }
        }
        
        if (neighbor2 >= 0 && neighbor2 < fN) {
            bool check2 = (indi.fLink[neighbor2][0] == i || indi.fLink[neighbor2][1] == i);
            if (!check2) {
                printf("[DEBUG_VERIFY FAILED at %s] Asymmetric link detected for city %d and %d\n", stage_name.c_str(), i, neighbor2);
                integrity_ok = false;
                
                // 如果是父代B，尝试修复它
                if (stage_name == "Parent B") {
                    needs_repair = true;
                    printf("[DEBUG_REPAIR] Fixing asymmetric link between %d and %d\n", i, neighbor2);
                    if (const_cast<TIndi&>(indi).fLink[neighbor2][0] != i) {
                        const_cast<TIndi&>(indi).fLink[neighbor2][1] = i;
                    } else {
                        const_cast<TIndi&>(indi).fLink[neighbor2][0] = i;
                    }
                }
            }
        }
    }
    
    // 如果是父代B，并且我们进行了修复，再次验证
    if (stage_name == "Parent B" && needs_repair) {
        printf("[DEBUG_REPAIR] Re-verifying Parent B after repairs\n");
        
        // 再次验证
        bool repair_ok = true;
        
        // 检查链接值
        for (int i = 0; i < fN; ++i) {
            int neighbor1 = indi.fLink[i][0];
            int neighbor2 = indi.fLink[i][1];
            
            if (neighbor1 < 0 || neighbor1 >= fN || neighbor2 < 0 || neighbor2 >= fN || neighbor1 == neighbor2 || neighbor1 == i || neighbor2 == i) {
                printf("[DEBUG_REPAIR FAILED] City %d still has invalid links: [%d, %d]\n", i, neighbor1, neighbor2);
                repair_ok = false;
            }
        }
        
        // 检查对称性
        for (int i = 0; i < fN; ++i) {
            int neighbor1 = indi.fLink[i][0];
            int neighbor2 = indi.fLink[i][1];
            
            if (neighbor1 >= 0 && neighbor1 < fN) {
                bool check1 = (indi.fLink[neighbor1][0] == i || indi.fLink[neighbor1][1] == i);
                if (!check1) {
                    printf("[DEBUG_REPAIR FAILED] Asymmetric link still detected for city %d and %d\n", i, neighbor1);
                    repair_ok = false;
                }
            }
            
            if (neighbor2 >= 0 && neighbor2 < fN) {
                bool check2 = (indi.fLink[neighbor2][0] == i || indi.fLink[neighbor2][1] == i);
                if (!check2) {
                    printf("[DEBUG_REPAIR FAILED] Asymmetric link still detected for city %d and %d\n", i, neighbor2);
                    repair_ok = false;
                }
            }
        }
        
        if (repair_ok) {
            printf("[DEBUG_REPAIR SUCCEEDED] Parent B has been fixed\n");
            integrity_ok = true;
        } else {
            printf("[DEBUG_REPAIR FAILED] Could not fully repair Parent B\n");
        }
    }
    
    if (integrity_ok) {
        printf("[DEBUG_VERIFY PASSED at %s]\n", stage_name.c_str());
    }
    
    return integrity_ok;
}

bool TCross::IsValidLinkStructure(int** temp_link) {
    // 1. 检查每个城市的链接值是否合法（无越界、无自环）
    for (int i = 0; i < fN; ++i) {
        int neighbor1 = temp_link[i][0];
        int neighbor2 = temp_link[i][1];
        
        // 检查越界
        if (neighbor1 < 0 || neighbor1 >= fN || neighbor2 < 0 || neighbor2 >= fN) {
            return false;
        }
        
        // 检查自环
        if (neighbor1 == i || neighbor2 == i || neighbor1 == neighbor2) {
            return false;
        }
    }
    
    // 2. 检查链接的对称性
    for (int i = 0; i < fN; ++i) {
        int neighbor1 = temp_link[i][0];
        int neighbor2 = temp_link[i][1];
        
        // 检查第一个邻居是否指向当前城市
        bool check1 = (temp_link[neighbor1][0] == i || temp_link[neighbor1][1] == i);
        if (!check1) {
            return false;
        }
        
        // 检查第二个邻居是否指向当前城市
        bool check2 = (temp_link[neighbor2][0] == i || temp_link[neighbor2][1] == i);
        if (!check2) {
            return false;
        }
    }
    
    // 3. 检查图的连通性（使用BFS）
    bool* visited = new bool[fN];
    for (int i = 0; i < fN; ++i) {
        visited[i] = false;
    }
    
    // 从城市0开始进行BFS遍历
    std::queue<int> q;
    q.push(0);
    visited[0] = true;
    int visited_count = 1;
    
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        
        // 访问当前城市的两个邻居
        int neighbor1 = temp_link[current][0];
        int neighbor2 = temp_link[current][1];
        
        if (!visited[neighbor1]) {
            visited[neighbor1] = true;
            q.push(neighbor1);
            visited_count++;
        }
        
        if (!visited[neighbor2]) {
            visited[neighbor2] = true;
            q.push(neighbor2);
            visited_count++;
        }
    }
    
    // 检查是否所有城市都被访问到
    bool all_visited = (visited_count == fN);
    
    delete[] visited;
    
    return all_visited;
}

