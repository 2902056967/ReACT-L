#ifndef __ENVIRONMENT__
#include "env.h"
#endif

#include <math.h> 

void MakeRandSol(TEvaluator* eval, TIndi& indi);
void Make2optSol(TEvaluator* eval, TIndi& indi);

TEnvironment::TEnvironment()
{
    fEvaluator = new TEvaluator();
}


TEnvironment::~TEnvironment()
{
    delete[] fIndexForMating;
    delete[] tCurPop;
    delete fEvaluator;
    delete tCross;
    delete fFeatureExtractor;

    int N = fEvaluator->Ncity;
    for (int i = 0; i < N; ++i)
        delete[] fEdgeFreq[i];
    delete[] fEdgeFreq;
}


void TEnvironment::Define()
{
    fEvaluator->SetInstance(fFileNameTSP);
    int N = fEvaluator->Ncity;

    fIndexForMating = new int[fNumOfPop + 1];

    tCurPop = new TIndi[fNumOfPop];
    for (int i = 0; i < fNumOfPop; ++i)
        tCurPop[i].Define(N);

    tBest.Define(N);

    tCross = new TCross(N);
    tCross->eval = fEvaluator;
    tCross->fNumOfPop = fNumOfPop;
    
    fFeatureExtractor = new TFeatureExtractor(fEvaluator);

    tKopt = new TKopt(N);
    tKopt->eval = fEvaluator;
    tKopt->SetInvNearList();

    fEdgeFreq = new int* [N];
    for (int i = 0; i < N; ++i)
        fEdgeFreq[i] = new int[N];
    
    try {
        pybind11::module_ sys = pybind11::module_::import("sys");
        sys.attr("path").attr("append")(".");

        pybind11::module_ bridge_api = pybind11::module_::import("bridge_api");
        printf("[ReACT-L INFO] Python module 'bridge_api' imported.\n");

        // 明确加载每一个需要的函数
        py_get_action_mask = bridge_api.attr("get_action_mask");
        printf("[ReACT-L INFO] Function 'get_action_mask' loaded.\n");

        py_store_experience = bridge_api.attr("store_experience");
        printf("[ReACT-L INFO] Function 'store_experience' loaded.\n");

        py_trigger_update = bridge_api.attr("trigger_update");
        printf("[ReACT-L INFO] Function 'trigger_update' loaded.\n");
    } catch (pybind11::error_already_set &e) {
        fprintf(stderr, "[ReACT-L ERROR] Failed to import Python module or function:\n%s\n", e.what());
        exit(1);
    }
}


void TEnvironment::DoIt()
{
    this->fTimeStart = clock();

    if (fFileNameInitPop == NULL)
        this->InitPop();
    else
        this->ReadPop(fFileNameInitPop);

    this->fTimeInit = clock();

    this->Init();
    this->GetEdgeFreq();

    // 定义更新周期K
    const int K = 5; // 每5代更新一次策略
    
    try {
    while (1)
    {
        this->SetAverageBest();
        printf("%d: %d %lf\n", fCurNumOfGen, fBestValue, fAverageValue);

        if (this->TerminationCondition()) break;

        this->SelectForMating();

        for (int s = 0; s < fNumOfPop; ++s)
        {
            this->GenerateKids(s);
            this->SelectForSurvival(s);
        }
        ++fCurNumOfGen;
            
            // 每K代触发一次策略更新
            if (fCurNumOfGen % K == 0) {
                printf("[ReACT-L INFO] Triggering PPO update at generation %d\n", fCurNumOfGen);
                py_trigger_update(K);
            }
        }
    } catch (pybind11::error_already_set &e) {
        fprintf(stderr, "[ReACT-L ERROR] Failed to call Python trigger_update function:\n%s\n", e.what());
        exit(1);
    }

    this->fTimeEnd = clock();
}


