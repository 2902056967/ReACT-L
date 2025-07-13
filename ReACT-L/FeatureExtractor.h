#ifndef __FEATURE_EXTRACTOR_H__
#define __FEATURE_EXTRACTOR_H__

#include <vector>
#include "evaluator.h" // 需要访问距离矩阵
#include "indi.h"       // 需要访问父代个体信息

// 前向声明 TCross 类，以避免在头文件中循环引用
class TCross;

class TFeatureExtractor {
public:
    // 构造函数，传入 TEvaluator 的指针以获取环境信息（如距离）
    TFeatureExtractor(TEvaluator* evaluator);
    ~TFeatureExtractor();

    // 核心公共方法：提取单个 AB 环的特征
    // ab_cycle_data: 指向 TCross::fABcycle 中某一行的数据指针
    // parentA, parentB: 当前交叉的两个父代个体
    std::vector<double> ExtractFeatures(const int* ab_cycle_data, const TIndi& parentA, const TIndi& parentB);

private:
    TEvaluator* fEvaluator; // 指向环境评估器的指针

    // 私有辅助方法，用于计算各项具体特征
    double CalculateF1_PortalCount(const int* cycle_data);
    double CalculateF2_SubtourCount(const int* cycle_data); // 占位符
    double CalculateF3_PortalToSubtourRatio(double f1, double f2); // 占位符
    double CalculateF4_AverageEdgeLength(const int* cycle_data);
    double CalculateF5_LongestEdge(const int* cycle_data);
    double CalculateF6_ShortestEdge(const int* cycle_data);
    double CalculateF7_ParentalSimilarity(const int* cycle_data, const TIndi& parentA, const TIndi& parentB); // 占位符
    double CalculateF8_CycleEdgeCount(const int* cycle_data);
};

#endif // __FEATURE_EXTRACTOR_H__ 