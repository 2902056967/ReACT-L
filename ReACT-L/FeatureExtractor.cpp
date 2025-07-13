#include "FeatureExtractor.h"
#include <vector>
#include <numeric>   // for std::accumulate
#include <algorithm> // for std::max/min
#include <limits>    // for std::numeric_limits

// 构造函数
TFeatureExtractor::TFeatureExtractor(TEvaluator* evaluator) 
    : fEvaluator(evaluator) {}

// 析构函数
TFeatureExtractor::~TFeatureExtractor() {}

// 核心方法：提取单个AB环的特征
std::vector<double> TFeatureExtractor::ExtractFeatures(const int* ab_cycle_data, const TIndi& parentA, const TIndi& parentB) {
    std::vector<double> features;
    
    double f1 = CalculateF1_PortalCount(ab_cycle_data);
    double f2 = CalculateF2_SubtourCount(ab_cycle_data);
    double f3 = CalculateF3_PortalToSubtourRatio(f1, f2);
    double f4 = CalculateF4_AverageEdgeLength(ab_cycle_data);
    double f5 = CalculateF5_LongestEdge(ab_cycle_data);
    double f6 = CalculateF6_ShortestEdge(ab_cycle_data);
    double f7 = CalculateF7_ParentalSimilarity(ab_cycle_data, parentA, parentB);
    double f8 = CalculateF8_CycleEdgeCount(ab_cycle_data);
    
    features.assign({f1, f2, f3, f4, f5, f6, f7, f8});
    
    return features;
}

// F1: 门户数量 (C-顶点数)
double TFeatureExtractor::CalculateF1_PortalCount(const int* cycle_data) {
    // 顶点数量（即AB环长度）存储在 TCross::fABcycle 数组的第一个元素 (cycle_data[0])
    return static_cast<double>(cycle_data[0]);
}

// F2: 产生的子环数量 (占位符)
double TFeatureExtractor::CalculateF2_SubtourCount(const int* cycle_data) {
    // 这是一个复杂特征，与修复过程相关。在第一步，我们用占位符。
    return 0.0;
}

// F3: 门户数/子环数比值 (占位符)
double TFeatureExtractor::CalculateF3_PortalToSubtourRatio(double f1, double f2) {
    // 依赖 F2，因此也使用占位符。
    return 0.0;
}

// F4: 平均边长
double TFeatureExtractor::CalculateF4_AverageEdgeLength(const int* cycle_data) {
    int vertex_count = cycle_data[0];
    if (vertex_count <= 1) return 0.0;
    
    double total_length = 0.0;
    // 环的顶点存储在 cycle_data[2] 到 cycle_data[2 + vertex_count - 1]
    for (int i = 0; i < vertex_count; ++i) {
        int city1 = cycle_data[2 + i];
        int city2 = cycle_data[2 + ((i + 1) % vertex_count)]; // 循环回到起点
        total_length += fEvaluator->fEdgeDis[city1][city2];
    }
    
    return total_length / vertex_count;
}

// F5: 最长边
double TFeatureExtractor::CalculateF5_LongestEdge(const int* cycle_data) {
    int vertex_count = cycle_data[0];
    if (vertex_count <= 1) return 0.0;
    
    double max_length = 0.0;
    for (int i = 0; i < vertex_count; ++i) {
        int city1 = cycle_data[2 + i];
        int city2 = cycle_data[2 + ((i + 1) % vertex_count)];
        max_length = std::max(max_length, static_cast<double>(fEvaluator->fEdgeDis[city1][city2]));
    }
    
    return max_length;
}

// F6: 最短边
double TFeatureExtractor::CalculateF6_ShortestEdge(const int* cycle_data) {
    int vertex_count = cycle_data[0];
    if (vertex_count <= 1) return 0.0;
    
    double min_length = std::numeric_limits<double>::max();
    for (int i = 0; i < vertex_count; ++i) {
        int city1 = cycle_data[2 + i];
        int city2 = cycle_data[2 + ((i + 1) % vertex_count)];
        min_length = std::min(min_length, static_cast<double>(fEvaluator->fEdgeDis[city1][city2]));
    }
    
    return min_length;
}

// F7: 与父代重合边比例
double TFeatureExtractor::CalculateF7_ParentalSimilarity(const int* cycle_data, const TIndi& parentA, const TIndi& parentB) {
    int vertex_count = cycle_data[0];
    if (vertex_count == 0) return 0.0;

    int shared_edge_count = 0;
    for (int i = 0; i < vertex_count; ++i) {
        int u = cycle_data[2 + i];
        int v = cycle_data[2 + ((i + 1) % vertex_count)];

        // 检查边(u,v)是否在父代A或父代B中
        bool in_parentA = (parentA.fLink[u][0] == v || parentA.fLink[u][1] == v);
        bool in_parentB = (parentB.fLink[u][0] == v || parentB.fLink[u][1] == v);

        if (in_parentA || in_parentB) {
            shared_edge_count++;
        }
    }
    return static_cast<double>(shared_edge_count) / vertex_count;
}

// F8: AB环长度（边数）
double TFeatureExtractor::CalculateF8_CycleEdgeCount(const int* cycle_data) {
    // AB环的边数等于其顶点数
    return static_cast<double>(cycle_data[0]);
} 