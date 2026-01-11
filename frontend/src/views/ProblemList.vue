<template>
    <div class="page-container">
        <!-- 头部区域 -->
        <div class="header-section">
            <div class="header-content">
                <div class="titles">
                    <h1 class="main-title">
                        <span class="icon">🧩</span> 题目列表
                    </h1>
                    <p class="subtitle">挑战自我，从每一行代码开始</p>
                </div>
                <div class="actions">
                    <RouterLink class="btn btn-secondary" :to="{ name: 'index' }">
                        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                            stroke-width="2">
                            <path
                                d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6" />
                        </svg>
                        首页
                    </RouterLink>
                    <button class="btn btn-primary" @click="reload" :disabled="loading">
                        <svg :class="{ 'spin': loading }" width="16" height="16" viewBox="0 0 24 24" fill="none"
                            stroke="currentColor" stroke-width="2">
                            <path
                                d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15" />
                        </svg>
                        {{ loading ? '刷新中' : '刷新' }}
                    </button>
                </div>
            </div>
        </div>

        <!-- 搜索栏 -->
        <div class="search-bar-wrapper">
            <div class="search-box">
                <svg class="search-icon" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                    stroke-width="2">
                    <circle cx="11" cy="11" r="8" />
                    <path d="M21 21l-4.35-4.35" />
                </svg>
                <input v-model="query" placeholder="搜索题目编号、标题或关键词..." class="search-input" />
                <div class="search-badge" v-if="query">
                    按 Enter 确认
                </div>
            </div>
            <div class="stats-badge">
                共 <span>{{ filteredProblems.length }}</span> 题
            </div>
        </div>

        <!-- 加载中 -->
        <div v-if="loading" class="problem-grid">
            <div v-for="n in 6" :key="n" class="skeleton-card"></div>
        </div>

        <!-- 错误提示 -->
        <div v-else-if="error" class="state-container error">
            <div class="state-icon">⚠️</div>
            <h3>加载失败</h3>
            <p>{{ error }}</p>
            <button class="btn btn-secondary" @click="reload">重试</button>
        </div>

        <!-- 空状态 -->
        <div v-else-if="!filteredProblems.length" class="state-container empty">
            <div class="state-icon">🍃</div>
            <h3>暂无题目</h3>
            <p>尝试更换搜索关键词，或稍后再来</p>
        </div>

        <!-- 题目列表 -->
        <TransitionGroup v-else name="list" tag="div" class="problem-grid">
            <RouterLink v-for="p in filteredProblems" :key="p.id" class="problem-card-link"
                :to="{ name: 'problem-detail', params: { id: p.id } }">
                <div class="problem-card">
                    <div class="card-top">
                        <span class="id-tag">#{{ p.id }}</span>
                        <div class="meta-badges">
                            <span class="badge time">
                                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                    stroke-width="2">
                                    <circle cx="12" cy="12" r="10" />
                                    <path d="M12 6v6l4 2" />
                                </svg>
                                {{ p.time_limit }}ms
                            </span>
                            <span class="badge mem">
                                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                    stroke-width="2">
                                    <path d="M4 6h16M4 10h16M4 14h16M4 18h16" />
                                </svg>
                                {{ p.mem_limit }}KB
                            </span>
                        </div>
                    </div>

                    <h3 class="problem-title" :title="p.title">{{ p.title }}</h3>

                    <div class="card-footer">
                        <div class="progress-section">
                            <div class="progress-info">
                                <span class="label">通过率</span>
                                <span class="value">{{ getPassRate(p) }}%</span>
                            </div>
                            <div class="progress-bar-bg">
                                <div class="progress-bar-fill" :style="{
                                    width: getPassRate(p) + '%',
                                    backgroundColor: getPassRateColor(getPassRate(p))
                                }"></div>
                            </div>
                        </div>
                        <div class="submit-count" title="提交总数">
                            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                                stroke-width="2">
                                <path d="M16 21v-2a4 4 0 00-4-4H5a4 4 0 00-4 4v2" />
                                <circle cx="8.5" cy="7" r="4" />
                                <path d="M20 8v6M23 11h-6" />
                            </svg>
                            {{ p.sub_count }}
                        </div>
                    </div>

                    <!-- 箭头现在会在右下角出现 -->
                    <div class="hover-arrow">
                        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                            stroke-width="2">
                            <path d="M5 12h14M12 5l7 7-7 7" />
                        </svg>
                    </div>
                </div>
            </RouterLink>
        </TransitionGroup>
    </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { RouterLink } from 'vue-router'
import { apiGet } from '../utils/api'

type Problem = {
    id: number
    title: string
    time_limit: number
    mem_limit: number
    sub_count: number
    ac_count: number
}

const problems = ref<Problem[]>([])
const loading = ref(false)
const error = ref('')
const query = ref('')

