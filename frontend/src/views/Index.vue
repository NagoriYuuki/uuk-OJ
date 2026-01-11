<template>
    <div class="home-container">
        <!-- 1. Hero 区域：视觉中心 -->
        <div class="hero-section">
            <div class="hero-content">
                <div class="hero-text">
                    <div class="badge-pill">✨ New: 噶人们我终于把这个写完了</div>
                    <h1 class="hero-title">
                        Code, Compile, <span class="gradient-text">Conquer.</span>
                    </h1>
                    <p class="hero-desc">
                        uuk-OJ 是一个简约、迅速且专注于用户体验的在线评测系统。<br>
                        无论你是算法新手还是竞赛老手，这里都是你磨练技艺的最佳场所。
                    </p>
                    <div class="hero-actions">
                        <RouterLink :to="{ name: 'problem-list' }" class="btn btn-primary btn-lg">
                            开始刷题
                            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                stroke-width="2">
                                <path d="M5 12h14M12 5l7 7-7 7" />
                            </svg>
                        </RouterLink>
                        <a href="https://github.com/NagoriYuuki/uuk-OJ" target="_blank" class="btn btn-secondary btn-lg">
                            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                stroke-width="2">
                                <path
                                    d="M9 19c-5 1.5-5-2.5-7-3m14 6v-3.87a3.37 3.37 0 0 0-.94-2.61c3.14-.35 6.44-1.54 6.44-7A5.44 5.44 0 0 0 20 4.77 5.07 5.07 0 0 0 19.91 1S18.73.65 16 2.48a13.38 13.38 0 0 0-7 0C6.27.65 5.09 1 5.09 1A5.07 5.07 0 0 0 5 4.77a5.44 5.44 0 0 0-1.5 3.78c0 5.42 3.3 6.61 6.44 7A3.37 3.37 0 0 0 9 18.13V22" />
                            </svg>
                            GitHub
                        </a>
                    </div>
                </div>
                <!-- 右侧装饰图 -->
                <div class="hero-decoration">
                    <div class="code-window">
                        <div class="window-header">
                            <span class="dot red"></span>
                            <span class="dot yellow"></span>
                            <span class="dot green"></span>
                        </div>
                        <div class="window-body">
                            <div class="code-line"><span class="c-k">#include</span> <span
                                    class="c-s">&lt;iostream&gt;</span></div>
                            <div class="code-line"><span class="c-k">using</span> <span class="c-k">namespace</span>
                                std;</div>
                            <div class="code-line">&nbsp;</div>
                            <div class="code-line"><span class="c-t">int</span> <span class="c-f">main</span>() {</div>
                            <div class="code-line indent">cout &lt;&lt; <span class="c-s">"Hello, uuk-OJ!"</span>;</div>
                            <div class="code-line indent"><span class="c-k">return</span> <span class="c-n">0</span>;
                            </div>
                            <div class="code-line">}</div>
                        </div>
                    </div>
                    <div class="blob"></div>
                </div>
            </div>

            <!-- 数据统计条 -->
            <div class="stats-bar">
                <div class="stat-item">
                    <div class="stat-value">G++ 23</div>
                    <div class="stat-label">编译环境</div>
                </div>
                <div class="stat-divider"></div>
                <div class="stat-item">
                    <div class="stat-value">1ms</div>
                    <div class="stat-label">评测延迟</div>
                </div>
                <div class="stat-divider"></div>
                <div class="stat-item">
                    <div class="stat-value">Secure</div>
                    <div class="stat-label">沙箱隔离</div>
                </div>
            </div>
        </div>

        <!-- 2. 功能导航卡片 -->
        <div class="section-title">
            <h2>探索</h2>
            <p>选择你的下一站</p>
        </div>

        <div class="nav-grid">
            <RouterLink :to="{ name: 'problem-list' }" class="nav-card">
                <div class="icon-box blue">
                    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20" />
                        <path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z" />
                    </svg>
                </div>
                <div class="card-content">
                    <h3>题库</h3>
                    <p>海量题目，涵盖多种算法分类</p>
                </div>
                <div class="arrow">&rarr;</div>
            </RouterLink>

            <RouterLink :to="{ name: 'submissions' }" class="nav-card">
                <div class="icon-box green">
                    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M22 11.08V12a10 10 0 1 1-5.93-9.14" />
                        <polyline points="22 4 12 14.01 9 11.01" />
                    </svg>
                </div>
                <div class="card-content">
                    <h3>评测记录</h3>
                    <p>实时查看代码运行状态与结果</p>
                </div>
                <div class="arrow">&rarr;</div>
            </RouterLink>

            <RouterLink v-if="isLoggedIn" to="/me" class="nav-card">
                <div class="icon-box purple">
                    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2" />
                        <circle cx="12" cy="7" r="4" />
                    </svg>
                </div>
                <div class="card-content">
                    <h3>个人中心</h3>
                    <p>你好，{{ displayName }}！查看你的成长轨迹</p>
                </div>
                <div class="arrow">&rarr;</div>
            </RouterLink>
            <RouterLink v-else :to="{ name: 'login' }" class="nav-card">
                <div class="icon-box orange">
                    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <path d="M15 3h4a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2h-4" />
                        <polyline points="10 17 15 12 10 7" />
                        <line x1="15" y1="12" x2="3" y2="12" />
                    </svg>
                </div>
                <div class="card-content">
                    <h3>登录 / 注册</h3>
                    <p>加入社区，保存你的提交记录</p>
                </div>
                <div class="arrow">&rarr;</div>
            </RouterLink>
        </div>

        <!-- 3. 公告与管理区 -->
        <div class="grid-layout">
            <!-- 系统公告 -->
            <div class="info-card">
                <div class="card-header">
                    <h3>📢 系统公告</h3>
                </div>
                <ul class="notice-list">
                    <li>
                        <span class="date">2026-01-10</span>
                        <span class="text">uuk-OJ 正式上线！欢迎提交 Issue。</span>
                    </li>
                </ul>
            </div>

            <!-- 快捷管理 (仅管理员可见) -->
            <div v-if="isAdmin" class="info-card admin-card">
                <div class="card-header">
                    <h3>🛡️ 管理员面板</h3>
                </div>
                <div class="admin-grid">
                    <RouterLink :to="{ name: 'problem-ma' }" class="admin-btn">
                        <span>题目管理</span>
                    </RouterLink>
                    <RouterLink :to="{ name: 'user-ma' }" class="admin-btn">
                        <span>用户管理</span>
                    </RouterLink>
                    <button class="admin-btn warning" @click="logout">
                        <span>退出登录</span>
                    </button>
                </div>
            </div>

            <!-- 未登录时的欢迎卡片 -->
            <div v-else-if="!isLoggedIn" class="info-card welcome-card">
                <h3>Ready to code?</h3>
                <p>登录后即可参与排名，记录每一次 AC 的喜悦。</p>
                <div class="welcome-actions">
                    <RouterLink :to="{ name: 'login' }" class="btn btn-sm btn-primary">立即登录</RouterLink>
                    <RouterLink :to="{ name: 'register' }" class="btn btn-sm btn-secondary">注册账号</RouterLink>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { RouterLink, useRouter } from 'vue-router'