void TEnvironment::Init()
{
    fAccumurateNumCh = 0;
    fCurNumOfGen = 0;
    fStagBest = 0;
    fMaxStagBest = 0;
    fStage = 1;          /* Stage I */
    fFlagC[0] = 4;     /* Diversity preservation: 1:Greedy, 2:--- , 3:Distance, 4:Entropy (see Section 4) */
    fFlagC[1] = 1;     /* Eset Type: 1:Single-AB, 2:Block2 (see Section 3) */
}


bool TEnvironment::TerminationCondition()
{
    if (fAverageValue - fBestValue < 0.001)
        return true;

    if (fStage == 1) /* Stage I */
    {
        if (fStagBest == int(1500 / fNumOfKids) && fMaxStagBest == 0) { /* 1500/N_ch (See Section 2.2) */
            fMaxStagBest = int(fCurNumOfGen / 10);                 /* fMaxStagBest = G/10 (See Section 2.2) */
        }
        else if (fMaxStagBest != 0 && fMaxStagBest <= fStagBest) { /* Terminate Stage I (proceed to Stage II) */
            fStagBest = 0;
            fMaxStagBest = 0;
            fCurNumOfGen1 = fCurNumOfGen;
            fFlagC[1] = 2;
            fStage = 2;
        }
        return false;
    }

    if (fStage == 2) { /* Stage II */
        if (fStagBest == int(1500 / fNumOfKids) && fMaxStagBest == 0) { /* 1500/N_ch */
            fMaxStagBest = int((fCurNumOfGen - fCurNumOfGen1) / 10); /* fMaxStagBest = G/10 (See Section 2.2) */
        }
        else if (fMaxStagBest != 0 && fMaxStagBest <= fStagBest) { /* Terminate Stage II and GA */
            return true;
        }

        return false;
    }

    return true;
}


void TEnvironment::SetAverageBest()
{
    int stockBest = tBest.fEvaluationValue;

    fAverageValue = 0.0;
    fBestIndex = 0;
    fBestValue = tCurPop[0].fEvaluationValue;

    for (int i = 0; i < fNumOfPop; ++i) {
        fAverageValue += tCurPop[i].fEvaluationValue;
        if (tCurPop[i].fEvaluationValue < fBestValue) {
            fBestIndex = i;
            fBestValue = tCurPop[i].fEvaluationValue;
        }
    }

    tBest = tCurPop[fBestIndex];
    fAverageValue /= (double)fNumOfPop;

    if (tBest.fEvaluationValue < stockBest) {
        fStagBest = 0;
        fBestNumOfGen = fCurNumOfGen;
        fBestAccumeratedNumCh = fAccumurateNumCh;
    }
    else ++fStagBest;
}


void TEnvironment::InitPop()
{
    for (int i = 0; i < fNumOfPop; ++i) {
        tKopt->MakeRandSol(tCurPop[i]);    /* Make a random tour */
        tKopt->DoIt(tCurPop[i]);           /* Apply the local search with the 2-opt neighborhood */
    }
}


void TEnvironment::SelectForMating()
{
    /* fIndexForMating[] <-- a random permutation of 0, ..., fNumOfPop-1 */
    tRand->Permutation(fIndexForMating, fNumOfPop, fNumOfPop);
    fIndexForMating[fNumOfPop] = fIndexForMating[0];
}

void TEnvironment::SelectForSurvival(int s)
{
}