async function load() {
    loading.value = true
    error.value = ''
    try {
        const res = await apiGet('/api/problems')
        problems.value = Array.isArray(res) ? res : []
    } catch (e: any) {
        error.value = e?.message ?? String(e)
    } finally {
        loading.value = false
    }
}

function reload() {
    load()
}

const filteredProblems = computed(() => {
    const q = query.value.trim().toLowerCase()
    if (!q) return problems.value
    return problems.value.filter((p) => {
        const idMatch = String(p.id).includes(q)
        const titleMatch = (p.title || '').toLowerCase().includes(q)
        return idMatch || titleMatch
    })
})

function getPassRate(p: Problem) {
    if (!p.sub_count) return 0
    return Math.round((p.ac_count / Math.max(1, p.sub_count)) * 100)
}

function getPassRateColor(rate: number) {
    if (rate >= 60) return '#10b981'
    if (rate >= 30) return '#f59e0b'
    return '#ef4444'
}

onMounted(load)
</script>

<style scoped>
:root {
    --primary: #4f46e5;
    --primary-hover: #4338ca;
    --bg-page: #f8fafc;
    --bg-card: #ffffff;
    --text-main: #1e293b;
    --text-muted: #64748b;
    --border: #e2e8f0;
    --shadow-sm: 0 1px 2px 0 rgb(0 0 0 / 0.05);
    --shadow-md: 0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1);
    --shadow-lg: 0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1);
    --radius: 12px;
}

.page-container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 2rem 1rem;
    color: var(--text-main);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
}

.header-section {
    margin-bottom: 2rem;
}

.header-content {
    display: flex;
    justify-content: space-between;
    align-items: flex-end;
    flex-wrap: wrap;
    gap: 1rem;
}

.main-title {
    font-size: 2rem;
    font-weight: 800;
    color: #0f172a;
    margin: 0;
    display: flex;
    align-items: center;
    gap: 0.5rem;
}

.subtitle {
    color: var(--text-muted);
    margin-top: 0.5rem;
    font-size: 0.95rem;
}

.actions {
    display: flex;
    gap: 0.75rem;
}

.btn {
    display: inline-flex;
    align-items: center;
    gap: 0.5rem;
    padding: 0.5rem 1rem;
    border-radius: 8px;
    font-size: 0.875rem;
    font-weight: 500;
    cursor: pointer;
    transition: all 0.2s;
    border: 1px solid transparent;
    text-decoration: none;
}

.btn-primary {
    background-color: var(--primary);
    color: white;
    box-shadow: 0 4px 6px -2px rgba(79, 70, 229, 0.3);
}

.btn-primary:hover:not(:disabled) {
    background-color: var(--primary-hover);
    transform: translateY(-1px);
}

.btn-secondary {
    background-color: white;
    border-color: var(--border);
    color: var(--text-main);
}

.btn-secondary:hover {
    background-color: #f1f5f9;
    border-color: #cbd5e1;
}

.btn:disabled {
    opacity: 0.7;
    cursor: not-allowed;
}

.spin {
    animation: spin 1s linear infinite;
}

@keyframes spin {
    from {
        transform: rotate(0deg);
    }

    to {
        transform: rotate(360deg);
    }
}

.search-bar-wrapper {
    display: flex;
    gap: 1rem;
    margin-bottom: 2rem;
    align-items: stretch;
}

.search-box {
    flex: 1;
    position: relative;
    display: flex;
    align-items: center;
}

.search-icon {
    position: absolute;
    left: 1rem;
    color: #94a3b8;
    pointer-events: none;
}

.search-input {
    width: 100%;
    padding: 0.85rem 1rem 0.85rem 3rem;
    border: 2px solid transparent;
    background-color: white;
    border-radius: var(--radius);
    font-size: 1rem;
    box-shadow: var(--shadow-sm);
    transition: all 0.3s;
    outline: none;
}

.search-input:focus {
    background-color: white;
    border-color: var(--primary);
    box-shadow: 0 0 0 4px rgba(79, 70, 229, 0.1);
}

.search-badge {
    position: absolute;
    right: 1rem;
    font-size: 0.75rem;
    color: #94a3b8;
    border: 1px solid #e2e8f0;
    padding: 0.1rem 0.4rem;
    border-radius: 4px;
    background: #f8fafc;
}

.stats-badge {
    display: flex;
    align-items: center;
    padding: 0 1.5rem;
    background: white;
    border-radius: var(--radius);
    box-shadow: var(--shadow-sm);
    font-weight: 500;
    color: var(--text-muted);
    white-space: nowrap;
}

.stats-badge span {
    color: var(--primary);
    font-weight: 700;
    margin: 0 0.25rem;
    font-size: 1.1rem;
}

/* --- 列表网格样式修改 --- */
.problem-grid {
    display: grid;
    /* 增加卡片间距，gap 同时控制横向和纵向，row-gap 单独加大纵向间距 */
    gap: 2rem;
    row-gap: 2.5rem;
    grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
}