import { computed, onMounted } from 'vue'
import { authState, clearSession, refreshUserInfo } from '../utils/auth'

const router = useRouter()

const isLoggedIn = computed(() => !!authState.token)
const isAdmin = computed(() => authState.role === 1)
const displayName = computed(() => authState.username || '用户')

onMounted(() => {
    refreshUserInfo(false)
})

function logout() {
    clearSession()
    router.push('/')
}
</script>

<style scoped>
/* 引入全局变量 */
:root {
    --primary: #4f46e5;
    --primary-hover: #4338ca;
    --secondary: #64748b;
    --bg-page: #f8fafc;
    --card-bg: #ffffff;
    --text-main: #0f172a;
    --text-muted: #64748b;
    --shadow-sm: 0 1px 2px 0 rgb(0 0 0 / 0.05);
    --shadow-md: 0 4px 6px -1px rgb(0 0 0 / 0.1);
    --shadow-lg: 0 10px 15px -3px rgb(0 0 0 / 0.1);
    --radius: 16px;
}

.home-container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 0 1.5rem 3rem;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    color: var(--text-main);
}

/* --- Hero Section --- */
.hero-section {
    position: relative;
    background: linear-gradient(135deg, #eef2ff 0%, #e0e7ff 100%);
    border-radius: 24px;
    padding: 4rem 3rem;
    margin-top: 1.5rem;
    margin-bottom: 3rem;
    overflow: hidden;
    display: flex;
    flex-direction: column;
    gap: 3rem;
}

.hero-content {
    display: flex;
    justify-content: space-between;
    align-items: center;
    position: relative;
    z-index: 2;
}

.hero-text {
    max-width: 550px;
}

.badge-pill {
    display: inline-block;
    background: rgba(79, 70, 229, 0.1);
    color: var(--primary);
    padding: 6px 12px;
    border-radius: 20px;
    font-size: 0.85rem;
    font-weight: 600;
    margin-bottom: 1.5rem;
}

.hero-title {
    font-size: 3.5rem;
    font-weight: 800;
    line-height: 1.1;
    margin: 0 0 1.5rem 0;
    letter-spacing: -1px;
}

.gradient-text {
    background: linear-gradient(to right, #4f46e5, #ec4899);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
}

.hero-desc {
    font-size: 1.15rem;
    color: var(--text-muted);
    line-height: 1.6;
    margin-bottom: 2rem;
}

.hero-actions {
    display: flex;
    gap: 1rem;
}

/* 装饰性代码窗口 */
.hero-decoration {
    position: relative;
    display: none;
    /* 移动端隐藏 */
}

@media (min-width: 900px) {
    .hero-decoration {
        display: block;
    }
}

.code-window {
    background: #1e293b;
    border-radius: 12px;
    padding: 1.5rem;
    width: 320px;
    box-shadow: 0 20px 25px -5px rgb(0 0 0 / 0.1), 0 8px 10px -6px rgb(0 0 0 / 0.1);
    position: relative;
    z-index: 2;
    transform: rotate(3deg);
    transition: transform 0.3s;
}

.code-window:hover {
    transform: rotate(0deg);
}

.window-header {
    display: flex;
    gap: 6px;
    margin-bottom: 1rem;
}

.dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
}

.red {
    background: #ef4444;
}

.yellow {
    background: #f59e0b;
}

.green {
    background: #10b981;
}

.window-body {
    font-family: 'Monaco', monospace;
    font-size: 0.85rem;
    color: #e2e8f0;
    line-height: 1.5;
}

.c-k {
    color: #c678dd;
}

.c-s {
    color: #98c379;
}

.c-t {
    color: #e5c07b;
}

.c-f {
    color: #61afef;
}

.c-n {
    color: #d19a66;
}

.indent {
    padding-left: 1.5rem;
}

/* 背景光晕 */
.blob {
    position: absolute;
    top: -50%;
    left: -50%;
    width: 200%;
    height: 200%;
    background: radial-gradient(circle, rgba(99, 102, 241, 0.15) 0%, rgba(255, 255, 255, 0) 70%);
    z-index: 1;
    pointer-events: none;
}

/* 统计条 */
.stats-bar {
    display: flex;
    background: rgba(255, 255, 255, 0.6);
    backdrop-filter: blur(10px);
    padding: 1.5rem;
    border-radius: 16px;
    justify-content: space-around;
    align-items: center;
    border: 1px solid rgba(255, 255, 255, 0.8);
}

.stat-item {
    text-align: center;
}

.stat-value {
    font-size: 1.5rem;
    font-weight: 800;
    color: #0f172a;
    margin-bottom: 4px;
}

.stat-label {
    font-size: 0.85rem;
    color: var(--text-muted);
    font-weight: 500;
    text-transform: uppercase;
    letter-spacing: 1px;
}

.stat-divider {
    width: 1px;
    height: 40px;
    background: #cbd5e1;
}

/* --- Navigation Grid --- */
.section-title {
    text-align: center;
    margin-bottom: 2rem;
}

.section-title h2 {
    font-size: 2rem;
    margin: 0 0 0.5rem 0;
    color: #0f172a;
}

.section-title p {
    color: var(--text-muted);
}

.nav-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    gap: 1.5rem;
    margin-bottom: 3rem;
}

