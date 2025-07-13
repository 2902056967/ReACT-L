"""
ReACT-L 缓冲区实现

这个模块提供了用于存储和管理强化学习经验数据的缓冲区。
"""

class RolloutBuffer:
    def __init__(self):
        """
        初始化一个空的经验回放缓冲区
        """
        self.states = []        # 状态 s_t
        self.actions = []       # 动作 a_t
        self.log_probs = []     # 动作的对数概率 log_prob(a_t|s_t)
        self.rewards = []       # 奖励 r_t
        self.dones = []         # 是否结束 done_t
        self.states_agg = []    # 聚合状态 s_agg_t (用于Critic)
        
    def add(self, state, action, log_prob, reward, done, state_agg=None):
        """
        添加一条经验到缓冲区
        
        Args:
            state: 状态向量 (8维特征)
            action: 执行的动作 (0或1)
            log_prob: 动作的对数概率
            reward: 获得的奖励
            done: 是否是终止状态
            state_agg: 聚合状态 (16维，用于Critic，可选)
        """
        self.states.append(state)
        self.actions.append(action)
        self.log_probs.append(log_prob)
        self.rewards.append(reward)
        self.dones.append(done)
        
        if state_agg is not None:
            self.states_agg.append(state_agg)
    
    def get_all_data(self):
        """
        获取缓冲区中的所有数据
        
        Returns:
            tuple: (states, actions, log_probs, rewards, dones, states_agg)
        """
        return (
            self.states,
            self.actions,
            self.log_probs,
            self.rewards,
            self.dones,
            self.states_agg
        )
    
    def __len__(self):
        """
        获取缓冲区中的经验数量
        
        Returns:
            int: 缓冲区中的经验数量
        """
        return len(self.states)
    
    def clear(self):
        """
        清空缓冲区
        """
        self.states = []
        self.actions = []
        self.log_probs = []
        self.rewards = []
        self.dones = []
        self.states_agg = [] 