void TEnvironment::GenerateKids(int s)
{
    // 记录父代的评估值（用于后续计算奖励）
    int parent1_eval = tCurPop[fIndexForMating[s]].fEvaluationValue;
    int parent2_eval = tCurPop[fIndexForMating[s + 1]].fEvaluationValue;
    
    // 在调用SetParents之前验证父代的完整性
    const TIndi& parentA = tCurPop[fIndexForMating[s]];
    const TIndi& parentB = tCurPop[fIndexForMating[s + 1]];

    printf("--- [DEBUG] Verifying integrity of parents before crossover ---\n");
    bool parentA_valid = tCross->VerifyTourIntegrity(parentA, "Parent A");
    bool parentB_valid = tCross->VerifyTourIntegrity(parentB, "Parent B");
    
    // 只有当两个父代都是有效的时才进行交叉
    if (parentA_valid && parentB_valid) {
        tCross->SetParents(parentA, parentB, fFlagC, fNumOfKids);

    /* Note: tCurPop[fIndexForMating[s]] is replaced with a best offspring solutions in tCorss->DoIt().
       fEegeFreq[][] is also updated there. */
        
        // 直接调用DoIt方法，所有的验证和回退逻辑已经封装在DoIt内部
        tCross->DoIt(tCurPop[fIndexForMating[s]], tCurPop[fIndexForMating[s]], tCurPop[fIndexForMating[s + 1]], 
                    fNumOfKids, 1, fFlagC, fEdgeFreq, fFeatureExtractor, py_get_action_mask, py_store_experience);

    fAccumurateNumCh += tCross->fNumOfGeneratedCh;
    } else {
        // 如果任一父代无效，则不进行交叉，保持父代A不变
        printf("[WARNING] Skipping crossover due to invalid parent(s).\n");
    }
}


void TEnvironment::GetEdgeFreq()
{
    int N = fEvaluator->Ncity;
    int k0, k1;

    for (int j1 = 0; j1 < N; ++j1)
        for (int j2 = 0; j2 < N; ++j2)
            fEdgeFreq[j1][j2] = 0;


    for (int i = 0; i < fNumOfPop; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            k0 = tCurPop[i].fLink[j][0];
            k1 = tCurPop[i].fLink[j][1];
            ++fEdgeFreq[j][k0];
            ++fEdgeFreq[j][k1];
        }
    }
}


void TEnvironment::PrintOn(int n, char* dstFile)
{
    printf("n = %d val = %d Gen = %d Time = %d %d\n",
        n,
        tBest.fEvaluationValue,
        fCurNumOfGen,
        (int)((double)(this->fTimeInit - this->fTimeStart) / (double)CLOCKS_PER_SEC),
        (int)((double)(this->fTimeEnd - this->fTimeStart) / (double)CLOCKS_PER_SEC));
    fflush(stdout);

    FILE* fp;
    char filename[80];
    sprintf(filename, "%s_Result", dstFile);
    fp = fopen(filename, "a");

    fprintf(fp, "%d %d %d %d %d\n",
        n,
        tBest.fEvaluationValue,
        fCurNumOfGen,
        (int)((double)(this->fTimeInit - this->fTimeStart) / (double)CLOCKS_PER_SEC),
        (int)((double)(this->fTimeEnd - this->fTimeStart) / (double)CLOCKS_PER_SEC));

    fclose(fp);
}


void TEnvironment::WriteBest(char* dstFile)
{
    FILE* fp;
    char filename[80];
    sprintf(filename, "%s_BestSol", dstFile);
    fp = fopen(filename, "a");

    fEvaluator->WriteTo(fp, tBest);

    fclose(fp);
}


void TEnvironment::WritePop(int n, char* dstFile)
{
    FILE* fp;
    char filename[80];
    sprintf(filename, "%s_POP_%d", dstFile, n);
    fp = fopen(filename, "w");

    for (int s = 0; s < fNumOfPop; ++s)
        fEvaluator->WriteTo(fp, tCurPop[s]);

    fclose(fp);
}


void TEnvironment::ReadPop(char* fileName)
{
    FILE* fp;

    if ((fp = fopen(fileName, "r")) == NULL) {
        printf("Read Error1\n");
        fflush(stdout);
        exit(1);
    }

    for (int i = 0; i < fNumOfPop; ++i) {
        if (fEvaluator->ReadFrom(fp, tCurPop[i]) == false) {
            printf("Read Error2\n");
            fflush(stdout);
            exit(1);
        }
    }
    fclose(fp);
}