.nav-card {
    background: white;
    border-radius: 16px;
    padding: 1.5rem;
    display: flex;
    align-items: center;
    gap: 1.2rem;
    text-decoration: none;
    color: inherit;
    border: 1px solid #f1f5f9;
    box-shadow: var(--shadow-sm);
    transition: all 0.3s ease;
    position: relative;
    overflow: hidden;
}

.nav-card:hover {
    transform: translateY(-5px);
    box-shadow: var(--shadow-lg);
    border-color: rgba(79, 70, 229, 0.1);
}

.icon-box {
    width: 50px;
    height: 50px;
    border-radius: 12px;
    display: flex;
    align-items: center;
    justify-content: center;
    flex-shrink: 0;
}

.blue {
    background: #eff6ff;
    color: #3b82f6;
}

.green {
    background: #f0fdf4;
    color: #10b981;
}

.purple {
    background: #f5f3ff;
    color: #8b5cf6;
}

.orange {
    background: #fff7ed;
    color: #f97316;
}

.card-content h3 {
    margin: 0 0 0.3rem 0;
    font-size: 1.1rem;
    color: #0f172a;
}

.card-content p {
    margin: 0;
    font-size: 0.85rem;
    color: var(--text-muted);
    line-height: 1.4;
}

.arrow {
    margin-left: auto;
    color: #cbd5e1;
    font-size: 1.2rem;
    transition: transform 0.3s;
}

