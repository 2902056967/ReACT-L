"""
ReACT-L Python桥接API

这个模块提供了C++和Python之间的通信接口。
它现在使用PPO智能体来生成决策，替代了之前的虚拟决策函数。
"""

import torch
import datetime
import numpy as np
from ppo_agent import PPOAgent
from buffers import RolloutBuffer

# 在模块加载时，创建一个全局的 PPOAgent 实例和 RolloutBuffer 实例
# 这确保了在整个 C++ 程序运行期间，我们使用的是同一个智能体和缓冲区
agent = PPOAgent()
buffer = RolloutBuffer()
print("[ReACT-L INFO] Python: PPOAgent and RolloutBuffer created and ready.")

# 用于临时存储最近的状态和动作信息，以便与奖励配对
last_states = []
last_actions = []
last_log_probs = []

def get_action_mask(feature_vectors):
    """
    真实的决策函数。
    它接收 C++ 传来的特征向量，并使用 PPO Agent 的 Actor 网络来生成决策。
    同时，它会缓存当前的状态和动作，以便后续与奖励配对。
    """
    global last_states, last_actions, last_log_probs
    
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    num_cycles = len(feature_vectors)
    
    # 清空上一次的缓存
    last_states = []
    last_actions = []
    last_log_probs = []
    
    # 将 C++ 传来的数据转换为 PyTorch Tensors
    action_mask = []
    for features in feature_vectors:
        state_tensor = torch.FloatTensor(features)
        action, log_prob = agent.select_action(state_tensor)
        
        # 缓存当前状态、动作和对数概率
        last_states.append(features)
        last_actions.append(action)
        last_log_probs.append(log_prob)
        
        action_mask.append(int(action))
    
    # 注：我们已经修复了C++代码中的问题，现在可以安全地选择多个AB环
    print(f"[INFO] Number of AB cycles selected: {sum(action_mask)}")
    
    print(f"--- [Python DEBUG at {timestamp}] Actor network generated action mask: {action_mask} ---")
    
    return action_mask

def store_experience(rewards, done=False):
    """
    存储经验到缓冲区。
    这个函数会被 C++ 端调用，传递奖励值。
    它会将奖励与之前缓存的状态、动作配对，形成完整的经验元组。
    
    Args:
        rewards: 一个列表，包含每个动作获得的奖励
        done: 表示当前episode是否结束
    """
    global last_states, last_actions, last_log_probs
    
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"--- [Python DEBUG at {timestamp}] Storing {len(rewards)} experiences with rewards: {rewards} ---")
    
    # 确保rewards和last_states等长度一致
    if len(rewards) != len(last_states):
        print(f"[WARNING] Rewards length ({len(rewards)}) doesn't match states length ({len(last_states)})")
        # 可能需要一些错误处理逻辑
    
    # 为每个状态-动作对创建聚合状态（用于Critic）
    # 这里简单地将相邻两个状态拼接起来作为聚合状态
    states_agg = []
    for i in range(len(last_states)):
        if i == 0:  # 第一个状态没有前一个状态，用零向量代替
            prev_state = np.zeros_like(last_states[i])
        else:
            prev_state = last_states[i-1]
        
        # 拼接前一个状态和当前状态
        agg_state = np.concatenate([prev_state, last_states[i]])
        states_agg.append(agg_state)
    
    # 将经验添加到缓冲区
    for i in range(len(rewards)):
        if i < len(last_states):  # 确保索引有效
            buffer.add(
                state=last_states[i],
                action=last_actions[i],
                log_prob=last_log_probs[i],
                reward=rewards[i],
                done=done,
                state_agg=states_agg[i]
            )
    
    print(f"--- [Python DEBUG] Stored {len(rewards)} experiences. Buffer now contains {len(buffer)} items. ---")
    print(f"--- [Python INFO] Added {len(rewards)} experiences to buffer. Total buffer size: {len(buffer)} ---")

def trigger_update(k=5):
    """
    触发PPO智能体的更新。
    这个函数会被 C++ 端定期调用（例如每 K 代），用于更新智能体的策略。
    
    Args:
        k: 更新周期（每k代更新一次）
    """
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"--- [Python INFO at {timestamp}] Received update trigger from C++. ---")
    
    # 检查缓冲区是否有足够的数据
    if len(buffer) < 10:  # 设置一个最小阈值
        print(f"--- [Python WARNING] Not enough data in buffer ({len(buffer)} < 10). Skipping update. ---")
        return
    
    # 调用智能体的更新方法
    agent.update(buffer)
    
    # 清空缓冲区，准备下一批数据
    buffer.clear()
    print(f"--- [Python INFO] Rollout buffer cleared. Ready for next batch. ---")

# 保留原始的dummy函数以备需要
def get_action_mask_dummy(feature_vectors):
    """
    一个虚拟的决策函数，用于测试 C++/Python 通信。
    它接收特征向量，打印一条消息，然后返回一个随机的动作掩码。
    
    Args:
        feature_vectors: 一个Python列表，包含多个AB环的特征向量
        
    Returns:
        action_mask: 一个二进制列表，表示是否选择对应的AB环
    """
    import random
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    num_cycles = len(feature_vectors)
    
    print(f"--- [Python DEBUG at {timestamp}] Function 'get_action_mask_dummy' was called with {num_cycles} AB-cycles. ---")

    # 生成一个随机的动作掩码
    action_mask = [random.choice([0, 1]) for _ in range(num_cycles)]
    
    print(f"--- [Python DEBUG] Returning random action mask: {action_mask} ---\n")

    return action_mask 