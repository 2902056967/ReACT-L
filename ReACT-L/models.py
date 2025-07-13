import torch
import torch.nn as nn

class Actor(nn.Module):
    """
    Actor 网络 (策略网络)
    输入: 单个 AB 环的 8 维特征
    输出: 选择该环的概率
    """
    def __init__(self, input_dim=8, hidden_dim=128, output_dim=1):
        super(Actor, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(input_dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, output_dim),
            nn.Sigmoid()  # 输出概率，所以用 Sigmoid
        )
    
    def forward(self, state):
        return self.network(state)

class Critic(nn.Module):
    """
    Critic 网络 (价值网络)
    输入: 聚合后的 16 维状态特征
    输出: 该状态的价值估计
    """
    def __init__(self, input_dim=16, hidden_dim=128, output_dim=1):
        super(Critic, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(input_dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, output_dim) # 输出价值，线性输出
        )
        
    def forward(self, state):
        return self.network(state) 