.nav-card:hover .arrow {
    transform: translateX(5px);
    color: var(--primary);
}

/* --- Info & Admin Layout --- */
.grid-layout {
    display: grid;
    grid-template-columns: 2fr 1fr;
    gap: 1.5rem;
}

@media (max-width: 768px) {
    .grid-layout {
        grid-template-columns: 1fr;
    }
}

.info-card {
    background: white;
    border-radius: 16px;
    padding: 1.5rem;
    border: 1px solid #f1f5f9;
    box-shadow: var(--shadow-sm);
}

.card-header h3 {
    margin: 0;
    font-size: 1.1rem;
    display: flex;
    align-items: center;
    gap: 8px;
}

.notice-list {
    list-style: none;
    padding: 0;
    margin: 1rem 0 0 0;
}

.notice-list li {
    display: flex;
    gap: 1rem;
    padding: 0.75rem 0;
    border-bottom: 1px dashed #e2e8f0;
    font-size: 0.95rem;
    color: #334155;
}

.notice-list li:last-child {
    border-bottom: none;
}

.notice-list .date {
    color: var(--text-muted);
    font-family: 'Monaco', monospace;
    font-size: 0.85rem;
    white-space: nowrap;
}

/* 管理员面板 */
.admin-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 10px;
    margin-top: 1rem;
}

.admin-btn {
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 10px;
    background: #f8fafc;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    color: #334155;
    text-decoration: none;
    font-size: 0.9rem;
    font-weight: 500;
    transition: all 0.2s;
    cursor: pointer;
}

.admin-btn:hover {
    background: #f1f5f9;
    border-color: #cbd5e1;
}

.admin-btn.warning:hover {
    background: #fef2f2;
    color: #ef4444;
    border-color: #fecaca;
}

/* 欢迎卡片 */
.welcome-card {
    text-align: center;
    display: flex;
    flex-direction: column;
    justify-content: center;
}

.welcome-card h3 {
    margin-bottom: 0.5rem;
    color: #0f172a;
}

.welcome-card p {
    font-size: 0.9rem;
    color: var(--text-muted);
    margin-bottom: 1.5rem;
}

.welcome-actions {
    display: flex;
    gap: 10px;
    justify-content: center;
}

/* 按钮通用 */
.btn {
    display: inline-flex;
    align-items: center;
    gap: 0.5rem;
    border-radius: 8px;
    font-weight: 600;
    text-decoration: none;
    transition: all 0.2s;
    cursor: pointer;
    border: none;
}

.btn-lg {
    padding: 0.75rem 1.5rem;
    font-size: 1rem;
}

.btn-sm {
    padding: 0.5rem 1rem;
    font-size: 0.85rem;
}

.btn-primary {
    background: var(--primary);
    color: white;
    box-shadow: 0 4px 10px rgba(79, 70, 229, 0.3);
}

.btn-primary:hover {
    background: var(--primary-hover);
    transform: translateY(-2px);
}

.btn-secondary {
    background: white;
    color: #0f172a;
    border: 1px solid #e2e8f0;
}

.btn-secondary:hover {
    background: #f8fafc;
    border-color: #cbd5e1;
}

@media (max-width: 640px) {
    .hero-title {
        font-size: 2.5rem;
    }

    .hero-section {
        padding: 2rem 1.5rem;
    }

    .hero-actions {
        flex-direction: column;
    }

    .btn {
        width: 100%;
        justify-content: center;
    }

    .stats-bar {
        flex-direction: column;
        gap: 1.5rem;
    }

    .stat-divider {
        width: 40px;
        height: 1px;
    }
}
</style>