.problem-card-link {
    text-decoration: none;
    color: inherit;
    display: block;
}

.problem-card {
    background: white;
    border-radius: var(--radius);
    padding: 1.25rem;
    height: 100%;
    display: flex;
    flex-direction: column;
    position: relative;
    /* 为箭头绝对定位提供锚点 */
    border: 1px solid transparent;
    box-shadow: var(--shadow-sm);
    transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    overflow: hidden;
}

.problem-card:hover {
    transform: translateY(-4px);
    box-shadow: var(--shadow-lg);
    border-color: rgba(79, 70, 229, 0.1);
}

.card-top {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 0.75rem;
}

.id-tag {
    font-family: 'Monaco', 'Consolas', monospace;
    font-size: 0.85rem;
    color: var(--text-muted);
    background: #f1f5f9;
    padding: 2px 8px;
    border-radius: 4px;
    font-weight: 600;
}

.meta-badges {
    display: flex;
    gap: 0.5rem;
}

.badge {
    font-size: 0.75rem;
    color: var(--text-muted);
    display: flex;
    align-items: center;
    gap: 3px;
    background: #f8fafc;
    padding: 2px 6px;
    border-radius: 4px;
}

.problem-title {
    margin: 0 0 1.5rem 0;
    font-size: 1.15rem;
    font-weight: 700;
    line-height: 1.4;
    color: #0f172a;
    display: -webkit-box;
    -webkit-line-clamp: 2;
    -webkit-box-orient: vertical;
    overflow: hidden;
    flex: 1;
}

.problem-card:hover .problem-title {
    color: var(--primary);
}

.card-footer {
    display: flex;
    justify-content: space-between;
    align-items: flex-end;
    gap: 1rem;
}

.progress-section {
    flex: 1;
}

.progress-info {
    display: flex;
    justify-content: space-between;
    font-size: 0.75rem;
    margin-bottom: 4px;
    color: var(--text-muted);
}

.progress-info .value {
    font-weight: 600;
    color: var(--text-main);
}

.progress-bar-bg {
    width: 100%;
    height: 6px;
    background: #f1f5f9;
    border-radius: 10px;
    overflow: hidden;
}

.progress-bar-fill {
    height: 100%;
    border-radius: 10px;
    transition: width 0.5s ease;
}

/* --- 提交次数与箭头交互动画 --- */
.submit-count {
    font-size: 0.75rem;
    color: #94a3b8;
    display: flex;
    align-items: center;
    gap: 4px;
    margin-bottom: 2px;
    transition: all 0.3s ease;
    /* 添加过渡 */
    opacity: 1;
    transform: translateX(0);
}

/* 箭头样式：定位到右下角 */
.hover-arrow {
    position: absolute;
    bottom: 1.25rem;
    /* 对齐底部 */
    right: 1.25rem;
    /* 对齐右侧 */
    opacity: 0;
    transform: translateX(10px);
    /* 初始向右偏移 */
    transition: all 0.3s ease;
    color: var(--primary);
}

/* 悬浮时的效果：提交次数隐去，箭头滑入 */
.problem-card:hover .submit-count {
    opacity: 0;
    transform: translateX(-10px);
}

.problem-card:hover .hover-arrow {
    opacity: 1;
    transform: translateX(0);
}

.state-container {
    text-align: center;
    padding: 4rem 1rem;
    background: white;
    border-radius: var(--radius);
    border: 2px dashed #e2e8f0;
}

.state-container.error {
    border-color: #fecaca;
    background: #fef2f2;
}

.state-icon {
    font-size: 3rem;
    margin-bottom: 1rem;
}

.state-container h3 {
    margin: 0 0 0.5rem 0;
    color: var(--text-main);
}

.state-container p {
    color: var(--text-muted);
    margin: 0 0 1.5rem 0;
}

.skeleton-card {
    height: 180px;
    background: #f1f5f9;
    border-radius: var(--radius);
    animation: pulse 1.5s cubic-bezier(0.4, 0, 0.6, 1) infinite;
}

@keyframes pulse {

    0%,
    100% {
        opacity: 1;
    }

    50% {
        opacity: 0.5;
    }
}

.list-enter-active,
.list-leave-active {
    transition: all 0.4s ease;
}

.list-enter-from,
.list-leave-to {
    opacity: 0;
    transform: translateY(20px);
}

@media (max-width: 640px) {
    .header-content {
        flex-direction: column;
        align-items: flex-start;
    }

    .actions {
        width: 100%;
        justify-content: space-between;
    }

    .search-bar-wrapper {
        flex-direction: column;
    }

    .stats-badge {
        justify-content: center;
        padding: 0.75rem;
    }

    .problem-grid {
        grid-template-columns: 1fr;
    }
}
</style>