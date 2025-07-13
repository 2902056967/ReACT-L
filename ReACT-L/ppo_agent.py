import torch
import torch.optim as optim
import torch.nn as nn
from torch.distributions import Bernoulli
from models import Actor, Critic

class PPOAgent:
    def __init__(self, state_dim=8, action_dim=1, learning_rate=1e-4, gamma=0.99, epsilon=0.2, c1=0.5, c2=0.01):
        """
        初始化 PPO 智能体
        """
        self.actor = Actor(input_dim=state_dim)
        self.critic = Critic(input_dim=state_dim*2) # Critic 输入是16维
        self.actor_optimizer = optim.Adam(self.actor.parameters(), lr=learning_rate)
        self.critic_optimizer = optim.Adam(self.critic.parameters(), lr=learning_rate)
        
        # 存储超参数
        self.gamma = gamma
        self.epsilon = epsilon
        self.c1 = c1 # 价值函数损失权重
        self.c2 = c2 # 熵奖励权重
        self.MseLoss = nn.MSELoss() # 添加均方误差损失函数

    def select_action(self, state_tensor):
        """
        根据单个状态（AB环特征），通过 Actor 网络输出一个决策动作
        """
        with torch.no_grad():
            action_prob = self.actor(state_tensor)
            dist = Bernoulli(action_prob)
            action = dist.sample()
            action_logprob = dist.log_prob(action)
        # 返回动作本身，以及它的对数概率
        return action.item(), action_logprob.item()

    def update(self, buffer):
        """
        使用缓冲区中的数据更新策略和价值函数
        
        Args:
            buffer: 包含经验数据的RolloutBuffer对象
        """
        print("\n--- [PPO UPDATE START] ---")
        print(f"Buffer size: {len(buffer)}")
        
        # 如果缓冲区为空，则跳过更新
        if len(buffer) == 0:
            print("Buffer is empty, skipping update.")
            return
        
        # 将缓冲区数据转换为张量
        states = torch.FloatTensor(buffer.states)
        actions = torch.FloatTensor(buffer.actions).view(-1, 1)
        log_probs = torch.FloatTensor(buffer.log_probs).view(-1, 1)
        rewards = torch.FloatTensor(buffer.rewards).view(-1, 1)
        dones = torch.FloatTensor(buffer.dones).view(-1, 1)
        states_agg = torch.FloatTensor(buffer.states_agg)
        
        print(f"Converted tensors - States: {states.shape}, Actions: {actions.shape}, Log_probs: {log_probs.shape}, Rewards: {rewards.shape}")
        
        # 计算优势函数估计
        with torch.no_grad():
            values = self.critic(states_agg)
            next_values = torch.zeros_like(values)
            for i in range(len(buffer) - 1):
                next_values[i] = values[i + 1]
            
            # 使用TD(λ)计算优势函数
            advantages = torch.zeros_like(rewards)
            returns = torch.zeros_like(rewards)
            gae = 0
            for i in reversed(range(len(buffer))):
                if i == len(buffer) - 1:
                    next_value = 0  # 最后一个状态的下一个值为0
                else:
                    next_value = values[i + 1]
                
                # TD误差
                delta = rewards[i] + self.gamma * next_value * (1 - dones[i]) - values[i]
                
                # 广义优势估计(GAE)
                gae = delta + self.gamma * 0.95 * (1 - dones[i]) * gae
                advantages[i] = gae
                
                # 计算回报
                returns[i] = advantages[i] + values[i]
        
        print(f"Calculated advantages: mean={advantages.mean().item():.4f}, std={advantages.std().item():.4f}")
        
        # 执行多个epoch的更新
        for _ in range(5):  # 通常PPO使用多个epoch
            # 获取新的动作概率
            action_probs = self.actor(states)
            dist = Bernoulli(action_probs)
            
            # 修复：确保new_log_probs的维度与log_probs一致
            actions_flat = actions.flatten()
            new_log_probs = []
            
            for i in range(len(actions_flat)):
                action = actions_flat[i]
                state = states[i]
                # 为每个状态-动作对单独计算对数概率
                action_prob = self.actor(state)
                dist_i = Bernoulli(action_prob)
                log_prob = dist_i.log_prob(torch.tensor(action))
                new_log_probs.append(log_prob.item())
            
            # 转换为张量并调整形状
            new_log_probs = torch.FloatTensor(new_log_probs).view(-1, 1)
            
            print(f"Shape check - new_log_probs: {new_log_probs.shape}, log_probs: {log_probs.shape}")
            
            # 计算策略比率
            ratio = torch.exp(new_log_probs - log_probs)
            
            # 计算裁剪的目标函数
            surr1 = ratio * advantages
            surr2 = torch.clamp(ratio, 1.0 - self.epsilon, 1.0 + self.epsilon) * advantages
            
            # 计算actor损失
            actor_loss = -torch.min(surr1, surr2).mean()
            
            # 计算critic损失
            value_pred = self.critic(states_agg)
            critic_loss = self.MseLoss(value_pred, returns)
            
            # 计算熵奖励
            entropy = dist.entropy().mean()
            
            # 总损失
            loss = actor_loss + self.c1 * critic_loss - self.c2 * entropy
            
            print(f"Loss components - Actor: {actor_loss.item():.4f}, Critic: {critic_loss.item():.4f}, Entropy: {entropy.item():.4f}")
            
            # 执行梯度更新
            self.actor_optimizer.zero_grad()
            self.critic_optimizer.zero_grad()
            loss.backward()
            self.actor_optimizer.step()
            self.critic_optimizer.step()
        
        print("--- [PPO UPDATE COMPLETE] ---\n") 