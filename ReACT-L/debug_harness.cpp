#include <cstdio>
#include <vector>
#include <string>
#include "env.h"
#include "cross.h"

int main() {
    printf("Starting debug harness...\n");
    
    // 初始化环境并加载TSP实例
    TEnvironment env;
    env.Define(); // 定义变量
    env.fFileNameTSP = (char*)"data/eil51.tsp";
    env.Init(); // 初始化GA
    printf("Loaded TSP instance\n");
    
    // 初始化种群
    env.InitPop();
    printf("Population initialized\n");
    
    // 从种群中取出两个父代个体
    TIndi parentA = env.tCurPop[0];
    TIndi parentB = env.tCurPop[1];
    printf("Selected two parent individuals\n");
    
    // 创建交叉算子
    TCross* tCross = new TCross(env.fEvaluator->Ncity);
    printf("Cross operator created\n");
    
    // 设置父代并生成AB环
    int flagC[10] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
    tCross->SetParents(parentA, parentB, flagC, 1);
    printf("Parents set, AB cycles generated\n");
    
    // 打印生成的AB环数量
    printf("Number of AB cycles: %d\n", tCross->fNumOfABcycle);
    
    // 创建一个子代
    TIndi tKid;
    tKid = parentA;
    
    // 手动应用前两个AB环
    if (tCross->fNumOfABcycle >= 2) {
        printf("Manually applying AB cycle 0...\n");
        tCross->ChangeSol(tKid, 0, 1);
        
        // 验证第一个环应用后的图结构
        tCross->VerifyTourIntegrity(tKid, "After applying first cycle");
        
        printf("Manually applying AB cycle 1...\n");
        tCross->ChangeSol(tKid, 1, 1);
        
        // 验证第二个环应用后的图结构
        tCross->VerifyTourIntegrity(tKid, "After applying second cycle");
        
        // 尝试执行MakeUnit
        printf("Calling MakeUnit...\n");
        tCross->MakeUnit();
        
        printf("MakeUnit completed successfully\n");
    } else {
        printf("Not enough AB cycles generated (need at least 2)\n");
    }
    
    printf("Debug harness completed successfully\n");
    
    delete tCross;
    return 0;
} 